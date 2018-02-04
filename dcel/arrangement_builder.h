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
//

#ifndef RIVET_CONSOLE_ARRANGEMENT_BUILDER_H
#define RIVET_CONSOLE_ARRANGEMENT_BUILDER_H

#include "dcel/arrangement.h"
#include "interface/progress.h"
#include "math/firep.h"

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
    void build_interior(std::shared_ptr<Arrangement> arrangement);
    //builds the interior of DCEL arrangement using a version of the Bentley-Ottmann algorithm
    //precondition: all achors have been stored via find_anchors()
    void find_edge_weights(Arrangement& arrangement, PersistenceUpdater& updater);
    void find_path(Arrangement& arrangement, std::vector<std::shared_ptr<Halfedge>>& pathvec);
    void find_subpath(Arrangement& arrangement, unsigned cur_node, std::vector<std::vector<unsigned>>& adj, std::vector<std::shared_ptr<Halfedge>>& pathvec);
};

#endif //RIVET_CONSOLE_ARRANGEMENT_BUILDER_H
