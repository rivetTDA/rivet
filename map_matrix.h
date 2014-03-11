/**
 * \class	MapMatrix
 * \brief	Stores a matrix representing a simplicial map and provides basic operations.
 * \author	Matthew L. Wright
 * \date	February 2014
 * \warning	In the matrix perspective, row and column indices start at 1, not at 0.
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
 
#include "map_matrix_node.h"

class MapMatrix
{
	public:
	//	MapMatrix();				//constructor
		MapMatrix(int i, int j);		//constructor to create matrix of specified size
		
		int width();				//returns the number of columns in the matrix
		int height();				//returns the number of rows in the matrix
		
		void set(int i, int j);			//sets (to 1) the entry in row i, column j   NOTE: row/column indices start at 1, not 0
		void clear(int i, int j);		//clears (to 0) the entry in row i, column j ------- NOT IMPLEMENTED YET!!!
		bool entry(int i, int j);		//returns true if entry (i,j) is 1, false otherwise
		
		int low(int j);				//returns the "low" index in the specified column, or 0 if the column is empty or does not exist
		
// NOW UNNECESSARY:		void append_block_diag(MapMatrix& other);	//modifies this matrix to create a larger, block-diagonal matrix with "other" positioned below and to the right of "this"
		void append_block(MapMatrix& other, int vert_offset);	//modifies this matrix to create a larger matrix with "other" positioned to the right of "this", with a specified vertical offset	
		
		void col_reduce();			//applies the column reduction algorithm to this matrix
		void col_reduce(MapMatrix* other);	//applies the column reduction algorithm to this matrix, and also performs the same column operations on the other matrix
		
		void add_column(int j, int k);		//adds column j to column k; RESULT: column j is not changed, column k contains sum of columns j and k (with mod-2 arithmetic)
		
		void remove_column(int j);			//removes column j from the matrix (decreasing width by 1)
		
		void print();				//prints the matrix to standard output (useful for testing)
		
		
	private:
		std::vector<MapMatrixNode*> columns;	//vector of pointers to nodes representing columns of the matrix
		
	//	std::vector<int> col_indexes;		//global simplex indexes for the columns of this matrix
	//	std::vector<int> row_indexes;		//global simplex indexes for the rows of this matrix
		
		int num_rows;				//number of rows in the matrix
};

#include "map_matrix.hpp"

#endif // __MapMatrix_H__

