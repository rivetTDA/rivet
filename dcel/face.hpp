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
class PersistenceData;

class Face
{
	public:
		Face(Halfedge* e);	//constructor, requires pointer to a boundary halfedge
		
		void set_boundary(Halfedge* e);	//set the pointer to a halfedge on the boundary of this face
		Halfedge* get_boundary();	//get the (pointer to the) boundary halfedge
		
		void set_data(PersistenceData* pd);	//set the persistence data associated with this face
		PersistenceData* get_data();		//returns the persistence data associated with this face
		
		friend std::ostream& operator<<(std::ostream& os, const Face& f);	//for printing the face
		
	private:
		Halfedge* boundary;	//pointer to one halfedge in the boundary of this cell
		PersistenceData* pdata;	//pointer to the persistence data associated with this face
	
};//end class Face


////////// implementation //////////

Face::Face(Halfedge* e) : boundary(e), pdata(NULL)
{ }

void Face::set_boundary(Halfedge* e)
{
	boundary = e;
}

Halfedge* Face::get_boundary()
{
	return boundary;
}

void Face::set_data(PersistenceData* pd)
{
	pdata = pd;
}

PersistenceData* Face::get_data()
{
	return pdata;
}

std::ostream& operator<<(std::ostream& os, const Face& f)
{
	Halfedge* start = f.boundary;
	Halfedge* curr = start;
	do{
		os << *(curr->get_origin()) << "--";
		curr = curr->get_next();
	}while(curr != start);
	os << "cycle; ";
	if(f.pdata == NULL)
		os << "NULL data";
	else
		os << "non-null data";
	return os;
}


#endif // __DCEL_Face_H__

