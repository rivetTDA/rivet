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
#include "type_tag.h"
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

    typedef ID<Vertex, long, -1> VertexId;
    typedef ID<Anchor, long, -1> AnchorId;
    typedef ID<Face, long, -1> FaceId;
    typedef ID<Halfedge, long, -1> HalfedgeId;

    std::vector<double> x_grades;   //floating-point values for x-grades
    std::vector<double> y_grades;   //floating-point values for y-grades

    HalfedgeId topleft;			//pointer to Halfedge that points down from top left corner (0,infty)
    HalfedgeId topright;         //pointer to Halfedge that points down from the top right corner (infty,infty)
    HalfedgeId bottomleft;       //pointer to Halfedge that points up from bottom left corner (0,-infty)
    HalfedgeId bottomright;      //pointer to Halfedge that points up from bottom right corner (infty,-infty)

    std::vector<HalfedgeId> vertical_line_query_list; //stores a pointer to the rightmost Halfedge of the "top" line of each unique slope, ordered from small slopes to big slopes (each Halfedge points to Anchor and Face for vertical-line queries)

    struct Anchor {

        unsigned x_coord;	//discrete x-coordinate
        unsigned y_coord;	//discrete y-coordinate

        //get_(x|y) needed to support AnchorComparator
        unsigned get_x() const { return x_coord; }
        unsigned get_y() const { return y_coord; }

        HalfedgeId dual_line;    //pointer to left-most halfedge corresponding to this Anchor in the arrangement
        unsigned position;      //relative position of Anchor line at sweep line, used for Bentley-Ottmann DCEL construction algorithm
        bool above_line;        //true iff this Anchor is above the current slice line, used for the vineyard-update process of storing persistence data in cells of the arrangement
        unsigned long weight;   //estimate of the cost of updating the RU-decomposition when crossing this anchor

        template<typename Archive>
        void serialize(Archive &ar, const unsigned int &version) {
            ar & x_coord & y_coord & dual_line & position & above_line & weight;
        }
    };

    struct AnchorStructComparator : AnchorComparator<MeshMessage::Anchor> {};

    struct Halfedge {

        VertexId origin;		//pointer to the vertex from which this halfedge originates
        HalfedgeId twin;		//pointer to the halfedge that, together with this halfedge, make one edge
        HalfedgeId next;		//pointer to the next halfedge around the boundary of the face to the right of this halfedge
        HalfedgeId prev;		//pointer to the previous halfedge around the boundary of the face to the right of this halfedge
        FaceId face;		    //pointer to the face to the right of this halfedge
        AnchorId anchor;		//stores the coordinates of the anchor corresponding to this halfedge

        template<typename Archive>
                void serialize(Archive &ar, const unsigned int & version) {
            ar & origin & twin & next & prev & face & anchor;
        }
    };

    struct Face {
        HalfedgeId boundary;     //pointer to one halfedge in the boundary of this cell
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
        HalfedgeId incident_edge;	//pointer to one edge incident to this vertex
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
            x_grades(mesh.x_grades),
            y_grades(mesh.y_grades),
            half_edges(mesh.halfedges.size()),
            vertices(mesh.vertices.size()),
            anchors(mesh.all_anchors.size()),
            faces(mesh.faces.size()),
            vertical_line_query_list(mesh.vertical_line_query_list.size())
    {
        //REALLY slow with all these VID, FID, etc. calls, but will do for now.
        for(auto face: mesh.faces) {
            faces.push_back(Face{HalfedgeId(mesh.HID(face->get_boundary())), face->get_barcode()});
        }
        for(auto half: mesh.halfedges) {
            half_edges.push_back(Halfedge {
                    VertexId(mesh.VID(half->get_origin())),
                    HalfedgeId(mesh.HID(half->get_twin())),
                    HalfedgeId(mesh.HID(half->get_next())),
                    HalfedgeId(mesh.HID(half->get_prev())),
                    FaceId(mesh.FID(half->get_face())),
                    AnchorId(mesh.AID(half->get_anchor()))
            });
        }
        for(auto anchor: mesh.all_anchors) {
            anchors.push_back(Anchor{
                anchor->get_x(),
                    anchor->get_y(),
                    HalfedgeId(mesh.HID(anchor->get_line())),
                    anchor->get_position(),
                    anchor->is_above(),
                    anchor->get_weight()
            });
        }
        for (auto vertex: mesh.vertices) {
            vertices.push_back(Vertex{
                    HalfedgeId(mesh.HID(vertex->get_incident_edge())),
                    vertex->get_x(),
                    vertex->get_y()
            });
        }

        for (auto query: mesh.vertical_line_query_list) {
            vertical_line_query_list.push_back(HalfedgeId(mesh.HID(query)));
        }

        topleft = HalfedgeId(mesh.HID(mesh.topleft));
        topright = HalfedgeId(mesh.HID(mesh.topright));
        bottomleft = HalfedgeId(mesh.HID(mesh.bottomleft));
        bottomright = HalfedgeId(mesh.HID(mesh.bottomright));
    }

    MeshMessage() : half_edges(), vertices(), anchors(), faces() {}

    template<class Archive>
    void serialize(Archive &ar, const unsigned int & version) {
        ar & x_grades & y_grades & half_edges & vertices & anchors & faces & topleft & topright & bottomleft & bottomright & vertical_line_query_list;
    }


//finds the first anchor that intersects the left edge of the arrangement at a point not less than the specified y-coordinate
//  if no such anchor, returns nullptr
    boost::optional<Anchor> find_least_upper_anchor(double y_coord)
    {
        //binary search to find greatest y-grade not greater than than y_coord
        unsigned best = 0;
        if(y_grades.size() >= 1 && y_grades[0] <= y_coord)
        {
            //binary search the vector y_grades
            unsigned min = 0;
            unsigned max = y_grades.size() - 1;

            while(max >= min)
            {
                unsigned mid = (max + min)/2;

                if(y_grades[mid] <= y_coord)    //found a lower bound, but search upper subarray for a better lower bound
                {
                    best = mid;
                    min = mid + 1;
                }
                else    //search lower subarray
                    max = mid - 1;
            }
        }
        else
            return boost::none;

        //if we get here, then y_grades[best] is the greatest y-grade not greater than y_coord
        //now find Anchor whose line intersects the left edge of the arrangement lowest, but not below y_grade[best]
        unsigned int zero = 0;  //disambiguate the following function call
        Anchor test{zero, best};
        auto it = std::lower_bound(anchors.begin(), anchors.end(), test, AnchorStructComparator());

        if(it == anchors.end())    //not found
        {
            return boost::none;
        }
        //else
        return *it;
    }//end find_least_upper_anchor()
//finds the (unbounded) cell associated to dual point of the vertical line with the given x-coordinate
//  i.e. finds the Halfedge whose Anchor x-coordinate is the largest such coordinate not larger than than x_coord; returns the Face corresponding to that Halfedge
    FaceId find_vertical_line(double x_coord)
    {
        //is there an Anchor with x-coordinate not greater than x_coord?
        if(vertical_line_query_list.size() >= 1
           && x_grades[ get(get(vertical_line_query_list[0]).anchor).x_coord ] <= x_coord)
        {
            //binary search the vertical line query list
            unsigned min = 0;
            unsigned max = vertical_line_query_list.size() - 1;
            unsigned best = 0;

            while(max >= min)
            {
                unsigned mid = (max + min)/2;
                auto test = get(get(vertical_line_query_list[mid]).anchor);

                if(x_grades[test.x_coord] <= x_coord)    //found a lower bound, but search upper subarray for a better lower bound
                {
                    best = mid;
                    min = mid + 1;
                }
                else    //search lower subarray
                    max = mid - 1;
            }

            return get(vertical_line_query_list[best]).face;
        }

        //if we get here, then either there are no Anchors or x_coord is less than the x-coordinates of all Anchors
        return get(get(bottomright).twin).face;

    }//end find_vertical_line()

////find a 2-cell containing the specified point
    FaceId find_point(double x_coord, double y_coord)
    {
        //start on the left edge of the arrangement, at the correct y-coordinate
        boost::optional<Anchor> start = find_least_upper_anchor(-1*y_coord);

        HalfedgeId finger; //for use in finding the cell

        if(!start)	//then starting point is in the top (unbounded) cell
        {
            finger = get(get(topleft).twin).next;   //this is the top edge of the top cell (at y=infty)
        }
        else
        {
            finger = start.get().dual_line;
        }

        FaceId cell; //will later point to the cell containing the specified point

        while(static_cast<long>(cell) < 0) //while not found
        {
            //find the edge of the current cell that crosses the horizontal line at y_coord
            VertexId next_pt = get(get(finger).next).origin;

            while(get(next_pt).y > y_coord)
            {
                finger = get(finger).next;
                next_pt = get(get(finger).next).origin;
            }

            Vertex vertex = get(next_pt);
            //now next_pt is at or below the horizontal line at y_coord
            //if (x_coord, y_coord) is to the left of crossing point, then we have found the cell; otherwise, move to the adjacent cell

            if(vertex.y == y_coord) //then next_pt is on the horizontal line
            {
                if(vertex.x >= x_coord)	//found the cell
                {
                    cell = get(finger).face;
                }
                else	//move to adjacent cell
                {
                    //find degree of vertex
                    HalfedgeId thumb = get(finger).next;
                    int deg = 1;
                    while(thumb != get(finger).twin)
                    {
                        thumb = get(get(thumb).twin).next;
                        deg++;
                    }

                    //move halfway around the vertex
                    finger = get(finger).next;
                    for(int i=0; i<deg/2; i++)
                    {
                        finger = get(get(finger).twin).next;
                    }
                }
            }
            else	//then next_pt is below the horizontal line
            {
                if(get(finger).anchor == AnchorId::invalid())	//then edge is vertical, so we have found the cell
                {
                    cell = get(finger).face;
                }
                else	//then edge is not vertical
                {
                    Anchor temp = get(get(finger).anchor);
                    double x_pos = ( y_coord + y_grades[temp.get_y()] )/x_grades[temp.get_x()];  //NOTE: division by zero never occurs because we are searching along a horizontal line, and thus we never cross horizontal lines in the arrangement

                    if(x_pos >= x_coord)	//found the cell
                    {
                        cell = get(finger).face;
                    }
                    else	//move to adjacent cell
                    {
                        finger = get(finger).twin;

                    }
                }
            }//end else
        }//end while(cell not found)
        return cell;
    }//end find_point()

//returns barcode template associated with the specified line (point)
//REQUIREMENT: 0 <= degrees <= 90

    Halfedge & get(HalfedgeId index) {
        return half_edges[static_cast<long>(index)];
    }
    Face & get(FaceId index) {
        return faces[static_cast<long>(index)];
    }
    Vertex & get(VertexId index) {
        return vertices[static_cast<long>(index)];
    }
    Anchor & get(AnchorId index) {
        return anchors[static_cast<long>(index)];
    }

    boost::optional<BarcodeTemplate> get_barcode_template(double degrees, double offset)
    {
        ///TODO: store some point/cell to seed the next query
        FaceId cell;
        if(degrees == 90) //then line is vertical
        {
            cell = find_vertical_line(-1*offset); //multiply by -1 to correct for orientation of offset
        } else if (degrees == 0) { //then line is horizontal
            auto anchor = find_least_upper_anchor(offset);

            if(anchor)
                cell = get(anchor.get().dual_line).face;
            else
                cell = get(get(topleft).twin).face;    //default
        } else {
            //else: the line is neither horizontal nor vertical
            double radians = degrees * 3.14159265 / 180;
            double slope = tan(radians);
            double intercept = offset / cos(radians);
            cell = find_point(slope, -1 * intercept);   //multiply by -1 for point-line duality
        }
        ///TODO: REPLACE THIS WITH A SEEDED SEARCH

        return get(cell).dbc;
    }//end get_barcode_template()
};



#endif //RIVET_CONSOLE_SERIALIZATION_H


