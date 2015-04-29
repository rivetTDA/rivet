/* implementation of Mesh class
 * Stores and manipulates the DCEL decomposition of the affine Grassmannian.
 */

#include "mesh.h"

#include <list>
#include <qdebug.h>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/metric_tsp_approx.hpp>


// Mesh constructor; sets up bounding box (with empty interior) for the affine Grassmannian
Mesh::Mesh(const std::vector<double> &xg, const std::vector<exact> &xe, const std::vector<double> &yg, const std::vector<exact> &ye, int v) :
    x_grades(xg), x_exact(xe), y_grades(yg), y_exact(ye),
    INFTY(std::numeric_limits<double>::infinity()),
    verbosity(v)
{
    //create vertices
    vertices.push_back( new Vertex(0, INFTY) );         //index 0
    vertices.push_back( new Vertex(INFTY, INFTY) );     //index 1
    vertices.push_back( new Vertex(INFTY, -INFTY) );    //index 2
    vertices.push_back( new Vertex(0, -INFTY) );        //index 3

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

//destructor
Mesh::~Mesh()
{
    for(std::vector<Vertex*>::iterator it = vertices.begin(); it != vertices.end(); ++it)
        delete (*it);

    for(std::vector<Halfedge*>::iterator it = halfedges.begin(); it != halfedges.end(); ++it)
        delete (*it);

    for(std::vector<Face*>::iterator it = faces.begin(); it != faces.end(); ++it)
        delete (*it);

    for(std::set<LCM*>::iterator it = all_lcms.begin(); it != all_lcms.end(); ++it)
        delete (*it);
}//end destructor

//builds the DCEL arrangement, computes and stores persistence data
//also stores ordered list of xi support points in the supplied vector
void Mesh::build_arrangement(MultiBetti& mb, std::vector<xiPoint>& xi_pts)
{
    //precondition: the constructor has already created the boundary of the arrangement

    //BarcodeCalculator object is able to do the calculations necessary for finding anchors and computing barcode templates
    BarcodeCalculator bcalc(this, mb, xi_pts);

    //first, compute anchors and store them in the vector Mesh::all_lcms
    bcalc.find_anchors();

    //now that we have all the anchors, we can build the interior of the arrangement
    build_interior();

    //now that the arrangement is constructed, we can find a path -- NOTE: path starts with a (near-vertical) line to the right of all multigrades
    std::vector<Halfedge*> path;
    find_path(path);

    //finally, we can traverse the path, computing and storing a barcode template in each 2-cell
    bcalc.store_barcodes(path);

}//end build_arrangement()

//function to build the arrangement using a version of the Bentley-Ottmann algorithm, given all LCMs
//preconditions:
//		all LCMs are in a list, ordered by lcm_left_comparator
//		boundary of the mesh is created (as in the mesh constructor)
void Mesh::build_interior()
{
    if(verbosity >= 5)
    {
        std::cout << "BUILDING ARRANGEMENT:  LCMs sorted for left edge of strip: ";
        for(std::set<LCM*, LCM_LeftComparator>::iterator it = all_lcms.begin(); it != all_lcms.end(); ++it)
            std::cout << "(" << (*it)->get_x() << "," << (*it)->get_y() << ") ";
        std::cout << "\n";
    }

    // DATA STRUCTURES

    //data structure for ordered list of lines
    std::vector<Halfedge*> lines;
    lines.reserve(all_lcms.size());

    //data structure for queue of future intersections
    std::priority_queue< Crossing*, std::vector<Crossing*>, CrossingComparator > crossings;

    //data structure for all pairs of LCMs whose potential crossings have been considered
    typedef std::pair<LCM*,LCM*> LCM_pair;
    std::set< LCM_pair > considered_pairs;

  // PART 1: INSERT VERTICES AND EDGES ALONG LEFT EDGE OF THE STRIP
    if(verbosity >= 5) { std::cout << "PART 1: LEFT EDGE OF STRIP\n"; }

    //for each LCM, create vertex and associated halfedges, anchored on the left edge of the strip
    Halfedge* leftedge = bottomleft;
    unsigned prev_y = std::numeric_limits<unsigned>::max();
    for(std::set<LCM*, LCM_LeftComparator>::iterator it = all_lcms.begin(); it != all_lcms.end(); ++it)
    {
        LCM* cur_lcm = *it;

        if(verbosity >= 6) { std::cout << "  Processing LCM " << cur_lcm << " at (" << cur_lcm->get_x() << "," << cur_lcm->get_y() << "), "; }

        if(cur_lcm->get_y() != prev_y)	//then create new vertex
        {
            double dual_point_y_coord = -1*y_grades[cur_lcm->get_y()];  //point-line duality requires multiplying by -1
            leftedge = insert_vertex(leftedge, 0, dual_point_y_coord);    //set leftedge to edge that will follow the new edge
            prev_y = cur_lcm->get_y();  //remember the discrete y-index
        }

        //now insert new edge at origin vertex of leftedge
        Halfedge* new_edge = create_edge_left(leftedge, cur_lcm);

        //remember Halfedge corresponding to this LCM
        lines.push_back(new_edge);

        //remember relative position of this LCM
        cur_lcm->set_position(lines.size() - 1);

        //remember line associated with this LCM
        cur_lcm->set_line(new_edge);
    }
    if(verbosity >= 6) { std::cout << "\n"; }

    //for each pair of consecutive lines, if they intersect, store the intersection
    for(unsigned i = 0; i < lines.size() - 1; i++)
    {
        LCM* a = lines[i]->get_LCM();
        LCM* b = lines[i+1]->get_LCM();
        if( a->comparable(b) )    //then the LCMs are (strongly) comparable, so we must store an intersection
            crossings.push(new Crossing(a, b, this));

        //remember that we have now considered this intersection
        considered_pairs.insert(LCM_pair(a,b));
    }

    //TESTING
//    std::cout << "THE PRIORITY QUEUE CONTAINS:\n";
//    while(!crossings.empty())
//    {
//        //get the next intersection from the queue
//        Crossing* cur = crossings.top();
//        crossings.pop();
//        std::cout << " intersection: LCM " << cur->a << " (" << cur->a->get_position() << "), LCM " << cur->b << " (" << cur->b->get_position() << ") intersect at x = " << cur->x << "\n";
//    }


  // PART 2: PROCESS INTERIOR INTERSECTIONS
    //    order: x left to right; for a given x, then y low to high
    if(verbosity >= 5) { std::cout << "PART 2: PROCESSING INTERIOR INTERSECTIONS\n"; }

    int status_counter = 0;
    int status_interval = 10000;    //controls frequency of output

    //current position of sweep line
    Crossing* sweep = NULL;

    while(!crossings.empty())
    {
        //get the next intersection from the queue
        Crossing* cur = crossings.top();
        crossings.pop();

        //process the intersection
        sweep = cur;
        unsigned first_pos = cur->a->get_position();   //most recent edge in the curve corresponding to LCM a
        unsigned last_pos = cur->b->get_position();   //most recent edge in the curve corresponding to LCM b

        if(verbosity >= 6) { std::cout << " next intersection: LCM " << cur->a << " (pos " << first_pos << "), LCM " << cur->b << " (pos " << last_pos << ")\n"; }

        if(last_pos != first_pos + 1)
        {
            std::cerr << "ERROR: intersection between non-consecutive curves [1]: x = " << sweep->x << "\n";
            throw std::exception();
        }

        //find out if more than two curves intersect at this point
        while( !crossings.empty() && sweep->x_equal(crossings.top()) && (cur->b == crossings.top()->a) )
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

        //compute y-coordinate of intersection
        double intersect_y = x_grades[sweep->a->get_x()]*(sweep->x) - y_grades[sweep->a->get_y()];

        if(verbosity >= 6) { std::cout << "  found intersection between " << (last_pos - first_pos + 1) << " edges at x = " << sweep->x << ", y = " << intersect_y << "\n"; }

        //create new vertex
        Vertex* new_vertex = new Vertex(sweep->x, intersect_y);
        vertices.push_back(new_vertex);

        //anchor edges to vertex and create new face(s) and edges	//TODO: check this!!!
        Halfedge* prev_new_edge = NULL;                 //necessary to remember the previous new edge at each interation of the loop
        Halfedge* first_incoming = lines[first_pos];   //necessary to remember the first incoming edge
        Halfedge* prev_incoming = NULL;                 //necessary to remember the previous incoming edge at each iteration of the loop
        for(unsigned cur_pos = first_pos; cur_pos <= last_pos; cur_pos++)
        {
            //anchor edge to vertex
            Halfedge* incoming = lines[cur_pos];
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
                new_twin->set_next( lines[last_pos]->get_twin() );
                lines[last_pos]->get_twin()->set_prev(new_twin);

                new_twin->set_face( lines[last_pos]->get_twin()->get_face() );
            }
            else    //then this is not the first iteration of the loop, so close up a face and create a new face
            {
                incoming->set_next( prev_incoming->get_twin() );
                incoming->get_next()->set_prev(incoming);

                Face* new_face = new Face(new_twin);
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

            //update lines vector
            lines[cur_pos] = new_edge; //the portion of this vector [first_pos, last_pos] must be reversed after this loop is finished!

            //remember position of this LCM
            new_edge->get_LCM()->set_position(last_pos - (cur_pos - first_pos));
//            if(verbosity >= 6) { std::cout << "    - set LCM " << new_edge->get_LCM() << " to position " << (last_pos - (cur_pos - first_pos)) << "\n"; }
        }

        //update lines vector: flip portion of vector [first_pos, last_pos]
        for(unsigned i = 0; i < (last_pos - first_pos + 1)/2; i++)
        {
            //swap curves[first_pos + i] and curves[last_pos - i]
            Halfedge* temp = lines[first_pos + i];
            lines[first_pos + i] = lines[last_pos - i];
            lines[last_pos - i] = temp;
        }

        //TESTING
//        std::cout << "      TESTING: ";
//        for(unsigned i = first_pos; i<= last_pos; i++)
//            std::cout << i << ": " << curves[i]->get_LCM() << "; ";
//        std::cout << "\n";

        //find new intersections and add them to intersections queue
        if(first_pos > 0)   //then consider lower intersection
        {
            LCM* a = lines[first_pos-1]->get_LCM();
            LCM* b = lines[first_pos]->get_LCM();

            if(considered_pairs.find(LCM_pair(a,b)) == considered_pairs.end()
                    && considered_pairs.find(LCM_pair(b,a)) == considered_pairs.end() )	//then this pair has not yet been considered
            {
                considered_pairs.insert(LCM_pair(a,b));
                if( a->comparable(b) )    //then the LCMs are (strongly) comparable, so we have found an intersection to store
                    crossings.push(new Crossing(a, b, this));
            }
        }

        if(last_pos + 1 < lines.size())    //then consider upper intersection
        {
            LCM* a = lines[last_pos]->get_LCM();
            LCM* b = lines[last_pos+1]->get_LCM();

            if( considered_pairs.find(LCM_pair(a,b)) == considered_pairs.end()
                    && considered_pairs.find(LCM_pair(b,a)) == considered_pairs.end() )	//then this pair has not yet been considered
            {
                considered_pairs.insert(LCM_pair(a,b));
                if( a->comparable(b) )    //then the LCMs are (strongly) comparable, so we have found an intersection to store
                    crossings.push(new Crossing(a, b, this));
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

  // PART 3: INSERT VERTICES ON RIGHT EDGE OF STRIP AND CONNECT EDGES           TODO: FIX THIS PART!!!!!!!!!!!!!!!!!!!!!!!!!!
    if(verbosity >= 5) { std::cout << "PART 3: RIGHT EDGE OF THE STRIP\n"; }

    Halfedge* rightedge = bottomright; //need a reference halfedge along the right side of the strip
    unsigned cur_x = 0;      //keep track of x-coordinate of last LCM whose line was connected to right edge (x-coordinate of LCM is slope of line)

    //connect each line to the right edge of the arrangement (at x = INFTY)
    //    requires creating a vertex for each unique slope (i.e. LCM x-coordinate)
    //    lines that have the same slope m are "tied together" at the same vertex, with coordinates (INFTY, INFTY)
    for(unsigned cur_pos = 0; cur_pos < lines.size(); cur_pos++)
    {
        Halfedge* incoming = lines[cur_pos];
        LCM* cur_lcm = incoming->get_LCM();

        if(cur_lcm->get_x() > cur_x || cur_pos == 0)    //then create a new vertex for this line
        {
            cur_x = cur_lcm->get_x();
            rightedge = insert_vertex( rightedge, INFTY, INFTY );
        }
        else    //no new vertex required, but update previous entry for vertical-line queries
            vertical_line_query_list.pop_back();

        //store Halfedge for vertical-line queries
        vertical_line_query_list.push_back(incoming->get_twin());

        //connect current line to the most-recently-inserted vertex
        Vertex* cur_vertex = rightedge->get_origin();
        incoming->get_twin()->set_origin(cur_vertex);

        //update halfedge pointers
        incoming->set_next(rightedge->get_twin()->get_next());
        incoming->get_next()->set_prev(incoming);

        incoming->get_next()->set_face(incoming->get_face());   //only necessary if incoming->get_next() is along the right side of the strip

        incoming->get_twin()->set_prev(rightedge->get_twin());
        rightedge->get_twin()->set_next(incoming->get_twin());

        rightedge->get_twin()->set_face(incoming->get_twin()->get_face());
    }
}//end build_interior()


//inserts a new vertex on the specified edge, with the specified coordinates, and updates all relevant pointers
//  i.e. new vertex is between initial and termainal points of the specified edge
//returns pointer to a new halfedge, whose initial point is the new vertex, and that follows the specified edge around its face
Halfedge* Mesh:: insert_vertex(Halfedge* edge, double x, double y)
{
	//create new vertex
    Vertex* new_vertex = new Vertex(x, y);
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

//creates the first pair of Halfedges in an LCM line, anchored on the left edge of the strip at origin of specified edge
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
    Face* new_face = new Face(new_edge);
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

//finds a pseudo-optimal path through all 2-cells of the arrangement
// path consists of a vector of Halfedges
// at each step of the path, the Halfedge points to the LCM being crossed and the 2-cell (Face) being entered
void Mesh::find_path(std::vector<Halfedge*>& pathvec)
{
  // PART 1: BUILD THE DUAL GRAPH OF THE ARRANGEMENT

    typedef boost::property<boost::edge_weight_t, int> EdgeWeightProperty;
    typedef boost::adjacency_list< boost::vecS, boost::vecS, boost::undirectedS,
                                   boost::no_property, EdgeWeightProperty > Graph;  //TODO: probably listS is a better choice than vecS, but I don't know how to make the adjacency_list work with listS
    Graph dual_graph;

    //make a map for reverse-lookup of face indexes by face pointers -- Is this really necessary? Can I avoid doing this?
    std::map<Face*, unsigned> face_indexes;
    for(unsigned i=0; i<faces.size(); i++)
        face_indexes.insert( std::pair<Face*, unsigned>(faces[i], i));

    //loop over all faces
    for(unsigned i=0; i<faces.size(); i++)
    {
        //consider all neighbors of this faces
        Halfedge* boundary = (faces[i])->get_boundary();
        Halfedge* current = boundary;
        do
        {
            //find index of neighbor
            Face* neighbor = current->get_twin()->get_face();
            if(neighbor != NULL)
            {
                std::map<Face*, unsigned>::iterator it = face_indexes.find(neighbor);
                unsigned j = it->second;

                //if i < j, then create an (undirected) edge between these faces
                if(i < j)
                    boost::add_edge(i, j, 1, dual_graph);   //for now, all edges have unit weight
                ///TODO: WEIGHT EDGES BY NUMBER OF SIMPLICES THAT SWAP FOR EACH LCM
            }
            //move to the next neighbor
            current = current->get_next();
        }while(current != boundary);
    }

    //TESTING -- print the edges in the dual graph
    if(verbosity >= 2)
    {
        std::cout << "EDGES IN THE DUAL GRAPH OF THE ARRANGEMENT: \n";
        typedef boost::graph_traits<Graph>::edge_iterator edge_iterator;
        std::pair<edge_iterator, edge_iterator> ei = boost::edges(dual_graph);
        for(edge_iterator it = ei.first; it != ei.second; ++it)
            std::cout << "  (" << boost::source(*it, dual_graph) << ", " << boost::target(*it, dual_graph) << ")\n";
    }


  // PART 2: FIND A MINIMAL SPANNING TREE

    typedef boost::graph_traits<Graph>::edge_descriptor Edge;
    std::vector<Edge> spanning_tree_edges;
    boost::kruskal_minimum_spanning_tree(dual_graph, std::back_inserter(spanning_tree_edges));

    if(verbosity >= 2)
    {
        std::cout << "num MST edges: " << spanning_tree_edges.size() << "\n";
        for(unsigned i=0; i<spanning_tree_edges.size(); i++)
            std::cout << "  (" << boost::source(spanning_tree_edges[i], dual_graph) << ", " << boost::target(spanning_tree_edges[i], dual_graph) << ")\n";
    }

//  // PART 2-ALTERNATE: FIND A HAMILTONIAN TOUR
//    ///This doesn't serve our purposes.
//    ///  It doesn't start at the cell representing a vertical line, but that could be fixed by using metric_tsp_approx_from_vertex(...).
//    ///  Worse, it always produces a cycle and doesn't give the intermediate nodes between non-adjacent nodes that appear consecutively in the tour.

//    typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
//    std::vector<Vertex> tsp_vertices;
//    boost::metric_tsp_approx_tour(dual_graph, std::back_inserter(tsp_vertices));

//    std::cout << "num TSP vertices: " << tsp_vertices.size() << "\n";
//    for(unsigned i=0; i<tsp_vertices.size(); i++)
//        std::cout << "  " << tsp_vertices[i] << "\n";

  // PART 3: CONVERT THE OUTPUT OF PART 2 TO A PATH

    //organize the edges of the minimal spanning tree so that we can traverse the tree
    std::vector< std::set<unsigned> > adjacencies(faces.size(), std::set<unsigned>());  //this will store all adjacency relationships in the spanning tree
    for(unsigned i=0; i<spanning_tree_edges.size(); i++)
    {
        unsigned a = boost::source(spanning_tree_edges[i], dual_graph);
        unsigned b = boost::target(spanning_tree_edges[i], dual_graph);

        (adjacencies[a]).insert(b);
        (adjacencies[b]).insert(a);
    }

    //traverse the tree
    //at each step in the traversal, append the corresponding Halfedge* to pathvec
    //make sure to start at the proper node (2-cell)
    Face* initial_cell = topleft->get_twin()->get_face();
    unsigned start = (face_indexes.find(initial_cell))->second;

    find_subpath(start, adjacencies, pathvec, false);

    //TESTING -- PRINT PATH
    if(verbosity >= 2)
    {
        std::cout << "PATH: " << start << ", ";
        for(int i=0; i<pathvec.size(); i++)
            std::cout << (face_indexes.find((pathvec[i])->get_face()))->second << ", ";
        std::cout << "\n";
    }



}//end find_path()

//recursive method to build part of the path
//return_path == TRUE iff we want the path to return to the current node after traversing all of its children
void Mesh::find_subpath(unsigned& cur_node, std::vector< std::set<unsigned> >& adj, std::vector<Halfedge*>& pathvec, bool return_path)
{

    while(!adj[cur_node].empty())   //cur_node still has children to traverse
    {
        std::set<unsigned>::iterator it = adj[cur_node].begin();
        unsigned next_node = *it;

        //find the next Halfedge and append it to pathvec
        Halfedge* cur_edge = (faces[cur_node])->get_boundary();
        while(cur_edge->get_twin()->get_face() != faces[next_node])     //do we really have to search for the correct edge???
        {
            cur_edge = cur_edge->get_next();

            if(cur_edge == (faces[cur_node])->get_boundary())   //THIS SHOULD NEVER HAPPEN
            {
                std::cerr << "ERROR: cannot find edge between 2-cells " << cur_node << " and " << next_node << "\n";
                throw std::exception();
            }
        }
        pathvec.push_back(cur_edge->get_twin());

        //remove adjacencies that have been already processed
        adj[cur_node].erase(it);        //removes (cur_node, next_node)
        adj[next_node].erase(cur_node); //removes (next_node, cur_node)

        //do we need to return to this node?
        bool return_here = return_path || !adj[cur_node].empty();

        //recurse through the next node
        find_subpath(next_node, adj, pathvec, return_here);

        //if we will return to this node, then add reverse Halfedge to pathvec
        if(return_here)
            pathvec.push_back(cur_edge);

    }//end while

}//end find_subpath()


//returns barcode template associated with the specified line (point)
BarcodeTemplate& Mesh::get_barcode_template(double degrees, double offset)
{
    if(degrees == 90) //then line is vertical
    {
        Face* cell = find_vertical_line(-1*offset); //multiply by -1 to correct for orientation of offset

        ///TODO: store some point/cell to seed the next query

        std::cout << " ||| vertical line found in cell " << FID(cell) << "\n";
        return cell->get_barcode();
    }

    if(degrees == 0) //then line is horizontal
    {
        Face* cell = topleft->get_twin()->get_face();    //default
        LCM* lcm = find_least_upper_LCM(offset);

        if(lcm != NULL)
            cell = lcm->get_line()->get_face();

        ///TODO: store some point/cell to seed the next query

        std::cout << " --- horizontal line found in cell " << FID(cell) << "\n";
        return cell->get_barcode();
    }

    //else: the line is neither horizontal nor vertical
    double radians = degrees * 3.14159265/180;
    double slope = tan(radians);
    double intercept = offset/cos(radians);

    Face* cell = find_point(slope, -1*intercept);   //multiply by -1 for point-line duality
        ///TODO: REPLACE THIS WITH A SEEDED SEARCH

    ///TODO: store seed

    return cell->get_barcode();  ////FIX THIS!!!
}//end get_barcode_template()

//finds the first LCM that intersects the left edge of the arrangement at a point not less than the specified y-coordinate
//  if no such LCM, returns NULL
LCM* Mesh::find_least_upper_LCM(double y_coord)
{
    //binary search to find greatest y-grade not greater than than y_coord
    unsigned best = 0;
    if(y_grades.size() >= 1 && y_grades[0] <= y_coord)
    {
        //binary search the vector y_grades
        unsigned min = 0;
        unsigned max = y_grades.size() - 1;

        while(max >= min)
        {
            unsigned mid = (max + min)/2;

            if(y_grades[mid] <= y_coord)    //found a lower bound, but search upper subarray for a better lower bound
            {
                best = mid;
                min = mid + 1;
            }
            else    //search lower subarray
                max = mid - 1;
        }
    }
    else
        return NULL;

    //if we get here, then y_grades[best] is the greatest y-grade not greater than y_coord
    //now find LCM whose line intersects the left edge of the arrangement lowest, but not below y_grade[best]
    unsigned int zero = 0;  //disambiguate the following function call
    LCM* test = new LCM(zero, best);
    std::set<LCM*, LCM_LeftComparator>::iterator it = all_lcms.lower_bound(test);
    delete test;

    if(it == all_lcms.end())    //not found
    {
        return NULL;
    }
    //else
    return *it;
}//end find_least_upper_LCM()

//finds the (unbounded) cell associated to dual point of the vertical line with the given x-coordinate
//  i.e. finds the Halfedge whose LCM x-coordinate is the largest such coordinate not larger than than x_coord; returns the Face corresponding to that Halfedge
Face* Mesh::find_vertical_line(double x_coord)
{
    //is there an LCM with x-coordinate not greater than x_coord?
    if(vertical_line_query_list.size() >= 1 && x_grades[ vertical_line_query_list[0]->get_LCM()->get_x() ] <= x_coord)
    {
        //binary search the vertical line query list
        unsigned min = 0;
        unsigned max = vertical_line_query_list.size() - 1;
        unsigned best = 0;

        while(max >= min)
        {
            unsigned mid = (max + min)/2;
            LCM* test = vertical_line_query_list[mid]->get_LCM();

            if(x_grades[test->get_x()] <= x_coord)    //found a lower bound, but search upper subarray for a better lower bound
            {
                best = mid;
                min = mid + 1;
            }
            else    //search lower subarray
                max = mid - 1;
        }

        //testing
        std::cout << "----vertical line search: found LCM with x-coordinate " << vertical_line_query_list[best]->get_LCM()->get_x() << "\n";

        return vertical_line_query_list[best]->get_face();
    }

    //if we get here, then either there are no LCMs or x_coord is less than the x-coordinates of all LCMs
    std::cout << "----vertical line search: returning lowest face\n";
    return bottomright->get_twin()->get_face();

}//end find_vertical_line()

//find a 2-cell containing the specified point
Face* Mesh::find_point(double x_coord, double y_coord)
{
    //start on the left edge of the arrangement, at the correct y-coordinate
    LCM* start = find_least_upper_LCM(-1*y_coord);

    Face* cell = NULL;		//will later point to the cell containing the specified point
    Halfedge* finger = NULL;	//for use in finding the cell

    if(start == NULL)	//then starting point is in the top (unbounded) cell
    {
        finger = topleft->get_twin()->get_next();   //this is the top edge of the top cell (at y=infty)
        if(verbosity >= 8) { std::cout << "  Starting in top (unbounded) cell\n"; }
    }
    else
    {
        finger = start->get_line();
        if(verbosity >= 8) { std::cout << "  Reference LCM: (" << x_grades[start->get_x()] << ", " << y_grades[start->get_y()] << "); halfedge " << HID(finger) << ".\n"; }
    }

    while(cell == NULL) //while not found
    {
        if(verbosity >= 8) { std::cout << "  Considering cell " << FID(finger->get_face()) << ".\n"; }

        //find the edge of the current cell that crosses the horizontal line at y_coord
        Vertex* next_pt = finger->get_next()->get_origin();
        while(next_pt->get_y() > y_coord)
        {
            finger = finger->get_next();
            next_pt = finger->get_next()->get_origin();

            if(verbosity >= 8) { std::cout << "    --next point: (" << next_pt->get_x() << ", " << next_pt->get_y() << "); finger: " << HID(finger) << ".\n"; }
        }
        //now next_pt is at or below the horizontal line at y_coord
        //if (slope, intercept) is to the left of crossing point, then we have found the cell; otherwise, move to the adjacent cell

        if(verbosity >= 8) { std::cout << "    --found next_pt at (" << next_pt->get_x() << ", " << next_pt->get_y() << "); finger: " << HID(finger) << ".\n"; }

        if(next_pt->get_y() == y_coord) //then next_pt is on the horizontal line
        {
            if(next_pt->get_x() >= x_coord)	//found the cell
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
                LCM* temp = finger->get_LCM();
                double x_pos = ( y_coord + y_grades[temp->get_y()] )/x_grades[temp->get_x()];  ///TODO: ENSURE NO DIVISION BY ZERO! (shouldn't happen because distinct horizontal lines don't intersect)

                if(x_pos >= x_coord)	//found the cell
                {
                    cell = finger->get_face();
                }
                else	//move to adjacent cell
                {
                    finger = finger->get_twin();
                }
            }
        }//end while(cell not found)
    }//end else

    if(verbosity >= 3) { std::cout << "  -> Found point (" << x_coord << ", " << y_coord << ") in cell " << FID(cell) << ".\n"; }

    return cell;
}//end find_point()


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
        std::cout << "(" << cur.get_x() << ", " << cur.get_y() << ") halfedge " << HID(cur.get_line()) << "; ";
	}
	std::cout << "\n";
	
}//end print()

/********** functions for testing **********/

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

        Halfedge* edge = lcm->get_line();
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

        }while(edge->get_origin()->get_x() < INFTY);
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
        std::cout << " y = " << redge->get_origin()->get_y() << "\n";
        redge = redge->get_next();
    }



}//end test_consistency()



/********** the following objects and functions are for exact comparisons **********/

//Crossing constructor
//precondition: LCMs a and b must be comparable
Mesh::Crossing::Crossing(LCM* a, LCM* b, Mesh* m) : a(a), b(b), m(m)
{
    //store the x-coordinate of the crossing for fast (inexact) comparisons
    x = (m->y_grades[a->get_y()] - m->y_grades[b->get_y()])/(m->x_grades[a->get_x()] - m->x_grades[b->get_x()]);

    //TESTING ONLY
    std::cout << "  Crossing created for curves " << a->get_position() << " (LCM  " << a << ") and " << b->get_position() << " (LCM " << b << "), which intersect at x = " << x << "\n";
}

//returns true iff this Crossing has (exactly) the same x-coordinate as other Crossing
bool Mesh::Crossing::x_equal(const Crossing *other) const
{
    if(Mesh::almost_equal(x, other->x)) //then compare exact values
    {
        //find exact x-values
        exact x1 = (m->y_exact[a->get_y()] - m->y_exact[b->get_y()])/(m->x_exact[a->get_x()] - m->x_exact[b->get_x()]);
        exact x2 = (m->y_exact[other->a->get_y()] - m->y_exact[other->b->get_y()])/(m->x_exact[other->a->get_x()] - m->x_exact[other->b->get_x()]);

        return x1 == x2;
    }
    //otherwise, x-coordinates are not equal
    return false;
}

//CrossingComparator for ordering crossings: first by x (left to right); for a given x, then by y (low to high)
bool Mesh::CrossingComparator::operator()(const Crossing* c1, const Crossing* c2) const	//returns true if c1 comes after c2
{
    //TESTING
    if(c1->a->get_position() >= c1->b->get_position() || c2->a->get_position() >= c2->b->get_position())
    {
        std::cerr << "INVERTED CROSSING ERROR\n";
        std::cerr << "crossing 1 involves LCMS " << c1->a << " (pos " << c1->a->get_position() << ") and " << c1->b << " (pos " << c1->b->get_position() << "),";
        std::cerr << "crossing 2 involves LCMS " << c2->a << " (pos " << c2->a->get_position() << ") and " << c2->b << " (pos " << c2->b->get_position() << "),";
        throw std::exception();
    }
//    if(c1->a->get_position() == c2->a->get_position())
//    {
//        std::cerr << "ILLEGAL CROSSING ERROR\n";
//        std::cerr << "crossing 1 involves LCMS " << c1->a << " (pos " << c1->a->get_position() << ") and " << c1->b << " (pos " << c1->b->get_position() << "),";
//        std::cerr << "crossing 2 involves LCMS " << c2->a << " (pos " << c2->a->get_position() << ") and " << c2->b << " (pos " << c2->b->get_position() << "),";
//        throw std::exception();
//    }


    Mesh* m = c1->m;    //makes it easier to reference arrays in the mesh

    //now do the comparison
    //if the x-coordinates are nearly equal as double values, then compare exact values
    if(Mesh::almost_equal(c1->x, c2->x))
    {
        //find exact x-values
        exact x1 = (m->y_exact[c1->a->get_y()] - m->y_exact[c1->b->get_y()])/(m->x_exact[c1->a->get_x()] - m->x_exact[c1->b->get_x()]);
        exact x2 = (m->y_exact[c2->a->get_y()] - m->y_exact[c2->b->get_y()])/(m->x_exact[c2->a->get_x()] - m->x_exact[c2->b->get_x()]);

        //if the x-values are exactly equal, then consider the y-values
        if(x1 == x2)
        {
            //find the y-values
            double c1y = m->x_grades[c1->a->get_x()]*(c1->x) - m->y_grades[c1->a->get_y()];
            double c2y = m->x_grades[c2->a->get_x()]*(c2->x) - m->y_grades[c2->a->get_y()];

            //if the y-values are nearly equal as double values, then compare exact values
            if(Mesh::almost_equal(c1y, c2y))
            {
                //find exact y-values
                exact y1 = m->x_exact[c1->a->get_x()]*x1 - m->y_exact[c1->a->get_y()];
                exact y2 = m->x_exact[c2->a->get_x()]*x2 - m->y_exact[c2->a->get_y()];

                //if the y-values are exactly equal, then sort by relative position of the lines
                if(y1 == y2)
                    return c1->a->get_position() > c2->a->get_position();   //Is there a better way???

                //otherwise, the y-values are not equal
                return y1 > y2;
            }
            //otherwise, the y-values are not almost equal
            return c1y > c2y;
        }
        //otherwise, the x-values are not equal
        return x1 > x2;
    }
    //otherwise, the x-values are not almost equal
    return c1->x > c2->x;
}

//epsilon value for use in comparisons
double Mesh::epsilon = pow(2,-30);

//test whether two double values are almost equal (indicating that we should do exact comparison)
bool Mesh::almost_equal(const double a, const double b)
{
    double diff = std::abs(a - b);
    if(diff <= epsilon)
        return true;

    if(diff <= (std::abs(a) + std::abs(b))*epsilon)
        return true;
    return false;
}

