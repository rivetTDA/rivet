/**
 * \brief	Vertex, Halfedge, and Face classes for building a DCEL arrangement
 *
 * A DCEL is a doubly connected edge list, see e.g. de Berg et. al.'s Computational
 * Geometry: Algorithms and Applications for background.
 * 
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __DCEL_H__
#define __DCEL_H__

class Halfedge;

#include <memory>
#include <vector>
#include <math/xi_point.h>
#include <numerics.h>
#include "barcode_template.h"

#include "debug.h"

class Vertex
{
    public:
        Vertex(double x_coord, double y_coord);	//constructor, sets (x, y)-coordinates of the vertex
        Vertex(); //For serialization

        void set_incident_edge(std::shared_ptr<Halfedge> edge);		//set the incident edge
        std::shared_ptr<Halfedge> get_incident_edge();			//get the incident edge

        double get_x();		//get the x-coordinate
        double get_y();		//get the y-coordinate

        friend Debug& operator<<(Debug &qd, const Vertex& v);	//for printing the vertex

        bool operator==(Vertex const &other);

    template <class Archive> void serialize(Archive &ar, const unsigned int version);

    private:
        std::shared_ptr<Halfedge> incident_edge;	//pointer to one edge incident to this vertex
        double x;			//x-coordinate of this vertex
        double y;			//y-coordinate of this vertex

};//end class Vertex


class Face;
class Anchor;

class Halfedge
{
    public:
        Halfedge(std::shared_ptr<Vertex> v, std::shared_ptr<Anchor> p); //constructor, requires origin vertex as well as Anchor corresponding to this halfedge (Anchor never changes)
        Halfedge();                     //constructor for a null Halfedge

        void set_twin(std::shared_ptr<Halfedge> e); //set the twin halfedge
        std::shared_ptr<Halfedge> get_twin() const; //get the twin halfedge

        void set_next(std::shared_ptr<Halfedge> e); //set the next halfedge in the boundary of the face that this halfedge borders
        std::shared_ptr<Halfedge> get_next() const; //get the next halfedge

        void set_prev(std::shared_ptr<Halfedge> e); //set the previous halfedge in the boundary of the face that this halfedge borders
        std::shared_ptr<Halfedge> get_prev() const; //get the previous halfedge

        void set_origin(std::shared_ptr<Vertex> v); //set the origin vertex
        std::shared_ptr<Vertex> get_origin() const; //get the origin vertex

        void set_face(std::shared_ptr<Face> f);     //set the face that this halfedge borders
        std::shared_ptr<Face> get_face() const;     //get the face that this halfedge borders

        std::shared_ptr<Anchor> get_anchor() const; //get the Anchor

        friend Debug& operator<<(Debug& qd, const Halfedge& e);	//for printing the halfedge

    template <class Archive>
    void serialize(Archive &ar, const unsigned int version);
    private:
        std::shared_ptr<Vertex> origin;		//pointer to the vertex from which this halfedge originates
        std::shared_ptr<Halfedge> twin;		//pointer to the halfedge that, together with this halfedge, make one edge
        std::shared_ptr<Halfedge> next;		//pointer to the next halfedge around the boundary of the face to the right of this halfedge
        std::shared_ptr<Halfedge> prev;		//pointer to the previous halfedge around the boundary of the face to the right of this halfedge
        std::shared_ptr<Face> face;		    //pointer to the face to the right of this halfedge
        std::shared_ptr<Anchor> anchor;		//stores the coordinates of the anchor corresponding to this halfedge

};//end class Halfedge


class Face
{
    public:
        Face(std::shared_ptr<Halfedge> e);      //constructor: requires pointer to a boundary halfedge
        Face(); // For serialization
        ~Face();                //destructor: destroys barcode template

        void set_boundary(std::shared_ptr<Halfedge> e);     //set the pointer to a halfedge on the boundary of this face
        std::shared_ptr<Halfedge> get_boundary();           //get the (pointer to the) boundary halfedge

        BarcodeTemplate& get_barcode();         //returns a reference to the barcode template stored in this cell
        void set_barcode(BarcodeTemplate& bt);  //stores (a copy of) the specified barcode template in this cell

        bool has_been_visited();            //true iff cell has been visited in the vineyard-update process (so that we can distinguish a cell with an empty barcode from an unvisited cell)
        void mark_as_visited();             //marks this cell as visited

        friend Debug& operator<<(Debug& os, const Face& f);	//for printing the face

        template<class Archive>
                void serialize(Archive & ar, const unsigned int version);
    private:
        std::shared_ptr<Halfedge> boundary;     //pointer to one halfedge in the boundary of this cell
        BarcodeTemplate dbc;    //barcode template stored in this cell
        bool visited;           //initially false, set to true after this cell has been visited in the vineyard-update process (so that we can distinguish a cell with an empty barcode from an unvisited cell)

};//end class Face

//This class exists only for data transfer between console and viewer
struct XiSupportMessage {
    std::vector<xiPoint> xi_support;
    unsigned_matrix homology_dimensions;
    std::vector<exact> x_exact;
    std::vector<exact> y_exact;

    template<class Archive>
            void serialize(Archive &ar, const unsigned int version) {
        ar & xi_support & homology_dimensions & x_exact & y_exact;
    }
};

#endif // __DCEL_H__
