/**
 * \brief	Vertex, Halfedge, and Face classes for building a DCEL arrangement
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __DCEL_H__
#define __DCEL_H__

class Vertex;
class Halfedge;
class Face;

#include "lcm.h"
#include "cell_persistence_data.h"


class Vertex
{
    public:
        Vertex(double x_coord, double y_coord);	//constructor, sets (x, y)-coordinates of the vertex

        void set_incident_edge(Halfedge* edge);		//set the incident edge
        Halfedge* get_incident_edge();			//get the incident edge

        double get_x();		//get the x-coordinate
        double get_y();		//get the y-coordinate

        friend std::ostream& operator<<(std::ostream& os, const Vertex& v);	//for printing the vertex

    private:
        Halfedge* incident_edge;	//pointer to one edge incident to this vertex
        double x;			//x-coordinate of this vertex
        double y;			//y-coordinate of this vertex
};//end class Vertex


class Halfedge
{
    public:
        Halfedge(Vertex* v, LCM* p);	//constructor, requires origin vertex as well as LCM corresponding to this halfedge (LCM never changes)
        Halfedge();			//constructor for a null Halfedge

        void set_twin(Halfedge* e);	//set the twin halfedge
        Halfedge* get_twin() const;		//get the twin halfedge

        void set_next(Halfedge* e);	//set the next halfedge in the boundary of the face that this halfedge borders
        Halfedge* get_next() const;		//get the next halfedge

        void set_prev(Halfedge* e);	//set the previous halfedge in the boundary of the face that this halfedge borders
        Halfedge* get_prev() const;		//get the previous halfedge

        void set_origin(Vertex* v); //set the origin vertex
        Vertex* get_origin() const;		//get the origin vertex

        void set_face(Face* f);		//set the face that this halfedge borders
        Face* get_face() const;		//get the face that this halfedge borders

        LCM* get_LCM() const;			//get the LCM coordinates

        friend std::ostream& operator<<(std::ostream& os, const Halfedge& e);	//for printing the halfedge

    private:
        Vertex* origin;		//pointer to the vertex from which this halfedge originates
        Halfedge* twin;		//pointer to the halfedge that, together with this halfedge, make one edge
        Halfedge* next;		//pointer to the next halfedge around the boundary of the face to the right of this halfedge
        Halfedge* prev;		//pointer to the previous halfedge around the boundary of the face to the right of this halfedge
        Face* face;		//pointer to the face to the right of this halfedge
        LCM* lcm;		//stores the (time, dist)-coordinates (in persistence space) of the LCM corresponding to this halfedge

};//end class Halfedge


class Face
{
    public:
        Face(Halfedge* e, int v);	//constructor, requires pointer to a boundary halfedge and verbosity value

        void set_boundary(Halfedge* e);	//set the pointer to a halfedge on the boundary of this face
        Halfedge* get_boundary();	//get the (pointer to the) boundary halfedge

        CellPersistenceData* get_data();		//returns the persistence data associated with this face

        //THE FOLLOWING FUNCTION WILL BE UNNECESSARY WHEN VINYARDS ARE IMPLEMENTED
        void store_interior_point(const std::vector<double>& x_grades, const std::vector<double>& y_grades);		//computes coordinates of a point inside this face and stores it in the persistence data object

        friend std::ostream& operator<<(std::ostream& os, const Face& f);	//for printing the face

    private:
        Halfedge* boundary;	//pointer to one halfedge in the boundary of this cell
        CellPersistenceData pdata;	//persistence data associated with this face -- TODO: should this be a pointer???

};//end class Face


#endif // __DCEL_H__
