//
// Created by Bryn Keller on 6/28/16.
//

#ifndef RIVET_CONSOLE_MESH_BUILDER_H
#define RIVET_CONSOLE_MESH_BUILDER_H

#include "math/multi_betti.h"
#include "dcel/mesh.h"
#include "interface/progress.h"

class MeshBuilder {
public:

    MeshBuilder(unsigned verbosity);

//builds the DCEL arrangement, computes and stores persistence data
//also stores ordered list of xi support points in the supplied vector
//precondition: the constructor has already created the boundary of the arrangement
    std::unique_ptr<Mesh> build_arrangement(MultiBetti& mb,
                                                         std::vector<exact> x_exact,
                                                         std::vector<exact> y_exact,
                                                         std::vector<xiPoint>& xi_pts,
                                                         Progress &progress);


//builds the DCEL arrangement from the supplied xi support points, but does NOT compute persistence data
    std::unique_ptr<Mesh> build_arrangement(
            std::vector<exact> x_exact,
            std::vector<exact> y_exact,
            std::vector<xiPoint>& xi_pts, std::vector<BarcodeTemplate>& barcode_templates, Progress &progress);

private:
    unsigned verbosity;
    void build_interior(Mesh &mesh);
    //builds the interior of DCEL arrangement using a version of the Bentley-Ottmann algorithm
    //precondition: all achors have been stored via find_anchors()
    void find_edge_weights(Mesh &mesh, PersistenceUpdater& updater);
    void find_path(Mesh &mesh, std::vector<Halfedge*>& pathvec);
    void find_subpath(Mesh &mesh, unsigned cur_node, std::vector< std::set<unsigned> >& adj, std::vector<Halfedge*>& pathvec, bool return_path);
};


#endif //RIVET_CONSOLE_MESH_BUILDER_H
