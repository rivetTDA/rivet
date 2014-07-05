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

typedef std::vector<int> Vector;

class MultiBetti
{
	public:
		MultiBetti(SimplexTree* st, int dim, int v);		//constructor sets up the data structure but doesn't compute the multi-graded Betti numbers xi_0 and xi_1
		
        void compute_fast();		//computes xi_0 and xi_1 at all multi-grades in a fast way
		
		
        int xi0(int x, int y);		//returns xi_0 at the specified multi-grade
        int xi1(int x, int y);		//returns xi_1 at the specified multi-grade

        //DEPRECATED METHODS
//        void compute_all_xi();			//computes xi_0 and xi_1 at ALL multi-indexes
//		void compute_xi(int time, int dist);	//computes xi_0 and xi_1 at a specified multi-index
		
		
		
	private:
		SimplexTree* bifiltration;		//pointer to the bifiltration
		
		int dimension;		//dimension of homology to compute
		
        int num_x_grades;  //number of grades in primary direction
        int num_y_grades;  //number of grades in secondary direction


        boost::multi_array<int, 3> xi;		//matrix to hold xi values; indices: xi[x][y][subscript]
		
		const int verbosity;	//controls display of output, for debugging

        //function to do column reduction for Edelsbrunner algorithm
        void reduce(MapMatrix* mm, int first_col, int last_col, Vector& lows, int& zeroed_cols);

        //DEPRECATED OBJECTS
        int num_times;		//number of time indexes
        int num_dists;		//number of distance indexes

		
};

#include "multi_betti.cpp"

#endif // __MultiBetti_H__
