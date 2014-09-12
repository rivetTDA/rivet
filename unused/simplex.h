/**
 * \class	Simplex
 * \brief	Stores the dimension of a simplex and indexes of its vertices.
 * \author	Matthew L. Wright
 * \date	January 2014
 * 
 * The Simplex class stores an abstract simplex, consisting of a dimension and an array of indexes of its vertices.
 * This class does not store any geometric data, for a vertex index is simply a number and has no spatial coordinates.
 * 
 * \todo	Vertex indexes should probably be ints, not doubles.
 */


/* Simplex class
 * stores dimension of simplex and indices of its vertices
 */


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


