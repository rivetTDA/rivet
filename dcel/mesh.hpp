/* implementation of Mesh class
 * Stores and manipulates the DCEL decomposition of the affine Grassmannian.
 */
 
//constructor; sets up bounding box (with empty interior) for the affine Grassmannian
Mesh::Mesh(int v) : 
	INFTY(std::numeric_limits<double>::infinity()),
	verbosity(v)
{
	//create vertices
	vertices.push_back( new Vertex(0, INFTY) );		//index 0
	vertices.push_back( new Vertex(HALF_PI, INFTY) );	//index 1
	vertices.push_back( new Vertex(HALF_PI, -INFTY) );	//index 2
	vertices.push_back( new Vertex(0, -INFTY) );		//index 3
	
	//create halfedges
	for(int i=0; i<4; i++)
	{
		halfedges.push_back( new Halfedge( vertices[i], NULL) );		//index 0, 2, 4, 6 (inside halfedges)
		halfedges.push_back( new Halfedge( vertices[(i+1)%4], NULL) );		//index 1, 3, 5, 7 (outside halfedges)
		halfedges[2*i]->set_twin( halfedges[2*i+1] );
		halfedges[2*i+1]->set_twin( halfedges[2*i] );
	}
	
	topleft = halfedges[7];	//remember this halfedge to make curve insertion easier
	
	//set pointers on vertices
	for(int i=0; i<4; i++)
	{
		vertices[i]->set_incident_edge( halfedges[2*i] );
	}
	
	//create face
	faces.push_back( new Face( halfedges[0] ) );
	
	//set the remaining pointers on the halfedges
	for(int i=0; i<4; i++)
	{
		Halfedge* inside = halfedges[2*i];
		inside->set_next( halfedges[(2*i+2)%8] );
		inside->set_prev( halfedges[(2*i+6)%8] );
		inside->set_face( faces[0] );
		
		Halfedge* outside = halfedges[2*i+1];
		outside->set_next( halfedges[(2*i+7)%8] );
		outside->set_prev( halfedges[(2*i+3)%8] );
	}
}//end constructor

//destructor: IMPLEMENT THIS, MAKE SURE ALL MEMORY IS RELEASED!!!!
Mesh::~Mesh()
{
	
}//end destructor

//adds a curve representing LCM to the mesh
// TODO: make this better (I think it works now, but it is messy, and I am concerned about round-off error.)
// IDEA: use a modified version of the Bentley-Ottmann algorithm
void Mesh::add_curve(double time, double dist)
{
	//create LCM object 
	LCM* current = new LCM(time, dist);
	if(verbosity >= 2)
		std::cout << "    creating LCM: (" << current->get_time() << ", " << current->get_dist() << ")\n";
	
	//set the following to true once the pointer is updated
	bool lcm_points_to_curve = false;
	
	//create set of LCMs ordered by LCM_AngleComparator
	std::set<LCM*, LCM_AngleComparator> intersections(current);
	
	//find all comparable, previously-inserted LCMs, add them to the set ordered by LCM_AngleComparator
	std::set<LCM*>::iterator it;
	for(it = inserted_lcms.begin(); it != inserted_lcms.end(); ++it)
	{
		if( ( (**it).get_time() >= time && (**it).get_dist() >= dist ) || ( (**it).get_time() <= time && (**it).get_dist() <= dist ) )	//then the LCMs are comparable
		{
			if(verbosity >= 6)
				std::cout << "      comparable LCM: (" << (**it).get_time() << ", " << (**it).get_dist() << ")\n";
			
			intersections.insert(*it);
		}
	}
	
	if(verbosity >= 4)
	{
		std::cout << "    intersections: ";
		for(it = intersections.begin(); it != intersections.end(); ++it)
		{
			std::cout << "(" << (**it).get_time() << ", " << (**it).get_dist() << "), ";
		}
		std::cout << "\n";
	}
	
	
	//find where to insert left endpoint -- use the ordered LCM container, inserted_lcms
	std::set<LCM*>::iterator itleft;
	itleft = inserted_lcms.upper_bound(current);	//returns an iterator to the first LCM that is greater than "current"

	if(verbosity >= 2)
	{
		if(itleft == inserted_lcms.end())
			std::cout << "    LCM curve to be inserted after all existing LCM curves\n";
		else
			std::cout << "    LCM curve to be inserted before curve corresponding to LCM (" << (**itleft).get_time() << ", " << (**itleft).get_dist() << ")\n";
	}
	
	//halfedge pointers to use for constructing the new curve
	Halfedge* leftedge;
	Halfedge* rightedge;
	
	//find or create left endpoint for the new curve (on theta=0 edge of the strip)
	if(itleft == inserted_lcms.end()) //then create a new vertex above the highest existing vertex on this edge of the strip
	{
		leftedge = insert_vertex(topleft->get_twin(), 0, dist);
	}
	else if( (**itleft).get_dist() == dist)	//then use existing vertex
	{
		leftedge = (**itleft).get_curve();	//CHECK THIS!!!
	}
	else	//then create new vertex
	{
		if(verbosity >= 8) { std::cout << "    --inserting vertex (0," << dist << ") using reference halfedge " << HID((**itleft).get_curve()->get_prev()) << "\n"; }
		
		leftedge = insert_vertex( (**itleft).get_curve()->get_prev(), 0, dist);
	}
	
	if(verbosity >= 8) { std::cout << "    ----leftedge: " << HID(leftedge) << " (origin: " << *(leftedge->get_origin()) << ", twin: " << HID(leftedge->get_twin()) << ", next: " << HID(leftedge->get_next()) << ", prev: " << HID(leftedge->get_prev()) << "\n"; }
	
	//loop through all existing curves that this new curve crosses
	std::set<LCM*>::iterator itint;
	for(itint = intersections.begin(); itint != intersections.end(); ++itint)
	{
		if(verbosity >= 6)
			std::cout << "    intersection with curve corresponding to LCM (" << (**itint).get_time() << ", " << (**itint).get_dist() << ")\n";
		
		//skip intersections that happen at theta=0
		if( (**itint).get_dist() == dist)
		{
			if(verbosity >= 8) { std::cout << "     ---intersection skipped since theta=0\n"; }
			
			continue;
		}
		
		//find or create right endpoint for the current face
		Halfedge* finger = leftedge->get_next();
		bool found_rightedge = false;
		while( finger != leftedge && !found_rightedge )	//this loop should end when we find rightedge, not when we traverse the entire face!
		{
			if(verbosity >= 8) { std::cout << "    ----checking finger: " << HID(finger) << "\n"; }
			
			//is this the edge we are looking for?
			if( finger->get_LCM() == *itint )	//then next intersection is with the curve that finger points to
			{
				LCM* intersect_LCM = finger->get_LCM();	//LCM corresponding to curve of next intersection
				
				if(verbosity >= 8) { std::cout << "    ------found halfedge for LCM (" << intersect_LCM->get_time() << "," << intersect_LCM->get_dist() << ")\n"; }
				
				if(intersect_LCM->get_time() == time)	//then next interesction is at theta=pi/2 (which must be at terminal vertex of edge pointed to by finger)
				{
					if(verbosity >= 8) { std::cout << "    ----intersection at theta=pi/2\n"; }
					
					rightedge = finger->get_next();
					found_rightedge = true;	//found rightedge
				}
				else
				{
					double intersect_angle = atan( (intersect_LCM->get_dist() - dist)/(intersect_LCM->get_time() - time) );	//theta-coordinate of next intersection
					//Vertex* endpt = finger->get_twin()->get_origin();	//terminal vertex of edge pointed to by finger
					double begin_angle = finger->get_origin()->get_theta();	//theta-coordinate of initial vertex of edge pointed to by finger
					double end_angle = finger->get_twin()->get_origin()->get_theta();	//theta-coordinate of terminal vertex of edge pointed to by finger
				
					if(verbosity >= 8) { std::cout << "    ------comparing angles: " << begin_angle << ", " << intersect_angle << ", and " << end_angle << "\n"; }
				
					if( (begin_angle < intersect_angle && intersect_angle < end_angle) 
						|| (end_angle < intersect_angle && intersect_angle < begin_angle) )	//then intersection is along edge pointed to by finger, so we must create a new vertex
					{
						double intersect_r;
						if(time != 0)
							intersect_r = sqrt( time*time + dist*dist ) * sin( atan(dist/time) - intersect_angle );	//////TODO: CHECK THIS; HOW CAN WE ELIMINATE ROUND-OFF ERROR???
						else
							intersect_r = sqrt( time*time + dist*dist ) * cos( intersect_angle );
					
						if(verbosity >= 8) { std::cout << "    ----creating new vertex for intersection at (" << intersect_angle << ", " << intersect_r << ")\n"; }
					
						rightedge = insert_vertex(finger, intersect_angle, intersect_r);
						found_rightedge = true;	//found rightedge
					}
					else	//then intersection is at terminal vertex of edge pointed to by finger //Can intersection be at INITIAL vertex? NO, because we would have detected it in previous iteration of the loop, in the endpoint check below.
					{
						rightedge = finger->get_next();
					
						if(verbosity >= 8) { std::cout << "    ----intersection at existing vertex " << *(rightedge->get_origin()) << "\n"; }
					
						found_rightedge = true;	//found rightedge
					}
				}
			}
			else	//the intersection could still be at the endpoint
			{
				Halfedge* thumb = finger->get_next();
				
				if(verbosity >= 8) { std::cout << "    ------checking thumb: " << HID(thumb) << "\n"; }
				
				while( thumb->get_LCM() != finger->get_LCM() && !found_rightedge)
				{
					if( thumb->get_LCM() == *itint )	//then next intersection is with the curve that thumb points to
					{
						if(verbosity >= 8) { std::cout << "    --------found halfedge for LCM (" << (**itint).get_time() << "," << (**itint).get_dist() << ")\n"; }
						
						if( thumb != finger->get_next() )	//then intersection is at this vertex and not along the next edge
						{
							rightedge = finger->get_next();
							found_rightedge = true;
						}
						else	//then intersection is either at this vertex or along the next edge
						{
							LCM* intersect_LCM = thumb->get_LCM();	//LCM corresponding to curve of next intersection
							
							if(intersect_LCM->get_time() == time)	//then next interesction is at theta=pi/2 (which must be at terminal vertex of edge pointed to by finger)
							{
								if(verbosity >= 8) { std::cout << "    ------intersection at theta=pi/2\n"; }
					
								rightedge = finger->get_next();
								found_rightedge = true;	//found rightedge
							}
							else
							{
								double intersect_angle = atan( (intersect_LCM->get_dist() - dist)/(intersect_LCM->get_time() - time) );	//theta-coordinate of next intersection
							
								if( intersect_angle == thumb->get_origin()->get_theta() )	//then intersection is at the vertex
								{
									rightedge = thumb;
									found_rightedge = true;
								}
								//else, intersection is along the edge and will be handled by next iteration of the outer loop
							}
						}
						
					}
					
					//consider next edge CCW around vertex
					thumb = thumb->get_twin()->get_next();
				}
				
			}//end else
			
			if(!found_rightedge)	//still not found; consider to next edge around the curent face
			{
				finger = finger->get_next();
			}
			
		}//end while( finger != leftedge && !found_rightedge )
		
		if(verbosity >= 8) { std::cout << "    ----rightedge: " << HID(rightedge) << "; finger: " << HID(finger) << "\n"; }
		
		//insert a new edge to subdivide the face
		insert_edge(leftedge, rightedge, current);
		
		//update LCM curve pointer, if necessary
		if(!lcm_points_to_curve)
		{
			current->set_curve(rightedge->get_prev());
			lcm_points_to_curve = true;
		}
		
		//make sure leftedge is correct for the next step
		Halfedge* thumb = rightedge;
		while( thumb->get_LCM() != finger->get_LCM() )
		{
			if(verbosity >= 8) { std::cout << "    ----updating leftedge; thumb: " << HID(thumb) << "\n"; }
		
			thumb = thumb->get_twin()->get_next();
		}
		leftedge = thumb->get_twin()->get_next();
		
		if(verbosity >= 8) { std::cout << "    ----leftedge: " << HID(leftedge) << "; finger: " << HID(finger) << "; thumb: " << HID(thumb) << "\n"; }
		
	}//end for
	
	//if necessary, create right endpoint for the new curve (on theta=pi/2 edge of the strip) and insert a new edge
	if( leftedge->get_origin()->get_theta() < HALF_PI )
	{
		//find the edge of the current face where theta=pi/2
		Halfedge* finger = leftedge->get_next();
		while(finger->get_origin()->get_theta() != HALF_PI)
		{
			if(verbosity >= 8) { std::cout << "    ----looking for right edge; finger: " << HID(finger) << "; leftedge: " << HID(leftedge) << "\n"; }
			
			finger = finger->get_next();
		}
		
		//insert a new vertex
		rightedge = insert_vertex(finger, HALF_PI, -1*time);
		
		if(verbosity >= 8) { std::cout << "    ----rightedge: " << HID(rightedge) << "\n"; }
		
		//insert a new edge to subdivide the face
		insert_edge(leftedge, rightedge, current);
		
		//update LCM curve pointer, if necessary
		if(!lcm_points_to_curve)
		{
			current->set_curve(rightedge->get_prev());
			lcm_points_to_curve = true;
		}
	}
	
	//add LCM object to the ordered LCM container
	inserted_lcms.insert(current);
	
	
	
	
	
}//end add_curve()

//inserts a new vertex on the specified edge, with the specified coordinates, and updates all relevant pointers
//  i.e. new vertex is between initial and termainal points of the specified edge
//returns pointer to a new halfedge, whose initial point is the new vertex, and that follows the specified edge around its face
Halfedge* Mesh::insert_vertex(Halfedge* edge, double t, double r)
{
	//create new vertex
	Vertex* new_vertex = new Vertex(t, r);
	vertices.push_back(new_vertex);
	
	//get twin and LCM of this edge
	Halfedge* twin = edge->get_twin();
	LCM* lcm = edge->get_LCM();
	
	//create new halfedges
	Halfedge* up = new Halfedge(new_vertex, lcm);
	halfedges.push_back(up);
	Halfedge* dn = new Halfedge(new_vertex, lcm);
	halfedges.push_back(dn);
		
	//update pointers
	up->set_next(edge->get_next());
	up->set_prev(edge);
	up->set_twin(twin);
	up->set_face(edge->get_face());
	
	up->get_next()->set_prev(up);
	
	edge->set_next(up);
	edge->set_twin(dn);
	
	dn->set_next(twin->get_next());
	dn->set_prev(twin);
	dn->set_twin(edge);
	dn->set_face(twin->get_face());
	
	dn->get_next()->set_prev(dn);
	
	twin->set_next(dn);
	twin->set_twin(up);
	
	
	new_vertex->set_incident_edge(up);
	
	//return pointer to up
	return up;
}

//inserts a new edge across an existing face
//requires leftedge and rightedge, coherently oriented around the existing face, and whose origin vertices will be endpoints of the new edge
//also requires the LCM to be associated with the new edge
void Mesh::insert_edge(Halfedge* leftedge, Halfedge* rightedge, LCM* lcm)
{
	//consistency checking -- make sure leftedge and rightedge point to the same face
	if(leftedge->get_face() != rightedge->get_face())	//THIS SHOULD NEVER HAPPEN
	{
		std::cout << "===>>>ERROR: leftedge and rightedge do not point to the same face in insert_edge()\n";
		throw std::exception();
	}
	
	if(verbosity >= 6) { std::cout << "    --inserting new edge using reference edges: " << HID(leftedge) << " and " << HID(rightedge) << "\n"; }
		
	//create new halfedges
	Halfedge* fromleft = new Halfedge(leftedge->get_origin(), lcm);
	halfedges.push_back(fromleft);
	Halfedge* fromright = new Halfedge(rightedge->get_origin(), lcm);
	halfedges.push_back(fromright);
	
	//update edge pointers
	fromleft->set_prev(leftedge->get_prev());
	fromleft->set_next(rightedge);
	fromleft->set_twin(fromright);
	
	fromright->set_prev(rightedge->get_prev());
	fromright->set_next(leftedge);
	fromright->set_twin(fromleft);
	
	leftedge->get_prev()->set_next(fromleft);
	leftedge->set_prev(fromright);
	
	rightedge->get_prev()->set_next(fromright);
	rightedge->set_prev(fromleft);
	
	//insert new face
	Face* newface = new Face(fromright);
	faces.push_back(newface);
	
	//update face pointers
	fromleft->set_face(rightedge->get_face());
	fromleft->get_face()->set_boundary(fromleft);
	
	Halfedge* finger = fromright;
	do
	{
		finger->set_face(newface);
		finger = finger->get_next();
	}while(finger != fromright);
}

//determines whether LCM (time, dist) is already represented in the mesh
bool Mesh::contains(double time, double dist)
{
	LCM* test = new LCM(time, dist);
	
	if( inserted_lcms.find(test) == inserted_lcms.end() )
	{
		delete test;
		return false;
	}
	/*else*/
	delete test;
	return true;
}


//print all the data from the mesh
void Mesh::print()
{
	std::cout << "  Vertices\n";
	for(int i=0; i<vertices.size(); i++)
	{
		std::cout << "    vertex " << i << ": " << *vertices[i] << "; incident edge: " << HID(vertices[i]->get_incident_edge()) << "\n";
	}
	
	std::cout << "  Halfedges\n";
	for(int i=0; i<halfedges.size(); i++)
	{
		Halfedge* e = halfedges[i];
		Halfedge* t = e->get_twin();
//		std::cout << "    halfedge " << i << " (" << e << "): " << *(e->get_origin()) << "--" << *(t->get_origin()) << "; ";	//also prints memory location
		std::cout << "    halfedge " << i << ": " << *(e->get_origin()) << "--" << *(t->get_origin()) << "; ";
		if(e->get_LCM() == NULL)
			std::cout << "LCM null; ";
		else
			std::cout << "LCM coords (" << e->get_LCM()->get_time() << ", " << e->get_LCM()->get_dist() << "); ";
		std::cout << "twin: " << HID(t) << "; next: " << HID(e->get_next()) << "; prev: " << HID(e->get_prev()) << "; face: " << FID(e->get_face()) << "\n";;
	}
	
	std::cout << "  Faces\n";
	for(int i=0; i<faces.size(); i++)
	{
//		std::cout << "    face " << i << " (" << faces[i] << "): " << *faces[i] << "\n";	//also prints memory location
		std::cout << "    face " << i << ": " << *faces[i] << "\n";
	}
	
	std::cout << "  Outside (unbounded) region: ";
	Halfedge* start = halfedges[1];
	Halfedge* curr = start;
	do{
		std::cout << *(curr->get_origin()) << "--";
		curr = curr->get_next();
	}while(curr != start);
	std::cout << "cycle\n";
	
	std::cout << "  LCM set: ";
	std::set<LCM*>::iterator it;
	for(it = inserted_lcms.begin(); it != inserted_lcms.end(); ++it)
	{
		LCM cur = **it;
		std::cout << "(" << cur.get_time() << ", " << cur.get_dist() << ") halfedge " << HID(cur.get_curve()) << "; ";
	}
	std::cout << "\n";
	
}//end print()

//look up halfedge ID, used in print() for debugging
// HID = halfedge ID
int Mesh::HID(Halfedge* h)
{
	for(int i=0; i<halfedges.size(); i++)
	{
		if(halfedges[i] == h)
			return i;
	}
	
	//we should never get here
	return -1;	
}

//look up face ID, used in print() for debugging
// FID = face ID
int Mesh::FID(Face* f)
{
	for(int i=0; i<faces.size(); i++)
	{
		if(faces[i] == f)
			return i;
	}
	
	//we should only get here if f is NULL (meaning the unbounded, outside face)
	return -1;	
}

