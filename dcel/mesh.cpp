/* implementation of Mesh class
 * Stores and manipulates the DCEL decomposition of the affine Grassmannian.
 */

#include "mesh.h"

#include "dcel.h"
#include "../computationthread.h"
#include "../dcel/barcode_template.h"
#include "../math/multi_betti.h"            //this include might not be necessary
#include "../math/persistence_updater.h"
#include "../cutgraph.h"

#include <QDebug>
#include <QTime>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
//#include <boost/graph/metric_tsp_approx.hpp> -- NOT CURRENTLY USED

#include <limits>	//necessary for infinity
#include <queue>    //for std::priority_queue
#include <stack>    //for find_subpath
#include <algorithm> //for find function in version 3 of find_subpath


// Mesh constructor; sets up bounding box (with empty interior) for the affine Grassmannian
Mesh::Mesh(const std::vector<double> &xg, const std::vector<exact> &xe, const std::vector<double> &yg, const std::vector<exact> &ye, int verbosity) :
    x_grades(xg), x_exact(xe), y_grades(yg), y_exact(ye),
    INFTY(std::numeric_limits<double>::infinity()),
    verbosity(verbosity)
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

    topleft = halfedges[7];     //remember this halfedge to make curve insertion easier
    topright = halfedges[2];    //remember this halfedge for starting the path that we use to find edge weights
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

    for(std::set<Anchor*>::iterator it = all_anchors.begin(); it != all_anchors.end(); ++it)
        delete (*it);
}//end destructor

//builds the DCEL arrangement, computes and stores persistence data
//also stores ordered list of xi support points in the supplied vector
//precondition: the constructor has already created the boundary of the arrangement
void Mesh::build_arrangement(MultiBetti& mb, std::vector<xiPoint>& xi_pts, ComputationThread* cthread)
{
    QTime timer;    //for timing the computations

    //first, create PersistenceUpdater
    //this also finds anchors and stores them in the vector Mesh::all_anchors -- JULY 2015 BUG FIX
    emit cthread->setCurrentProgress(10);
    timer.start();
    PersistenceUpdater updater(this, mb.bifiltration, xi_pts);   //PersistenceUpdater object is able to do the calculations necessary for finding anchors and computing barcode templates
    qDebug() << "  --> finding anchors took" << timer.elapsed() << "milliseconds";

    //now that we have all the anchors, we can build the interior of the arrangement
    emit cthread->setCurrentProgress(25);
    timer.start();
    build_interior();
    qDebug() << "  --> building the interior of the line arrangement took" << timer.elapsed() << "milliseconds";
    print_stats();

    //compute the edge weights
    emit cthread->setCurrentProgress(50);
    timer.start();
    find_edge_weights(updater);
    qDebug() << "  --> computing the edge weights took" << timer.elapsed() << "milliseconds";

    //now that the arrangement is constructed, we can find a path -- NOTE: path starts with a (near-vertical) line to the right of all multigrades
    emit cthread->setCurrentProgress(75);
    std::vector<Halfedge*> path;
    timer.start();
    find_path(path);
    qDebug() << "  --> finding the path took" << timer.elapsed() << "milliseconds";

    //update the progress dialog box
    cthread->advanceProgressStage();            //update now in stage 5 (compute discrete barcodes)
    cthread->setProgressMaximum(path.size());

    //finally, we can traverse the path, computing and storing a barcode template in each 2-cell
    updater.store_barcodes_with_reset(path, cthread);

}//end build_arrangement()

//builds the DCEL arrangement from the supplied xi support points, but does NOT compute persistence data
void Mesh::build_arrangement(std::vector<xiPoint>& xi_pts, std::vector<BarcodeTemplate>& barcode_templates, ComputationThread* cthread)
{
    QTime timer;

    //first, compute anchors and store them in the vector Mesh::all_anchors
    emit cthread->setCurrentProgress(10);
    timer.start();
    PersistenceUpdater updater(this, xi_pts);   //we only use the PersistenceUpdater to find and store the anchors
    qDebug() << "  --> finding anchors took" << timer.elapsed() << "milliseconds";

    //now that we have all the anchors, we can build the interior of the arrangement
    emit cthread->setCurrentProgress(30);
    timer.start();
    build_interior();   ///TODO: build_interior() should update its status!
    qDebug() << "  --> building the interior of the line arrangement took" << timer.elapsed() << "milliseconds";
    print_stats();

    //check
    if(faces.size() != barcode_templates.size())
        qDebug() << "ERROR: number of faces does not match number of barcode templates";
    else
        qDebug() << "number of faces = number of barcode templates";

    //now store the barcode templates
    for(unsigned i = 0; i < barcode_templates.size(); i++)
    {
        set_barcode_template(i, barcode_templates[i]);
    }
}//end build_arrangement()


//function to build the arrangement using a version of the Bentley-Ottmann algorithm, given all Anchors
//preconditions:
//   all Anchors are in a list, ordered by Anchor_LeftComparator
//   boundary of the mesh is created (as in the mesh constructor)
void Mesh::build_interior()
{
    if(verbosity >= 6)
    {
        QDebug qd = qDebug().nospace();
        qd << "BUILDING ARRANGEMENT:  Anchors sorted for left edge of strip: ";
        for(std::set<Anchor*, Anchor_LeftComparator>::iterator it = all_anchors.begin(); it != all_anchors.end(); ++it)
            qd << "(" << (*it)->get_x() << "," << (*it)->get_y() << ") ";
    }

  // DATA STRUCTURES

    //data structure for ordered list of lines
    std::vector<Halfedge*> lines;
    lines.reserve(all_anchors.size());

    //data structure for queue of future intersections
    std::priority_queue< Crossing*, std::vector<Crossing*>, CrossingComparator > crossings;

    //data structure for all pairs of Anchors whose potential crossings have been considered
    typedef std::pair<Anchor*,Anchor*> Anchor_pair;
    std::set< Anchor_pair > considered_pairs;

  // PART 1: INSERT VERTICES AND EDGES ALONG LEFT EDGE OF THE ARRANGEMENT
    if(verbosity >= 6) { qDebug() << "PART 1: LEFT EDGE OF ARRANGEMENT"; }

    //for each Anchor, create vertex and associated halfedges, anchored on the left edge of the strip
    Halfedge* leftedge = bottomleft;
    unsigned prev_y = std::numeric_limits<unsigned>::max();
    for(std::set<Anchor*, Anchor_LeftComparator>::iterator it = all_anchors.begin(); it != all_anchors.end(); ++it)
    {
        Anchor* cur_anchor = *it;

        if(verbosity >= 8) { qDebug() << "  Processing Anchor" << cur_anchor << "at (" << cur_anchor->get_x() << "," << cur_anchor->get_y() << ")"; }

        if(cur_anchor->get_y() != prev_y)	//then create new vertex
        {
            double dual_point_y_coord = -1*y_grades[cur_anchor->get_y()];  //point-line duality requires multiplying by -1
            leftedge = insert_vertex(leftedge, 0, dual_point_y_coord);    //set leftedge to edge that will follow the new edge
            prev_y = cur_anchor->get_y();  //remember the discrete y-index
        }

        //now insert new edge at origin vertex of leftedge
        Halfedge* new_edge = create_edge_left(leftedge, cur_anchor);

        //remember Halfedge corresponding to this Anchor
        lines.push_back(new_edge);

        //remember relative position of this Anchor
        cur_anchor->set_position(lines.size() - 1);

        //remember line associated with this Anchor
        cur_anchor->set_line(new_edge);
    }

    //for each pair of consecutive lines, if they intersect, store the intersection
    for(unsigned i = 0; i+1 < lines.size(); i++)
    {
        Anchor* a = lines[i]->get_anchor();
        Anchor* b = lines[i+1]->get_anchor();
        if( a->comparable(b) )    //then the Anchors are (strongly) comparable, so we must store an intersection
            crossings.push(new Crossing(a, b, this));

        //remember that we have now considered this intersection
        considered_pairs.insert(Anchor_pair(a,b));
    }


  // PART 2: PROCESS INTERIOR INTERSECTIONS
    //    order: x left to right; for a given x, then y low to high
    if(verbosity >= 6) { qDebug() << "PART 2: PROCESSING INTERIOR INTERSECTIONS\n"; }

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
        unsigned first_pos = cur->a->get_position();   //most recent edge in the curve corresponding to Anchor a
        unsigned last_pos = cur->b->get_position();   //most recent edge in the curve corresponding to Anchor b

        if(verbosity >= 8) { qDebug() << " next intersection: Anchor" << cur->a << " (pos" << first_pos << "), Anchor" << cur->b << " (pos" << last_pos; }

        if(last_pos != first_pos + 1)
        {
            qDebug() << "ERROR: intersection between non-consecutive curves [1]: x = " << sweep->x << "\n";
            throw std::exception();
        }

        //find out if more than two curves intersect at this point
        while( !crossings.empty() && sweep->x_equal(crossings.top()) && (cur->b == crossings.top()->a) )
        {
            cur = crossings.top();
            crossings.pop();

            if(cur->b->get_position() != last_pos + 1)
            {
                qDebug() << "ERROR: intersection between non-consecutive curves [2]\n";
                throw std::exception();
            }

            last_pos++; //last_pos = cur->b->get_position();

            if(verbosity >= 8) { qDebug() << " |---also intersects Anchor" << cur->b << "(" << last_pos << ")"; }
        }

        //compute y-coordinate of intersection
        double intersect_y = x_grades[sweep->a->get_x()]*(sweep->x) - y_grades[sweep->a->get_y()];

        if(verbosity >= 8) { qDebug() << "  found intersection between" << (last_pos - first_pos + 1) << "edges at x =" << sweep->x << ", y =" << intersect_y; }

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
            Halfedge* new_edge = new Halfedge(new_vertex, incoming->get_anchor());	//points AWAY FROM new_vertex
            halfedges.push_back(new_edge);
            Halfedge* new_twin = new Halfedge(NULL, incoming->get_anchor());		//points TOWARDS new_vertex
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

            //remember position of this Anchor
            new_edge->get_anchor()->set_position(last_pos - (cur_pos - first_pos));
        }

        //update lines vector: flip portion of vector [first_pos, last_pos]
        for(unsigned i = 0; i < (last_pos - first_pos + 1)/2; i++)
        {
            //swap curves[first_pos + i] and curves[last_pos - i]
            Halfedge* temp = lines[first_pos + i];
            lines[first_pos + i] = lines[last_pos - i];
            lines[last_pos - i] = temp;
        }

        //find new intersections and add them to intersections queue
        if(first_pos > 0)   //then consider lower intersection
        {
            Anchor* a = lines[first_pos-1]->get_anchor();
            Anchor* b = lines[first_pos]->get_anchor();

            if(considered_pairs.find(Anchor_pair(a,b)) == considered_pairs.end()
                    && considered_pairs.find(Anchor_pair(b,a)) == considered_pairs.end() )	//then this pair has not yet been considered
            {
                considered_pairs.insert(Anchor_pair(a,b));
                if( a->comparable(b) )    //then the Anchors are (strongly) comparable, so we have found an intersection to store
                    crossings.push(new Crossing(a, b, this));
            }
        }

        if(last_pos + 1 < lines.size())    //then consider upper intersection
        {
            Anchor* a = lines[last_pos]->get_anchor();
            Anchor* b = lines[last_pos+1]->get_anchor();

            if( considered_pairs.find(Anchor_pair(a,b)) == considered_pairs.end()
                    && considered_pairs.find(Anchor_pair(b,a)) == considered_pairs.end() )	//then this pair has not yet been considered
            {
                considered_pairs.insert(Anchor_pair(a,b));
                if( a->comparable(b) )    //then the Anchors are (strongly) comparable, so we have found an intersection to store
                    crossings.push(new Crossing(a, b, this));
            }
        }

        //output status
        if(verbosity >= 2)
        {
            status_counter++;
            if(status_counter % status_interval == 0)
                qDebug() << "      processed" << status_counter << "intersections; sweep position =" << sweep;
        }
    }//end while

  // PART 3: INSERT VERTICES ON RIGHT EDGE OF ARRANGEMENT AND CONNECT EDGES
    if(verbosity >= 6) { qDebug() << "PART 3: RIGHT EDGE OF THE ARRANGEMENT"; }

    Halfedge* rightedge = bottomright; //need a reference halfedge along the right side of the strip
    unsigned cur_x = 0;      //keep track of discrete x-coordinate of last Anchor whose line was connected to right edge (x-coordinate of Anchor is slope of line)

    //connect each line to the right edge of the arrangement (at x = INFTY)
    //    requires creating a vertex for each unique slope (i.e. Anchor x-coordinate)
    //    lines that have the same slope m are "tied together" at the same vertex, with coordinates (INFTY, Y)
    //    where Y = INFTY if m is positive, Y = -INFTY if m is negative, and Y = 0 if m is zero
    for(unsigned cur_pos = 0; cur_pos < lines.size(); cur_pos++)
    {
        Halfedge* incoming = lines[cur_pos];
        Anchor* cur_anchor = incoming->get_anchor();

        if(cur_anchor->get_x() > cur_x || cur_pos == 0)    //then create a new vertex for this line
        {
            cur_x = cur_anchor->get_x();

            double Y = INFTY;               //default, for lines with positive slope
            if(x_grades[cur_x] < 0)
                Y = -1*Y;                   //for lines with negative slope
            else if(x_grades[cur_x] == 0)
                Y = 0;                      //for horizontal lines

            rightedge = insert_vertex( rightedge, INFTY, Y );
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
Halfedge* Mesh::insert_vertex(Halfedge* edge, double x, double y)
{
	//create new vertex
    Vertex* new_vertex = new Vertex(x, y);
	vertices.push_back(new_vertex);

    //get twin and Anchor of this edge
	Halfedge* twin = edge->get_twin();
    Anchor* anchor = edge->get_anchor();

	//create new halfedges
    Halfedge* up = new Halfedge(new_vertex, anchor);
	halfedges.push_back(up);
    Halfedge* dn = new Halfedge(new_vertex, anchor);
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

//creates the first pair of Halfedges in an Anchor line, anchored on the left edge of the strip at origin of specified edge
//  also creates a new face (the face below the new edge)
//  CAUTION: leaves NULL: new_edge.next and new_twin.prev
Halfedge* Mesh::create_edge_left(Halfedge* edge, Anchor* anchor)
{
    //create new halfedges
    Halfedge* new_edge = new Halfedge(edge->get_origin(), anchor); //points AWAY FROM left edge
    halfedges.push_back(new_edge);
    Halfedge* new_twin = new Halfedge(NULL, anchor);   //points TOWARDS left edge
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

//computes and stores the edge weight for each anchor line
void Mesh::find_edge_weights(PersistenceUpdater& updater)
{
    qDebug() << " FINDING EDGE WEIGHTS:";

    std::vector<Halfedge*> pathvec;
    Halfedge* cur_edge = topright;

    //find a path across all anchor lines
    while(cur_edge->get_twin() != bottomright)  //then there is another vertex to consider on the right edge
    {
        cur_edge = cur_edge->get_next();
        while(cur_edge->get_twin()->get_face() != NULL) //then we have another edge crossing to append to the path
        {
            cur_edge = cur_edge->get_twin();
            pathvec.push_back(cur_edge);
            cur_edge = cur_edge->get_next();
        }
    }

    //run the "main algorithm" without any matrices
    updater.set_anchor_weights(pathvec);

    //reset the PersistenceUpdater to its state at the beginning of this function
    updater.clear_levelsets();

}//end set_edge_weights()

//finds a pseudo-optimal path through all 2-cells of the arrangement
// path consists of a vector of Halfedges
// at each step of the path, the Halfedge points to the Anchor being crossed and the 2-cell (Face) being entered
void Mesh::find_path(std::vector<Halfedge*>& pathvec)
{
  // PART 1: BUILD THE DUAL GRAPH OF THE ARRANGEMENT

    typedef boost::property<boost::edge_weight_t, unsigned long> EdgeWeightProperty;
    typedef boost::adjacency_list< boost::vecS, boost::vecS, boost::undirectedS,
                                   boost::no_property, EdgeWeightProperty > Graph;  //TODO: probably listS is a better choice than vecS, but I don't know how to make the adjacency_list work with listS
    Graph dual_graph;

    //make a map for reverse-lookup of face indexes by face pointers -- Is this really necessary? Can I avoid doing this?
    std::map<Face*, unsigned> face_indexes;
    for(unsigned i=0; i<faces.size(); i++)
        face_indexes.insert( std::pair<Face*, unsigned>(faces[i], i));

    // distance vector for sorting the adjacency list
    std::vector<std::vector<unsigned> > distances(faces.size(), std::vector<unsigned>(faces.size(), -1));
    for (int i = 0; i < distances.size(); ++i)
    	distances.at(i).at(i) = 0;

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
                {
                    boost::add_edge(i, j, current->get_anchor()->get_weight(), dual_graph);
                    distances.at(i).at(j) = current->get_anchor()->get_weight();
                    distances.at(j).at(i) = current->get_anchor()->get_weight();
                }
            }
            //move to the next neighbor
            current = current->get_next();
        }while(current != boundary);
    }

    //TESTING -- print the edges in the dual graph
    if(verbosity >= 10)
    {
//        QDebug qd = qDebug().nospace();
        qDebug() << "EDGES IN THE DUAL GRAPH OF THE ARRANGEMENT: ";
        typedef boost::graph_traits<Graph>::edge_iterator edge_iterator;
        std::pair<edge_iterator, edge_iterator> ei = boost::edges(dual_graph);
        for(edge_iterator it = ei.first; it != ei.second; ++it)
            qDebug().nospace() << "  (" << boost::source(*it, dual_graph) << "\t, " << boost::target(*it, dual_graph) << "\t) \tweight = " << boost::get(boost::edge_weight_t(), dual_graph, *it);
    }


  // PART 2: FIND A MINIMAL SPANNING TREE

    typedef boost::graph_traits<Graph>::edge_descriptor Edge;
    std::vector<Edge> spanning_tree_edges;
    boost::kruskal_minimum_spanning_tree(dual_graph, std::back_inserter(spanning_tree_edges));

    if(verbosity >= 10)
    {
//        QDebug qd = qDebug().nospace();
        qDebug() << "num MST edges: " << spanning_tree_edges.size() << "\n";
        for(unsigned i=0; i<spanning_tree_edges.size(); i++)
            qDebug().nospace() << "  (" << boost::source(spanning_tree_edges[i], dual_graph) << "\t, " << boost::target(spanning_tree_edges[i], dual_graph) << "\t) \tweight = " << boost::get(boost::edge_weight_t(), dual_graph, spanning_tree_edges[i]);
    }

//  // PART 2-ALTERNATE: FIND A HAMILTONIAN TOUR
//    ///This doesn't serve our purposes.
//    ///  It doesn't start at the cell representing a vertical line, but that could be fixed by using metric_tsp_approx_from_vertex(...).
//    ///  Worse, it always produces a cycle and doesn't give the intermediate nodes between non-adjacent nodes that appear consecutively in the tour.

//    typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
//    std::vector<Vertex> tsp_vertices;
//    boost::metric_tsp_approx_tour(dual_graph, std::back_inserter(tsp_vertices));

//    qDebug() << "num TSP vertices: " << tsp_vertices.size() << "\n";
//    for(unsigned i=0; i<tsp_vertices.size(); i++)
//        qDebug() << "  " << tsp_vertices[i] << "\n";

  // PART 3: CONVERT THE OUTPUT OF PART 2 TO A PATH

    //organize the edges of the minimal spanning tree so that we can traverse the tree
    std::vector< std::vector<unsigned> > adjList(faces.size(), std::vector<unsigned>()); //this will store all adjacency relationships in the spanning tree
    for(unsigned i=0; i<spanning_tree_edges.size(); i++)
    {
        unsigned a = boost::source(spanning_tree_edges[i], dual_graph);
        unsigned b = boost::target(spanning_tree_edges[i], dual_graph);

        adjList.at(a).push_back(b);
        adjList.at(b).push_back(a);
    }

    //traverse the tree
    //at each step in the traversal, append the corresponding Halfedge* to pathvec
    //make sure to start at the proper node (2-cell)
    Face* initial_cell = topleft->get_twin()->get_face();
    unsigned start = (face_indexes.find(initial_cell))->second;

    // sort the adjacencies to minimize backtracking in the path
    sortAdjacencies(adjList, distances, start);

    // find_subpath(start, adjacencies, pathvec, false);
    find_subpath(start, adjList, pathvec);

    //TESTING -- PRINT PATH
    if(verbosity >= 10)
    {
        QDebug qd = qDebug().nospace();
        qd << "PATH: " << start << ", ";
        for(unsigned i=0; i<pathvec.size(); i++)
            qd << (face_indexes.find((pathvec[i])->get_face()))->second << ", ";
        qd << "\n";
    }
}//end find_path()

//recursive method to build part of the path
//return_path == TRUE iff we want the path to return to the current node after traversing all of its children
void Mesh::find_subpath(unsigned cur_node, std::vector< std::vector<unsigned> >& adj, std::vector<Halfedge*>& pathvec)
{
	// VERSION 1 -- MAYBE BETTER THAN VERSION 2
	// Uses a stack for nodes and erases adjacencies that have already been discovered
	// Uses erase and find functions which add some overhead
	// Could possibly be reimplemented to use std::set<unsigned> instead of std::vector<unsigned> for adjacency list
		// but this might be a bit difficult because of set sorting (we want to sort so as to minimize backtracking)
		// might have to use a mapping of some sort, a struct, or a pair to get the sorting to work properly
		// which will add some more overhead

	std::stack<unsigned> nodes; // stack for nodes as we do DFS
    std::stack<Halfedge*> backtrack; // stack for storing extra copy of Halfedge* so we don't have to recalculate when popping
    unsigned node = cur_node;
    nodes.push(node); // push node onto the node stack

    while (!nodes.empty()) // while we have not traversed the whole tree
    {
		node = nodes.top(); // let node be the current node that we are considering

        if (adj.at(node).size() != 0) // if we have not traversed all of node's children
        {
        	// find the halfedge to be traversed
            unsigned next_node = (unsigned) adj.at(node).back();

            Halfedge* cur_edge = (faces[node])->get_boundary();
            while (cur_edge->get_twin()->get_face() != faces[next_node])
            {
                cur_edge = cur_edge->get_next();

                if (cur_edge == (faces[node])->get_boundary())
                {
                    qDebug() << "ERROR:  cannot find edge between 2-cells " << node << " and " << next_node << "\n";
                    throw std::exception();
                }
            }
            // and push it onto pathvec
            pathvec.push_back(cur_edge->get_twin());
            // push a copy onto the backtracking stack so we don't have to search for this Halfedge* again when popping
            backtrack.push(cur_edge->get_twin());
            // push the next node onto the stack
            nodes.push(next_node);

            // erase the adjacencies so they will not be considered again
            adj.at(node).erase(adj.at(node).end() - 1); // NOTE:  could possible use resize(adj.at(node).size() - 1) if that is faster than erase but I'm not sure if it would be
            adj.at(next_node).erase( find(adj.at(next_node).begin(), adj.at(next_node).end(), node) );

        } // end if

        else // if we have traversed all of node's children
        {
            nodes.pop(); // pop node off of the node stack

            if (!backtrack.empty()) // if there is still backtracking to be done
            {
                pathvec.push_back(backtrack.top()); // push the top of backtrack onto pathvec
                backtrack.pop(); // and pop that Halfedge* off of backtrack
            }
        }
    } // end while




	// VERSION 2 -- REQUIRES REVERSING pairCompare FUNCTION IN cutgraph.cpp
	// Uses stacks for nodes and backtracking and uses boolean array for keeping track of which vertices have been discovered
	// Must perfonm linear search at each iteration of the while loop to find which node(s) (if any) need to be considered
	/*
	bool discovered[adj.size()]; // boolean array for keeping track of which nodes have been visited
	// populate the boolean array with false
	for (int i = 0; i < adj.size(); ++i)
	{
		discovered[i] = false;
	}
	std::stack<unsigned> nodes; // stack for nodes as we do DFS
    std::stack<Halfedge*> backtrack; // stack for storing extra copy of Halfedge* so we don't have to recalculate when popping
    unsigned node = cur_node, edgeIndex = 0;
    nodes.push(node); // push node onto the node stack
    discovered[node] = true; // mark node as discovered

    while (!nodes.empty()) // while we have not traversed the whole tree
    {
		node = nodes.top(); // let node be the current node that we are considering
		// find the next undiscovered node
		edgeIndex = adj.at(node).size(); // set edgeIndex such that we will skip to the else if we don't find an undiscovered adjacency
        // look for an undiscovered adjacency
		for (int i = 0; i < adj.at(node).size(); ++i)
		{
			if (!discovered[adj.at(node).at(i)])
			{
				edgeIndex = i;
				break;
			}
		}

        if (edgeIndex < adj.at(node).size()) // if we have not traversed all of node's children
        {
        	discovered[adj.at(node).at(edgeIndex)] = true; // discover our next node

        	// find the halfedge to be traversed
            unsigned next_node = (unsigned) adj.at(node).at(edgeIndex);

            Halfedge* cur_edge = (faces[node])->get_boundary();
            while (cur_edge->get_twin()->get_face() != faces[next_node])
            {
                cur_edge = cur_edge->get_next();

                if (cur_edge == (faces[node])->get_boundary())
                {
                    qDebug() << "ERROR:  cannot find edge between 2-cells " << node << " and " << next_node << "\n";
                    throw std::exception();
                }
            }
            // and push it onto pathvec
            pathvec.push_back(cur_edge->get_twin());
            // push a copy onto the backtracking stack so we don't have to search for this Halfedge* again when popping
            backtrack.push(cur_edge->get_twin());
            // push the next node onto the stack
            nodes.push( adj.at(node).at(edgeIndex) );

        } // end if

        else // if we have traversed all of node's children
        {
            nodes.pop(); // pop node off of the node stack

            if (!backtrack.empty()) // if there is still backtracking to be done
            {
                pathvec.push_back(backtrack.top()); // push the top of backtrack onto pathvec
                backtrack.pop(); // and pop that Halfedge* off of backtrack
            }
        }
    } // end while
    */

}//end find_subpath()


//returns barcode template associated with the specified line (point)
//REQUIREMENT: 0 <= degrees <= 90
BarcodeTemplate& Mesh::get_barcode_template(double degrees, double offset)
{
    if(degrees == 90) //then line is vertical
    {
        Face* cell = find_vertical_line(-1*offset); //multiply by -1 to correct for orientation of offset

        ///TODO: store some point/cell to seed the next query

        if(verbosity >= 3) { qDebug() << " ||| vertical line found in cell " << FID(cell); }
        return cell->get_barcode();
    }

    if(degrees == 0) //then line is horizontal
    {
        Face* cell = topleft->get_twin()->get_face();    //default
        Anchor* anchor = find_least_upper_anchor(offset);

        if(anchor != NULL)
            cell = anchor->get_line()->get_face();

        ///TODO: store some point/cell to seed the next query

        if(verbosity >= 3) { qDebug() << " --- horizontal line found in cell " << FID(cell); }
        return cell->get_barcode();
    }

    //else: the line is neither horizontal nor vertical
    double radians = degrees * 3.14159265/180;
    double slope = tan(radians);
    double intercept = offset/cos(radians);

//    qDebug().nospace() << "  Line (deg, off) = (" << degrees << ", " << offset << ") transformed to (slope, int) = (" << slope << ", " << intercept << ")";

    Face* cell = find_point(slope, -1*intercept);   //multiply by -1 for point-line duality
        ///TODO: REPLACE THIS WITH A SEEDED SEARCH

    ///TODO: store seed

    return cell->get_barcode();  ////FIX THIS!!!
}//end get_barcode_template()

//returns the barcode template associated with faces[i]
BarcodeTemplate& Mesh::get_barcode_template(unsigned i)
{
    return faces[i]->get_barcode();
}

//stores (a copy of) the given barcode template in faces[i]
void Mesh::set_barcode_template(unsigned i, BarcodeTemplate& bt)
{
    faces[i]->set_barcode(bt);
}

//returns the number of 2-cells, and thus the number of barcode templates, in the arrangement
unsigned Mesh::num_faces()
{
    return faces.size();
}

//creates a new anchor in the vector all_anchors
void Mesh::add_anchor(xiMatrixEntry* entry)
{
    all_anchors.insert(new Anchor(entry));
}

//finds the first anchor that intersects the left edge of the arrangement at a point not less than the specified y-coordinate
//  if no such anchor, returns NULL
Anchor* Mesh::find_least_upper_anchor(double y_coord)
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
    //now find Anchor whose line intersects the left edge of the arrangement lowest, but not below y_grade[best]
    unsigned int zero = 0;  //disambiguate the following function call
    Anchor* test = new Anchor(zero, best);
    std::set<Anchor*, Anchor_LeftComparator>::iterator it = all_anchors.lower_bound(test);
    delete test;

    if(it == all_anchors.end())    //not found
    {
        return NULL;
    }
    //else
    return *it;
}//end find_least_upper_anchor()

//finds the (unbounded) cell associated to dual point of the vertical line with the given x-coordinate
//  i.e. finds the Halfedge whose Anchor x-coordinate is the largest such coordinate not larger than than x_coord; returns the Face corresponding to that Halfedge
Face* Mesh::find_vertical_line(double x_coord)
{
    //is there an Anchor with x-coordinate not greater than x_coord?
    if(vertical_line_query_list.size() >= 1 && x_grades[ vertical_line_query_list[0]->get_anchor()->get_x() ] <= x_coord)
    {
        //binary search the vertical line query list
        unsigned min = 0;
        unsigned max = vertical_line_query_list.size() - 1;
        unsigned best = 0;

        while(max >= min)
        {
            unsigned mid = (max + min)/2;
            Anchor* test = vertical_line_query_list[mid]->get_anchor();

            if(x_grades[test->get_x()] <= x_coord)    //found a lower bound, but search upper subarray for a better lower bound
            {
                best = mid;
                min = mid + 1;
            }
            else    //search lower subarray
                max = mid - 1;
        }

        //testing
        if(verbosity >= 6) { qDebug() << "----vertical line search: found anchor with x-coordinate " << vertical_line_query_list[best]->get_anchor()->get_x(); }

        return vertical_line_query_list[best]->get_face();
    }

    //if we get here, then either there are no Anchors or x_coord is less than the x-coordinates of all Anchors
    if(verbosity >= 6) { qDebug() << "----vertical line search: returning lowest face"; }
    return bottomright->get_twin()->get_face();

}//end find_vertical_line()

//find a 2-cell containing the specified point
Face* Mesh::find_point(double x_coord, double y_coord)
{
    //start on the left edge of the arrangement, at the correct y-coordinate
    Anchor* start = find_least_upper_anchor(-1*y_coord);

    Face* cell = NULL;		//will later point to the cell containing the specified point
    Halfedge* finger = NULL;	//for use in finding the cell

    if(start == NULL)	//then starting point is in the top (unbounded) cell
    {
        finger = topleft->get_twin()->get_next();   //this is the top edge of the top cell (at y=infty)
        if(verbosity >= 8) { qDebug() << "  Starting in top (unbounded) cell"; }
    }
    else
    {
        finger = start->get_line();
        if(verbosity >= 8) { qDebug() << "  Reference Anchor: (" << x_grades[start->get_x()] << "," << y_grades[start->get_y()] << "); halfedge" << HID(finger); }
    }

    while(cell == NULL) //while not found
    {
        if(verbosity >= 8) { qDebug() << "  Considering cell " << FID(finger->get_face()); }

        //find the edge of the current cell that crosses the horizontal line at y_coord
        Vertex* next_pt = finger->get_next()->get_origin();

        if(verbosity >= 8)
        {
            if(finger->get_anchor() != NULL)
                qDebug() << "     -- next point: (" << next_pt->get_x() << "," << next_pt->get_y() << ") vertex ID" << VID(next_pt) << "; along line corresponding to anchor at (" << finger->get_anchor()->get_x() << "," << finger->get_anchor()->get_y() << ")";
            else
                qDebug() << "     -- next point: (" << next_pt->get_x() << "," << next_pt->get_y() << ") vertex ID" << VID(next_pt) << "; along line corresponding to NULL anchor";
        }

        while(next_pt->get_y() > y_coord)
        {
            finger = finger->get_next();
            next_pt = finger->get_next()->get_origin();

            if(verbosity >= 8)
            {
                if(finger->get_anchor() != NULL)
                    qDebug() << "     -- next point: (" << next_pt->get_x() << "," << next_pt->get_y() << ") vertex ID" << VID(next_pt) << "; along line corresponding to anchor at (" << finger->get_anchor()->get_x() << "," << finger->get_anchor()->get_y() << ")";
                else
                    qDebug() << "     -- next point: (" << next_pt->get_x() << "," << next_pt->get_y() << ") vertex ID" << VID(next_pt) << "; along line corresponding to NULL anchor";
            }
        }

        //now next_pt is at or below the horizontal line at y_coord
        //if (x_coord, y_coord) is to the left of crossing point, then we have found the cell; otherwise, move to the adjacent cell

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
            if(finger->get_anchor() == NULL)	//then edge is vertical, so we have found the cell
            {
                cell = finger->get_face();
            }
            else	//then edge is not vertical
            {
                Anchor* temp = finger->get_anchor();
                double x_pos = ( y_coord + y_grades[temp->get_y()] )/x_grades[temp->get_x()];  //NOTE: division by zero never occurs because we are searching along a horizontal line, and thus we never cross horizontal lines in the arrangement

                if(x_pos >= x_coord)	//found the cell
                {
                    cell = finger->get_face();
                }
                else	//move to adjacent cell
                {
                    finger = finger->get_twin();

                    if(verbosity >= 8) { qDebug().nospace() << "   --- crossing line dual to anchor (" << temp->get_x() << "," << temp->get_y() << ") at x = " << x_pos; }
                }
            }
        }//end else
    }//end while(cell not found)

    if(verbosity >= 3) { qDebug() << "  Found point (" << x_coord << "," << y_coord << ") in cell" << FID(cell); }

    return cell;
}//end find_point()


//prints a summary of the arrangement information, such as the number of anchors, vertices, halfedges, and faces
void Mesh::print_stats()
{
    qDebug() << "The arrangement contains:" << all_anchors.size() << "anchors," << vertices.size() << "vertices" << halfedges.size() << "halfedges, and" << faces.size() << "faces";
}

//print all the data from the mesh
void Mesh::print()
{
    qDebug() << "  Vertices";
    for(unsigned i=0; i<vertices.size(); i++)
	{
        qDebug() << "    vertex " << i << ": " << *vertices[i] << "; incident edge: " << HID(vertices[i]->get_incident_edge());
	}

    qDebug() << "  Halfedges";
    for(unsigned i=0; i<halfedges.size(); i++)
	{
		Halfedge* e = halfedges[i];
		Halfedge* t = e->get_twin();
        qDebug() << "    halfedge " << i << ": " << *(e->get_origin()) << "--" << *(t->get_origin()) << "; ";
        if(e->get_anchor() == NULL)
            qDebug() << "Anchor null; ";
		else
            qDebug() << "Anchor coords (" << e->get_anchor()->get_x() << ", " << e->get_anchor()->get_y() << "); ";
        qDebug() << "twin: " << HID(t) << "; next: " << HID(e->get_next()) << "; prev: " << HID(e->get_prev()) << "; face: " << FID(e->get_face());
	}

    qDebug() << "  Faces";
    for(unsigned i=0; i<faces.size(); i++)
	{
        qDebug() << "    face " << i << ": " << *faces[i];
	}

/*    qDebug() << "  Outside (unbounded) region: ";
	Halfedge* start = halfedges[1];
	Halfedge* curr = start;
	do{
        qDebug() << *(curr->get_origin()) << "--";
		curr = curr->get_next();
	}while(curr != start);
    qDebug() << "cycle";
*/
    qDebug() << "  Anchor set: ";
    std::set<Anchor*>::iterator it;
    for(it = all_anchors.begin(); it != all_anchors.end(); ++it)
	{
        Anchor cur = **it;
        qDebug() << "(" << cur.get_x() << ", " << cur.get_y() << ") halfedge " << HID(cur.get_line()) << "; ";
	}
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

//look up vertex ID, used in print() for debugging
// VID = vertex ID
unsigned Mesh::VID(Vertex* v)
{
    for(unsigned i=0; i<vertices.size(); i++)
    {
        if(vertices[i] == v)
            return i;
    }

    //we should only get here if f is NULL (meaning the unbounded, outside face)
    return -1;
}

//attempts to find inconsistencies in the DCEL arrangement
void Mesh::test_consistency()
{
    //check faces
    qDebug() << "Checking faces:";
    bool face_problem = false;
    std::set<int> edges_found_in_faces;

    for(std::vector<Face*>::iterator it = faces.begin(); it != faces.end(); ++it)
    {
        Face* face = *it;
        qDebug() << "  Checking face " << FID(face);

        if(face->get_boundary() == NULL)
        {
            qDebug() << "    PROBLEM: face" << FID(face) << "has null edge pointer.";
            face_problem = true;
        }
        else
        {
            Halfedge* start = face->get_boundary();
            edges_found_in_faces.insert(HID(start));

            if(start->get_face() != face)
            {
                qDebug() << "    PROBLEM: starting halfedge edge" << HID(start) << "of face" << FID(face) << "doesn't point back to face.";
                face_problem = true;
            }

            if(start->get_next() == NULL)
                qDebug() << "    PROBLEM: starting halfedge" << HID(start) << "of face" << FID(face) << "has NULL next pointer.";
            else
            {
                Halfedge* cur = start->get_next();
                int i = 0;
                while(cur != start)
                {
                    edges_found_in_faces.insert(HID(cur));

                    if(cur->get_face() != face)
                    {
                        qDebug() << "    PROBLEM: halfedge edge" << HID(cur) << "points to face" << FID(cur->get_face()) << "instead of face" << FID(face);
                        face_problem = true;
                        break;
                    }

                    if(cur->get_next() == NULL)
                    {
                        qDebug() << "    PROBLEM: halfedge" << HID(cur) << "has NULL next pointer.";
                        face_problem = true;
                        break;
                    }
                    else
                        cur = cur->get_next();

                    i++;
                    if(i >= 1000)
                    {
                        qDebug() << "    PROBLEM: halfedges of face" << FID(face) << "do not form a cycle (or, if they do, it has more than 1000 edges).";
                        face_problem = true;
                        break;
                    }
                }
            }

        }
    }//end face loop
    if(!face_problem)
        qDebug() << "   ---No problems detected among faces.";
    else
        qDebug() << "   ---Problems detected among faces.";

    //find exterior halfedges
    Halfedge* start = halfedges[1];
    Halfedge* cur = start;
    do{
        edges_found_in_faces.insert(HID(cur));

        if(cur->get_next() == NULL)
        {
            qDebug() << "    PROBLEM: halfedge " << HID(cur) << " has NULL next pointer.";
            break;
        }
        cur = cur->get_next();
    }while(cur != start);

    //check if all edges were found
    bool all_edges_found = true;
    for(unsigned i=0; i<halfedges.size(); i++)
    {
        if(edges_found_in_faces.find(i) == edges_found_in_faces.end())
        {
            qDebug() << "  PROBLEM: halfedge" << i << "not found in any face";
            all_edges_found = false;
        }
    }
    if(all_edges_found)
        qDebug() << "   ---All halfedges found in faces, as expected.";


    //check anchor lines
    qDebug() << "Checking anchor lines:\n";
    bool curve_problem = false;
    std::set<int> edges_found_in_curves;

    for(std::set<Anchor*>::iterator it = all_anchors.begin(); it != all_anchors.end(); ++it)
    {
        Anchor* anchor = *it;
        qDebug() << "  Checking line for anchor (" << anchor->get_x() <<"," << anchor->get_y() << ")";

        Halfedge* edge = anchor->get_line();
        do{
            edges_found_in_curves.insert(HID(edge));
            edges_found_in_curves.insert(HID(edge->get_twin()));

            if(edge->get_anchor() != anchor)
            {
                qDebug() << "    PROBLEM: halfedge" << HID(edge) << "does not point to this Anchor.";
                curve_problem = true;
            }
            if(edge->get_twin()->get_anchor() != anchor)
            {
                qDebug() << "    PROBLEM: halfedge" << HID(edge->get_twin()) << ", twin of halfedge " << HID(edge) << ", does not point to this anchor.";
                curve_problem = true;
            }

            if(edge->get_next() == NULL)
            {
                qDebug() << "    PROBLEM: halfedge" << HID(edge) << "has NULL next pointer.";
                curve_problem = true;
                break;
            }

            //find next edge in this line
            edge = edge->get_next();
            while(edge->get_anchor() != anchor)
                edge = edge->get_twin()->get_next();

        }while(edge->get_origin()->get_x() < INFTY);
    }//end anchor line loop

    //ignore halfedges on both sides of boundary
    start = halfedges[1];
    cur = start;
    do{
        edges_found_in_curves.insert(HID(cur));
        edges_found_in_curves.insert(HID(cur->get_twin()));

        if(cur->get_next() == NULL)
        {
            qDebug() << "    PROBLEM: halfedge" << HID(cur) << "has NULL next pointer.";
            break;
        }
        cur = cur->get_next();
    }while(cur != start);

    //check if all edges were found
    all_edges_found = true;
    for(unsigned i=0; i<halfedges.size(); i++)
    {
        if(edges_found_in_curves.find(i) == edges_found_in_curves.end())
        {
            qDebug() << "  PROBLEM: halfedge" << i << "not found in any anchor line";
            all_edges_found = false;
        }
    }
    if(all_edges_found)
        qDebug() << "   ---All halfedges found in curves, as expected.";


    if(!curve_problem)
        qDebug() << "   ---No problems detected among anchor lines.";
    else
        qDebug() << "   ---Problems detected among anchor lines.";


    //check anchor lines
    qDebug() << "Checking order of vertices along right edge of the strip:";
    Halfedge* redge = halfedges[3];
    while(redge != halfedges[1])
    {
        qDebug() << " y = " << redge->get_origin()->get_y() << "at vertex" << VID(redge->get_origin());
        redge = redge->get_next();
    }
}//end test_consistency()



/********** the following objects and functions are for exact comparisons **********/

//Crossing constructor
//precondition: Anchors a and b must be comparable
Mesh::Crossing::Crossing(Anchor* a, Anchor* b, Mesh* m) : a(a), b(b), m(m)
{
    //store the x-coordinate of the crossing for fast (inexact) comparisons
    x = (m->y_grades[a->get_y()] - m->y_grades[b->get_y()])/(m->x_grades[a->get_x()] - m->x_grades[b->get_x()]);
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
        qDebug() << "INVERTED CROSSING ERROR\n";
        qDebug() << "crossing 1 involves anchors " << c1->a << " (pos " << c1->a->get_position() << ") and " << c1->b << " (pos " << c1->b->get_position() << "),";
        qDebug() << "crossing 2 involves anchors " << c2->a << " (pos " << c2->a->get_position() << ") and " << c2->b << " (pos " << c2->b->get_position() << "),";
        throw std::exception();
    }

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

