/* Edge class
 * stores indices of two points, with the distance between them and time of appearance
 */

#include <iostream>

class Edge {
	public:
		Edge();					//constructor
		Edge(int, int , double, double);		//constructor
		void set_all(int, int, double, double);	//sets all attributes of this edge
		int get_p1();			//returns index of endpoint
		int get_p2();			//returns index of other endpoint
		double get_length();	//returns length of edge
		double get_birth();		//returns the time that this edge appears
		
	private:
		int p1, p2;			//indices of points (endpoints of this edge)
		double length;		//length of this edge
		double birth;		//time that this edge appears = latest birth time of endpoints
};

Edge::Edge()
{ 
	
}

Edge::Edge(int a, int b, double len, double bir)
{
	p1 = a;
	p2 = b;
	length = len;
	birth = bir;
}

void Edge::set_all(int a, int b, double len, double bir)
{
	p1 = a;
	p2 = b;
	length = len;
	birth = bir;
}

int Edge::get_p1()
{
	return p1;
}

int Edge::get_p2()
{
	return p2;
}

double Edge::get_length()
{
	return length;
}

double Edge::get_birth()
{
	return birth;
}

