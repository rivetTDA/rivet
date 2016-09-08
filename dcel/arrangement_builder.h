//
// Created by Bryn Keller on 6/28/16.
//

#ifndef RIVET_CONSOLE_MESH_BUILDER_H
#define RIVET_CONSOLE_MESH_BUILDER_H

#include "dcel/arrangement.h"
#include "interface/progress.h"
#include "math/multi_betti.h"

class ArrangementBuilder {
public:
    ArrangementBuilder(unsigned verbosity);

    //builds the DCEL arrangement, computes and stores persistence data
    //also stores ordered list of xi support points in the supplied vector
    //precondition: the constructor has already created the boundary of the arrangement
    std::shared_ptr<Arrangement> build_arrangement(MultiBetti& mb,
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

#endif //RIVET_CONSOLE_MESH_BUILDER_H
