/**
 * \class	LCM_LeftComparator
 * \brief	Compares LCMS to establish ordering on left edge (theta = 0) of affine Grassmannian strip
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __DCEL_LCM_LEFT_COMPARATOR_H__
#define __DCEL_LCM_LEFT_COMPARATOR_H__

//includes????  namespace????
#include "lcm.hpp"

class LCM_LeftComparator
{
	public:
		bool operator() (const LCM* lhs, const LCM* rhs) const
		{
			if(lhs->get_dist() < rhs->get_dist())		//first compare distance value (natural order)
				return true;
			if(lhs->get_dist() == rhs->get_dist() && lhs->get_time() > rhs->get_time())		//then compare time value (reverse order!)
				return true;
			return false;
		}
};

#endif // __DCEL_LCM_LEFT_COMPARATOR_H__

