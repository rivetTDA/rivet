/**********************************************************************
Copyright 2014-2016 The RIVET Developers. See the COPYRIGHT file at
the top-level directory of this distribution.

This file is part of RIVET.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/
//
// Created by Bryn Keller on 6/28/16.
// Modifications by Mike Lesnick April 18, 2018.
//

#ifndef RIVET_CONSOLE_ARRANGEMENT_BUILDER_H
#define RIVET_CONSOLE_ARRANGEMENT_BUILDER_H

#include "dcel/arrangement.h"
#include "interface/progress.h"
#include "math/firep.h"
#include <stack>

class ArrangementBuilder {
public:
    ArrangementBuilder(unsigned verbosity);

    //builds the DCEL arrangement, computes and stores persistence data
    //also stores ordered list of xi support points in the supplied vector
    //precondition: the constructor has already created the boundary of the arrangement
    std::shared_ptr<Arrangement> build_arrangement(FIRep& fir,
        std::vector<exact> x_exact,
        std::vector<exact> y_exact,
        std::vector<TemplatePoint>& template_points,
        Progress& progress);

    //builds the DCEL arrangement from the supplied xi support points, but does NOT compute persistence data
    std::shared_ptr<Arrangement> build_arrangement(
        std::vector<exact> x_exact,
        std::vector<exact> y_exact,
        std::vector<TemplatePoint>& template_points, std::vector<BarcodeTemplate>& barcode_templates, Progress& progress);

private:
    unsigned verbosity;
    void build_interior(Arrangement &arrangement);
    //builds the interior of DCEL arrangement using a version of the Bentley-Ottmann algorithm
    //precondition: all achors have been stored via find_anchors()
    void find_edge_weights(Arrangement& arrangement, PersistenceUpdater& updater);
    void find_path(Arrangement& arrangement, std::vector<Halfedge*>& pathvec);
    void find_subpath(Arrangement& arrangement, unsigned cur_node, std::vector<std::vector<unsigned>>& adj, std::vector<Halfedge*>& pathvec);

    typedef std::vector<std::pair<unsigned,long>> NodeAdjacencyList;
    
    /* technical function to convert an undirected tree into a directed representation
       a simplified version of a function formerly called sortAdjacencies
       though this name did not fully capture what the function did.)
       input:
          adjList: list of all adjacencies in the tree; adjList[a] = b iff nodes a and b are adjacent
          start: index of the node in the tree that will be regarded as root
       output: children[i] is a vector of indexes of the children of node i
    
       NOTE: unlike its predecessor sortAdjacencies, treeToDirectedRep does not
       sort children in decreasing rder of branch weight.  That sorting was
       introduced to speed up barcode template computation, but the
       implementation was problematic and causing a memory issue.  For now, I have
       implemented the quickest fix, which is to do away with the sorting altogether.
       I don't expect this to impact the performance of RIVET signicantly,
       especially since the cost of the barcode template computation is often
       negligible compared to that of computing a mimimal presentation.
       -Mike Lesnick, April 18 2018.
    */
    void tree_to_directed_tree(std::vector<NodeAdjacencyList>& adj_list, unsigned start, std::vector<std::vector<unsigned>>& children);

};

#endif //RIVET_CONSOLE_ARRANGEMENT_BUILDER_H
