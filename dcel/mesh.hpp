/* implementation of Mesh class
 * Stores and manipulates the DCEL decomposition of the affine Grassmannian.
 */
 
//constructor; sets up bounding box (with empty interior) for the affine Grassmannian
Mesh::Mesh() : INFTY(std::numeric_limits<double>::infinity())
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
void Mesh::add_curve(double time, double dist)
{
	//create LCM object
	LCM current = LCM(time, dist);
	if(verbose)
		std::cout << "    inserting LCM: (" << current.get_time() << ", " << current.get_dist() << ")\n";
	
	//create set of LCMs ordered by LCM_AngleComparator
	std::set<LCM, LCM_AngleComparator> intersections(current);
	
	//find all comparable, previously-inserted LCMs, add them to the set ordered by LCM_AngleComparator
	std::set<LCM>::iterator it;
	for(it = inserted_lcms.begin(); it != inserted_lcms.end(); ++it)
	{
		if( (it->get_time() >= time && it->get_dist() >= dist) || (it->get_time() <= time && it->get_dist() <= dist) )	//then the LCMs are comparable
		{
			if(verbose)
				std::cout << "      comparable LCM: (" << it->get_time() << ", " << it->get_dist() << ")\n";
			
			intersections.insert(*it);
		}
	}
	
	if(verbose)
	{
		std::cout << "    intersections: ";
		for(it = intersections.begin(); it != intersections.end(); ++it)
		{
			LCM cur = *it;
			std::cout << "(" << cur.get_time() << ", " << cur.get_dist() << "), ";
		}
		std::cout << "\n";
	}
	
	
	//find where to insert left endpoint -- use the ordered LCM container, inserted_lcms
	std::set<LCM>::iterator itleft;
	itleft = inserted_lcms.upper_bound(current);	//returns an iterator to the first LCM that is greater than "current"

	if(verbose)
	{
		if(itleft == inserted_lcms.end())
			std::cout << "    LCM curve to be inserted after all existing LCM curves\n";
		else
			std::cout << "    LCM curve to be inserted before curve corresponding to LCM (" << itleft->get_time() << ", " << itleft->get_dist() << ")\n";
	}
	
	//halfedge pointers to use for constructing the new curve
	Halfedge* leftedge, rightedge;
	
	//find or create left endpoint for the new curve (on theta=0 edge of the strip)
	if(itleft == inserted_lcms.end()) //then create a new vertex above the highest existing vertex on this edge of the strip
	{
		leftedge = insert_vertex(topleft->get_twin(), 0, dist);
		
	}
	else if(itleft->get_dist() == dist)	//then use existing vertex
	{
		//FINISH THIS PART!!!
		
		leftedge= ???
		
	}
	else	//then create new vertex
	{
		leftedge = insert_vertex(itleft->get_curve()->get_prev(), 0, dist);
		
	}
	
	
	//loop through all existing curves that this new curve crosses
	
		//find or create right endpoint for the current face
		
		
		
		//subdivide the face
	
	
	
	//find or create right endpoint for the new curve (on theta=pi/2 edge of the strip)
	
	
	
	
/*	//insert left endpoint (on theta=0 edge of the strip)
	Halfedge* leftedge = halfedges[6];	//left edge
	Halfedge* lefttwin = leftedge->get_twin();
	
	Vertex* leftend = new Vertex(0,dist);	//coordinates
	vertices.push_back(leftend);
	
	Halfedge* leftup = new Halfedge(leftend, leftedge->get_LCM());
	halfedges.push_back(leftup);
	leftup->set_twin(lefttwin);
	leftup->set_next(leftedge->get_next());
	
	Halfedge* leftdown = new Halfedge(leftend, lefttwin->get_LCM());
	halfedges.push_back(leftdown);
	leftdown->set_twin(leftedge);
	leftdown->set_next(lefttwin->get_next());
	leftdown->set_prev(lefttwin);
	lefttwin->set_next(leftdown);
	
	leftedge->set_twin(leftdown);
	lefttwin->set_twin(leftup);
	
	//loop through the ordered LCMs that this new curve will intersect
		
		//identify or create right endpoint of current segment
		
		
		//insert the segment
		
		
		//insert new face and update pointers
		
		
		
	
	
	//insert right endpoint (on theta=pi/2 edge of strip)
	Halfedge* rightedge = halfedges[2];	//right edge
	Halfedge* righttwin = rightedge->get_twin();
	
	Vertex* rightend = new Vertex(HALF_PI,-1*time);	//coordinates
	vertices.push_back(rightend);
	
	Halfedge* rightup = new Halfedge(rightend, righttwin->get_LCM());
	halfedges.push_back(rightup);
	rightup->set_twin(rightedge);
	rightup->set_next(righttwin->get_next());
	rightup->set_prev(righttwin);
	righttwin->set_next(rightup);
	
	Halfedge* rightdown = new Halfedge(rightend, rightedge->get_LCM());
	halfedges.push_back(rightdown);
	rightdown->set_twin(righttwin);
	rightdown->set_next(rightedge->get_next());
	
	rightedge->set_twin(rightup);
	righttwin->set_twin(rightdown);
	
	//insert new edge
	LCM* lcm = new LCM(time, dist);
	Halfedge* fromleft = new Halfedge(leftend, lcm);
	halfedges.push_back(fromleft);
	
	Halfedge* fromright = new Halfedge(rightend, lcm);
	halfedges.push_back(fromright);
	
	fromleft->set_twin(fromright);
	fromright->set_twin(fromleft);
	
	fromleft->set_next(rightdown);
	rightdown->set_prev(fromleft);
	fromleft->set_prev(leftedge);
	leftedge->set_next(fromleft);
	
	fromright->set_next(leftup);
	leftup->set_prev(fromright);
	fromright->set_prev(rightedge);
	rightedge->set_next(fromright);
	
	//insert new face and update face pointers
	//   (old face becomes the face above the new edge; new face is the one below the new edge)
	Face* topface = rightedge->get_face();
	fromright->set_face(topface);
	leftup->set_face(topface);
	
	Face* bottomface = new Face(fromleft);
	faces.push_back(bottomface);
	
	Halfedge* curr = leftedge;	//loop around new face and update face pointers
	do{
		curr->set_face(bottomface);
		curr = curr->get_next();
	}while(curr != leftedge);
	
*/	
	//add pointer to curve to LCM current
	
	
	
	//add this LCM to the ordered LCM container
	inserted_lcms.insert( current );
	
	
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
	up.set_next(edge->get_next());
	up.set_prev(edge);
	up.set_twin(twin);
	up.set_face(edge->get_face());
	
	edge.set_next(up);
	edge.set_twin(dn);
	
	dn.set_next(twin->get_next());
	dn.set_prev(twin);
	dn.set_twin(edge);
	dn.set_face(twin->get_face());
	
	twin.set_next(dn);
	twin.set_twin(up);
	
	//return pointer to up
	return up;
}




//print all the data from the mesh
void Mesh::print()
{
	std::cout << "  Vertices\n";
	for(int i=0; i<vertices.size(); i++)
	{
		std::cout << "    vertex " << i << ": " << *vertices[i] << "\n";
	}
	
	std::cout << "  Halfedges\n";
	for(int i=0; i<halfedges.size(); i++)
	{
		std::cout << "    halfedge " << i << ": " << *halfedges[i] << "\n";
	}
	
	std::cout << "  Faces\n";
	for(int i=0; i<faces.size(); i++)
	{
		std::cout << "    face " << i << " (" << faces[i] << "): " << *faces[i] << "\n";
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
	std::set<LCM>::iterator it;
	for(it = inserted_lcms.begin(); it != inserted_lcms.end(); ++it)
	{
		LCM cur = *it;
		std::cout << "(" << cur.get_time() << ", " << cur.get_dist() << ") ";
	}
	std::cout << "\n";
	
}//end print()

