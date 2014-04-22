/**
 * \class	PersistenceData
 * \brief	Stores the persistence data associated with each face in the DCEL
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __DCEL_PERSISTENCE_DATA_H__
#define __DCEL_PERSISTENCE_DATA_H__

//includes????  namespace????

class PersistenceData
{
	public:
		PersistenceData(int v);	//constructor
		
		void set_theta(double t);	//set the theta coordinate
		double get_theta();		//get the theta coordinate
		
		void set_r(double x);	//set the theta coordinate
		double get_r();		//get the theta coordinate
		
		void compute_data(std::vector<std::pair<int, int> > & xi, SimplexTree* bifiltration);
			//computes the persistence data (IN PROGRESS), requires all support points of xi_0 and xi_1, and the bifiltration
		
	private:
		double theta;		//theta-coordinate of line along which this persistence data is computed
		double r;		//r-coordinate of line along which this persistence data is computed
		
		std::pair<bool, int> project(double x, double y)	//computes the 1-D coordinate of the projection of a point (x,y) onto the line
				//returns a pair: first value is true if there is a projection, false otherwise; second value contains projection coordinate

		
		const int verbosity;			//controls display of output, for debugging
		
		static const double HALF_PI = 1.570796327;
};


////////// implementation //////////

PersistenceData::PersistenceData(int v) : verbosity(v)
{ }

void PersistenceData::set_theta(double t)
{
	if(verbosity >= 6) { std::cout << "    --theta set to " << theta << "\n"; }
	
	theta = t;
}

double PersistenceData::get_theta()
{
	return theta;
}

void PersistenceData::set_r(double x)
{
	if(verbosity >= 6) { std::cout << "    --r set to " << theta << "\n"; }
	
	r = x;
}

double PersistenceData::get_r()
{
	return r;
}

//computes the persistence data, requires all support points of xi_0 and xi_1, and the bifiltration
void PersistenceData::compute_data(std::vector<std::pair<int, int> > & xi, SimplexTree* bifiltration)
{
	if(verbosity >=2) { std::cout << "    Computing persistence data (in PersistenceData class)...\n"; }
	
	//loop through all support points
	for(int i=0; i<xi.size(); i++)
	{
		///////////FINISH THIS!!!
		double p = project(xi[i].first, xi[i].second);
		
		///////////do something with the pair (i,p)!!!
	}
	
	
	
	
	
	
	
}//end compute_data()

//computes the 1-D coordinate of the projection of a point (x,y) onto the line
//returns a pair: first value is true if there is a projection, false otherwise; second value contains projection coordinate
std::pair<bool, int> PersistenceData::project(double x, double y)
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
	
	return std::pair<bool, int>(b,p);
}//end project()




#endif // __DCEL_PERSISTENCE_DATA_H__

