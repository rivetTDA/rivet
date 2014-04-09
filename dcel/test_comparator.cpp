// program to test the LCM comparator

#include <iostream>
#include <set>
#include "lcm.hpp"
#include "lcm_left_comparator.hpp"
#include "lcm_angle_comparator.hpp"

int main(int argc, char* argv[])
{
	std::cout << "TESTING LEFT COMPARATOR:\n";
	
	std::set<LCM*, LCM_LeftComparator> myset;
	
	std::cout << "  Creating LCMs:\n";
	
	LCM* lcm45 = new LCM(4,5);
	std::cout << "    LCM(4,5) stored at address " << lcm45 << "\n";
	myset.insert( lcm45 );
	
	LCM* lcm13 = new LCM(1,3);
	std::cout << "    LCM(1,3) stored at address " << lcm13 << "\n";
	myset.insert( lcm13 );
	
	LCM* lcm23 = new LCM(2,3);
	std::cout << "    LCM(2,3) stored at address " << lcm23 << "\n";
	myset.insert( lcm23 );
	
	LCM* lcm43 = new LCM(4,3);
	std::cout << "    LCM(4,3) stored at address " << lcm43 << "\n";
	myset.insert( lcm43 );
	
	LCM* lcm25 = new LCM(2,5);
	std::cout << "    LCM(2,5) stored at address " << lcm25 << "\n";
	myset.insert( lcm25 );
	
	LCM* lcm65 = new LCM(6,5);
	std::cout << "    LCM(6,5) stored at address " << lcm65 << "\n";
	myset.insert( lcm65 );
	
	LCM* lcm44 = new LCM(4,4);
	std::cout << "    LCM(4,4) stored at address " << lcm44 << "\n";
	myset.insert( lcm44 );
	
	
	std::cout << "  Contents of the set:\n";
	std::set<LCM*>::iterator it;
	for(it = myset.begin(); it != myset.end(); ++it)
	{
		LCM* cur = *it;
		std::cout << "    LCM(" << cur->get_time() << "," << cur->get_dist() << ") stored at address " << cur << "; test: " << (**it).get_time() << "\n";
	}
	
	
	
	std::cout << "TESTING ANGLE COMPARATOR:\n";
	
	LCM* reference = new LCM(3,3);
	std::cout << "  reference LCM: (" << reference->get_time() << ", " << reference->get_dist() << ")\n";
	
	std::set<LCM*, LCM_AngleComparator> newset(reference);
	
	LCM* lcm11 = new LCM(1,1);
	LCM* lcm12 = new LCM(1,2);
	LCM* lcm21 = new LCM(2,1);
	LCM* lcm35 = new LCM(3,5);
	LCM* lcm40 = new LCM(4,0);
	LCM* lcm46 = new LCM(4,6);
	
	newset.insert( lcm45 );
	newset.insert( lcm11 );
	newset.insert( lcm12 );
	newset.insert( lcm21 );
	newset.insert( lcm43 );
	newset.insert( lcm25 );
	newset.insert( lcm35 );
	newset.insert( lcm65 );
	newset.insert( lcm44 );
	newset.insert( lcm40 );
	newset.insert( lcm46 );
	
	std::set<LCM*>::iterator it2;
	for(it2 = newset.begin(); it2 != newset.end(); ++it2)
	{
		LCM* cur = *it2;
		
		std::cout << "    LCM(" << cur->get_time() << "," << cur->get_dist() << ") stored at address " << cur << "\n";
	}
	
	std::cout << "Done.\n\n";
}
