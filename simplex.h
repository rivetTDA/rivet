/* Simplex class
 * stores dimension of simplex and indices of its vertices
 */

#include <iostream>

class Simplex {
	public:
		Simplex();					//constructor
		Simplex(int, double*);		//constructor
		void set_all(int, double*);	//sets all attributes of this simplex
		int get_dim();				//returns dimension of this simplex
		double* get_vertices();		//returns array of vertices of this simplex (each vertex is an index of a point)
		
	private:
		int dim;				//dimension
		double* vertices;		//vertices
};

Simplex::Simplex()
{ 
	
}

Simplex::Simplex(int d, double* v)
{
	dim = d;
	vertices = v;
}

void Simplex::set_all(int d, double* v)
{
	dim = d;
	vertices = v;
}

int Simplex::get_dim()
{
	return dim;
}

double* Simplex::get_vertices()
{
	return vertices;
}


