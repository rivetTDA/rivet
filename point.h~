/* point class
 * stores an n-tuple with a time of appearance
 */

#include <iostream>

class Point {
	public:
		Point(double*, double);		//constructor
		double* get_coords();		//returns the n-tuple of coordinates
		double get_birth();		//returns the time that this point appears
		void print();	//TESTING
		
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
