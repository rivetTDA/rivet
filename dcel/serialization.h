//
// Created by Bryn Keller on 7/6/16.
//

#ifndef RIVET_CONSOLE_SERIALIZATION_H
#define RIVET_CONSOLE_SERIALIZATION_H

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/optional.hpp>
//#include <boost/serialization/assume_abstract.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/export.hpp>
#include <boost/serialization/vector.hpp>

#include "numerics.h"
#include "anchor.h"
#include "dcel.h"
#include "math/xi_point.h"
#include "math/xi_support_matrix.h"
#include "barcode_template.h"

//namespace boost {
//    namespace multiprecision {
//        template<class Archive>
//        void load(Archive &ar, exact &num, const unsigned int version) {
//            std::string rep;
//            ar &rep;
//            num.assign(rep;
//        }
//
//        template<class Archive>
//        void save(Archive &ar, exact const &num , const unsigned int version) {
//            std::stringstream ss;
//            ss & num;
//            ar &ss.str();
//        }
//    }
//}

BOOST_CLASS_EXPORT(BarcodeTemplate);
BOOST_CLASS_EXPORT(BarTemplate);
BOOST_CLASS_EXPORT(xiMatrixEntry);
BOOST_CLASS_EXPORT(Anchor);
BOOST_CLASS_EXPORT(Face);
BOOST_CLASS_EXPORT(Halfedge);
BOOST_CLASS_EXPORT(Mesh);
BOOST_CLASS_EXPORT(Vertex);


template <class Archive>
void Vertex::serialize(Archive &ar, const unsigned int version) {
    ar &incident_edge & x & y;
}

template <class Archive>
void Halfedge::serialize(Archive &ar, const unsigned int version) {
    ar &origin & twin & next & prev & face & anchor;
}

template<class Archive>
void Face::serialize(Archive & ar, const unsigned int version) {
    ar &boundary & dbc & visited;
}

template <class Archive>
void Anchor::serialize(Archive &ar, const unsigned int version) {
    ar &x_coord & y_coord & entry & dual_line & position & above_line & weight;
}

template <class Archive>
void serialize(Archive &ar, xiMatrixEntry &x, const unsigned int version) {
    ar &x.x & x.y & x.index & x.down & x.left & x.low_simplices & x.high_simplices & x.low_count & x.high_count &
       x.low_index & x.high_index;
}

template <class Archive>
void serialize(Archive &ar, Multigrade &m, const unsigned int version) {
    ar &m.num_cols & m.simplex_index & m.x & m.y;
}

template <class Archive>
void Mesh::serialize(Archive &ar, const unsigned int version) {
ar & x_exact
    & y_exact
    & x_grades
    & y_grades;
    std::cout << "Processing faces" << std::endl;
      ar & topleft
      & topright
      & bottomleft
      & bottomright;

    std::cout << "Processing collections" << std::endl;
    ar
    & all_anchors
    & halfedges
    & faces
    & verbosity
    & vertical_line_query_list
    & vertices;
}

#include <boost/serialization/split_free.hpp>
BOOST_SERIALIZATION_SPLIT_FREE(unsigned_matrix)

namespace boost {


    template<class Archive>
    void save(Archive &ar, unsigned_matrix const &mat, const unsigned int &version) {
        assert(mat.num_dimensions() == 2);
        std::vector<unsigned> dims(mat.shape(), mat.shape() + mat.num_dimensions());
        std::vector<unsigned> data(mat.data(), mat.data() + mat.num_elements());
        ar &dims &  data;
    }

    template <class Archive>
    void load(Archive & ar, unsigned_matrix &mat, const unsigned int &version) {
        std::vector<unsigned> dims;
        std::vector<unsigned> data;
        ar &dims & data;
        unsigned_matrix::extent_gen extents;
        auto size = extents[dims[0]][dims[1]];
        mat.resize(size);
        std::memcpy(data.data(), mat.data(), data.size() * sizeof(unsigned));
    }
}

class MeshMessage {
//    BOOST_CLASS_EXPORT(BarcodeTemplate);
//    BOOST_CLASS_EXPORT(BarTemplate);
//    BOOST_CLASS_EXPORT(xiMatrixEntry);
//    BOOST_CLASS_EXPORT(Anchor);
//    BOOST_CLASS_EXPORT(Face);
//    BOOST_CLASS_EXPORT(Halfedge);
//    BOOST_CLASS_EXPORT(Mesh);
//    BOOST_CLASS_EXPORT(Vertex);
// Multigrade

    typedef long VertexIndex;
    typedef long AnchorIndex;
    typedef long FaceIndex;
    typedef long HalfedgeIndex;

    struct Anchor {

        unsigned x_coord;	//discrete x-coordinate
        unsigned y_coord;	//discrete y-coordinate

        HalfedgeIndex dual_line;    //pointer to left-most halfedge corresponding to this Anchor in the arrangement
        unsigned position;      //relative position of Anchor line at sweep line, used for Bentley-Ottmann DCEL construction algorithm
        bool above_line;        //true iff this Anchor is above the current slice line, used for the vineyard-update process of storing persistence data in cells of the arrangement
        unsigned long weight;   //estimate of the cost of updating the RU-decomposition when crossing this anchor

        template<typename Archive>
        void serialize(Archive &ar, const unsigned int &version) {
            ar & x_coord & y_coord & dual_line & position & above_line & weight;
        }
    };

    struct Halfedge {

        VertexIndex origin;		//pointer to the vertex from which this halfedge originates
        HalfedgeIndex twin;		//pointer to the halfedge that, together with this halfedge, make one edge
        HalfedgeIndex next;		//pointer to the next halfedge around the boundary of the face to the right of this halfedge
        HalfedgeIndex prev;		//pointer to the previous halfedge around the boundary of the face to the right of this halfedge
        FaceIndex face;		    //pointer to the face to the right of this halfedge
        AnchorIndex anchor;		//stores the coordinates of the anchor corresponding to this halfedge

        template<typename Archive>
                void serialize(Archive &ar, const unsigned int & version) {
            ar & origin & twin & next & prev & face & anchor;
        }
    };

    struct Face {
        HalfedgeIndex boundary;     //pointer to one halfedge in the boundary of this cell
        boost::optional<BarcodeTemplate> dbc;    //barcode template stored in this cell


        template<typename Archive>
        void save(Archive &ar, const unsigned int & version) const {
            if (dbc) {
                ar & boundary & true & dbc.get();
            } else {
                ar & boundary & false;
            }
        }

        template<typename Archive>
        void load(Archive &ar, const unsigned int &version) {
            bool has_dbc;
            ar & boundary & has_dbc;
            if (has_dbc) {
                BarcodeTemplate temp;
                ar & temp;
                dbc.reset(temp);
            } else {
                dbc.reset();
            }
        }
        BOOST_SERIALIZATION_SPLIT_MEMBER()
    };

    struct Vertex {
        HalfedgeIndex incident_edge;	//pointer to one edge incident to this vertex
        double x;			//x-coordinate of this vertex
        double y;			//y-coordinate of this vertex

        template<typename Archive>
        void serialize(Archive &ar, const unsigned int & version) {
            ar & incident_edge & x & y;
        }
    };

    std::vector<Halfedge> half_edges;
    std::vector<Vertex> vertices;
    std::vector<Anchor> anchors;
    std::vector<Face> faces;

public:

    MeshMessage(Mesh const &mesh):
            half_edges(mesh.halfedges.size()),
            vertices(mesh.vertices.size()),
            anchors(mesh.all_anchors.size()),
            faces(mesh.faces.size())
    {
        //REALLY slow with all these VID, FID, etc. calls, but will do for now.
        for(auto face: mesh.faces) {
            faces.push_back(Face{mesh.HID(face->get_boundary()), face->get_barcode()});
        }
        for(auto half: mesh.halfedges) {
            half_edges.push_back(Halfedge {
                    mesh.VID(half->get_origin()),
                    mesh.HID(half->get_twin()),
                    mesh.HID(half->get_next()),
                    mesh.HID(half->get_prev()),
                    mesh.FID(half->get_face()),
                    mesh.AID(half->get_anchor())
            });
        }
        for(auto anchor: mesh.all_anchors) {
            anchors.push_back(Anchor{
                anchor->get_x(),
                    anchor->get_y(),
                    mesh.HID(anchor->get_line()),
                    anchor->get_position(),
                    anchor->is_above(),
                    anchor->get_weight()
            });
        }
        for (auto vertex: mesh.vertices) {
            vertices.push_back(Vertex{
                    mesh.HID(vertex->get_incident_edge()),
                    vertex->get_x(),
                    vertex->get_y()
            });
        }
    }

    MeshMessage() : half_edges(), vertices(), anchors(), faces() {}

    template<class Archive>
    void serialize(Archive &ar, const unsigned int & version) {
        ar & half_edges & vertices & anchors & faces;
    }

};



#endif //RIVET_CONSOLE_SERIALIZATION_H


