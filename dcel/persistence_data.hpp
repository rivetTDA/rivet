/**
 * \class	PersistenceData
 * \brief	Stores the persistence data associated with each face in the DCEL
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __DCEL_PERSISTENCE_DATA_H__
#define __DCEL_PERSISTENCE_DATA_H__

//includes????  namespace????
#include <map>

class PersistenceData
{
	public:
		PersistenceData(int v);	//constructor
		
		void set_theta(double t);	//set the theta coordinate
		double get_theta();		//get the theta coordinate
		
		void set_r(double x);	//set the theta coordinate
		double get_r();		//get the theta coordinate
		
		void compute_data(std::vector<std::pair<int, int> > & xi, SimplexTree* bifiltration, int dim);
			//computes the persistence data (IN PROGRESS), requires all support points of xi_0 and xi_1, the bifiltration, and the dimension of homology
		
	private:
		double theta;		//theta-coordinate of line along which this persistence data is computed
		double r;		//r-coordinate of line along which this persistence data is computed
		
		std::vector<int> xi_global;	//stores global indexes of xi support points, ordered by projection onto the line determined by theta and r
		
		//NEED: data structure to hold persistence pairs...
		
		
		std::pair<bool, double> project(double x, double y);	//computes the 1-D coordinate of the projection of a point (x,y) onto the line
				//returns a pair: first value is true if there is a projection, false otherwise; second value contains projection coordinate

		
		const int verbosity;			//controls display of output, for debugging
		
		static const double HALF_PI = 1.570796327;
};


////////// implementation //////////

PersistenceData::PersistenceData(int v) : verbosity(v)
{ }

void PersistenceData::set_theta(double t)
{
	theta = t;
	
	if(verbosity >= 6) { std::cout << "    --theta set to " << theta << "\n"; }
}

double PersistenceData::get_theta()
{
	return theta;
}

void PersistenceData::set_r(double x)
{
	r = x;
	
	if(verbosity >= 6) { std::cout << "    --r set to " << r << "\n"; }
}

double PersistenceData::get_r()
{
	return r;
}

//struct used in the following function, PersistenceData::compute_data()
struct SimplexProjData
{
	int gid;	//global simplex index
	int sid;	//index of corresponding xi support point, with respect to the projection
	int dim;	//dimension of simplex
	
	SimplexProjData(int g, int s, int d) : gid(g), sid(s), dim(d) { }
}

//computes the persistence data, requires all support points of xi_0 and xi_1, and the bifiltration
void PersistenceData::compute_data(std::vector<std::pair<int, int> > & xi, SimplexTree* bifiltration, int dim)
{
	//map to hold ordered list of projections (key = projection_coordinate, value = global_xi_support_point_index; i.e. map: projection_coord -> global_xi_support_point_index)
	std::map<double,int> xi_proj;
	
	//loop through all support points, compute projections and store the unique ones
	for(int i=0; i<xi.size(); i++)
	{
		std::pair<bool,double> projection = project(xi[i].first, xi[i].second);
		
		if(projection.first == true)	//then the projection exists
		{
			xi_proj.insert( std::pair<double,int>(projection.second, i) );
			
			if(verbosity >= 8) { std::cout << "    ----support point " << i << " (" << xi[i].first << ", " << xi[i].second << ") projected to coordinate " << projection.second << "\n"; }
		}
	}
	
	//build two lists/maps of xi support points
	//  vector xi_global is stored permanently in this data structure, and is used as a map: order_xi_support_point_index -> global_xi_support_point_index
	//  map xi_order is used temporarily in this function, as a map: global_xi_support_point_index -> order_xi_support_point_index
	std::map<int,int> xi_order;
	int i=0;
	for (std::map<double,int>::iterator it=xi_proj.begin(); it!=xi_proj.end(); ++it)
	{
		xi_global.push_back( it->second );
		xi_order.insert( std::pair<int,int>( it->second, i ) );
		i++;
	}
		
	if(verbosity >= 6)
	{
		std::cout << "    --Ordering of xi support points for this cell: ";
		for (std::vector<int>::iterator it=xi_global.begin(); it!=xi_global.end(); ++it)
			std::cout << *it << ", ";
		std::cout << "\n";
	}
	
	//get simplex projection data
	
		///////////////// TODO: FIX THE FOLLOWING; ONLY NEED SIMPLICES OF DIMENSION d AND d+1
		
		//IDEA: iterate over all d-simplices in the SimplexTree --- how hard would this be?
	
			std::vector<SimplexProjData> simplex_proj;
			simplex_proj.reserve(bifiltration->get_num_simplices());	//this will avoid resize operations later

			//simplex ordering: first produce partial order based only on projections onto the line
			//loop through all simplices in bifiltration
			for(int i=0; i<bifiltration->get_num_simplices(); i++)
			{
				//get multi-index and dimension of simplex
				SimplexData sdata = bifiltration->get_simplex_data(i);	//TODO: should we cache this data somewhere to increase speed?
	
				if(verbosity >= 8) { std::cout << "    --Simplex " << i << " has multi-index (" << sdata.time << ", " << sdata.dist << "), dimension " << sdata.dim; }
	
				//project multi-index onto the line
				std::pair<bool, double> p_pair = project(sdata.time, sdata.dist);
	
				if(p_pair.first == true)	//then projection exists
				{
					std::map<double,int>::iterator xi_it = xi_proj.lower_bound(p_pair.second);	//finds the index of the closest support point projection greater than or equal to the multi-index projection
		
					if(verbosity >= 8) { std::cout << "; projected to " << p_pair.second << ", support point " << xi_it->second << "\n"; }
		
					simplex_proj.push_back( SimplexProjData( i, xi_it->second, sdata.dim ) );
				}
			}		
			//now we have a partial order on simplices, consisting of triples (global simplex index, index of support-point projection, dimension)

			//simplex ordering: convert partial order to a total order by ordering simplices that project to the same support point
			std::vector<int> simplex_order;
	
	
	//WORKING HERE...
	
	
	
	//now we have a total order of d-simplices, and a total order of (d+1)-simplices, each with respect to the projection onto the line
	
	
	//WORKING HERE...
	
	
	
	
	//create boundary matrix for homology of dimension d
		// for (d+1)-simplices, we need a map: order_simplex_index -> global_simplex_index
		// for d-simplices, we need a map: global_simplex_index -> order_simplex_index
		
	//reduce boundary matrix via Edelsbrunner column algorithm
	
	//identify and store persistence pairs
		// for (d+1)-simplices, we need a map: order_simplex_index -> order_xi_support_point_index
		// similarly, for d-simplices, need map: order_simplex_index -> order_xi_support_point_index
	
	//persistence pair consists of the two xi support point indexes
	//each essential cycle is represented by an un-paired xi support-point index (corresponding to a d-simplex)
	
	
	
}//end compute_data()

//computes the 1-D coordinate of the projection of a point (x,y) onto the line
//returns a pair: first value is true if there is a projection, false otherwise; second value contains projection coordinate
//TODO: possible optimization: when computing projections initially, the horizontal and vertical cases should not occur (since an interior point in each cell cannot be on the boundary)
std::pair<bool, double> PersistenceData::project(double x, double y)
{
	double p = 0;	//if there is a projection, then this will be set to its coordinate
	bool b = true;	//if there is no projection, then this will be set to false
	
	if(theta == 0)	//horizontal line
	{
		if(y <= r)	//then point is below line
			p = x;
		else	//no projection
			b = false;
	}
	else if(theta < HALF_PI)	//line is neither horizontal nor vertical
	//TODO: this part is correct up to a linear transfomation; is it good enough?
	{
		if(y > x*tan(theta) + r/cos(theta))	//then point is above line
			p = y; //project right
		else
			p = x*tan(theta) + r/cos(theta); //project up
	}
	else	//vertical line
	{
		if(x <= r)
			p = y;
		else	//no projection
			b = false;
	}
	
	return std::pair<bool, double>(b,p);
}//end project()


#endif // __DCEL_PERSISTENCE_DATA_H__

