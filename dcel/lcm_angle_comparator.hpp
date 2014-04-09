/**
 * \class	LCM_AngleComparator
 * \brief	Compares LCMS to establish ordering based on slopes to a reference LCM.
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __DCEL_LCM_ANGLE_COMPARATOR_H__
#define __DCEL_LCM_ANGLE_COMPARATOR_H__

//includes????  namespace????
#include "lcm.hpp"

class LCM_AngleComparator
{
	public:
		LCM_AngleComparator(const LCM* ref) : 		//constructor, requires a reference LCM
			time(ref->get_time()), 
			dist(ref->get_dist()) 
		{ };
		
		bool operator() (const LCM* lhs, const LCM* rhs) const
		{
			double x1 = lhs->get_time() - time;
			double y1 = lhs->get_dist() - dist;
			
			double x2 = rhs->get_time() - time;
			double y2 = rhs->get_dist() - dist;
			
			if(x1 == 0)	//then lhs corresponds to vertical line
			{
				return false;
			}
			if(x2 == 0)	//then rhs corresponds to vertical line (and lhs does not)
			{
				return true;
			}
			//if we get here, then neither side corresponds to a vertical line, so it is safe to divide by x1 and x2
			bool result = ( (y1/x1) < (y2/x2) );
			return result;
		}
	
	private:
		double time;
		double dist;
};

#endif // __DCEL_LCM_ANGLE_COMPARATOR_H__

