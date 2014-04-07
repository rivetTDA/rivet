/**
 * \class	Mesh
 * \brief	Stores and manipulates the DCEL decomposition of the affine Grassmannian.
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __DCEL_Mesh_H__
#define __DCEL_Mesh_H__

//includes????  namespace????
#include <vector>
#include <iostream>
#include <limits>	//necessary for infinity
#include <set>
#include "lcm.hpp"
#include "lcm_left_comparator.hpp"
#include "lcm_angle_comparator.hpp"
#include "vertex.hpp"
#include "halfedge.hpp"
#include "face.hpp"

class Mesh
{
	public:
		Mesh();		//constructor; sets up bounding box (with empty interior) for the affine Grassmannian
		
		~Mesh();	//destructor: IMPLEMENT THIS, MAKE SURE ALL MEMORY IS RELEASED!!!!
		
		void add_curve(double time, double dist);	//adds a curve representing LCM (time, dist) to the mesh
								//assumption: the same LCM may not be inserted into the mesh twice!
		
		//need ability to iterate over faces and associate a persistence diagram to each face...
		
		//need ability to return persistence diagram associated with the face containing a specified point...
		
		
		void print();	//prints all the data from the mesh
		
		
	private:
		std::vector<Vertex*> vertices;		//all vertices in the mesh
		std::vector<Halfedge*> halfedges;	//all halfedges in the mesh
		std::vector<Face*> faces;		//all faces in the mesh
		
		const double INFTY;
		static const double HALF_PI = 1.570796327;
		
		std::set<LCM, LCM_LeftComparator> inserted_lcms;	//ordered container of LCMs that are represented in the mesh
		
		Halfedge* topleft;			//pointer to halfedge that points down from top left corner (theta=0, r=infty)
		
		static const bool verbose = true;	//controls display of output, for debugging
		
		Halfedge* Mesh::insert_vertex(Halfedge* edge, double t, double r)	//inserts a new vertex on the specified edge, with the specified coordinates, and updates all relevant pointers
		
};//end class Mesh

#include "mesh.hpp"

#endif // __DCEL_Mesh_H__

