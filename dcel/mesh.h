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
#include <math.h>	//necessary for atan
#include <set>
#include "lcm.hpp"
#include "lcm_left_comparator.hpp"
#include "lcm_angle_comparator.hpp"
#include "vertex.hpp"
#include "halfedge.hpp"
#include "face.hpp"
#include "../persistence_diagram.hpp"

class Mesh
{
	public:
		Mesh(int v);		//constructor; sets up bounding box (with empty interior) for the affine Grassmannian
		
		~Mesh();	//destructor: IMPLEMENT THIS, MAKE SURE ALL MEMORY IS RELEASED!!!!
		
		void add_curve(double time, double dist);	//adds a curve representing LCM (time, dist) to the mesh
								//assumption: the same LCM may not be inserted into the mesh twice!
		
		bool contains(double time, double dist);	//determines whether LCM (time, dist) is already represented in the mesh
		
		void build_persistence_data(std::vector<std::pair<int, int> > & xi, SimplexTree* bifiltration, int dim);
			//associates persistence data to each face, requires all support points of xi_0 and xi_1, the bifiltration, and the dimension of homology
		
		PersistenceDiagram* get_persistence_diagram(double angle, double offset, std::vector<std::pair<int, int> > & xi, SimplexTree* bifiltration);
			//returns a persistence diagram associated with the specified point
		
		void print();	//prints all the data from the mesh
		
		
	private:
		std::vector<Vertex*> vertices;		//all vertices in the mesh
		std::vector<Halfedge*> halfedges;	//all halfedges in the mesh
		std::vector<Face*> faces;		//all faces in the mesh
		
		const double INFTY;
		static const double HALF_PI = 1.570796327;
		
		std::set<LCM*, LCM_LeftComparator> inserted_lcms;	//ordered container of LCMs that are represented in the mesh
		
		Halfedge* topleft;			//pointer to halfedge that points down from top left corner (theta=0, r=infty)
		
		const int verbosity;			//controls display of output, for debugging
		
		Halfedge* insert_vertex(Halfedge* edge, double t, double r);	//inserts a new vertex on the specified edge, with the specified coordinates, and updates all relevant pointers
		void insert_edge(Halfedge* leftedge, Halfedge* rightedge, LCM* lcm);	//inserts a new edge across an existing face; requires leftedge and rightedge, coherently oriented around the existing face, and whose origin vertices will be endpoints of the new edge; also requires the LCM to be associated with the new edge
		
		std::pair<bool, double> project(double angle, double offset, double x, double y);	//projects (x,y) onto the line determined by angle and offset
		
		int HID(Halfedge* h);		//halfedge ID, for printing and debugging
		int FID(Face* f);		//face ID, for printing and debugging
};//end class Mesh

#include "mesh.hpp"

#endif // __DCEL_Mesh_H__

