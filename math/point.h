/**
 * \class	Point
 * \brief	Stores an n-tuple with a time of appearance.
 * \author	Matthew L. Wright
 * \date	January 2014
 * \warning	This class does not store the dimension of the point.
 * 
 * The Point class stores the coordinates of a geometric point (as a double array) along with its birth time (a double).
 * 
 * \todo	Only the less-than operator is overloaded; other comparison operators should also be overloaded.
 */


#ifndef __Point_H__
#define __Point_H__

#include <iostream>

class Point {
	public:
		Point(double*, double);		//constructor
		double* get_coords();		//returns the n-tuple of coordinates
		double get_birth();		//returns the time that this point appears
		void print();	//TESTING
		bool operator < (const Point &) const;	//comparison operator
		
	private:
		double* coords;	//stores the n-tuple of coordinates
		double birth;		//stores the time that this point appears
		
};


Point::Point(double* cor, double bir)
{
	//std::cout << "in constructor, cor: " << cor[0] << ", " << cor[1] << "\n";
	coords = cor;
	//std::cout << "in constructor, coords: " << coords[0] << ", " << coords[1] << "\n";
	birth = bir;
	
}

double* Point::get_coords()
{
	return coords;
}

double Point::get_birth()
{
	return birth;
}

void Point::print()
{
	std::cout << "  Point.print(): " << coords[0] << ", " << coords[1] << "; " << birth << "; coords-address: " << coords << "\n ";
}

//overloaded less than operator, for sorting a list of Points
bool Point::operator < (const Point & other) const
{
	if(birth < other.birth)
		return true;
	/* else */
	return false;
}

//SHOULD OVERLOAD OTHER COMPARISON OPERATORS TOO!!!

#endif // __Point_H__


