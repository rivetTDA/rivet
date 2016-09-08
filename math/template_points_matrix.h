#ifndef XI_SUPPORT_MATRIX_H
#define XI_SUPPORT_MATRIX_H

//forward declarations
class MultiBetti;
struct Multigrade;
class TemplatePoint;
class Arrangement;

#include <list>
#include <memory>
#include <vector>

//// these are the nodes in the sparse matrix
struct TemplatePointsMatrixEntry {
    //data structures
    unsigned x; //discrete x-grade of this support point
    unsigned y; //discrete y-grade of this support point
    unsigned index; //index of this support point in the vector of support points stored in VisualizationWindow

    std::shared_ptr<TemplatePointsMatrixEntry> down; //pointer to the next support point below this one
    std::shared_ptr<TemplatePointsMatrixEntry> left; //pointer to the next support point left of this one

    std::list<std::shared_ptr<Multigrade>> low_simplices; //associated multigrades for simplices of lower dimension
    std::list<std::shared_ptr<Multigrade>> high_simplices; //associated multigrades for simplices of higher dimension

    unsigned low_count; //number of columns in matrix of simplices of lower dimension that are mapped to this TemplatePointsMatrixEntry
    unsigned high_count; //number of columns in matrix of simplices of higher dimension that are mapped to this TemplatePointsMatrixEntry
    int low_index; //index of rightmost column in matrix of simplices of lower dimension that is mapped to this TemplatePointsMatrixEntry
    int high_index; //index of rightmost column in matrix of simplices of higher dimension that is mapped to this TemplatePointsMatrixEntry
    //NOTE: if TemplatePointsMatrixEntry is a LUB-index but there are no low (resp. high) columns mapped to it, then low_index (resp. high_index) is the index of the column just left of where such columns would appear (could be -1)

    //functions
    TemplatePointsMatrixEntry(); //empty constructor
    TemplatePointsMatrixEntry(unsigned x, unsigned y, unsigned i, std::shared_ptr<TemplatePointsMatrixEntry> d, std::shared_ptr<TemplatePointsMatrixEntry> l); //regular constructor
    TemplatePointsMatrixEntry(unsigned x, unsigned y); //constructor for temporary entries used in counting switches

    void add_multigrade(unsigned x, unsigned y, unsigned num_cols, int index, bool low); //associates a (new) multigrades to this xi entry
    //the "low" argument is true if this multigrade is for low_simplices, and false if it is for high_simplices

    void insert_multigrade(std::shared_ptr<Multigrade> mg, bool low); //inserts a Multigrade at the end of the list for the given dimension; does not update column counts!
};

//// each node in the sparse matrix maintains two lists of multigrades
struct Multigrade {
    unsigned x; //x-coordinate of this multigrade
    unsigned y; //y-coordinate of this multigrade

    unsigned num_cols; //number of columns (i.e. simplices) at this multigrade
    int simplex_index; //last dim_index of the simplices at this multigrade; necessary so that we can build the boundary matrix, and also used for non-vineyard updates to the RU-decomposition

    Multigrade(unsigned x, unsigned y, unsigned num_cols, int simplex_index); //constructor

    Multigrade(); // For serialization

    static bool LexComparator(const Multigrade& first, const Multigrade& second); //comparator for sorting Multigrades lexicographically
};

//// sparse matrix to store the set U of support points of the multi-graded Betti numbers
//// Also known as 'tPtsMat' in the RIVET paper.
class TemplatePointsMatrix {
public:
    TemplatePointsMatrix(unsigned width, unsigned height); //constructor

    std::vector<std::shared_ptr<TemplatePointsMatrixEntry>> fill_and_find_anchors(std::vector<TemplatePoint>& xi_pts); //stores xi support points in the xiSupportMatrix
    //also finds anchors, which are stored both in the matrix and in the vector xi_pts
    //precondition: xi_pts contains the support points in lexicographical order

    std::shared_ptr<TemplatePointsMatrixEntry> get_row(unsigned r); //gets a pointer to the rightmost entry in row r; returns NULL if row r is empty
    std::shared_ptr<TemplatePointsMatrixEntry> get_col(unsigned c); //gets a pointer to the top entry in column c; returns NULL if column c is empty

    unsigned height(); //retuns the number of rows;

    void clear_grade_lists(); //clears the level set lists for all entries in the matrix

private:
    std::vector<std::shared_ptr<TemplatePointsMatrixEntry>> columns;
    std::vector<std::shared_ptr<TemplatePointsMatrixEntry>> rows;
};

#endif // XI_SUPPORT_MATRIX_H
