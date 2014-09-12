/* implementation of Mesh class
 * Stores and manipulates the DCEL decomposition of the affine Grassmannian.
 */

#include "mesh.h"

//constructor; sets up bounding box (with empty interior) for the affine Grassmannian
Mesh::Mesh(int v) : 
	INFTY(std::numeric_limits<double>::infinity()),
    HALF_PI(boost::math::constants::pi<double>()/2),
    EPSILON(1/pow(double(10), 10)),
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
    bottomleft = halfedges[6];  //remember these halfedges
    bottomright = halfedges[3]; //    for the Bentley-Ottmann algorithm
	
	//set pointers on vertices
	for(int i=0; i<4; i++)
	{
		vertices[i]->set_incident_edge( halfedges[2*i] );
	}
	
	//create face
	faces.push_back( new Face( halfedges[0], verbosity ) );
	
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

//adds an LCM; curve will be created when build_arrangement() is called
void Mesh::add_lcm(double x, double y)
{
    all_lcms.insert(new LCM(x, y));
}


///// DATA STRUCTURES FOR BENTLEY-OTTMANN ALGORITHM /////
//struct to hold a future intersection event
struct Crossing {
    LCM* a;     //pointer to one curve
    LCM* b;     //pointer to the other curve -- must ensure that curve for LCM a is below curve for LCM b just before the crossing point!!!!!
    double t;    //theta-coordinate of intersection point
    double r;    //r-coordinate of the intersection point

    Crossing(LCM* a, LCM* b, double t, double r) : a(a), b(b), t(t), r(r) { }
};

//comparator class for ordering crossings: first by theta (left to right); then for a given theta, by r (low to high)
struct CrossingComparator {
    bool operator()(const Crossing* c1, const Crossing* c2) const	//returns true if c1 comes after c2
    {
        //TESTING
        if(c1->a->get_position() >= c1->b->get_position() || c2->a->get_position() >= c2->b->get_position())
        {
            std::cerr << "INVERTED CROSSING ERROR\n";
            std::cerr << "crossing 1 involves LCMS " << c1->a << " (pos " << c1->a->get_position() << ") and " << c1->b << " (pos " << c1->b->get_position() << "),";
            std::cerr << "crossing 2 involves LCMS " << c2->a << " (pos " << c2->a->get_position() << ") and " << c2->b << " (pos " << c2->b->get_position() << "),";
            throw std::exception();
        }

        double epsilon = pow(2,-30);

        if(c1->t - c2->t > epsilon) //then c1 > c2      <<<------------ THIS ISN'T SO GOOD....IMPROVE!!!
            return true;
        if(c1->t - c2->t < -1*epsilon) //then c1 < c2     <<<------------ THIS ISN'T SO GOOD....IMPROVE!!!
            return false;

        //else, then c1->t == c2->t, so consider r-coordinate

        //TESTING
        if(c1->a->get_position() == c2->a->get_position())
        {
            std::cerr << "ILLEGAL CROSSING ERROR\n";
            std::cerr << "crossing 1 involves LCMS " << c1->a << " (pos " << c1->a->get_position() << ") and " << c1->b << " (pos " << c1->b->get_position() << "),";
            std::cerr << "crossing 2 involves LCMS " << c2->a << " (pos " << c2->a->get_position() << ") and " << c2->b << " (pos " << c2->b->get_position() << "),";
            throw std::exception();
        }

        //
//        return (c1->r - c2->r > epsilon);   //true if c1->r > c2->r; false if c1->r < c2->r or they are too close to tell

        if(c1->r - c2->r > epsilon) //then c1 > c2
            return true;
        if(c1->r - c2->r < -1*epsilon) //then c1 < c2
            return false;

        //else, theta and r coordinates are both equal, then sort by relative position of curves
        return c1->a->get_position() > c2->a->get_position();   //IS THERE A BETTER WAY???
    }
};

//tests whether x and y are with epsilon -- TO BE REPLACED LATER BY EXACT COMPARISON
bool Mesh::equal(double x, double y)
{
    return abs(x-y) < EPSILON;
}

//function to build the arrangement using a version of the Bentley-Ottmann algorithm, given all LCMs
//preconditions:
//		all LCMs are in a list, ordered by lcm_left_comparator
//		boundary of the mesh is created (as in the mesh constructor)
void Mesh::build_arrangement()
{
    if(verbosity >= 5)
    {
        std::cout << "BUILDING ARRANGEMENT:  LCMs sorted for left edge of strip: ";
        for(std::set<LCM*, LCM_LeftComparator>::iterator it = all_lcms.begin(); it != all_lcms.end(); ++it)
            std::cout << "(" << (*it)->get_x() << "," << (*it)->get_y() << ") ";
        std::cout << "\n";
    }

    // DATA STRUCTURES

    //data structure for ordered list of curves
    std::vector<Halfedge*> curves;
    curves.reserve(all_lcms.size());

    //data structure for queue of future intersections
    std::priority_queue< Crossing*, std::vector<Crossing*>, CrossingComparator > crossings;

    //data structure for all pairs of LCMs whose potential crossings have been considered
    typedef std::pair<LCM*,LCM*> LCM_pair;
    std::set< LCM_pair > considered_pairs;

    //current position of sweep line
    double sweep = 0;

  // PART 1: INSERT VERTICES AND EDGES ALONG LEFT EDGE OF THE STRIP
    if(verbosity >= 5) { std::cout << "PART 1: LEFT EDGE OF STRIP\n"; }

    //for each LCM, create vertex and associated halfedges, anchored on the left edge of the strip
    Halfedge* leftedge = bottomleft;
    for(std::set<LCM*, LCM_LeftComparator>::iterator it = all_lcms.begin(); it != all_lcms.end(); ++it)
    {
        LCM* cur_lcm = *it;

        if(verbosity >= 6) { std::cout << "  Processing LCM(" << cur_lcm->get_x() << "," << cur_lcm->get_y() << "):"; }

        if(cur_lcm->get_y() != leftedge->get_origin()->get_r())	//then create new vertex and set leftedge to edge that will follow the new edge
            leftedge = insert_vertex(leftedge, 0, cur_lcm->get_y());

        //now insert new edge at origin vertex of leftedge
        Halfedge* new_edge = create_edge_left(leftedge, cur_lcm);

        //remember Halfedge corresponding to this LCM
        curves.push_back(new_edge);

        //remember relative position of this LCM
        cur_lcm->set_position(curves.size() - 1);

        //remember curve associated with this LCM --- THIS MIGHT BE UNNECESSARY AFTER TESTING IS COMPLETE!!!
        cur_lcm->set_curve(new_edge);
    }

    //for each pair of consecutive curves, if they intersect, store the intersection
    for(unsigned i = 0; i < curves.size() - 1; i++)
    {
        LCM* a = curves[i]->get_LCM();
        LCM* b = curves[i+1]->get_LCM();

        considered_pairs.insert(LCM_pair(a,b));

        double t = find_intersection(a, b);
        if(verbosity >= 6) { std::cout << "  curves " << i << " (LCM  " << a << ") and " << i+1 << " (LCM " << b << ") intersect at theta = " << t << "\n"; }

        if(t > 0)
            crossings.push(new Crossing(a, b, t, find_r(a, t)));
    }

  // PART 2: PROCESS INTERIOR INTERSECTIONS
    //    order: theta left to right; for a given theta, r low to high
    if(verbosity >= 5) { std::cout << "PART 2: PROCESSING INTERIOR INTERSECTIONS\n"; }

    int status_counter = 0;
    int status_interval = 10000;

    while(!crossings.empty())
    {
        //get the next intersection from the queue
        Crossing* cur = crossings.top();
        crossings.pop();

        //process the intersection
        sweep = cur->t;
        unsigned first_pos = cur->a->get_position();   //most recent edge in the curve corresponding to LCM a
        unsigned last_pos = cur->b->get_position();   //most recent edge in the curve corresponding to LCM b

        if(verbosity >= 6) { std::cout << " next intersection: LCM " << cur->a << " (" << first_pos << "), LCM " << cur->b << " (" << last_pos << ")\n"; }

        if(last_pos != first_pos + 1)
        {
            std::cerr << "ERROR: intersection between non-consecutive curves [1]: theta = " << sweep << ", r = " << find_r( curves[first_pos]->get_LCM(), sweep ) << "\n";
            throw std::exception();
        }

        //find out if more than two curves intersect at this point
        while(!crossings.empty() && equal(sweep, crossings.top()->t) && (cur->b == crossings.top()->a) )
        {
            cur = crossings.top();
            crossings.pop();

            if(cur->b->get_position() != last_pos + 1)
            {
                std::cerr << "ERROR: intersection between non-consecutive curves [2]\n";
                throw std::exception();
            }

            last_pos++; //last_pos = cur->b->get_position();

            if(verbosity >= 6) { std::cout << " |---also intersects LCM " << cur->b << " (" << last_pos << ")\n"; }

        }

    //TESTING
//        Crossing* next = crossings.top();
//        std::cout << "      [looking ahead: " << next->a << " (" << next->a->get_position() << "), " << next->b << " (" << next->b->get_position() << ") at theta = " << next->t << "]\n";

        //find r-coordinate of intersection
//        double r = find_r( curves[first_pos]->get_LCM(), sweep ); ---NO LONGER NECESSARY!!!

        if(verbosity >= 6) { std::cout << "  found intersection between " << (last_pos - first_pos + 1) << " edges at theta = " << sweep << ", r = " << cur->r << "\n"; }

        //create new vertex
        Vertex* new_vertex = new Vertex(sweep, cur->r);
        vertices.push_back(new_vertex);

        //anchor edges to vertex and create new face(s) and edges	//TODO: check this!!!
        Halfedge* prev_new_edge = NULL;                 //necessary to remember the previous new edge at each interation of the loop
        Halfedge* first_incoming = curves[first_pos];   //necessary to remember the first incoming edge
        Halfedge* prev_incoming = NULL;                 //necessary to remember the previous incoming edge at each iteration of the loop
        for(unsigned cur_pos = first_pos; cur_pos <= last_pos; cur_pos++)
        {
            //anchor edge to vertex
            Halfedge* incoming = curves[cur_pos];
            incoming->get_twin()->set_origin(new_vertex);

            //create next pair of twin halfedges along the current curve (i.e. curves[incident_edges[i]] )
            Halfedge* new_edge = new Halfedge(new_vertex, incoming->get_LCM());	//points AWAY FROM new_vertex
            halfedges.push_back(new_edge);
            Halfedge* new_twin = new Halfedge(NULL, incoming->get_LCM());		//points TOWARDS new_vertex
            halfedges.push_back(new_twin);

            //update halfedge pointers
            new_edge->set_twin(new_twin);
            new_twin->set_twin(new_edge);

            if(cur_pos == first_pos)    //then this is the first iteration of the loop
            {
                new_twin->set_next( curves[last_pos]->get_twin() );
                curves[last_pos]->get_twin()->set_prev(new_twin);

                new_twin->set_face( curves[last_pos]->get_twin()->get_face() );
            }
            else    //then this is not the first iteration of the loop, so close up a face and create a new face
            {
                incoming->set_next( prev_incoming->get_twin() );
                incoming->get_next()->set_prev(incoming);

                Face* new_face = new Face(new_twin, verbosity);
                faces.push_back(new_face);

                new_twin->set_face(new_face);
                prev_new_edge->set_face(new_face);

                new_twin->set_next(prev_new_edge);
                prev_new_edge->set_prev(new_twin);
            }

            //remember important halfedges for the next iteration of the loop
            prev_incoming = incoming;
            prev_new_edge = new_edge;

            if(cur_pos == last_pos)  //then this is the last iteration of loop
            {
                new_edge->set_prev(first_incoming);
                first_incoming->set_next(new_edge);

                new_edge->set_face( first_incoming->get_face() );
            }

            //update curves vector
            curves[cur_pos] = new_edge; //the portion of this vector [first_pos, last_pos] must be reversed after this loop is finished!

            //remember position of this LCM
            new_edge->get_LCM()->set_position(last_pos - (cur_pos - first_pos));
//            if(verbosity >= 6) { std::cout << "    - set LCM " << new_edge->get_LCM() << " to position " << (last_pos - (cur_pos - first_pos)) << "\n"; }
        }

        //update curves vector: flip portion of vector [first_pos, last_pos]
        for(unsigned i = 0; i < (last_pos - first_pos + 1)/2; i++)
        {
            //swap curves[first_pos + i] and curves[last_pos - i]
            Halfedge* temp = curves[first_pos + i];
            curves[first_pos + i] = curves[last_pos - i];
            curves[last_pos - i] = temp;
        }

        //TESTING
//        std::cout << "      TESTING: ";
//        for(unsigned i = first_pos; i<= last_pos; i++)
//            std::cout << i << ": " << curves[i]->get_LCM() << "; ";
//        std::cout << "\n";

        //find new intersections and add them to intersections queue
        if(first_pos > 0)   //then consider lower intersection
        {
            LCM* a = curves[first_pos-1]->get_LCM();
            LCM* b = curves[first_pos]->get_LCM();
            if(considered_pairs.find(LCM_pair(a,b)) == considered_pairs.end())	//then this pair has not yet been considered
            {
                considered_pairs.insert(LCM_pair(a,b));
                double t = find_intersection(a, b);
                if(verbosity >= 6) { std::cout << "  curves " << a << " (" << a->get_position() << ") and " << b << " (" << b->get_position() << ") intersect at theta = " << t << "\n"; }

                if(t > sweep)
                    crossings.push(new Crossing(a, b, t, find_r(a, t)));
            }
        }

        if(last_pos + 1 < curves.size())    //then consider upper intersection
        {
            LCM* a = curves[last_pos]->get_LCM();
            LCM* b = curves[last_pos+1]->get_LCM();
            if(considered_pairs.find(LCM_pair(a,b)) == considered_pairs.end())	//then this pair has not yet been considered
            {
                considered_pairs.insert(LCM_pair(a,b));
                double t = find_intersection(a, b);
                if(verbosity >= 6) { std::cout << "  curves " << a << " (" << a->get_position() << ") and " << b << " (" << b->get_position() << ") intersect at theta = " << t << "\n"; }

                if(t > sweep)
                    crossings.push(new Crossing(a, b, t, find_r(a, t)));
            }
        }

        //output status
        if(verbosity >= 2)
        {
            status_counter++;
            if(status_counter % status_interval == 0)
                std::cout << "      processed " << status_counter << " intersections; theta = " << sweep << "\n";
        }
    }//end while

  // PART 3: INSERT VERTICES ON RIGHT EDGE OF STRIP AND CONNECT EDGES
    if(verbosity >= 5) { std::cout << "PART 3: RIGHT EDGE OF THE STRIP\n"; }

    Halfedge* rightedge = bottomright; //need a reference halfedge along the right side of the strip
    double last_x = INFTY;      //keep track of x-coordinate of last LCM whose curve was connected to right side of strip; this will decrease from infinity

    //connect each curve to the right edge of the strip
    for(unsigned cur_pos = 0; cur_pos < curves.size(); cur_pos++)
    {
        LCM* cur_lcm = curves[cur_pos]->get_LCM();

        if(cur_lcm->get_x() < last_x)    //then insert a new vertex
        {
            last_x = cur_lcm->get_x();
            rightedge = insert_vertex( rightedge, HALF_PI, -1*last_x );
        }
        //otherwise, connect current curve to previously-inserted vertex

        Vertex* cur_vertex = rightedge->get_origin();

        //anchor halfedge to vertex
        Halfedge* incoming = curves[cur_pos];
        incoming->get_twin()->set_origin(cur_vertex);

        //update halfedge pointers
        incoming->set_next(rightedge->get_twin()->get_next());
        incoming->get_next()->set_prev(incoming);

        incoming->get_next()->set_face(incoming->get_face());   //only necessary if incoming->get_next() is along the right side of the strip

        incoming->get_twin()->set_prev(rightedge->get_twin());
        rightedge->get_twin()->set_next(incoming->get_twin());

        rightedge->get_twin()->set_face(incoming->get_twin()->get_face());
    }
}//end build_arrangement()


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
}//end insert_vertex()

//creates the first pair of Halfedges in an LCM curve, anchored on the left edge of the strip at origin of specified edge
//  also creates a new face (the face below the new edge)
//  CAUTION: leaves NULL: new_edge.next and new_twin.prev
Halfedge* Mesh::create_edge_left(Halfedge* edge, LCM* lcm)
{
    //create new halfedges
    Halfedge* new_edge = new Halfedge(edge->get_origin(), lcm); //points AWAY FROM left edge
    halfedges.push_back(new_edge);
    Halfedge* new_twin = new Halfedge(NULL, lcm);   //points TOWARDS left edge
    halfedges.push_back(new_twin);

    //create new face
    Face* new_face = new Face(new_edge, verbosity);
    faces.push_back(new_face);

    //update Halfedge pointers
    new_edge->set_prev(edge->get_prev());
    new_edge->set_twin(new_twin);
    new_edge->set_face(new_face);

    edge->get_prev()->set_next(new_edge);
    edge->get_prev()->set_face(new_face);
    if(edge->get_prev()->get_prev() != NULL)
        edge->get_prev()->get_prev()->set_face(new_face);

    new_twin->set_next(edge);
    new_twin->set_twin(new_edge);
    new_twin->set_face(edge->get_face());

    edge->set_prev(new_twin);

    //return pointer to new_edge
    return new_edge;
}//end create_edge_left()

//returns the theta-coordinate of intersection of two LCM curves if intersection occurs for 0 < theta < pi/2
//  otherwise, returns -1
double Mesh::find_intersection(LCM* a, LCM* b)
{
    double ax = a->get_x();
    double ay = a->get_y();
    double bx = b->get_x();
    double by = b->get_y();

    if( (ax - bx)*(ay - by) < 0 )   //then the LCMS are incomparable, so no intersection exists
        return -1;

    if(ax == bx || ay == by)    //then the intersection is at theta = pi/2 or theta = 0
        return -1;

    //otherwise, the intersection is at 0 <= theta < pi/2
    return atan( (ay - by)/(ax - bx) );
}

//returns the r-coordinate corresponding to theta = t on the curve for LCM a
double Mesh::find_r(LCM* a, double t)
{
    double x = a->get_x();
    double y = a->get_y();

    if(x != 0)
        return sqrt( x*x + y*y ) * sin( atan(y/x) - t );

    return sqrt(x*x + y*y) * cos(t);
}

//associates a persistence diagram to each face (IN PROGRESS)
void Mesh::build_persistence_data(std::vector<std::pair<int, int> > & xi, SimplexTree* bifiltration, int dim)
{
	//loop through all faces (NOTE: this can probably be optimized to take into account adjacency relationships among faces)
    for(unsigned i=0; i<faces.size(); i++)
	{
        if(verbosity >= 4) { std::cout << "  Computing persistence data for face " << i << ":\n"; }
		
		faces[i]->store_interior_point();
		
        faces[i]->get_data()->compute_data(xi, bifiltration, dim);
	}
}//end build_persistence_data()


//returns a persistence diagram associated with the specified point
//uses a naive point-location algorithm to find the cell containing the point
PersistenceData* Mesh::get_persistence_data(double angle, double offset, std::vector<std::pair<int, int> > & xi, SimplexTree* bifiltration)
{
	//find the cell that contains the point
	//first, find starting point on left edge of strip
	LCM* current = new LCM(0, offset);
	std::set<LCM*>::iterator itleft;
    itleft = all_lcms.upper_bound(current);	//returns an iterator to the first LCM that is greater than "current"
	
	Face* cell = NULL;		//will later point to the cell containing the specified point
	Halfedge* finger = NULL;	//for use in finding the cell
	
    if(itleft == all_lcms.end())	//then point is above all LCMS on the left edge of the strip
	{
		if(verbosity >= 8) { std::cout << "  Point is in top cell.\n"; }
		finger = topleft->get_twin();
		cell = finger->get_face();	//found the cell
	}
	else
	{
		finger = (**itleft).get_curve();
		
        if(verbosity >= 8) { std::cout << "  Reference LCM: (" << (**itleft).get_x() << ", " << (**itleft).get_y() << "); halfedge " << HID(finger) << ".\n"; }
		
		while(cell == NULL)
		{
			if(verbosity >= 8) { std::cout << "  Considering cell " << FID(finger->get_face()) << ".\n"; }
		
			//find the edge of the current cell that crosses the horizontal line at offset
			Vertex* next_pt = finger->get_next()->get_origin();
			while(next_pt->get_r() > offset)
			{
				if(verbosity >= 8) { std::cout << "    --finger: " << HID(finger) << ".\n"; }
		
				finger = finger->get_next();
				next_pt = finger->get_next()->get_origin();
			}
			//now next_pt is at or below the horizontal line at offset
			//if (angle, offset) is to the left of crossing point, then we have found the cell; otherwise, move to the adjacent cell
			
			if(verbosity >= 8) { std::cout << "    --found next_pt at (" << next_pt->get_theta() << ", " << next_pt->get_r() << "); finger: " << HID(finger) << ".\n"; }
			
			if(next_pt->get_r() == offset) //then next_pt is on the horizontal line
			{
				if(next_pt->get_theta() >= angle)	//found the cell
				{
					cell = finger->get_face();
				}
				else	//move to adjacent cell
				{
					//find degree of vertex
					Halfedge* thumb = finger->get_next();
					int deg = 1;
					while(thumb != finger->get_twin())
					{
						thumb = thumb->get_twin()->get_next();
						deg++;
					}
					
					//move halfway around the vertex
					finger = finger->get_next();
					for(int i=0; i<deg/2; i++)
					{
						finger = finger->get_twin()->get_next();
					}
				}
			}
			else	//then next_pt is below the horizontal line
			{
				if(finger->get_LCM() == NULL)	//then edge is vertical, so we have found the cell
				{
					cell = finger->get_face();
				}
				else	//then edge is not vertical
				{
					LCM* curr_lcm = finger->get_LCM();
                    double test_offset = curr_lcm->get_r_coord(angle);
				
					if(test_offset >= offset)	//found the cell
					{
						cell = finger->get_face();
					}
					else	//move to adjacent cell
					{
						finger = finger->get_twin();
					}
				}
			}
		}//end while(cell not found)
	}//end else
	
    if(verbosity >= 3) { std::cout << "  -> Found point (" << angle << ", " << offset << ") in cell " << FID(cell) << ".\n"; }
	
	//get persistence data associated with the cell
    CellPersistenceData* cell_data = cell->get_data();
    std::vector<int> xi_global = *(cell_data->get_xi_global());
	
	//compute projections of the xi support points relevant to this cell
	std::vector<std::pair<bool,double> > xi_proj;	//index is order_xi_support_point_index; first entry indicates whether the projection exists; second entry is projection coordinate

    for(unsigned i=0; i<xi_global.size(); i++)
	{
		//get absolute coordinates of this xi support point
//        double x = xi[xi_global[i]].first;
//        double y = xi[xi_global[i]].second;
//        std::cout << "XI: xi_global[" << i << "] = " << xi_global[i] << "; xi.size() = " << xi.size() << "\n";
//        std::cout << "    xi[xi_global[i]].first = " << xi[xi_global[i]].first << "\n";// maps to " << bifiltration->get_time(xi[xi_global[i]].first) << "\n";
        double x = bifiltration->grade_x_value(xi[xi_global[i]].first);
        double y = bifiltration->grade_y_value(xi[xi_global[i]].second);


		//project onto line
		std::pair<bool,double> proj = project(angle, offset, x, y);
		
		if(verbosity >= 3)
		{
			std::cout << "  xi support point: global index " << xi_global[i] << ", (time, dist) = (" << x << ", " << y << "), ";
			if(proj.first)
				std::cout << "projected to " << proj.second << "\n";
			else
				std::cout << "has no projection onto this line\n";
		}
		
		//store projection
		xi_proj.push_back(proj);
    }

	//build persistence diagram
    PersistenceData* pdata = new PersistenceData();			//DELETE this item later!
	
	//now we simply have to translate and store the persistence pairs and essential cycles
    std::vector< std::pair<int,int> > pairs;
    pairs = *(cell_data->get_pairs());
	
    for(unsigned i=0; i<pairs.size(); i++)
	{
		std::pair<bool,double> birth_proj = xi_proj[pairs[i].first];
		std::pair<bool,double> death_proj = xi_proj[pairs[i].second];
		
		if(birth_proj.first && death_proj.first)	//both projections exist, so store pair
		{
            pdata->add_pair(birth_proj.second, death_proj.second);
            if(verbosity >= 3) { std::cout << "    persistence pair: (" << pairs[i].first << ", " << pairs[i].second << ") -> (" << birth_proj.second << ", " << death_proj.second << ")\n"; }
		}
		else if(birth_proj.first)	//only the birth projection exists, so store cycle
		{
            pdata->add_cycle(birth_proj.second);
			if(verbosity >= 3) { std::cout << "    cycle: " << birth_proj.second << " (death projected to infinity)\n"; }
		}
	}
	
	std::vector<int> cycles;
    cycles = *(cell_data->get_cycles());
	
    for(unsigned i=0; i<cycles.size(); i++)
	{
		std::pair<bool,double> birth_proj = xi_proj[cycles[i]];
		
		if(birth_proj.first)	//projection exists, so store cycle
		{
            pdata->add_cycle(birth_proj.second);
			if(verbosity >= 3) { std::cout << "    cycle: " << birth_proj.second << "\n"; }
		}
	}

	//clean up
	delete current;
	
    //return persistence data
    return pdata;
}//end get_persistence_data

//projects (x,y) onto the line determined by angle and offset
std::pair<bool, double> Mesh::project(double angle, double offset, double x, double y)
{
	double p = 0;	//if there is a projection, then this will be set to its coordinate
	bool b = true;	//if there is no projection, then this will be set to false
	
	if(angle == 0)	//horizontal line
	{
		if(y <= offset)	//then point is below line
			p = x;
		else	//no projection
			b = false;
	}
	else if(angle < HALF_PI)	//line is neither horizontal nor vertical
	{
		if(y > x*tan(angle) + offset/cos(angle))	//then point is above line
			p = y/sin(angle) - offset/tan(angle); //project right
		else
			p = x/cos(angle) + offset*tan(angle); //project up
	}
	else	//vertical line
	{
		if(x <= -1*offset)
			p = y;
		else	//no projection
			b = false;
	}
	
	return std::pair<bool, double>(b,p);
}//end project()

//prints a summary of the arrangement information, such as the number of LCMS, vertices, halfedges, and faces
void Mesh::print_stats()
{
    std::cout << "The arrangement contains: \n";
    std::cout << "    " << all_lcms.size() << " LCMs\n";
    std::cout << "    " << vertices.size() << " vertices\n";
    std::cout << "    " << halfedges.size() << " halfedges\n";
    std::cout << "    " << faces.size() << " faces\n";
}

//print all the data from the mesh
void Mesh::print()
{
	std::cout << "  Vertices\n";
    for(unsigned i=0; i<vertices.size(); i++)
	{
		std::cout << "    vertex " << i << ": " << *vertices[i] << "; incident edge: " << HID(vertices[i]->get_incident_edge()) << "\n";
	}
	
	std::cout << "  Halfedges\n";
    for(unsigned i=0; i<halfedges.size(); i++)
	{
		Halfedge* e = halfedges[i];
		Halfedge* t = e->get_twin();
//		std::cout << "    halfedge " << i << " (" << e << "): " << *(e->get_origin()) << "--" << *(t->get_origin()) << "; ";	//also prints memory location
		std::cout << "    halfedge " << i << ": " << *(e->get_origin()) << "--" << *(t->get_origin()) << "; ";
		if(e->get_LCM() == NULL)
			std::cout << "LCM null; ";
		else
            std::cout << "LCM coords (" << e->get_LCM()->get_x() << ", " << e->get_LCM()->get_y() << "); ";
		std::cout << "twin: " << HID(t) << "; next: " << HID(e->get_next()) << "; prev: " << HID(e->get_prev()) << "; face: " << FID(e->get_face()) << "\n";;
	}
	
	std::cout << "  Faces\n";
    for(unsigned i=0; i<faces.size(); i++)
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
    for(it = all_lcms.begin(); it != all_lcms.end(); ++it)
	{
		LCM cur = **it;
        std::cout << "(" << cur.get_x() << ", " << cur.get_y() << ") halfedge " << HID(cur.get_curve()) << "; ";
	}
	std::cout << "\n";
	
}//end print()

//look up halfedge ID, used in print() for debugging
// HID = halfedge ID
unsigned Mesh::HID(Halfedge* h)
{
    for(unsigned i=0; i<halfedges.size(); i++)
	{
		if(halfedges[i] == h)
			return i;
	}
	
	//we should never get here
	return -1;	
}

//look up face ID, used in print() for debugging
// FID = face ID
unsigned Mesh::FID(Face* f)
{
    for(unsigned i=0; i<faces.size(); i++)
	{
		if(faces[i] == f)
			return i;
	}
	
	//we should only get here if f is NULL (meaning the unbounded, outside face)
	return -1;	
}

//attempts to find inconsistencies in the DCEL arrangement
void Mesh::test_consistency()
{
    //check faces
    std::cout << "Checking faces:\n";
    bool face_problem = false;
    std::set<int> edges_found_in_faces;

    for(std::vector<Face*>::iterator it = faces.begin(); it != faces.end(); ++it)
    {
        Face* face = *it;
        std::cout << "  Checking face " << FID(face) << "\n";

        if(face->get_boundary() == NULL)
        {
            std::cout << "    PROBLEM: face " << FID(face) << " has null edge pointer.\n";
            face_problem = true;
        }
        else
        {
            Halfedge* start = face->get_boundary();
//            std::cout << "    starting halfedge " << HID(start) << " [" << *start << "]\n";
            edges_found_in_faces.insert(HID(start));

            if(start->get_face() != face)
            {
                std::cout << "    PROBLEM: starting halfedge edge " << HID(start) << " of face " << FID(face) << " doesn't point back to face.\n";
                face_problem = true;
            }

            if(start->get_next() == NULL)
                std::cout << "    PROBLEM: starting halfedge " << HID(start) << " of face " << FID(face) << " has NULL next pointer.\n";
            else
            {
                Halfedge* cur = start->get_next();
                int i = 0;
                while(cur != start)
                {
//                    std::cout << "    halfedge " << HID(cur) << " [" << *cur << "]\n";
                    edges_found_in_faces.insert(HID(cur));

                    if(cur->get_face() != face)
                    {
                        std::cout << "    PROBLEM: halfedge edge " << HID(cur) << " points to face " << FID(cur->get_face()) << " instead of face " << FID(face) << ".\n";
                        face_problem = true;
                        break;
                    }

                    if(cur->get_next() == NULL)
                    {
                        std::cout << "    PROBLEM: halfedge " << HID(cur) << " has NULL next pointer.\n";
                        face_problem = true;
                        break;
                    }
                    else
                        cur = cur->get_next();

                    i++;
                    if(i >= 1000)
                    {
                        std::cout << "    PROBLEM: halfedges of face " << FID(face) << " do not form a cycle (or, if they do, it has more than 1000 edges).\n";
                        face_problem = true;
                        break;
                    }
                }
            }

        }
    }//end face loop
    if(!face_problem)
        std::cout << "   ---No problems detected among faces.\n";
    else
        std::cout << "   ---Problems detected among faces.\n";

    //find exterior halfedges
//    std::cout << "  Checking exterior face\n";
    Halfedge* start = halfedges[1];
    Halfedge* cur = start;
    do{
        edges_found_in_faces.insert(HID(cur));

        if(cur->get_next() == NULL)
        {
            std::cout << "    PROBLEM: halfedge " << HID(cur) << " has NULL next pointer.\n";
            break;
        }
        cur = cur->get_next();
    }while(cur != start);

    //check if all edges were found
    bool all_edges_found = true;
    for(int i=0; i<halfedges.size(); i++)
    {
        if(edges_found_in_faces.find(i) == edges_found_in_faces.end())
        {
            std::cout << "  PROBLEM: halfedge " << i << "not found in any face\n";
            all_edges_found = false;
        }
    }
    if(all_edges_found)
        std::cout << "   ---All halfedges found in faces, as expected.\n";


    //check curves
    std::cout << "Checking curves:\n";
    bool curve_problem = false;
    std::set<int> edges_found_in_curves;

    for(std::set<LCM*>::iterator it = all_lcms.begin(); it != all_lcms.end(); ++it)
    {
        LCM* lcm = *it;
        std::cout << "  Checking curve for LCM (" << lcm->get_x() <<", " << lcm->get_y() << ")\n";

        Halfedge* edge = lcm->get_curve();
        do{
            edges_found_in_curves.insert(HID(edge));
            edges_found_in_curves.insert(HID(edge->get_twin()));

            if(edge->get_LCM() != lcm)
            {
                std::cout << "    PROBLEM: halfedge " << HID(edge) << " does not point to this LCM.\n";
                curve_problem = true;
            }
            if(edge->get_twin()->get_LCM() != lcm)
            {
                std::cout << "    PROBLEM: halfedge " << HID(edge->get_twin()) << ", twin of halfedge " << HID(edge) << ", does not point to this LCM.\n";
                curve_problem = true;
            }

            if(edge->get_next() == NULL)
            {
                std::cout << "    PROBLEM: halfedge " << HID(edge) << " has NULL next pointer.\n";
                curve_problem = true;
                break;
            }

            //find next edge in this curve
            edge = edge->get_next();
            while(edge->get_LCM() != lcm)
                edge = edge->get_twin()->get_next();

        }while(edge->get_origin()->get_theta() < HALF_PI);
    }//end curve loop

    //ignore halfedges on both sides of boundary
    start = halfedges[1];
    cur = start;
    do{
        edges_found_in_curves.insert(HID(cur));
        edges_found_in_curves.insert(HID(cur->get_twin()));

        if(cur->get_next() == NULL)
        {
            std::cout << "    PROBLEM: halfedge " << HID(cur) << " has NULL next pointer.\n";
            break;
        }
        cur = cur->get_next();
    }while(cur != start);

    //check if all edges were found
    all_edges_found = true;
    for(int i=0; i<halfedges.size(); i++)
    {
        if(edges_found_in_curves.find(i) == edges_found_in_curves.end())
        {
            std::cout << "  PROBLEM: halfedge " << i << " not found in any LCM curve";
            all_edges_found = false;
        }
    }
    if(all_edges_found)
        std::cout << "   ---All halfedges found in curves, as expected.\n";


    if(!curve_problem)
        std::cout << "   ---No problems detected among LCM curves.\n";
    else
        std::cout << "   ---Problems detected among LCM curves.\n";


    //check curves
    std::cout << "Checking order of vertices along right edge of the strip:\n";
    Halfedge* redge = halfedges[3];
    while(redge != halfedges[1])
    {
        std::cout << " r = " << redge->get_origin()->get_r() << "\n";
        redge = redge->get_next();
    }



}//end test_consistency()


