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
		halfedges.push_back( new Halfedge( vertices[i], -1, -1) );		//index 0, 2, 4, 6 (inside halfedges)
		halfedges.push_back( new Halfedge( vertices[(i+1)%4], -1, -1) );		//index 1, 3, 5, 7 (outside halfedges)
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
	
	
}//end add_curve()


//print all the data from the mesh
void Mesh::print()
{
	std::cout << "  Vertices\n";
	for(int i=0; i<vertices.size(); i++)
	{
		//Vertex* v = &(vertices[i]);
		std::cout << "    vertex " << i << ": " << *vertices[i] << "\n";
	}
	
	std::cout << "  Halfedges\n";
	for(int i=0; i<halfedges.size(); i++)
	{
		//Halfedge* e = &(halfedges[i]);
		std::cout << "    halfedge " << i << ": " << *halfedges[i] << "\n";
	}
	
	std::cout << "  Faces\n";
	for(int i=0; i<faces.size(); i++)
	{
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
	
}//end print()

