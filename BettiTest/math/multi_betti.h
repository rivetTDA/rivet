/**
 * \class	MultiBetti
 * \brief	Computes the multi-graded Betti numbers of a bifiltration.
 * \author	Matthew L. Wright
 * \date	February 2014
 * 
 * Given a bifiltration and a dimension of homology, this class computes the multi-graded Betti numbers (xi_0 and xi_1).
 */
 
#ifndef __MultiBetti_H__
#define __MultiBetti_H__

#include <vector>
#include "simplex_tree.h"
#include "index_matrix.h"
#include <boost/multi_array.hpp>

typedef std::vector<unsigned> Vector;

struct ColumnList;  //necessary for column reduction in MultiBetti::reduce(...)

class MultiBetti
{
	public:
		MultiBetti(SimplexTree* st, int dim, int v);		//constructor sets up the data structure but doesn't compute the multi-graded Betti numbers xi_0 and xi_1
		
		
        //OLD functions to compute xi_0 and xi_1
        void compute_fast();		//computes xi_0 and xi_1 at all multi-grades in a fast way
        void compute_nullities();
        void compute_ranks();
        void compute_alpha();
        void compute_eta();

        //NEW functions to compute xi_0 and xi_1
        void compute_parallel();





        int xi0(unsigned x, unsigned y);		//returns xi_0 at the specified (discrete) multi-grade
        int xi1(unsigned x, unsigned y);		//returns xi_1 at the specified (discrete) multi-grade
		
        void print_lows(Vector &lows);  //TESTING ONLY
		
        SimplexTree* bifiltration;		//pointer to the bifiltration

        const int dimension;		//dimension of homology to compute

	private:

        unsigned num_x_grades;  //number of grades in primary direction
        unsigned num_y_grades;  //number of grades in secondary direction


        boost::multi_array<int, 3> xi;		//matrix to hold xi values; indices: xi[x][y][subscript]
		
		const int verbosity;	//controls display of output, for debugging


        void reduce(MapMatrix* mm, int first_col, int last_col, Vector& lows, int& zero_cols);
            //column reduction for Edelsbrunner algorithm

        void reduce_also(MapMatrix* mm, MapMatrix* m2, int first_col, int last_col, Vector& lows, int y_grade, ColumnList &zero_list, int &zero_cols);
            //column reduction for Edelsbrunner algorithm, also performs column additions on a second matrix

        void reduce_spliced(MapMatrix* m_left, MapMatrix* m_right, IndexMatrix* ind_left, IndexMatrix* ind_right, ColumnList& right_cols, int grade_x, int grade_y, Vector& lows, int& zero_cols);
            //column reduction for Edelsbrunner algorithm on a two-part matrix (two matrices spliced together, treated as one matrix for the column reduction)

		
};


#endif // __MultiBetti_H__
