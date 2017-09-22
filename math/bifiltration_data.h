/**
 * \class	BifiltrationData
 * \brief	Computes and stores the information about a bifiltration needed to compute homology in fixed dimension d.  Together with Input_Manager, handles 1-critical or multicritical Rips bifiltrations, as defined in the RIVET paper.
 * \author  Roy Zhao; edited by Michael Lesnick.
 * \date    March 2017; edited September 2017.
 */

#ifndef BIFILTRATION_DATA_H
#define BIFILTRATION_DATA_H

#include <set>
#include <vector>
#include <unordered_map>
#include <boost/functional/hash.hpp>

//Pair of coordinates specifying grade of appearance with additional sorting operator. Sorted first by y then x grade in REVERSE-LEXICOGRAPHIC ORDER.
struct Grade
{
    int x;
    int y;
    
    bool operator==(const Grade &other) const
    {
        return x==other.x && y==other.y;
    }
    
    bool operator<(const Grade &other) const
    {
        if (y != other.y)
            return y < other.y;
        else
            return x < other.x;
    }
     
    Grade() {}

    Grade(int set_x, int set_y) : x(set_x), y(set_y)
    { }
};


//typedef
typedef std::vector<int> Simplex;
typedef std::vector<Grade> AppearanceGrades;


// We only need a single grade for each simplex in the low dimension because for homology, it suffices to consider a greatest lower bound of all grades.  For mid and high dimensions, we need a vector of grades for each simplex, for the multicritical case.

//In the future, we might specify a vertex using a combinatorial number system, as in DIPHA or Ripser, but this will do for now
struct LowSimplexData
{
    Simplex s;
    Grade gr;
    
    LowSimplexData(Simplex simp, Grade g) : s(simp), gr(g)
    {}
};


struct MidHighSimplexData
{
    Simplex s;
    AppearanceGrades ag;
    
    //column index of the generator corresponding to each bigrade.  Used to construct high boundary matrix.
    std::vector<unsigned> ind;
    
    //TODO: Maybe slightly cleaner to use an iterator pointing to ind than an iterator pointing to ag?
    //std::vector<unsigned>::iterator ind_it;
    AppearanceGrades::iterator ag_it;
    
    bool high;
    
    virtual bool is_high() const
        {return high;}
    
    //TODO: This is a little inefficient, since high_simplex data doesn't actually need the data of ind or ag_it.  But it's not too bad
    MidHighSimplexData(Simplex simp, AppearanceGrades app_gr, bool h) : s(simp), ag(app_gr), ind(std::vector<unsigned>()), ag_it(ag.begin()), high(h)
    {}

};




class BifiltrationData {
    friend class FIRep;

    public:
        BifiltrationData(int dim, int v);	//constructor; requires verbosity parameter

        ~BifiltrationData(); //destructor

        void build_VR_complex(const std::vector<unsigned>& times, const std::vector<unsigned>& distances, const unsigned num_x, const unsigned num_y);
                    //builds BifiltrationData representing a bifiltered Vietoris-Rips complex from discrete data
                    //requires a list of birth times (one for each point), a list of distances between pairs of points, max dimension of simplices to construct, and number of grade values in x- and y-directions
                    //NOTE: automatically computes global indexes and dimension indexes
                    //CONVENTION: the x-coordinate is "birth time" for points and the y-coordinate is "distance" between points

        void build_BR_complex(const unsigned num_vertices, const std::vector<unsigned>& distances, const std::vector<unsigned>& degrees, const unsigned num_x, const unsigned num_y);
                    //builds BifiltrationData representing a bifiltered Rips complex from discrete data
                    //requires number of vertices, a list of distances between pairs of points, list for degree to y value exchange, and number of grade values in x- and y-directions
                    //CONVENTION: the x-coordinate is "scale parameter" for points and the y-coordinate is "degree parameter"

    
    
        void add_simplex(const std::vector<int> &vertices, const AppearanceGrades &grades);	//adds a simplex to BifiltrationData, grades is a vector of appearance grades

        void set_xy_grades(unsigned num_x, unsigned num_y); //Sets x_grades and y_grades. Used when reading in a bifiltration.

        unsigned num_x_grades();                     //returns the number of unique x-coordinates of the multi-grades
        unsigned num_y_grades();                     //returns the number of unique y-coordinates of the multi-grades

        int get_size(int dim);                  //returns the number of simplices of dimension (hom_dim-1), hom_dim, or (hom_dim+1). Returns -1 if invalid dim.

        const int hom_dim;      //the dimension of homology to be computed; max dimension of simplices is one more than this
        const int verbosity;	//controls display of output, for debugging

        //print bifiltration in the RIVET bifiltration input format
        void print_bifiltration();

    private:
    
        unsigned x_grades;  //the number of x-grades that exist in this bifiltration
        unsigned y_grades;  //the number of y-grades that exist in this bifiltration

        //TODO: It might be more efficient to store all simplices in a single vector, with pointers in.
        std::vector<LowSimplexData> low_simplices;
        std::vector<MidHighSimplexData> mid_simplices, high_simplices;
    
        void build_VR_subcomplex(const std::vector<unsigned>& times, const std::vector<unsigned>& distances, std::vector<int> &vertices, const unsigned prev_time, const unsigned prev_dist);	//recursive function used in build_VR_complex()

        void build_BR_subcomplex(const std::vector<unsigned>& distances, std::vector<int>& parent_indexes, const std::vector<int>& candidates, const AppearanceGrades& parent_grades, const std::vector<AppearanceGrades>& vertexMultigrades);	//recursive function used in build_BR_complex()

        void generateVertexMultigrades(std::vector<AppearanceGrades>& multigrades, const unsigned vertices, const std::vector<unsigned>& distances, const std::vector<unsigned>& degrees); //Generates required multigrades for build_BR_complex()

        void combineMultigrades(AppearanceGrades& merged, const AppearanceGrades& grades1, const AppearanceGrades& grades2, unsigned mindist); //Finds the grades of appearance of when both simplices exist subject to minimal scale parameter, used in build_BR_complex()
    
        //Note: Changed behavior of add_simplices so that it no longer adds in faces.  No longer need this helper function
        //TODO: Remove.
        /*
        void add_faces(const std::vector<int>& vertices, const AppearanceGrades& grades);	//recursively adds faces of a simplex to the BifiltrationData; WARNING: doesn't make sure multigrades are incomparable
        */
         
        void update_grades(AppearanceGrades& grades); //Sorts the grades of appearance in reverse lexicographic order and makes sure they are all incomparable

        //total number of simplces of dimension hom_dim, counting mutiplicity in grades of appearance.
        //used to avoid unnecessary copying of arrays in firep constructor.
        unsigned mid_count;
        unsigned high_count;

};

#endif // BIFILTRATION_DATA_H
