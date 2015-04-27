/**
 * \class	MapMatrix and related classes
 * \brief	Stores a matrix representing a simplicial map and provides operations for persistence calculations.
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
 * The MapMatrix_Base class provides the basic structures and functionality; it is the parent class and is not meant to be instantiated directly.
 * The class MapMatrix inherits MapMatrix_Base and stores matrices in a column-sparse format, designed for basic persistence calcuations.
 * The class MapMatrix_Perm inherits MapMatrix, adding functionality for row and column permutations; it is designed for the reduced matrices of vineyard updates.
 * Lastly, the class MapMatrix_RowPriority_Perm inherits MapMatrix_Base and stores matrices in a row-sparse format with row and column permutations; it is designed for the upper-triangular matrices of vineyard updates.
 */
 
#ifndef __MapMatrix_H__
#define __MapMatrix_H__

#include <vector>
#include <stdexcept>


//base class simply implements features common to all MapMatrices, whether column-priority or row-priority
//written here using column-priority terminology, but this class is meant to be inherited, not instantiated directly
class MapMatrix_Base {
    protected:
        MapMatrix_Base(unsigned rows, unsigned cols);  //constructor to create matrix of specified size (all entries zero)
        MapMatrix_Base(unsigned size);                 //constructor to create a (square) identity matrix
        virtual ~MapMatrix_Base();                             //destructor

        virtual unsigned width();				//returns the number of columns in the matrix
        virtual unsigned height();				//returns the number of rows in the matrix

        virtual void set(unsigned i, unsigned j);       //sets (to 1) the entry in row i, column j
        virtual void clear(unsigned i, unsigned j);     //clears (sets to 0) the entry in row i, column j
        virtual bool entry(unsigned i, unsigned j);     //returns true if entry (i,j) is 1, false otherwise

        virtual void add_column(unsigned j, unsigned k);		//adds column j to column k; RESULT: column j is not changed, column k contains sum of columns j and k (with mod-2 arithmetic)

        class MapMatrixNode {       //subclass for the nodes in the MapMatrix
            public:
                MapMatrixNode(unsigned row);		//constructor

                unsigned get_row();                 //returns the row index
                void set_next(MapMatrixNode* n);	//sets the pointer to the next node in the column
                MapMatrixNode* get_next();          //returns a pointer to the next node in the column

            private:
                unsigned row_index;			//index of matrix row corresponding to this node
                MapMatrixNode* next;		//pointer to the next entry in the column containing this node
        };

        std::vector<MapMatrixNode*> columns;	//vector of pointers to nodes representing columns of the matrix

        unsigned num_rows;                      //number of rows in the matrix
};


//MapMatrix is a column-priority matrix designed for standard persistence calculations
class MapMatrix : public MapMatrix_Base
{
	public:
        MapMatrix(unsigned rows, unsigned cols);  //constructor to create matrix of specified size (all entries zero)
        MapMatrix(unsigned size);                 //constructor to create a (square) identity matrix
        virtual ~MapMatrix();                             //destructor
		
        unsigned width();               //returns the number of columns in the matrix
        unsigned height();              //returns the number of rows in the matrix
		
        virtual void set(unsigned i, unsigned j);       //sets (to 1) the entry in row i, column j
        virtual bool entry(unsigned i, unsigned j);     //returns true if entry (i,j) is 1, false otherwise
		
        virtual int low(unsigned j);                    //returns the "low" index in the specified column, or -1 if the column is empty
        bool col_is_empty(unsigned j);                  //returns true iff column j is empty (for columns that are not empty, this method is faster than low(j))

        void add_column(unsigned j, unsigned k);    //adds column j to column k; RESULT: column j is not changed, column k contains sum of columns j and k (with mod-2 arithmetic)
        void add_column(MapMatrix* other, unsigned j, unsigned k);    //adds column j from MapMatrix* other to column k of this matrix

        void col_reduce();          //applies the column reduction algorithm to this matrix
		
        virtual void print();       //prints the matrix to standard output (for testing)
};


//MapMatrix with row/column permutations and low array, designed for "vineyard updates"
    class MapMatrix_RowPriority_Perm;   //forward declaration
class MapMatrix_Perm : public MapMatrix
{
    public:
        MapMatrix_Perm(unsigned rows, unsigned cols);
        MapMatrix_Perm(unsigned size);
        ~MapMatrix_Perm();

        void set(unsigned i, unsigned j);       //sets (to 1) the entry in row i, column j
        bool entry(unsigned i, unsigned j);     //returns true if entry (i,j) is 1, false otherwise

        MapMatrix_RowPriority_Perm* decompose_RU();  //reduces this matrix, fills the low array, and returns the corresponding upper-triangular matrix for the RU-decomposition
            //NOTE -- only to be called before any rows are swapped!

        int low(unsigned j);        //returns the "low" index in the specified column, or -1 if the column is empty
        int find_low(unsigned l);   //returns the index of the column with low l, or -1 if there is no such column

        void swap_rows(unsigned i, bool update_lows);     //transposes rows i and i+1, optionally updates low array
        void swap_columns(unsigned j, bool update_lows);  //transposes columns j and j+1, optionally updates low array

      ///FOR TESTING ONLY
        virtual void print();       //prints the matrix to standard output (for testing)
        void check_lows();          //checks for inconsistencies in low arrays

    protected:
        std::vector<unsigned> perm;     //permutation vector
        std::vector<unsigned> mrep;     //inverse permutation vector
        std::vector<int> low_by_row;    //stores index of column with each low number, or -1 if no such column exists -- NOTE: only accurate after decompose_RU() is called
        std::vector<int> low_by_col;    //stores the low number for each column, or -1 if the column is empty -- NOTE: only accurate after decompose_RU() is called

//        std::vector<unsigned> col_perm; ///column permutation vector -- FOR TESTING ONLY
};


//MapMatrix stored in row-priority format, with row/column permutations, designed for upper-triangular matrices in vineyard updates
class MapMatrix_RowPriority_Perm: public MapMatrix_Base
{
    public:
        MapMatrix_RowPriority_Perm(unsigned size);   //constructs the identity matrix of specified size
        ~MapMatrix_RowPriority_Perm();

        unsigned width();				//returns the number of columns in the matrix
        unsigned height();				//returns the number of rows in the matrix

        void set(unsigned i, unsigned j);       //sets (to 1) the entry in row i, column j
        void clear(unsigned i, unsigned j);     //clears (sets to 0) the entry in row i, column j
        bool entry(unsigned i, unsigned j);     //returns true if entry (i,j) is 1, false otherwise

        void add_row(unsigned j, unsigned k);   //adds row j to row k; RESULT: row j is not changed, row k contains sum of rows j and k (with mod-2 arithmetic)

        void swap_rows(unsigned i);    //transposes rows i and i+1
        void swap_columns(unsigned j); //transposes columns j and j+1

        void print();			//prints the matrix to standard output (for testing)

    protected:
        std::vector<unsigned> perm;     //permutation vector
        std::vector<unsigned> mrep;     //inverse permutation vector
};

#endif // __MapMatrix_H__
