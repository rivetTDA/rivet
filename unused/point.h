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

//THIS CLASS IS NO LONGER USED!!!

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


#endif // __Point_H__


