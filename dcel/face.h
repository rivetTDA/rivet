/**
 * \class	Face
 * \brief	Stores 2-dimensional cell in the DCEL decomposition of the affine Grassmannian.
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __DCEL_Face_H__
#define __DCEL_Face_H__

//includes????  namespace????
class Halfedge;

class Face
{
	public:
		Face(Halfedge* e);	//constructor, requires pointer to a boundary halfedge
		
		void set_boundary(Halfedge* e);	//set the pointer to a halfedge on the boundary of this face
		Halfedge* get_boundary();	//get the (pointer to the) boundary halfedge
		
		
		//Persistence... get_diagram();	//need ability to return the persistence diagram associated with this face
		
		friend std::ostream& operator<<(std::ostream& os, const Face& f);	//for printing the face
		
	private:
		Halfedge* boundary;	//pointer to one halfedge in the boundary of this cell
		//Persistence Diagram....WHAT TYPE IS THIS???
	
};//end class Face


////////// implementation //////////

Face::Face(Halfedge* e)
{
	boundary = e;
}

void Face::set_boundary(Halfedge* e)
{
	boundary = e;
}

Halfedge* Face::get_boundary()
{
	return boundary;
}

std::ostream& operator<<(std::ostream& os, const Face& f)
{
	Halfedge* start = f.boundary;
	Halfedge* curr = start;
	do{
		os << *(curr->get_origin()) << "--";
		curr = curr->get_next();
	}while(curr != start);
	os << "cycle";
	return os;
}


#endif // __DCEL_Face_H__

