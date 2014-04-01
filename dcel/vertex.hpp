/**
 * \class	Vertex
 * \brief	Stores one vertex in the DCEL decomposition of the affine Grassmannian
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __DCEL_Vertex_H__
#define __DCEL_Vertex_H__

//includes????  namespace????
#include <iostream>

class Halfedge;

class Vertex
{
	public:
		Vertex(double theta_coord, double r_coord);	//constructor, sets (theta, r)-coordinates of the vertex
	
		void set_incident_edge(Halfedge* edge);		//set the incident edge
		Halfedge* get_incident_edge();			//get the incident edge
		
		double get_theta();		//get the theta-coordinate
		double get_r();			//get the r-coordinate
		
		friend std::ostream& operator<<(std::ostream& os, const Vertex& v);	//for printing the vertex
	
	private:
		Halfedge* incident_edge;	//pointer to one edge incident to this vertex
		double theta;			//theta-coordinate of this vertex in the affine Grassmannian
		double r;			//r-coordinate of this vertex in the affine Grassmannian
	
};//end class Vertex


////////// implementation //////////

Vertex::Vertex(double theta_coord, double r_coord) : 
	incident_edge(NULL),
	theta(theta_coord),
	r(r_coord)
{ }

void Vertex::set_incident_edge(Halfedge* edge)
{
	incident_edge = edge;
}

Halfedge* Vertex::get_incident_edge()
{
	return incident_edge;
}

double Vertex::get_theta()
{
	return theta;
}

double Vertex::get_r()
{
	return r;
}

std::ostream& operator<<(std::ostream& os, const Vertex& v)
{
	os << "(" << v.theta << ", " << v.r << ")";
	return os;
}

#endif // __DCEL_Vertex_H__

