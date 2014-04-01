// program to test the LCM comparator

#include <iostream>
#include <set>
#include "lcm.hpp"
#include "lcm_left_comparator.hpp"
#include "lcm_angle_comparator.hpp"

int main(int argc, char* argv[])
{
	std::cout << "TESTING LEFT COMPARATOR:\n";
	
	std::set<LCM, LCM_LeftComparator> myset;
	
	myset.insert( LCM(4,5) );
	myset.insert( LCM(1,3) );
	myset.insert( LCM(2,3) );
	myset.insert( LCM(4,3) );
	myset.insert( LCM(2,5) );
	myset.insert( LCM(6,5) );
	myset.insert( LCM(4,4) );
	
	std::set<LCM>::iterator it;
	for(it = myset.begin(); it != myset.end(); ++it)
	{
		LCM cur = *it;
		
		std::cout << "    (" << cur.get_time() << ", " << cur.get_dist() << ")\n";
	}
	
	
	std::cout << "TESTING ANGLE COMPARATOR:\n";
	
	LCM reference = LCM(3,3);
	std::cout << "  reference LCM: (" << reference.get_time() << ", " << reference.get_dist() << ")\n";
	
	std::set<LCM, LCM_AngleComparator> newset(reference);
	
	
	newset.insert( LCM(4,5) );
	newset.insert( LCM(1,1) );
	newset.insert( LCM(1,2) );
	newset.insert( LCM(2,1) );
	newset.insert( LCM(4,3) );
	newset.insert( LCM(2,5) );
	newset.insert( LCM(3,5) );
	newset.insert( LCM(6,5) );
	newset.insert( LCM(4,4) );
	newset.insert( LCM(4,0) );
	newset.insert( LCM(4,6) );
	
	for(it = newset.begin(); it != newset.end(); ++it)
	{
		LCM cur = *it;
		
		std::cout << "    (" << cur.get_time() << ", " << cur.get_dist() << ")\n";
	}
	
	std::cout << "Done.\n\n";
}
