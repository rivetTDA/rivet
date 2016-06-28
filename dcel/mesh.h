/**
 * \class	Mesh
 * \brief	Stores and manipulates the DCEL decomposition of the affine Grassmannian.
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __DCEL_Mesh_H__
#define __DCEL_Mesh_H__

//forward declarations
class BarcodeTemplate;
class ComputationThread;
class Face;
class Halfedge;
class MultiBetti;
class PersistenceUpdater;
class Vertex;

#include "anchor.h"
#include "../math/xi_point.h"
#include "numerics.h"
#include <vector>
#include <set>
#include "interface/progress.h"


//std::ostream& write_grades(std::ostream &stream, const std::vector<exact> &x_exact, const std::vector<exact> &y_exact);

template<typename T>
T& write_grades(T &stream, const std::vector<exact> &x_exact, const std::vector<exact> &y_exact) {

    //write x-grades
    stream << "x-grades" << std::endl;
    for(std::vector<exact>::const_iterator it = x_exact.begin(); it != x_exact.end(); ++it)
    {
        std::ostringstream oss;
        oss << *it;
        stream << oss.str() << std::endl;
    }
    stream << std::endl;

    //write y-grades
    stream << "y-grades" << std::endl;
    for(std::vector<exact>::const_iterator it = y_exact.begin(); it != y_exact.end(); ++it)
    {
        std::ostringstream oss;
        oss << *it;
        stream << oss.str() << std::endl;
    }
    stream << std::endl;
    return stream;
}

class Mesh
{
//TODO: refactor so Mesh doesn't need friends.
    friend class PersistenceUpdater;
    friend class MeshBuilder;
    public:
    Mesh(std::vector<exact> xe, std::vector<exact> ye, unsigned verbosity);
            //constructor; sets up bounding box (with empty interior) for the affine Grassmannian
            //  requires references to vectors of all multi-grade values (both double and exact values)
		
        ~Mesh();	//destructor: deletes all cells and anchors --- CHECK THIS!!!
		

        BarcodeTemplate& get_barcode_template(double degrees, double offset);
            //returns barcode template associated with the specified line (point)

        BarcodeTemplate& get_barcode_template(unsigned i);
            //returns the barcode template associated with faces[i]

        unsigned num_faces();   //returns the number of 2-cells, and thus the number of barcode templates, in the arrangement
		
        //JULY 2015 BUG FIX:
        void add_anchor(Anchor anchor);  //creates a new anchor in the vector all_anchors

        //TESTING ONLY
        void print_stats(); //prints a summary of the arrangement information, such as the number of anchors, vertices, halfedges, and faces
		void print();	//prints all the data from the mesh
        void test_consistency();    //attempts to find inconsistencies in the DCEL arrangement
		

        //references to vectors of multi-grade values
        const std::vector<exact> x_exact;     //exact values for all x-grades
        const std::vector<exact> y_exact;     //exact values for all y-grades

        //these are necessary for comparisons, but should they really be static members of Mesh???
        static double epsilon;
        static bool almost_equal(const double a, const double b);

    friend std::ostream& operator<<(std::ostream&, const Mesh&);
    friend std::istream& operator>>(std::istream&, Mesh&);

    private:
      //data structures
      std::vector<double> x_grades;   //floating-point values for x-grades
    std::vector<double> y_grades;   //floating-point values for y-grades
        std::vector<Vertex*> vertices;		//all vertices in the mesh
		std::vector<Halfedge*> halfedges;	//all halfedges in the mesh
		std::vector<Face*> faces;		//all faces in the mesh
		
		const double INFTY;

    unsigned verbosity;

        std::set<Anchor*, Anchor_LeftComparator> all_anchors;	//set of Anchors that are represented in the mesh, ordered by position of curve along left side of the arrangement, from bottom to top
		
        Halfedge* topleft;			//pointer to Halfedge that points down from top left corner (0,infty)
        Halfedge* topright;         //pointer to Halfedge that points down from the top right corner (infty,infty)
        Halfedge* bottomleft;       //pointer to Halfedge that points up from bottom left corner (0,-infty)
        Halfedge* bottomright;      //pointer to Halfedge that points up from bottom right corner (infty,-infty)
		
        std::vector<Halfedge*> vertical_line_query_list; //stores a pointer to the rightmost Halfedge of the "top" line of each unique slope, ordered from small slopes to big slopes (each Halfedge points to Anchor and Face for vertical-line queries)

      //functions for creating the arrangement

        Halfedge* insert_vertex(Halfedge* edge, double x, double y);	//inserts a new vertex on the specified edge, with the specified coordinates, and updates all relevant pointers
        Halfedge* create_edge_left(Halfedge* edge, Anchor *anchor);    //creates the first pair of Halfedges in an anchor line, anchored on the left edge of the strip

        void find_edge_weights(PersistenceUpdater& updater);    //computes and stores the edge weight for each anchor line

        void find_path(std::vector<Halfedge *> &pathvec);   //finds a pseudo-optimal path through all 2-cells of the arrangement
        void find_subpath(unsigned cur_node, std::vector< std::set<unsigned> >& adj, std::vector<Halfedge*>& pathvec, bool return_path); //builds the path recursively

        void set_barcode_template(unsigned i, BarcodeTemplate& bt);    //stores (a copy of) the given barcode template in faces[i]; used for re-building the arrangement from a RIVET data file

      //functions for searching the arrangement
        Anchor* find_least_upper_anchor(double y_coord); //finds the first anchor that intersects the left edge of the arrangement at a point not less than the specified y-coordinate; if no such anchor, returns NULL

        Face* find_vertical_line(double x_coord); //finds the (unbounded) cell associated to dual point of the vertical line with the given x-coordinate
            //i.e. finds the Halfedge whose anchor x-coordinate is the largest such coordinate not larger than than x_coord; returns the Face corresponding to that Halfedge

        Face* find_point(double x_coord, double y_coord);    //finds a 2-cell containing the specified point

      //functions for testing
        unsigned HID(Halfedge* h);		//halfedge ID, for printing and debugging
        unsigned FID(Face* f);		//face ID, for printing and debugging
        unsigned VID(Vertex* v);    //vertex ID, for printing and debugging


      //struct to hold a future intersection event
        struct Crossing {
            Anchor* a;     //pointer to one line
            Anchor* b;     //pointer to the other line -- must ensure that line for anchor a is below line for anchor b just before the crossing point!!!!!
            double x;   //x-coordinate of intersection point (floating-point)
            Mesh* m;    //pointer to the mesh, so the Crossing has access to the vectors x_grades, x_exact, y_grades, and y_exact

            Crossing(Anchor* a, Anchor* b, Mesh* m);  //precondition: Anchors a and b must be comparable
            bool x_equal(const Crossing* other) const;  //returns true iff this Crossing has (exactly) the same x-coordinate as other Crossing
        };

      //comparator class for ordering crossings: first by x (left to right); for a given x, then by y (low to high)
        struct CrossingComparator {
            bool operator()(const Crossing* c1, const Crossing* c2) const;	//returns true if c1 comes after c2
        };

};//end class Mesh

#endif // __DCEL_Mesh_H__

