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
		LCM(double t, double d);		//constructor, requires only time and distance values
		LCM(double t, double d, Halfedge* e);	//constructor, requires all three parameters
		LCM(const LCM& other);			//copy constructor
		
		LCM& operator= (const LCM& other);	//assignment operator
		bool operator== (const LCM& other) const;	//equality operator
		
		double get_time() const;		//get the time-coordinate of the multi-index
		double get_dist() const;		//get the distance-coordinate of the multi-index
		
		void set_curve(Halfedge* e);	//set the pointer to the curve corresponding to this LCM in the arrangement
		Halfedge* get_curve() const;		//get the pointer to the curve corresponding to this LCM in the arrangement
		
	private:
		double time;		//time-coordinate of multi-index
		double dist;		//distance-coordinate of multi-inded
		Halfedge* curve;	//pointer to left-most halfedge corresponding to this LCM in the arrangement
		
};


////////// implementation //////////

LCM::LCM(double t, double d) : time(t), dist(d), curve(NULL)
{ }

LCM::LCM(double t, double d, Halfedge* e) : time(t), dist(d), curve(e)
{ }

LCM::LCM(const LCM& other)
{
	time = other.time;
	dist = other.dist;
	curve = other.curve;
}

LCM& LCM::operator= (const LCM& other)
{
	//check for self-assignment
	if(this == &other)
		return *this;
	
	//do the copy
	time = other.time;
	dist = other.dist;
	curve = other.curve;
	
	return *this;
}

bool LCM::operator== (const LCM& other) const
{
	return (time == other.time && dist == other.dist);
}

double LCM::get_time() const
{
	return time;
}

double LCM::get_dist() const
{
	return dist;
}

void LCM::set_curve(Halfedge* e)
{
	curve = e;
}

Halfedge* LCM::get_curve() const
{
	return curve;
}


#endif // __DCEL_LCM_H__

