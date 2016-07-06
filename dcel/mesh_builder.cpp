//
// Created by Bryn Keller on 6/28/16.
//

#include <math/persistence_updater.h>
#include <boost/graph/properties.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <math/simplex_tree.h>
#include "dcel/mesh_builder.h"
#include "dcel/mesh.h"
#include "dcel/dcel.h"
#include "timer.h"
#include "debug.h"


MeshBuilder::MeshBuilder(unsigned verbosity) : verbosity(verbosity) {}

//builds the DCEL arrangement, computes and stores persistence data
//also stores ordered list of xi support points in the supplied vector
//precondition: the constructor has already created the boundary of the arrangement
std::shared_ptr<Mesh> MeshBuilder::build_arrangement(MultiBetti& mb,
                                                     std::vector<exact> x_exact,
                                                     std::vector<exact> y_exact,
                                                     std::vector<xiPoint>& xi_pts,
                                                     Progress &progress)
{
    debug() << "build_arrangement with multibetti";
    Timer timer;

    //first, create PersistenceUpdater
    //this also finds anchors and stores them in the vector Mesh::all_anchors -- JULY 2015 BUG FIX
    progress.progress(10);
    std::shared_ptr<Mesh> mesh(new Mesh(x_exact, y_exact, verbosity));
    PersistenceUpdater updater(*mesh, mb.bifiltration, xi_pts, verbosity);   //PersistenceUpdater object is able to do the calculations necessary for finding anchors and computing barcode templates
    debug() << "  --> finding anchors took " << timer.elapsed() << " milliseconds" ;

    //now that we have all the anchors, we can build the interior of the arrangement
    progress.progress(25);
    timer.restart();
    build_interior(mesh);
    debug() << "  --> building the interior of the line arrangement took " << timer.elapsed() << " milliseconds" ;
    mesh->print_stats();

    //compute the edge weights
    progress.progress(50);
    timer.restart();
    find_edge_weights(*mesh, updater);
    debug() << "  --> computing the edge weights took " << timer.elapsed() << " milliseconds" ;

    //now that the arrangement is constructed, we can find a path -- NOTE: path starts with a (near-vertical) line to the right of all multigrades
    progress.progress(75);
    std::vector<std::shared_ptr<Halfedge>> path;
    timer.restart();
    find_path(*mesh, path);
    debug() << "  --> finding the path took " << timer.elapsed() << " milliseconds" ;

    //update the progress dialog box
    progress.advanceProgressStage();            //update now in stage 5 (compute discrete barcodes)
    progress.setProgressMaximum(path.size());

    //finally, we can traverse the path, computing and storing a barcode template in each 2-cell
    updater.store_barcodes_with_reset(path, progress);

    return mesh;

}//end build_arrangement()

//builds the DCEL arrangement from the supplied xi support points, but does NOT compute persistence data
std::shared_ptr<Mesh> MeshBuilder::build_arrangement(std::vector<exact> x_exact, std::vector<exact> y_exact,
                                                     std::vector<xiPoint>& xi_pts, std::vector<BarcodeTemplate>& barcode_templates,
                                                     Progress &progress)
{
    Timer timer;

    //first, compute anchors and store them in the vector Mesh::all_anchors
    progress.progress(10);
    //TODO: this is odd, fix.
    SimplexTree dummy_tree(0,0);
    std::shared_ptr<Mesh> mesh(new Mesh(x_exact, y_exact, verbosity));
    PersistenceUpdater updater(*mesh, dummy_tree, xi_pts, verbosity);   //we only use the PersistenceUpdater to find and store the anchors
    debug() << "  --> finding anchors took " << timer.elapsed() << " milliseconds";

    //now that we have all the anchors, we can build the interior of the arrangement
    progress.progress(30);
    timer.restart();
    build_interior(mesh);   ///TODO: build_interior() should update its status!
    debug() << "  --> building the interior of the line arrangement took" << timer.elapsed() << "milliseconds";
    mesh->print_stats();

    //check
    if(mesh->faces.size() != barcode_templates.size()) {
        std::stringstream ss;
        ss << "Number of faces: " << mesh->faces.size()
        << " does not match number of barcode templates: " << barcode_templates.size();
        throw std::runtime_error(ss.str());
    } else {
        debug() << "number of faces = number of barcode templates";
    }

    //now store the barcode templates
    for(unsigned i = 0; i < barcode_templates.size(); i++)
    {
        mesh->set_barcode_template(i, barcode_templates[i]);
    }
    return mesh;
}//end build_arrangement()


//function to build the arrangement using a version of the Bentley-Ottmann algorithm, given all Anchors
//preconditions:
//   all Anchors are in a list, ordered by Anchor_LeftComparator
//   boundary of the mesh is created (as in the mesh constructor)
void MeshBuilder::build_interior(std::shared_ptr<Mesh> mesh)
{
    if(verbosity >= 6)
    {
        debug() << "BUILDING ARRANGEMENT:  Anchors sorted for left edge of strip: " ;
        for(std::set<std::shared_ptr<Anchor>, Anchor_LeftComparator>::iterator it = mesh->all_anchors.begin();
            it != mesh->all_anchors.end(); ++it)
            debug(true) << "(" << (*it)->get_x() << "," << (*it)->get_y() << ") ";
    }

    // DATA STRUCTURES

    //data structure for ordered list of lines
    std::vector<std::shared_ptr<Halfedge>> lines;
    lines.reserve(mesh->all_anchors.size());

    //data structure for queue of future intersections
    std::priority_queue< Mesh::Crossing*, std::vector<Mesh::Crossing*>, Mesh::CrossingComparator > crossings;

    //data structure for all pairs of Anchors whose potential crossings have been considered
    typedef std::pair<std::shared_ptr<Anchor>,std::shared_ptr<Anchor>> Anchor_pair;
    std::set< Anchor_pair > considered_pairs;

    // PART 1: INSERT VERTICES AND EDGES ALONG LEFT EDGE OF THE ARRANGEMENT
    if(verbosity >= 6) { debug() << "PART 1: LEFT EDGE OF ARRANGEMENT" ; }

    //for each Anchor, create vertex and associated halfedges, anchored on the left edge of the strip
    std::shared_ptr<Halfedge> leftedge = mesh->bottomleft;
    unsigned prev_y = std::numeric_limits<unsigned>::max();
    for(std::set<std::shared_ptr<Anchor>, Anchor_LeftComparator>::iterator it = mesh->all_anchors.begin();
        it != mesh->all_anchors.end(); ++it)
    {
        std::shared_ptr<Anchor> cur_anchor = *it;

        if(verbosity >= 8) { debug() << "  Processing Anchor" << cur_anchor << "at (" << cur_anchor->get_x() << "," << cur_anchor->get_y() << ")"; }

        if(cur_anchor->get_y() != prev_y)	//then create new vertex
        {
            double dual_point_y_coord = -1*mesh->y_grades[cur_anchor->get_y()];  //point-line duality requires multiplying by -1
            leftedge = mesh->insert_vertex(leftedge, 0, dual_point_y_coord);    //set leftedge to edge that will follow the new edge
            prev_y = cur_anchor->get_y();  //remember the discrete y-index
        }

        //now insert new edge at origin vertex of leftedge
        std::shared_ptr<Halfedge> new_edge = mesh->create_edge_left(leftedge, cur_anchor);

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
        std::shared_ptr<Anchor> a = lines[i]->get_anchor();
        std::shared_ptr<Anchor> b = lines[i+1]->get_anchor();
        if( a->comparable(*b) )    //then the Anchors are (strongly) comparable, so we must store an intersection
            crossings.push(new Mesh::Crossing(a, b, mesh));

        //remember that we have now considered this intersection
        considered_pairs.insert(Anchor_pair(a,b));
    }


    // PART 2: PROCESS INTERIOR INTERSECTIONS
    //    order: x left to right; for a given x, then y low to high
    if(verbosity >= 6) { debug() << "PART 2: PROCESSING INTERIOR INTERSECTIONS\n"; }

    int status_counter = 0;
    int status_interval = 10000;    //controls frequency of output

    //current position of sweep line
    Mesh::Crossing* sweep = NULL;

    while(!crossings.empty())
    {
        //get the next intersection from the queue
        Mesh::Crossing* cur = crossings.top();
        crossings.pop();

        //process the intersection
        sweep = cur;
        unsigned first_pos = cur->a->get_position();   //most recent edge in the curve corresponding to Anchor a
        unsigned last_pos = cur->b->get_position();   //most recent edge in the curve corresponding to Anchor b

        if(verbosity >= 8) { debug() << " next intersection: Anchor" << cur->a << " (pos" << first_pos << "), Anchor" << cur->b << " (pos" << last_pos; }

        if(last_pos != first_pos + 1)
        {
            debug() << "ERROR: intersection between non-consecutive curves [1]: x = " << sweep->x << "\n";
            throw std::exception();
        }

        //find out if more than two curves intersect at this point
        while( !crossings.empty() && sweep->x_equal(crossings.top()) && (cur->b == crossings.top()->a) )
        {
            cur = crossings.top();
            crossings.pop();

            if(cur->b->get_position() != last_pos + 1)
            {
                debug() << "ERROR: intersection between non-consecutive curves [2]\n";
                throw std::exception();
            }

            last_pos++; //last_pos = cur->b->get_position();

            if(verbosity >= 8) { debug() << " |---also intersects Anchor" << cur->b << "(" << last_pos << ")"; }
        }

        //compute y-coordinate of intersection
        double intersect_y = mesh->x_grades[sweep->a->get_x()]*(sweep->x) - mesh->y_grades[sweep->a->get_y()];

        if(verbosity >= 8) { debug() << "  found intersection between" << (last_pos - first_pos + 1) << "edges at x =" << sweep->x << ", y =" << intersect_y; }

        //create new vertex
        std::shared_ptr<Vertex> new_vertex(new Vertex(sweep->x, intersect_y));
        mesh->vertices.push_back(new_vertex);

        //anchor edges to vertex and create new face(s) and edges	//TODO: check this!!!
        std::shared_ptr<Halfedge> prev_new_edge = NULL;                 //necessary to remember the previous new edge at each interation of the loop
        std::shared_ptr<Halfedge> first_incoming = lines[first_pos];   //necessary to remember the first incoming edge
        std::shared_ptr<Halfedge> prev_incoming = NULL;                 //necessary to remember the previous incoming edge at each iteration of the loop
        for(unsigned cur_pos = first_pos; cur_pos <= last_pos; cur_pos++)
        {
            //anchor edge to vertex
            std::shared_ptr<Halfedge> incoming = lines[cur_pos];
            incoming->get_twin()->set_origin(new_vertex);

            //create next pair of twin halfedges along the current curve (i.e. curves[incident_edges[i]] )
            std::shared_ptr<Halfedge> new_edge(new Halfedge(new_vertex, incoming->get_anchor()));	//points AWAY FROM new_vertex
            mesh->halfedges.push_back(new_edge);
            std::shared_ptr<Halfedge> new_twin(new Halfedge(NULL, incoming->get_anchor()));		//points TOWARDS new_vertex
            mesh->halfedges.push_back(new_twin);

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

                std::shared_ptr<Face> new_face(new Face(new_twin));
                mesh->faces.push_back(new_face);

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
            std::shared_ptr<Halfedge> temp = lines[first_pos + i];
            lines[first_pos + i] = lines[last_pos - i];
            lines[last_pos - i] = temp;
        }

        //find new intersections and add them to intersections queue
        if(first_pos > 0)   //then consider lower intersection
        {
            std::shared_ptr<Anchor> a = lines[first_pos-1]->get_anchor();
            std::shared_ptr<Anchor> b = lines[first_pos]->get_anchor();

            if(considered_pairs.find(Anchor_pair(a,b)) == considered_pairs.end()
               && considered_pairs.find(Anchor_pair(b,a)) == considered_pairs.end() )	//then this pair has not yet been considered
            {
                considered_pairs.insert(Anchor_pair(a,b));
                if( a->comparable(*b) )    //then the Anchors are (strongly) comparable, so we have found an intersection to store
                    crossings.push(new Mesh::Crossing(a, b, mesh));
            }
        }

        if(last_pos + 1 < lines.size())    //then consider upper intersection
        {
            std::shared_ptr<Anchor> a = lines[last_pos]->get_anchor();
            std::shared_ptr<Anchor> b = lines[last_pos+1]->get_anchor();

            if( considered_pairs.find(Anchor_pair(a,b)) == considered_pairs.end()
                && considered_pairs.find(Anchor_pair(b,a)) == considered_pairs.end() )	//then this pair has not yet been considered
            {
                considered_pairs.insert(Anchor_pair(a,b));
                if( a->comparable(*b) )    //then the Anchors are (strongly) comparable, so we have found an intersection to store
                    crossings.push(new Mesh::Crossing(a, b, mesh));
            }
        }

        //output status
        if(verbosity >= 2)
        {
            status_counter++;
            if(status_counter % status_interval == 0)
                debug() << "      processed" << status_counter << "intersections; sweep position =" << sweep;
        }
    }//end while

    // PART 3: INSERT VERTICES ON RIGHT EDGE OF ARRANGEMENT AND CONNECT EDGES
    if(verbosity >= 6) { debug() << "PART 3: RIGHT EDGE OF THE ARRANGEMENT" ; }

    std::shared_ptr<Halfedge> rightedge = mesh->bottomright; //need a reference halfedge along the right side of the strip
    unsigned cur_x = 0;      //keep track of discrete x-coordinate of last Anchor whose line was connected to right edge (x-coordinate of Anchor is slope of line)

    //connect each line to the right edge of the arrangement (at x = INFTY)
    //    requires creating a vertex for each unique slope (i.e. Anchor x-coordinate)
    //    lines that have the same slope m are "tied together" at the same vertex, with coordinates (INFTY, Y)
    //    where Y = INFTY if m is positive, Y = -INFTY if m is negative, and Y = 0 if m is zero
    for(unsigned cur_pos = 0; cur_pos < lines.size(); cur_pos++)
    {
        std::shared_ptr<Halfedge> incoming = lines[cur_pos];
        std::shared_ptr<Anchor> cur_anchor = incoming->get_anchor();

        if(cur_anchor->get_x() > cur_x || cur_pos == 0)    //then create a new vertex for this line
        {
            cur_x = cur_anchor->get_x();

            double Y = mesh->INFTY;               //default, for lines with positive slope
            if(mesh->x_grades[cur_x] < 0)
                Y = -1*Y;                   //for lines with negative slope
            else if(mesh->x_grades[cur_x] == 0)
                Y = 0;                      //for horizontal lines

            rightedge = mesh->insert_vertex( rightedge, mesh->INFTY, Y );
        }
        else    //no new vertex required, but update previous entry for vertical-line queries
            mesh->vertical_line_query_list.pop_back();

        //store Halfedge for vertical-line queries
        mesh->vertical_line_query_list.push_back(incoming->get_twin());

        //connect current line to the most-recently-inserted vertex
        std::shared_ptr<Vertex> cur_vertex = rightedge->get_origin();
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

//finds a pseudo-optimal path through all 2-cells of the arrangement
// path consists of a vector of Halfedges
// at each step of the path, the Halfedge points to the Anchor being crossed and the 2-cell (Face) being entered
void MeshBuilder::find_path(Mesh &mesh, std::vector<std::shared_ptr<Halfedge>>& pathvec)
{
    // PART 1: BUILD THE DUAL GRAPH OF THE ARRANGEMENT

    typedef boost::property<boost::edge_weight_t, unsigned long> EdgeWeightProperty;
    typedef boost::adjacency_list< boost::vecS, boost::vecS, boost::undirectedS,
            boost::no_property, EdgeWeightProperty > Graph;  //TODO: probably listS is a better choice than vecS, but I don't know how to make the adjacency_list work with listS
    Graph dual_graph;

    //make a map for reverse-lookup of face indexes by face pointers -- Is this really necessary? Can I avoid doing this?
    std::map<std::shared_ptr<Face>, unsigned> face_indexes;
    for(unsigned i=0; i<mesh.faces.size(); i++)
        face_indexes.insert( std::pair<std::shared_ptr<Face>, unsigned>(mesh.faces[i], i));

    //loop over all faces
    for(unsigned i=0; i<mesh.faces.size(); i++)
    {
        //consider all neighbors of this faces
        std::shared_ptr<Halfedge> boundary = (mesh.faces[i])->get_boundary();
        std::shared_ptr<Halfedge> current = boundary;
        do
        {
            //find index of neighbor
            std::shared_ptr<Face> neighbor = current->get_twin()->get_face();
            if(neighbor != NULL)
            {
                std::map<std::shared_ptr<Face>, unsigned>::iterator it = face_indexes.find(neighbor);
                unsigned j = it->second;

                //if i < j, then create an (undirected) edge between these faces
                if(i < j)
                {
                    boost::add_edge(i, j, current->get_anchor()->get_weight(), dual_graph);
                }
            }
            //move to the next neighbor
            current = current->get_next();
        }while(current != boundary);
    }

    //TESTING -- print the edges in the dual graph
//    if(verbosity >= 10)
//    {
////        Debug qd = debug(true);
//        debug() << "EDGES IN THE DUAL GRAPH OF THE ARRANGEMENT: ";
//        typedef boost::graph_traits<Graph>::edge_iterator edge_iterator;
//        std::pair<edge_iterator, edge_iterator> ei = boost::edges(dual_graph);
//        for(edge_iterator it = ei.first; it != ei.second; ++it)
//            debug(true) << "  (" << boost::source(*it, dual_graph) << "\t, " << boost::target(*it, dual_graph) << "\t) \tweight = " << boost::get(boost::edge_weight_t(), dual_graph, *it);
//    }


    // PART 2: FIND A MINIMAL SPANNING TREE

    typedef boost::graph_traits<Graph>::edge_descriptor Edge;
    std::vector<Edge> spanning_tree_edges;
    boost::kruskal_minimum_spanning_tree(dual_graph, std::back_inserter(spanning_tree_edges));

//    if(verbosity >= 10)
//    {
////        Debug qd = debug(true);
//        debug() << "num MST edges: " << spanning_tree_edges.size() << "\n";
//        for(unsigned i=0; i<spanning_tree_edges.size(); i++)
//            debug(true) << "  (" << boost::source(spanning_tree_edges[i], dual_graph) << "\t, " << boost::target(spanning_tree_edges[i], dual_graph) << "\t) \tweight = " << boost::get(boost::edge_weight_t(), dual_graph, spanning_tree_edges[i]);
//    }

//  // PART 2-ALTERNATE: FIND A HAMILTONIAN TOUR
//    ///This doesn't serve our purposes.
//    ///  It doesn't start at the cell representing a vertical line, but that could be fixed by using metric_tsp_approx_from_vertex(...).
//    ///  Worse, it always produces a cycle and doesn't give the intermediate nodes between non-adjacent nodes that appear consecutively in the tour.

//    typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
//    std::vector<Vertex> tsp_vertices;
//    boost::metric_tsp_approx_tour(dual_graph, std::back_inserter(tsp_vertices));

//    debug() << "num TSP vertices: " << tsp_vertices.size() << "\n";
//    for(unsigned i=0; i<tsp_vertices.size(); i++)
//        debug() << "  " << tsp_vertices[i] << "\n";

    // PART 3: CONVERT THE OUTPUT OF PART 2 TO A PATH

    //organize the edges of the minimal spanning tree so that we can traverse the tree
    std::vector< std::set<unsigned> > adjacencies(mesh.faces.size(), std::set<unsigned>());  //this will store all adjacency relationships in the spanning tree
    for(unsigned i=0; i<spanning_tree_edges.size(); i++)
    {
        unsigned a = boost::source(spanning_tree_edges[i], dual_graph);
        unsigned b = boost::target(spanning_tree_edges[i], dual_graph);

        (adjacencies[a]).insert(b);
        (adjacencies[b]).insert(a);
    }

    //traverse the tree
    //at each step in the traversal, append the corresponding std::shared_ptr<Halfedge> to pathvec
    //make sure to start at the proper node (2-cell)
    std::shared_ptr<Face> initial_cell = mesh.topleft->get_twin()->get_face();
    unsigned start = (face_indexes.find(initial_cell))->second;

    find_subpath(mesh, start, adjacencies, pathvec, false);

    //TESTING -- PRINT PATH
//    if(verbosity >= 10)
//    {
//        debug() << "PATH: " << start << ", ";
//        for(unsigned i=0; i<pathvec.size(); i++)
//            debug(true) << (face_indexes.find((pathvec[i])->get_face()))->second << ", ";
//        debug(true) << "\n";
//    }
}//end find_path()

//recursive method to build part of the path
//return_path == TRUE iff we want the path to return to the current node after traversing all of its children
void MeshBuilder::find_subpath(Mesh &mesh, unsigned cur_node, std::vector< std::set<unsigned> >& adj, std::vector<std::shared_ptr<Halfedge>>& pathvec, bool return_path)
{

    while(!adj[cur_node].empty())   //cur_node still has children to traverse
    {
        //get the next node that is adjacent to cur_node
        unsigned next_node = *(adj[cur_node].begin());

        //find the next Halfedge and append it to pathvec
        std::shared_ptr<Halfedge> cur_edge = (mesh.faces[cur_node])->get_boundary();
        while(cur_edge->get_twin()->get_face() != mesh.faces[next_node])     //do we really have to search for the correct edge???
        {
            cur_edge = cur_edge->get_next();

            if(cur_edge == (mesh.faces[cur_node])->get_boundary())   //THIS SHOULD NEVER HAPPEN
            {
                debug() << "ERROR: cannot find edge between 2-cells " << cur_node << " and " << next_node << "\n";
                throw std::exception();
            }
        }
        pathvec.push_back(cur_edge->get_twin());

        //remove adjacencies that have been already processed
        adj[cur_node].erase(next_node);     //removes (cur_node, next_node)
        adj[next_node].erase(cur_node);     //removes (next_node, cur_node)

        //do we need to return to this node?
        bool return_here = return_path || !adj[cur_node].empty();

        //recurse through the next node
        find_subpath(mesh, next_node, adj, pathvec, return_here);

        //if we will return to this node, then add reverse Halfedge to pathvec
        if(return_here)
            pathvec.push_back(cur_edge);

    }//end while

}//end find_subpath()

//computes and stores the edge weight for each anchor line
void MeshBuilder::find_edge_weights(Mesh &mesh,PersistenceUpdater& updater)
{
    debug() << " FINDING EDGE WEIGHTS:";

    std::vector<std::shared_ptr<Halfedge>> pathvec;
    std::shared_ptr<Halfedge> cur_edge = mesh.topright;

    //find a path across all anchor lines
    while(cur_edge->get_twin() != mesh.bottomright)  //then there is another vertex to consider on the right edge
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
