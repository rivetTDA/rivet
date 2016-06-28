//
// Created by Bryn Keller on 6/28/16.
//

#ifndef RIVET_CONSOLE_MESH_BUILDER_H
#define RIVET_CONSOLE_MESH_BUILDER_H


class MeshBuilder {
public:
    std::unique_ptr<Mesh> build_arrangement(MultiBetti& mb, std::vector<xiPoint>& xi_pts, Progress &progress);
    //builds the DCEL arrangement, and computes and stores persistence data
    //also stores ordered list of xi support points in the supplied vector

    std::unique_ptr<Mesh> build_arrangement(std::vector<xiPoint>& xi_pts, std::vector<BarcodeTemplate>& barcode_templates, Progress &progress);
    //builds the DCEL arrangement from the supplied xi support points, but does NOT compute persistence data
private:
    void build_interior();
    //builds the interior of DCEL arrangement using a version of the Bentley-Ottmann algorithm
    //precondition: all achors have been stored via find_anchors()
};


#endif //RIVET_CONSOLE_MESH_BUILDER_H
