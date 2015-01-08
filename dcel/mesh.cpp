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
    verbosity(v),
    xi_matrix(x_grades.size(), y_grades.size())
{
	//create vertices
	vertices.push_back( new Vertex(0, INFTY) );		//index 0
    vertices.push_back( new Vertex(INFTY, INFTY) );	//index 1
    vertices.push_back( new Vertex(INFTY, -INFTY) );	//index 2
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

//builds the DCEL arrangement, computes and stores persistence data
//also stores ordered list of xi support points in the supplied vector
void Mesh::build_arrangement(MultiBetti& mb, std::vector<xiPoint>& xi_pts)
{
    //step 0: the constructor has already created the boundary of the arrangement

    //step 1: store xi support points, and compute and store LCMs
    store_xi_points(mb, xi_pts);

    //step 2: build the interior of the arrangement
    build_interior();

    //step 3: store persistence data in each 2-cell
    store_persistence_data(mb.bifiltration, mb.dimension);

}//end build_arrangement()


//stores xi support points from MultiBetti in Mesh (in a sparse array) and in the supplied vector
//    also computes and stores LCMs in Mesh; LCM curves will be created when build_arrangment() is called
void Mesh::store_xi_points(MultiBetti& mb, std::vector<xiPoint>& xi_pts)
{
    if(verbosity >= 2) { std::cout << "Storing xi support points in sparse matrix and computing LCMs\n"; }

    //store xi points in the sparse matrix and the vector
    xi_matrix.fill(mb, xi_pts);

    //get pointers to top entries in nonempty columns
    std::list<xiMatrixEntry*> nonempty_cols;
    for(unsigned i = 0; i < x_grades.size(); i++)
    {
        xiMatrixEntry* col_entry = xi_matrix.get_col(i); //top entry in row j, possibly NULL
        if(col_entry != NULL)
            nonempty_cols.push_front(col_entry);
    }

    //compute and store LCMs
    for(int j = y_grades.size() - 1; j >= 0; j--)  //loop through all rows, top to bottom
    {
        xiMatrixEntry* row_entry = xi_matrix.get_row(j); //rightmost entry in row j, possibly NULL

        std::list<xiMatrixEntry*>::iterator it = nonempty_cols.begin();
        while( it != nonempty_cols.end() )  //loop through all nonempty columns, right to left
        {
            if(row_entry == NULL)   //then there is nothing else in this row
                break;

            //check if there is a LCM in position (i,j)
            xiMatrixEntry* col_entry = *it;
            if(col_entry == NULL)
                 qDebug() << "ERROR in Mesh::store_xi_points() : NULL col_entry";
            if(row_entry != col_entry)  //then there is a non-weak LCM at (col_entry->x, row_entry->y)
            {
                all_lcms.insert(new LCM(col_entry, row_entry));
                if(verbosity >= 4) { std::cout << "  LCM (non-weak) found at (" << col_entry->x << ", " << row_entry->y << ")\n"; }
            }
            else    //then row_entry == col_entry, so there might be a weak LCM at (col_entry->x, row_entry->y), or there might be no LCM here
            {
                if(col_entry->down != NULL && row_entry->left != NULL)  //then there is a strong-AND-weak LCM
                {
                    all_lcms.insert(new LCM(col_entry, true));
                    if(verbosity >= 4) { std::cout << "  LCM (strong-and-weak) found at (" << col_entry->x << ", " << col_entry->y << ")\n"; }
                }
                else if(col_entry->down != NULL || row_entry->left != NULL)  //then there is a weak, non-strong LCM
                {
                    all_lcms.insert(new LCM(col_entry, false));
                    if(verbosity >= 4) { std::cout << "  LCM (weak, non-strong) found at (" << col_entry->x << ", " << col_entry->y << ")\n"; }
                }
            }

            //update cur_row_entry, if necessary
            if( row_entry->x == col_entry->x )  //then set row_entry to the next non-null entry in this row, if such entry exists
                row_entry = row_entry->left;

            //update nonempty_cols, if necessary
            if( col_entry->y == j )  //then replace this column entry with the next non-null entry in this column, if such entry exists
            {
                it = nonempty_cols.erase(it);   //NOTE: this advances the column iterator!

                if(col_entry->down != NULL)
                    nonempty_cols.insert(it, col_entry->down);   ///TODO: CHECK THIS!!!
            }
            else    //then advance the column iterator
                ++it;
        }//end column loop
    }//end row loop
}//end store_xi_points()

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
            leftedge = insert_vertex(leftedge, 0, y_grades[cur_lcm->get_y()]);    //set leftedge to edge that will follow the new edge
            prev_y = cur_lcm->get_y();  //remember the discrete y-index
        }

        //now insert new edge at origin vertex of leftedge
        Halfedge* new_edge = create_edge_left(leftedge, cur_lcm);

        //remember Halfedge corresponding to this LCM
        lines.push_back(new_edge);

        //remember relative position of this LCM
        cur_lcm->set_position(lines.size() - 1);

        //remember curve associated with this LCM --- THIS MIGHT BE UNNECESSARY AFTER TESTING IS COMPLETE!!!
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
    int status_interval = 10000;

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
    unsigned cur_x = 0;      //keep track of x-coordinate of last LCM whose line was connected to right edge

    //connect each line to the right edge of the arrangement (at x = INFTY)
    //    requires creating a vertex for each unique slope (i.e. LCM x-coordinate)
    //    lines that have the same slope m are "tied together" at the same vertex, with coordinates (INFTY, m)
    for(unsigned cur_pos = 0; cur_pos < lines.size(); cur_pos++)
    {
        LCM* cur_lcm = lines[cur_pos]->get_LCM();

        if(cur_lcm->get_x() > cur_x || cur_pos == 0)    //then insert a new vertex
        {
            cur_x = cur_lcm->get_x();
            rightedge = insert_vertex( rightedge, INFTY, x_grades[cur_x] );
        }
        //otherwise, connect current line to the previously-inserted vertex

        Vertex* cur_vertex = rightedge->get_origin();

        //anchor halfedge to vertex
        Halfedge* incoming = lines[cur_pos];
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
Halfedge* Mesh::insert_vertex(Halfedge* edge, double x, double y)
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


//associates a discrete barcode to each 2-cell of the arrangement (IN PROGRESS)
void Mesh::store_persistence_data(SimplexTree* bifiltration, int dim)
{
  // PARTS 1 and 2: FIND A PATH THROUGH ALL 2-CELLS OF THE ARRANGEMENT
    std::vector<Halfedge*> path;    ///TODO: is this what we want?
    find_path(path);


  // PART 3: INITIAL PERSISTENCE COMPUTATION

    //get multi-grade data in each dimension
    if(verbosity >= 4) { std::cout << "Mapping low simplices:\n"; }
    IndexMatrix* ind_low = bifiltration->get_index_mx(dim);    //can we improve this with something more efficient than IndexMatrix?
    store_multigrades(ind_low, true);
    delete ind_low;

    if(verbosity >= 4) { std::cout << "Mapping high simplices:\n"; }
    IndexMatrix* ind_high = bifiltration->get_index_mx(dim + 1);    //again, could be improved?
    store_multigrades(ind_high, false);
    delete ind_high;


    //get boundary matrices --- need extra structure to support vineyard updates???
//    MapMatrix* bdry1 = bifiltration->get_boundary_mx(dim);
//    MapMatrix* bdry2 = bifiltration->get_boundary_mx(dim + 1);


    ///TODO: reduce matrices and store discrete barcode in initial cell



  // PART 4: TRAVERSE THE HAMILTONIAN PATH AND DO VINEYARD UPDATES

    //note: tsp_vertices[0] represents a near-vertical line to the right of all multigrades

    //traverse the path
    for(unsigned i=1; i<tsp_vertices.size(); i++)
    {
        //determine which LCM is represented by this edge
        LCM* cur_lcm = ......;  ///TODO: FINISH THIS!

        //get equivalence classes for this LCM
        xiMatrixEntry* down = cur_lcm->get_down();
        xiMatrixEntry* left = cur_lcm->get_left();

        //if this is a strong LCM, then swap simplices
        ///TODO: IMPLEMENT LAZY SWAPPING!
        if(left != NULL) //then this is a strong LCM and some simplices swap
        {
            if(down == NULL)    //then this is also a weak LCM
            {
                down = left->down;
                left = left->left;
            }//now down and left are correct (and should not be NULL)

            if(down->high_index <= left->high_index && down->low_index <= left->low_index) //then LCM is crossed from below to above
            {
                //get column indexes (so we know which columns to move)
                unsigned low_col = down->low_index;   //rightmost column index of low simplices for this equivalence class
                unsigned high_col = down->high_index; //rightmost column index of high simplices for this equivalence class

                //set column indexes for the "down" class to their final position
                down->low_index = left->low_index;
                down->high_index = left->high_index;

                //loop over all xiMatrixEntrys in the "down" equivalence class
                while(down != NULL)
                {
                    //move all "low" simplices for this xiMatrixEntry
                    std::list<Multigrade*>::iterator it = down->low_simplices.begin();
                    while(it != down->low_simplices.end())
                    {
                        Multigrade* cur_grade = *it;

                        if(cur_grade->x > left->x)  //then move columns at cur_grade past columns at xiMatrixEntry left; map F does not change
                        {
                            move_low_columns(low_col, cur_grade->num_cols, left->low_index);
                            left->low_index -= cur_grade->num_cols;
                            ++it;
                        }
                        else    //then determine where these columns map to under F
                        {
                            xiMatrixEntry* target = left;
                            unsigned target_col = left->low_index - left->low_count;
                            while( (target->left() != NULL) && (cur_grade->x <= target->left()->x) )
                            {
                                target = target->left();
                                target_col -= target->low_count;
                            }

                            //associate cur_grade with target
                            cur_grade->xi_entry = target;
                            target->insert_multigrade(cur_grade, true);
                            low_simplices.erase(it);    //NOTE: advances the iterator!!!

                            //if target is not the leftmost entry in its equivalence class, then move columns at cur_grade to the block of columns for target
                            if(target->left() != NULL)
                                move_low_columns(low_col, cur_grade->num_cols, target_col);

                            //update column counts
                            down->low_count -= cur_grade->num_cols;
                            target->low_count += cur_grade->num_cols;
                        }

                        //update column index
                        low_col -= cur_grade->num_cols;
                    }//end low simplex loop

                    //move all "high" simplices for this xiMatrixEntry
                    ///TODO: FINISH!!! this is almost identical to the above loop for "low" simplices
                    for(std::list<Multigrade*>::iterator it = first->high_simplices.begin(); it != first->high_simplices.end(); ++it)
                    {
                        //determine where columns for this multigrade must move to


                        //call a function to move block of columns to its new position


                    }//end low simplex loop

                    //advance to the next xiMatrixEntry in the "down" equivalence class
                    down = down->down();
                }//end loop over "down" class




            }
            else    //then LCM is crossed form above to below
            {
                ///TODO: FINISH!!! this is dual to the above case!!!
            }


        }//end if

        //if this is a weak LCM, then we still have to split or merge equivalence classes


        //if this cell does not yet have a discrete barcode, then store the discrete barcode here


    }//end path traversal


}//end build_persistence_data()

//finds a pseudo-optimal path through all 2-cells of the arrangement
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
            }
            //move to the next neighbor
            current = current->get_next();
        }while(current != boundary);
    }

    //TESTING -- print the edges in the dual graph
    std::cout << "EDGES IN THE DUAL GRAPH OF THE ARRANGEMENT: \n";
    typedef boost::graph_traits<Graph>::edge_iterator edge_iterator;
    std::pair<edge_iterator, edge_iterator> ei = boost::edges(dual_graph);
    for(edge_iterator it = ei.first; it != ei.second; ++it)
        std::cout << "  (" << boost::source(*it, dual_graph) << ", " << boost::target(*it, dual_graph) << ")\n";


  // PART 2a: FIND A MINIMAL SPANNING TREE

    typedef boost::graph_traits<Graph>::edge_descriptor Edge;
    std::vector<Edge> spanning_tree_edges;
    boost::kruskal_minimum_spanning_tree(dual_graph, std::back_inserter(spanning_tree_edges));

    std::cout << "num MST edges: " << spanning_tree_edges.size() << "\n";
    for(unsigned i=0; i<spanning_tree_edges.size(); i++)
        std::cout << "  (" << boost::source(spanning_tree_edges[i], dual_graph) << ", " << boost::target(spanning_tree_edges[i], dual_graph) << ")\n";


//  // PART 2b: FIND A HAMILTONIAN TOUR
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

    //traverse the tree
    //at each step in the traversal, append the corresponding Halfedge* to pathvec
    //make sure to start at the proper node (2-cell)



}//end find_path()

//moves a block of n columns, the rightmost of which is column s, to a new position following column t (NOTE: assumes s <= t)
///TODO: FINISH THIS!!! for now, it just prints transpositions to std::cout
void Mesh::move_low_columns(unsigned s, unsigned n, unsigned t)
{
    std::cout << "Transpositions for low simplices: ";
    for(unsigned c=0; c<n; c++) //move column that starts at s-c
    {
        for(unsigned i=s; i<t;i++)
        {
            unsigned a = i-c;
            std::cout << "(" << a << "," << (a+1) << ")";
        }
    }
    std::cout << "\n";
}//end move_low_columns()


//stores multigrade info for the persistence computations
//  low is true for simplices of dimension hom_dim, false for simplices of dimension hom_dim+1
///TODO: ALSO NEED TO BUILD A BOUNDARY MATRIX WITH THE PROPER ORDER OF SIMPLICES
/// e.g. this method sends a list of xiMatrixEntries to SimplexTree, which builds a boundary matrix based on the list
void Mesh::store_multigrades(IndexMatrix* ind, bool low)
{
    //initialize linked list to track the "frontier"
    typedef std::list<xiMatrixEntry*> Frontier;
    Frontier frontier;

    //loop through rows of xiSupportMatrix, from top to bottom
    for(int y = ind->height() - 1; y >= 0; y--)
    {
        //update the frontier for row y
        xiMatrixEntry* cur = xi_matrix.get_row(y);
        if(cur != NULL)
        {
            //advance an iterator to the first entry that is not right of cur
            Frontier::iterator it = frontier.begin();
            while( it != frontier.end() && (*it)->x > cur->x )
                ++it;

            //erase any entries from this position to the end of the frontier
            frontier.erase(it, frontier.end());

            //insert cur at the end of the frontier
            frontier.push_back(cur);

            //now add all other non-null entries in this row to the frontier
            cur = cur->left;
            while(cur != NULL)
            {
                frontier.push_back(cur);
                cur = cur->left;
            }
        }

        //store all multigrades and simplices whose y-grade is y
        Frontier::iterator it = frontier.begin();
        for(int x = ind->width() - 1; x >= 0; x--)
        {
            //get range of column indexes for simplices at multigrade (x,y)
            int last_col = ind->get(y, x);  //arguments are row, then column
            int first_col = -1;
            if(x > 0)
                first_col = ind->get(y, x-1);
            else if(y > 0)
                first_col = ind->get(y-1, ind->width()-1);

            //if there are any simplices at (x,y), then add multigrade (x,y)
            if(last_col > first_col)
            {
                //if the frontier is empty or if x is to the right of the first element, then map multigrade (x,y) to infinity
                if( it == frontier.end() || (*it)->x < x )    //NOTE: if iterator has advanced from frontier.begin(), then it MUST be the case that x < (*it)->x
                {
//                    qDebug() << "      processing columns" << first_col << "to" << last_col << ": map to infinity";
                    xi_matrix.get_infinity()->add_multigrade(x, y, last_col - first_col, low);
                    if(verbosity >= 4) { std::cout << "    simplices at (" << x << ", " << y << "), in columns " << (first_col + 1) << " to " << last_col << ", mapped to infinity\n"; }
                }
                else    //then map multigrade (x,y) to the last element of the frontier such that (*it)->x >= x
                {
//                    qDebug() << "      processing columns" << first_col << "to" << last_col << ": map to finite point";
                    //advance the iterator to the first element of the frontier such that (*it)->x < x
                    while( it != frontier.end() && (*it)->x >= x )
                        ++it;

                    //back up one position, to the last element of the frontier such that (*it)->x >= x
                    --it;

                    //now map the multigrade to the element given by the iterator
                    (*it)->add_multigrade(x, y, last_col - first_col, low);
                    if(verbosity >= 4) { std::cout << "    simplices at (" << x << ", " << y << "), in columns " << (first_col + 1) << " to " << last_col << ", mapped to xi support point (" << (*it)->x << ", " << (*it)->y << ")\n"; }
                }
            }
        }//end x loop
    }//end y loop

}//end store_multigrades()


//returns a persistence diagram associated with the specified point
//uses a naive point-location algorithm to find the cell containing the point
PersistenceData* Mesh::get_persistence_data(double angle, double offset, std::vector<std::pair<unsigned, unsigned> > &xi)
{
/*	//find the cell that contains the point
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
        double x = x_grades[xi[xi_global[i]].first];
        double y = y_grades[xi[xi_global[i]].second];


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
    */
}//end get_persistence_data

//projects (x,y) onto the line determined by angle and offset
std::pair<bool, double> Mesh::project(double angle, double offset, double x, double y)
{
	double p = 0;	//if there is a projection, then this will be set to its coordinate
	bool b = true;	//if there is no projection, then this will be set to false
	
/*	if(angle == 0)	//horizontal line
	{
		if(y <= offset)	//then point is below line
			p = x;
		else	//no projection
			b = false;
	}
    else if(angle < HALF_PI)	//line is neither horizontal nor vertical       ----- FIX THIS!!!!!
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
    */
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

