/**
 * \class	CellPersistenceData
 * \brief	Stores the persistence data associated with each face in the DCEL.
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __DCEL_CELL_PERSISTENCE_DATA_H__
#define __DCEL_CELL_PERSISTENCE_DATA_H__

//includes????  namespace????

#include <map>
#include "../math/simplex_tree.h"

class CellPersistenceData
{
	public:
        CellPersistenceData(int v);	//constructor
		
        void set_x(double t);	//set the theta coordinate
        double get_x();		//get the theta coordinate
		
        void set_y(double y);	//set the theta coordinate
        double get_y();		//get the theta coordinate
		
        void compute_data(std::vector<std::pair<unsigned, unsigned> > & xi, SimplexTree* bifiltration, int dim, const std::vector<double>& x_grades, const std::vector<double>& y_grades);
			//computes the persistence data, requires all support points of xi_0 and xi_1, the bifiltration, and the dimension of homology
		
		std::vector<int>* get_xi_global();		//returns vector of global indexes of xi support points
		std::vector< std::pair<int,int> >* get_pairs();	//returns a vector of persistence pairs (with respect to order xi indexes)
		std::vector<int>* get_cycles();			//returns a vector of essential cycles (with respect to order xi indexes)
		
	private:
        double x;		//x-coordinate of line along which this persistence data is computed
        double y;		//y-coordinate of line along which this persistence data is computed
		
		const int verbosity;			//controls display of output, for debugging
		static const double HALF_PI = 1.570796327;
		
		std::vector<int> xi_global;	//stores global indexes of xi support points, ordered by projection onto the line determined by theta and r
			//this is a map: order_xi_support_point_index -> global_xi_support_point_index
			//IDEA: a redesign could eliminate this, storing global (instead of local) xi support point indices in the following structure
		
		std::vector< std::pair<int,int> > persistence_pairs;	//stores persistence pairs (each pair will be mapped to a point in the persistence diagram)
			//entry (i,j) means that a bar is born at projection of xi support point with order index i and dies at projection of xi support point with order index j
		
		std::vector<int> essential_cycles;	//stores essential cycles (i.e. persistence "pairs" that never die)
			//entry i means that an essential cycle is born at the projection of xi support point with order index i
		
		std::pair<bool, double> project(double x, double y);	//computes the 1-D coordinate of the projection of a point (x,y) onto the line
			//returns a pair: first value is true if there is a projection, false otherwise; second value contains projection coordinate
};


#endif // __DCEL_CELL_PERSISTENCE_DATA_H__
