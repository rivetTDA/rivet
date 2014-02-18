/* map matrix class
 * stores a matrix representing a simplicial map
 *
 * based on survey paper by Edelsbrunner and Harer
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
		int heigth();				//returns the number of rows in the matrix
		
		void set(int i, int j);			//sets (to 1) the entry in row i, column j
		void clear(int i, int j);		//clears (to 0) the entry in row i, column j ------- NOT IMPLEMENTED YET!!!
		bool entry(int i, int j);		//returns true if entry (i,j) is 1, false otherwise
		
		int low(int j);				//returns the "low" index in the specified column
		
		void add_column(int j, int k);		//adds column j to column k
		
		void print();				//prints the matrix to standard output (useful for testing)
		
		
	private:
		std::vector<MapMatrixNode*> columns;	//vector of pointers to nodes representing columns of the matrix
		
		std::vector<int> col_indexes;		//global simplex indexes for the columns of this matrix
		std::vector<int> row_indexes;		//global simplex indexes for the rows of this matrix
		
		int height;				//number of rows in the matrix
};

#include "map_matrix.hpp"

#endif // __MapMatrix_H__

