/**
 * \class	MultiBetti
 * \brief	Computes the multi-graded Betti numbers of a bifiltration.
 * \author	Matthew L. Wright
 * \date	February 2014
 * 
 * Given a bifiltration and a dimension of homology, this class computes the multi-graded Betti numbers (xi_0 and xi_1),
 * either at all multi-index pairs or at specific pairs.
 */
 
#ifndef __MultiBetti_H__
#define __MultiBetti_H__

#include <vector>
#include "simplex_tree.h"
#include <boost/multi_array.hpp>

class MultiBetti
{
	public:
		MultiBetti(SimplexTree* st, int dim, int v);		//constructor sets up the data structure but doesn't compute the multi-graded Betti numbers xi_0 and xi_1
		
		void compute_all_xi();			//computes xi_0 and xi_1 at ALL multi-indexes
		void compute_xi(int time, int dist);	//computes xi_0 and xi_1 at a specified multi-index
		
		
		int xi0(int time, int dist);		//returns xi_0 at the specified multi-index
		int xi1(int time, int dist);		//returns xi_1 at the specified multi-index		
		
		
	private:
		SimplexTree* bifiltration;		//pointer to the bifiltration
		
		int dimension;		//dimension of homology to compute
		int num_times;		//number of time indexes
		int num_dists;		//number of distance indexes
		
		boost::multi_array<int, 3> xi;		//matrix to hold xi values; indices: xi[time][dist][subscript]
		
		const int verbosity;	//controls display of output, for debugging
		
};

#include "multi_betti.cpp"

#endif // __MultiBetti_H__
