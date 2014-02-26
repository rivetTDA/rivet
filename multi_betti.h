/* multi-graded Betti number class
 * takes a bifiltration and computes the multi-graded Betti numbers
 *
 */
 
#ifndef __MultiBetti_H__
#define __MultiBetti_H__

#include <vector>
#include "simplex_tree.h"
#include <boost/multi_array.hpp>

class MultiBetti
{
	public:
		MultiBetti(SimplexTree* st, int dim);		//constructor, which does the work of computing all the multi-graded Betti numbers
		
		int xi0(int time, int dist);		//returns xi_0 at the specified multi-index
		int xi1(int time, int dist);		//returns xi_1 at the specified multi-index		
		
		
	private:
		SimplexTree* bifiltration;		//pointer to the bifiltration
		
		int dimension;		//dimension of homology to compute
		int num_times;		//number of time indexes
		int num_dists;		//number of distance indexes
		
		boost::multi_array<int, 3> xi;		//matrix to hold xi values; indices: xi[time][dist][subscript]

		static const bool print_matrices = true;	//controls display of output, for debugging
};

#include "multi_betti.hpp"

#endif // __MultiBetti_H__
