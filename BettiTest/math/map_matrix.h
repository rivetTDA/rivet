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
 */
 
#ifndef __MapMatrix_H__
#define __MapMatrix_H__

#include <vector>
#include <stdexcept>

class MapMatrixNode;



class MapMatrix
{
	public:
        MapMatrix(unsigned rows, unsigned cols);		//constructor to create matrix of specified size
		
        unsigned width();				//returns the number of columns in the matrix
        unsigned height();				//returns the number of rows in the matrix
		
        void set(unsigned i, unsigned j);			//sets (to 1) the entry in row i, column j
        bool entry(unsigned i, unsigned j);		//returns true if entry (i,j) is 1, false otherwise
		
        //UPDATED!!! now an empty column has "low" index max_unsigned_int
        unsigned low(unsigned j);				//returns the "low" index in the specified column, or max_unsigned_int if the column is empty (i.e. low does not exist)

		void col_reduce();			//applies the column reduction algorithm to this matrix
		
        void add_column(unsigned j, unsigned k);		//adds column j to column k; RESULT: column j is not changed, column k contains sum of columns j and k (with mod-2 arithmetic)
        void add_column(MapMatrix* other, int j, int k);    //adds column j from MapMatrix* other to column k of this matrix

        ///// TESTING
            void print();				//prints the matrix to standard output (useful for testing)
		
	private:
		std::vector<MapMatrixNode*> columns;	//vector of pointers to nodes representing columns of the matrix
		
        unsigned num_rows;				//number of rows in the matrix
};


class MapMatrixNode {
    public:
        MapMatrixNode(unsigned);		//constructor

        unsigned get_row();			//returns the row index
        void set_next(MapMatrixNode*);	//sets the pointer to the next node in the column
        MapMatrixNode* get_next();	//returns a pointer to the next node in the column

    private:
        unsigned row_index;			//index of matrix row corresponding to this node
        MapMatrixNode * next;		//pointer to the next entry in the column containing this node
};


#endif // __MapMatrix_H__

