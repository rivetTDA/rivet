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
#include <queue>
#include <iostream>
#include <limits>	//necessary for infinity
#include <math.h>	//necessary for atan
#include <set>

#include "lcm.h"
#include "dcel.h"
#include "cell_persistence_data.h"
#include "../math/persistence_data.h"

#include <boost/multiprecision/cpp_int.hpp>
typedef boost::multiprecision::cpp_rational exact;

#include <boost/math/constants/constants.hpp>

class Mesh
{
	public:
        Mesh(int v, const std::vector<double>& xg, const std::vector<exact>& xe, const std::vector<double>& yg, const std::vector<exact>& ye);
            //constructor; sets up bounding box (with empty interior) for the affine Grassmannian
            //  requires references to vectors of all multi-grade values (both double and exact values)
		
		~Mesh();	//destructor: IMPLEMENT THIS, MAKE SURE ALL MEMORY IS RELEASED!!!!
		
        void add_lcm(double x, double y);   //adds an LCM; curve will be created when build_arrangement() is called

        void build_arrangement();    //function to build the arrangement using a version of the Bentley-Ottmann algorithm, given all LCMs
		
		void build_persistence_data(std::vector<std::pair<int, int> > & xi, SimplexTree* bifiltration, int dim);
			//associates persistence data to each face, requires all support points of xi_0 and xi_1, the bifiltration, and the dimension of homology
		
        PersistenceData* get_persistence_data(double angle, double offset, std::vector<std::pair<int, int> > & xi, SimplexTree* bifiltration);
            //returns persistence diagram data associated with the specified point (line); angle should be in RADIANS
		
        void print_stats(); //prints a summary of the arrangement information, such as the number of LCMS, vertices, halfedges, and faces
		void print();	//prints all the data from the mesh
        void test_consistency();    //attempts to find inconsistencies in the DCEL arrangement
		
	private:
        const std::vector<double>& x_grades;   //floating-point values for x-grades
        const std::vector<exact>& x_exact;     //exact values for all x-grades
        const std::vector<double>& y_grades;   //floating-point values for y-grades
        const std::vector<exact>& y_exact;     //exact values for all y-grades

        std::vector<Vertex*> vertices;		//all vertices in the mesh
		std::vector<Halfedge*> halfedges;	//all halfedges in the mesh
		std::vector<Face*> faces;		//all faces in the mesh
		
		const double INFTY;
        const double HALF_PI;
        const double EPSILON;
        bool equal(double x, double y);   //tests whether x and y are with epsilon -- TO BE IMPROVED LATER BY EXACT COMPARISON
		
        std::set<LCM*, LCM_LeftComparator> all_lcms;	//set of LCMs that are represented in the mesh, ordered by position of curve along left side of strip
		
        Halfedge* topleft;			//pointer to Halfedge that points down from top left corner (theta=0, r=infty)
        Halfedge* bottomleft;       //pointer to Halfedge that points up from bottom left corner (theta=0, r=-infty)
        Halfedge* bottomright;      //pointer to Halfedge that points up from bottom right corner (theta=pi/2, r=-infty)
		
		const int verbosity;			//controls display of output, for debugging
		


        Halfedge* insert_vertex(Halfedge* edge, double t, double r);	//inserts a new vertex on the specified edge, with the specified coordinates, and updates all relevant pointers
        Halfedge* create_edge_left(Halfedge*edge, LCM*lcm);    //creates the first pair of Halfedges in an LCM curve, anchored on the left edge of the strip

        double find_intersection(LCM* a, LCM* b);   //returns the theta-coordinate of intersection of two LCM curves, or -1 if no intersection exists
        double find_r(LCM* a, double t);      //returns the r-coordinate corresponding to theta = t on the curve for LCM a
		

		std::pair<bool, double> project(double angle, double offset, double x, double y);	//projects (x,y) onto the line determined by angle and offset
		
        unsigned HID(Halfedge* h);		//halfedge ID, for printing and debugging
        unsigned FID(Face* f);		//face ID, for printing and debugging
};//end class Mesh

#include "mesh.cpp"

#endif // __DCEL_Mesh_H__

