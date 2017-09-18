/**
 * \class	BifiltrationData
 * \brief	Computes and stores the information about a bifiltration needed to compute homology in fixed dimension d.  Together with Input_Manager, handles 1-critical or multicritical Rips bifiltrations, as defined in the RIVET paper.
 * \author	Roy Zhao
 * \date	May 2016
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
    
    //removing dimension index, as this is better kept separately.
    bool operator<(const Grade& other) const
    {
        if (y != other.y)
            return y < other.y;
        else
            return x < other.x;
    }

    bool operator==(const Grade& other) const
    {
        return (y == other.y) && (x == other.x);
    }

    Grade() {}

    Grade(int set_x, int set_y) : x(set_x), y(set_y)
    { }
};


struct GradeHash
{
    std::size_t operator()(Grade const& grade) const
    {
        size_t seed = 0;
        boost::hash_combine(seed, grade.x);
        boost::hash_combine(seed, grade.y);
        return seed;
    }
};

struct VectorHash
{
    std::size_t operator()(std::vector<int> const& v) const
    {
        return boost::hash_range(v.begin(), v.end());
    }
};

//typedef
typedef std::vector<Grade> AppearanceGrades;
typedef std::unordered_map<std::vector<int>, size_t, GradeHash> SimplexHashLow; //key = vector representing a simplex; value = index in vector<Grade>.
typedef std::unordered_map<std::vector<int>, size_t, VectorHash> SimplexHashMid; //keylist = vector representing a simplex; value = index in vector<vector<Grades>>.

// Note: No need for a hash table representation of simplices in the highest index, because we will not consider these simplices as boundaries, and there is no possibility of inserting the same simplex twice.

// We only need a single grade for each simplex in the low dimension because for homology, it suffices to consider a greatest lower bound of all grades.  For mid and high dimensions, we need a vector of grades for each simplex, in the multicritical case.
typedef std::vector<std::pair<Grade,SimplexHashLow::iterator> SimplexVecLow;
typedef std::vector<std::pair<AppearanceGrades,SimplexHashLow::iterator> SimplexVecMid;
typedef std::vector<AppearanceGrades> SimplexVecHigh;


class BifiltrationData {
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

        void add_simplex(std::vector<int>& vertices, const AppearanceGrades& grades);	//adds a simplex (and its faces) to BifiltrationData, grades is a vector of appearance grades
    
        void add_simplex(std::vector<int>& vertices, const AppearanceGrades& grades);	//adds a simplex (and its faces) to BifiltrationData, grades is a vector of appearance grades
    
        void add_simplex(std::vector<int>& vertices, const Grade& grade);	//adds a simplex (and its faces) to BifiltrationData, grade is a single appearance grade.
    
        void set_xy_grades(unsigned num_x, unsigned num_y); //Sets x_grades and y_grades. Used when reading in a bifiltration.

        unsigned num_x_grades();                     //returns the number of unique x-coordinates of the multi-grades
        unsigned num_y_grades();                     //returns the number of unique y-coordinates of the multi-grades

        int get_size(int dim);                  //returns the number of simplices of dimension (hom_dim-1), hom_dim, or (hom_dim+1). Returns -1 if invalid dim.


        /*
        SimplexListLow* getSimplexVecLow(); //returns pointer to the list of simplices in dimension (hom_dim-1)
    
        SimplexListMid* getSimplexVecMid(); //returns pointer to the list of simplices in dimension (hom_dim) or (hom_dim+1)
    
        SimplexListHigh* getSimplexVecHigh(); //returns pointer to the list of simplices in dimension (hom_dim) or (hom_dim+1)
    
        SimplexListLow* getSimplexVecLow; //returns pointer to the list of simplices in dimension (hom_dim-1)
    
        SimplexListMidHigh* getSimplexVecMidHigh(int dim); //returns point to the list of simplices in dimension (hom_dim) or (hom_dim+1)
        */

        const int hom_dim;      //the dimension of homology to be computed; max dimension of simplices is one more than this
        const int verbosity;	//controls display of output, for debugging

        //print bifiltration in the RIVET bifiltration input format
        void print_bifiltration();

    private:
    
        unsigned x_grades;  //the number of x-grades that exist in this bifiltration
        unsigned y_grades;  //the number of y-grades that exist in this bifiltration

        //firep is a friend class, which allows it to work directly with the following private data.
        SimplexHashMidHigh high_ht;    //hash table giving the indices of simplices of dimension hom_dim
        SimplexHashLow low_ht;    //hash table with key the simplices of dimension hom_dim-1 and values the indices of their representation in a vector.
        SimplexHashMidHigh mid_ht;  //hash table with key the simplices of dimension hom_dim and values the indices of their representation in a vector.
    
        //lists of simplices constructed here will be sorted later by the FI-Rep constructor
        //TODO: That is the way it is currently done, but is that the best organization?
        SimpVectLow low_simplices;
        SimplexVecMid mid_simplices;
        SimplexVecHigh high_simplices;
    
        void build_VR_subcomplex(const std::vector<unsigned>& times, const std::vector<unsigned>& distances, std::vector<int> &vertices, const unsigned prev_time, const unsigned prev_dist);	//recursive function used in build_VR_complex()

        void build_BR_subcomplex(const std::vector<unsigned>& distances, std::vector<int>& parent_indexes, const std::vector<int>& candidates, const AppearanceGrades& parent_grades, const std::vector<AppearanceGrades>& vertexMultigrades);	//recursive function used in build_BR_complex()

        void generateVertexMultigrades(std::vector<AppearanceGrades>& multigrades, const unsigned vertices, const std::vector<unsigned>& distances, const std::vector<unsigned>& degrees); //Generates required multigrades for build_BR_complex()

        void combineMultigrades(AppearanceGrades& merged, const AppearanceGrades& grades1, const AppearanceGrades& grades2, unsigned mindist); //Finds the grades of appearance of when both simplices exist subject to minimal scale parameter, used in build_BR_complex()

        //void addSimplicesToGrade(GradeInfo* orderedGrades, const std::vector<int> simplex, const AppearanceGrades& grades); //Takes a simplex and its grades of appearance and adds it to ordered_high_grades, ordered_grades, or ordered_low_grades

        void add_faces(const std::vector<int>& vertices, const AppearanceGrades& grades);	//recursively adds faces of a simplex to the BifiltrationData; WARNING: doesn't make sure multigrades are incomparable

        void update_grades(AppearanceGrades& grades); //Sorts the grades of appearance in reverse lexicographic order and makes sure they are all incomparable
};

#endif // BIFILTRATION_DATA_H
