/**
 * \class	LCM
 * \brief	Stores a LCM: a multi-index pair along with a pointer to the curve representing the LCM in the arrangement
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __DCEL_LCM_H__
#define __DCEL_LCM_H__

//includes????  namespace????
class Halfedge;

class LCM
{
	public:
		LCM(double t, double d);	//constructor, requires time and distance values
		
		double get_time();		//get the time-coordinate of the multi-index
		double get_dist();		//get the distance-coordinate of the multi-index
		
		void set_curve(Halfedge* e);	//set the pointer to the curve corresponding to this LCM in the arrangement
		Halfedge* get_curve();		//get the pointer to the curve corresponding to this LCM in the arrangement
		
	private:
		double time;		//time-coordinate of multi-index
		double dist;		//distance-coordinate of multi-inded
		Halfedge* curve;	//pointer to left-most halfedge corresponding to this LCM in the arrangement
		
};


////////// implementation //////////

LCM::LCM(double t, double d)
{
	time = t;
	dist = d;
}

double LCM::get_time()
{
	return time;
}

double LCM::get_dist()
{
	return dist;
}

void LCM::set_curve(Halfedge* e)
{
	curve = e;
}

Halfedge* LCM::get_curve()
{
	return curve;
}


#endif // __DCEL_LCM_H__

