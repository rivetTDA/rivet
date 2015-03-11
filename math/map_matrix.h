/**
 * \class	MapMatrix
 * \brief	Stores a matrix representing a simplicial map and provides basic operations.
 * \author	Matthew L. Wright
 * \date	February 2014
 * 
 * The MapMatrix class stores a matrix representing a simplicial map, such as a boundary map.
 * Such a matrix is a sparse matrix with entries in the two-element field.
 * This implementation is based on that described in the persistent homology survey paper by Edelsbrunner and Harer.
 * Operations are those necessary for persistence computations.
 *
 * Implementation details: A vector contains pointers to the first element (possibly null) in each column; each column is represented by a linked list.
 * Linked lists connecting entries in each row are not implemented.
 * Each entry in the matrix is an instance of the MapMatrixNode class.
 *
 * Several classes inherit the MapMatrix class, adding extra functionality for the algebra of "vineyard updates."
 */
 
#ifndef __MapMatrix_H__
#define __MapMatrix_H__

#include <vector>
#include <stdexcept>

class MapMatrixNode {
    public:
        MapMatrixNode(unsigned row);		//constructor

        unsigned get_row();			//returns the row index
        void set_next(MapMatrixNode* n);	//sets the pointer to the next node in the column
        MapMatrixNode* get_next();	//returns a pointer to the next node in the column

    private:
        unsigned row_index;			//index of matrix row corresponding to this node
        MapMatrixNode* next;		//pointer to the next entry in the column containing this node
};


class MapMatrix
{
	public:
        MapMatrix(unsigned rows, unsigned cols);  //constructor to create matrix of specified size (all entries zero)
        MapMatrix(unsigned size);                 //constructor to create a (square) identity matrix
        ~MapMatrix();                             //destructor
		
        unsigned width();				//returns the number of columns in the matrix
        unsigned height();				//returns the number of rows in the matrix
		
        void set(unsigned i, unsigned j);       //sets (to 1) the entry in row i, column j
        void clear(unsigned i, unsigned j);     //clears (sets to 0) the entry in row i, column j
        bool entry(unsigned i, unsigned j);     //returns true if entry (i,j) is 1, false otherwise
		
        int low(unsigned j);				//returns the "low" index in the specified column, or -1 if the column is empty or does not exist
            //NOTE CHANGE: now an empty column has "low" index -1, not 0

		void col_reduce();			//applies the column reduction algorithm to this matrix
		
        MapMatrix* decompose_RU();  //reduces this matrix and returns the TRANSPOSE of the corresponding upper-triangular matrix for the RU-decomposition

        void add_column(unsigned j, unsigned k);		//adds column j to column k; RESULT: column j is not changed, column k contains sum of columns j and k (with mod-2 arithmetic)
        void add_column(MapMatrix* other, unsigned j, unsigned k);    //adds column j from MapMatrix* other to column k of this matrix

        void swap_columns(unsigned j); //transposes columns j and j+1

        ///// TESTING
            void print();				//prints the matrix to standard output (useful for testing)
            void print_transpose();     //prints the transpose of the matrix to stadard output
		
    protected:
		std::vector<MapMatrixNode*> columns;	//vector of pointers to nodes representing columns of the matrix
		
        unsigned num_rows;				//number of rows in the matrix
};

//MapMatrix with row permutations
class MapMatrix_P : public MapMatrix
{
    public:
        MapMatrix_P(unsigned rows, unsigned cols);
        MapMatrix_P(unsigned size);
        ~MapMatrix_P();

        int low(unsigned j);  ///TODO: THIS MUST BE DIFFERENT WITH A PERMUTATION ARRAY FOR ROWS

        void clear(unsigned i, unsigned j); ///TODO: THIS MUST BE DIFFERENT WITH A PERMUTATION ARRAY FOR ROWS

        void swap_rows(unsigned i);  //transposes rows i and i+1

        ///TODO: ANYTHING ELSE???

    protected:
        std::vector<unsigned> perm;     //permutation vector
        std::vector<unsigned> perm_inv; //inverse permutation vector
};

//MapMatrix with row permutations and low array
class MapMatrix_PL : public MapMatrix_P
{

};

//MapMatrix stored in row-priority format, with column permutations (for the upper-triangular matrices in vineyard updates)
class MapMatrix_RP : public MapMatrix_P
{

};

#endif // __MapMatrix_H__

