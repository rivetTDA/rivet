/**
 * \class	Face 
 * \brief	Stores 2-dimensional cell in the DCEL decomposition of the affine Grassmannian.
 * \author	Matthew L. Wright
 * \date	March 2014
 */

#ifndef __DCEL_Face_H__
#define __DCEL_Face_H__

//includes????  namespace????
#include "persistence_data.hpp"

class Halfedge;
class PersistenceData;

class Face
{
	public:
		Face(Halfedge* e, int v);	//constructor, requires pointer to a boundary halfedge and verbosity value
		
		void set_boundary(Halfedge* e);	//set the pointer to a halfedge on the boundary of this face
		Halfedge* get_boundary();	//get the (pointer to the) boundary halfedge
		
	//UNNECESSARY:	void set_data(PersistenceData* pd);	//set the persistence data associated with this face
		PersistenceData* get_data();		//returns the persistence data associated with this face
		
		void store_interior_point();		//computes coordinates of a point inside this face and stores it in the persistence data object
		
		friend std::ostream& operator<<(std::ostream& os, const Face& f);	//for printing the face
		
	private:
		Halfedge* boundary;	//pointer to one halfedge in the boundary of this cell
		PersistenceData pdata;	//pointer to the persistence data associated with this face
	
};//end class Face


////////// implementation //////////

Face::Face(Halfedge* e, int v) : boundary(e), pdata(v)
{ }

void Face::set_boundary(Halfedge* e)
{
	boundary = e;
}

Halfedge* Face::get_boundary()
{
	return boundary;
}

//void Face::set_data(PersistenceData* pd)
//{
//	pdata = pd;
//}

PersistenceData* Face::get_data()
{
	return &pdata;
}

void Face::store_interior_point()
{
	//find min and max theta coordinates of vertices adjacent to this face
	double min_theta = boundary->get_origin()->get_theta();
	double max_theta = min_theta;
	
	Halfedge* current = boundary->get_next();
	
	while(current != boundary)
	{
		double current_theta = current->get_origin()->get_theta();
		
		if(current_theta < min_theta)
			min_theta = current_theta;
		
		if(current_theta > max_theta)
			max_theta = current_theta;
		
		current = current->get_next();
	}
	
	//find theta coordinate between min and max theta
	double mid_theta = (min_theta + max_theta)/2;
	
	//find top and bottom edges that cross the vertical line theta=mid_theta
	Halfedge* top_edge;
	Halfedge* bottom_edge;
	
	current = boundary;
	do{
		double origin = current->get_origin()->get_theta();
		double end = current->get_next()->get_origin()->get_theta();
		
		if(origin <= mid_theta && end > mid_theta)
			top_edge = current;
		
		if(origin >= mid_theta && end < mid_theta)
			bottom_edge = current;
		
		current = current->get_next();
	}while(current != boundary);
	
	//find midpoint along vertical line theta=mid_theta
	double mid_r = 0;
	
	//handle boundary cases
	if(top_edge->get_LCM() == NULL)	//then top edge is at r=infinity
	{
		if(bottom_edge->get_LCM() == NULL)	//then bottom edge is also at r=infinity
		{
			mid_r = 0;
		}
		else
		{
			mid_r = bottom_edge->get_LCM()->get_r_coord(mid_theta) + 1;
		}
	}
	else	//then top edge is not at r=infinity
	{
		if(bottom_edge->get_LCM() == NULL)	//then bottom edge is at r=infinity
		{
			mid_r = top_edge->get_LCM()->get_r_coord(mid_theta) - 1;
		}
		else
		{
			double top = top_edge->get_LCM()->get_r_coord(mid_theta);
			double bottom = bottom_edge->get_LCM()->get_r_coord(mid_theta);
			mid_r = (top + bottom)/2;
		}
		
	}
	
	//store the coordinates
	pdata.set_theta(mid_theta);
	pdata.set_r(mid_r);
	
}//end store_interior_point()



std::ostream& operator<<(std::ostream& os, const Face& f)
{
	Halfedge* start = f.boundary;
	Halfedge* curr = start;
	do{
		os << *(curr->get_origin()) << "--";
		curr = curr->get_next();
	}while(curr != start);
	os << "cycle; ";
	return os;
}


#endif // __DCEL_Face_H__

