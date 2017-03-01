/**
 * \class	BifiltrationData
 * \brief	Stores the raw bifiltration data (simplices and their multigrades of apppearance). Used in calculating the free implicit representation.
 * \author	Roy Zhao
 * \date	May 2016
 */

#ifndef BIFILTRATION_DATA_H
#define BIFILTRATION_DATA_H


//forward declarations
class IndexMatrix;
class MapMatrix;
class MapMatrix_Perm;

#include <set>
#include <vector>
#include <boost/unordered_map.hpp>

//Pair of coordinates specifying grade of appearance with additional sorting operator. Sorted first by y then x grade in REVERSE-LEXICOGRAPHIC ORDER.
struct Grade
{
    int x;
    int y;
    bool operator<(Grade other) const
    {
        if (y != other.y)
            return y < other.y;
        else
            return x < other.x;
    }

    bool operator==(Grade other) const
    {
        return (y == other.y) && (x == other.x);
    }

    Grade(int set_x, int set_y) : x(set_x), y(set_y)
    { }
};

struct GradeHash
    : std::unary_function<Grade, std::size_t>
{
    std::size_t operator()(Grade const& grade) const
    {
        return boost::hash<int>()(grade.x) ^ boost::hash<int>()(grade.y);
    }
};

//typedef
typedef std::vector<Grade> AppearanceGrades;
typedef boost::unordered::unordered_map<std::vector<int>, AppearanceGrades> SimplexInfo; //vector key is list of vertices in the simplex, value is grades of appearance of simplex
typedef boost::unordered::unordered_map<Grade, std::vector<std::vector<int> >, GradeHash> GradeInfo; //Grade key is a grade, value is the list of simplices which are born at that grade.

//now the BifiltrationData class
class BifiltrationData {
    public:
        BifiltrationData(int dim, int v);	//constructor; requires verbosity parameter

        ~BifiltrationData(); //destructor

        void build_VR_complex(std::vector<unsigned>& times, std::vector<unsigned>& distances, unsigned num_x, unsigned num_y);
                    //builds BifiltrationData representing a bifiltered Vietoris-Rips complex from discrete data
                    //requires a list of birth times (one for each point), a list of distances between pairs of points, max dimension of simplices to construct, and number of grade values in x- and y-directions
                    //NOTE: automatically computes global indexes and dimension indexes
                    //CONVENTION: the x-coordinate is "birth time" for points and the y-coordinate is "distance" between points

        void build_BR_complex(unsigned num_vertices, std::vector<unsigned>& distances, std::vector<unsigned>& degrees, unsigned num_x, unsigned num_y);
                    //builds BifiltrationData representing a bifiltered Rips complex from discrete data
                    //requires number of vertices, a list of distances between pairs of points, list for degree to y value exchange, and number of grade values in x- and y-directions
                    //CONVENTION: the x-coordinate is "scale parameter" for points and the y-coordinate is "degree parameter"

        void add_simplex(std::vector<int>& vertices, AppearanceGrades& grades);	//adds a simplex (and its faces) to BifiltrationData, grades is a vector of appearance grades
        void createGradeInfo(); //Assuming that SimplexInfo is fully created, creates the inverse mapping GradeInfo

        void set_xy_grades(unsigned num_x, unsigned num_y); //Sets x_grades and y_grades. Used when reading in a bifiltration.

        unsigned num_x_grades();                     //returns the number of unique x-coordinates of the multi-grades
        unsigned num_y_grades();                     //returns the number of unique y-coordinates of the multi-grades

        int get_size(int dim);                  //returns the number of simplices of dimension (hom_dim-1), hom_dim, or (hom_dim+1). Returns -1 if invalid dim.

        SimplexInfo* getSimplices(int dim); //returns the list of simplices and their grades of appearance in dimension (hom_dim-1), hom_dim, or (hom_dim+1)

        GradeInfo* getGrades(int dim); //returns the list of grades and their simplices in dimension (hom_dim-1), hom_dim, or (hom_dim+1)

        const int hom_dim;      //the dimension of homology to be computed; max dimension of simplices is one more than this
        const int verbosity;	//controls display of output, for debugging

        //print bifiltration in the RIVET bifiltration input format
        void print_bifiltration();

    private:

        unsigned x_grades;  //the number of x-grades that exist in this bifiltration
        unsigned y_grades;  //the number of y-grades that exist in this bifiltration

        SimplexInfo* ordered_high_simplices;   //pointers to simplices of dimension (hom_dim + 1) in reverse-lexicographical multi-grade order
        SimplexInfo* ordered_simplices;        //pointers to simplices of dimension hom_dim in reverse-lexicographical multi-grade order
        SimplexInfo* ordered_low_simplices;    //pointers to simplices of dimension (hom_dim - 1) in reverse-lexicographical multi-grade order

        GradeInfo* ordered_high_grades;   //pointers to simplices of dimension (hom_dim + 1) in reverse-lexicographical multi-grade order
        GradeInfo* ordered_grades;        //pointers to simplices of dimension hom_dim in reverse-lexicographical multi-grade order
        GradeInfo* ordered_low_grades;    //pointers to simplices of dimension (hom_dim - 1) in reverse-lexicographical multi-grade order

        void build_VR_subcomplex(std::vector<unsigned>& times, std::vector<unsigned>& distances, std::vector<int> &vertices, unsigned prev_time, unsigned prev_dist);	//recursive function used in build_VR_complex()

        void build_BR_subcomplex(std::vector<unsigned>& distances, std::vector<int>& parent_indexes, std::vector<int>& candidates, AppearanceGrades& parent_grades, std::vector<AppearanceGrades>& vertexMultigrades);	//recursive function used in build_BR_complex()

        void generateVertexMultigrades(std::vector<AppearanceGrades>& multigrades, unsigned vertices, std::vector<unsigned>& distances, std::vector<unsigned>& degrees); //Generates required multigrades for build_BR_complex()

        void combineMultigrades(AppearanceGrades& merged, AppearanceGrades& grades1, AppearanceGrades& grades2, unsigned mindist); //Finds the grades of appearance of when both simplices exist subject to minimal scale parameter, used in build_BR_complex()

        void addSimplicesToGrade(GradeInfo* orderedGrades, std::vector<int> simplex, AppearanceGrades& grades); //Takes a simplex and its grades of appearance and adds it to ordered_high_grades, ordered_grades, or ordered_low_grades

        void add_faces(std::vector<int>& vertices, AppearanceGrades& grades);	//recursively adds faces of a simplex to the BifiltrationData; WARNING: doesn't make sure multigrades are incomparable

        void update_grades(AppearanceGrades& grades); //Sorts the grades of appearance in reverse lexicographic order and makes sure they are all incomparable
};

#endif // BIFILTRATION_DATA_H
