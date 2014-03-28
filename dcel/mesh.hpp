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
	vertices.push_back( new Vertex(0, -INFTY) );	//index 3
	
	//create halfedges
	for(int i=0; i<4; i++)
	{
		halfedges.push_back( new Halfedge( vertices[i], NULL) );		//index 0, 2, 4, 6 (inside halfedges)
		halfedges.push_back( new Halfedge( vertices[(i+1)%4], NULL) );		//index 1, 3, 5, 7 (outside halfedges)
		halfedges[2*i]->set_twin( halfedges[2*i+1] );
		halfedges[2*i+1]->set_twin( halfedges[2*i] );
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
void Mesh::add_curve(double time, double dist)
{
	//insert left endpoint
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
	
	//insert right endpoint
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
	
	
	
	//add this LCM to the ordered LCM container
	inserted_lcms.insert( LCM(time, dist, fromleft) );
	
	
}//end add_curve()


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

