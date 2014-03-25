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
#include <limits>
#include "lcm.h"
#include "vertex.h"
#include "halfedge.h"
#include "face.h"

class Mesh
{
	public:
		Mesh();		//constructor; sets up bounding box (with empty interior) for the affine Grassmannian
		
		~Mesh();	//destructor: IMPLEMENT THIS, MAKE SURE ALL MEMORY IS RELEASED!!!!
		
		void add_curve(double time, double dist);	//adds a curve representing LCM (time, dist) to the mesh
		
		//need ability to iterate over faces and associate a persistence diagram to each face...
		
		//need ability to return persistence diagram associated with the face containing a specified point...
		
		
		void print();	//prints all the data from the mesh
		
		
	private:
		std::vector<Vertex*> vertices;		//all vertices in the mesh
		std::vector<Halfedge*> halfedges;	//all halfedges in the mesh
		std::vector<Face*> faces;		//all faces in the mesh
		
		const double INFTY;
		static const double HALF_PI = 1.570796327;
		
		//list (maybe a set) of LCMs that are represented in the mesh
		
	
};//end class Mesh

#include "mesh.hpp"

#endif // __DCEL_Mesh_H__

