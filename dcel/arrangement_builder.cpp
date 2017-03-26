/**********************************************************************
Copyright 2014-2016 The RIVET Developers. See the COPYRIGHT file at
the top-level directory of this distribution.

This file is part of RIVET.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#include "dcel/arrangement_builder.h"
#include "dcel/anchor.h"
#include "dcel/arrangement.h"
#include "dcel/dcel.h"
#include "debug.h"
#include "timer.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/properties.hpp>
#include <math/persistence_updater.h>
#include <math/firep.h>
#include <math/bifiltration_data.h>

#include <algorithm> //for find function in version 3 of find_subpath
#include <cutgraph.h>
#include <stack> //for find_subpath

    using rivet::numeric::INFTY;

ArrangementBuilder::ArrangementBuilder(unsigned verbosity)
    : verbosity(verbosity)
{
}

//builds the DCEL arrangement, computes and stores persistence data
//also stores ordered list of xi support points in the supplied vector
//precondition: the constructor has already created the boundary of the arrangement
std::shared_ptr<Arrangement> ArrangementBuilder::build_arrangement(MultiBetti& mb,
    std::vector<exact> x_exact,
    std::vector<exact> y_exact,
    std::vector<TemplatePoint>& template_points,
    Progress& progress)
{
    Timer timer;

    //first, create PersistenceUpdater
    //this also finds anchors and stores them in the vector Arrangement::all_anchors -- JULY 2015 BUG FIX
    progress.progress(10);
    std::shared_ptr<Arrangement> arrangement(new Arrangement(x_exact, y_exact, verbosity));
    PersistenceUpdater updater(*arrangement, mb.bifiltration, template_points, verbosity); //PersistenceUpdater object is able to do the calculations necessary for finding anchors and computing barcode templates
    if (verbosity >= 2) {
        debug() << "Anchors found; this took " << timer.elapsed() << " milliseconds.";
    }

    //now that we have all the anchors, we can build the interior of the arrangement
    progress.progress(25);
    timer.restart();
    build_interior(arrangement);
    if (verbosity >= 2) {
        debug() << "Line arrangement constructed; this took " << timer.elapsed() << " milliseconds.";
        if (verbosity >= 4) {
            arrangement->print_stats();
        }
    }

    //compute the edge weights
    progress.progress(50);
    timer.restart();
    find_edge_weights(*arrangement, updater);
    if (verbosity >= 2) {
        debug() << "Edge weights computed; this took " << timer.elapsed() << " milliseconds.";
    }

    //now that the arrangement is constructed, we can find a path -- NOTE: path starts with a (near-vertical) line to the right of all multigrades
    progress.progress(75);
    std::vector<std::shared_ptr<Halfedge>> path;
    timer.restart();
    find_path(*arrangement, path);
    if (verbosity >= 2) {
        debug() << "Found path through the arrangement; this took " << timer.elapsed() << " milliseconds.";
    }

    //update the progress dialog box
    progress.advanceProgressStage(); //update now in stage 5 (compute discrete barcodes)
    progress.setProgressMaximum(path.size());

    //finally, we can traverse the path, computing and storing a barcode template in each 2-cell
    updater.store_barcodes_with_reset(path, progress);

    return arrangement;

} //end build_arrangement()

//builds the DCEL arrangement from the supplied xi support points, but does NOT compute persistence data
std::shared_ptr<Arrangement> ArrangementBuilder::build_arrangement(std::vector<exact> x_exact, std::vector<exact> y_exact,
    std::vector<TemplatePoint>& xi_pts, std::vector<BarcodeTemplate>& barcode_templates,
    Progress& progress)
{
    Timer timer;

    //first, compute anchors and store them in the vector Arrangement::all_anchors
    progress.progress(10);
    //TODO: this is odd, fix.
    BifiltrationData dummy_data(0, 0);
    FIRep dummy_tree(dummy_data, 0);
    std::shared_ptr<Arrangement> arrangement(new Arrangement(x_exact, y_exact, verbosity));
    PersistenceUpdater updater(*arrangement, dummy_tree, xi_pts, verbosity); //we only use the PersistenceUpdater to find and store the anchors
    if (verbosity >= 2) {
        debug() << "Anchors found; this took " << timer.elapsed() << " milliseconds.";
    }

    //now that we have all the anchors, we can build the interior of the arrangement
    progress.progress(30);
    timer.restart();
    build_interior(arrangement); ///TODO: build_interior() should update its status!
    if (verbosity >= 2) {
        debug() << "Line arrangement constructed; this took " << timer.elapsed() << " milliseconds.";
        if (verbosity >= 4) {
            arrangement->print_stats();
        }
    }

    //check
    if (arrangement->faces.size() != barcode_templates.size()) {
        std::stringstream ss;
        ss << "Number of faces: " << arrangement->faces.size()
           << " does not match number of barcode templates: " << barcode_templates.size();
        throw std::runtime_error(ss.str());
    } else if (verbosity >= 4) {
        debug() << "Check: number of faces = number of barcode templates";
    }

    //now store the barcode templates
    for (unsigned i = 0; i < barcode_templates.size(); i++) {
        arrangement->set_barcode_template(i, barcode_templates[i]);
    }
    return arrangement;
} //end build_arrangement()

//function to build the arrangement using a version of the Bentley-Ottmann algorithm, given all Anchors
//preconditions:
//   all Anchors are in a list, ordered by Anchor_LeftComparator
//   boundary of the arrangement is created (as in the arrangement constructor)
void ArrangementBuilder::build_interior(std::shared_ptr<Arrangement> arrangement)
{
    if (verbosity >= 8) {
        debug() << "BUILDING ARRANGEMENT:  Anchors sorted for left edge of strip: ";
        for (std::set<std::shared_ptr<Anchor>, Anchor_LeftComparator>::iterator it = arrangement->all_anchors.begin();
             it != arrangement->all_anchors.end(); ++it)
            debug(true) << "(" << (*it)->get_x() << "," << (*it)->get_y() << ") ";
    }

    // DATA STRUCTURES

    //data structure for ordered list of lines
    std::vector<std::shared_ptr<Halfedge>> lines;
    lines.reserve(arrangement->all_anchors.size());

    //data structure for queue of future intersections
    std::priority_queue<Arrangement::Crossing*, std::vector<Arrangement::Crossing*>, Arrangement::CrossingComparator> crossings;

    //data structure for all pairs of Anchors whose potential crossings have been considered
    typedef std::pair<std::shared_ptr<Anchor>, std::shared_ptr<Anchor>> Anchor_pair;
    std::set<Anchor_pair> considered_pairs;

    // PART 1: INSERT VERTICES AND EDGES ALONG LEFT EDGE OF THE ARRANGEMENT
    if (verbosity >= 8) {
        debug() << "PART 1: LEFT EDGE OF ARRANGEMENT";
    }

    //for each Anchor, create vertex and associated halfedges, anchored on the left edge of the strip
    std::shared_ptr<Halfedge> leftedge = arrangement->bottomleft;
    unsigned prev_y = std::numeric_limits<unsigned>::max();
    for (std::set<std::shared_ptr<Anchor>, Anchor_LeftComparator>::iterator it = arrangement->all_anchors.begin();
         it != arrangement->all_anchors.end(); ++it) {
        std::shared_ptr<Anchor> cur_anchor = *it;

        if (verbosity >= 10) {
            debug() << "  Processing Anchor"
                    << " at (" << cur_anchor->get_x() << "," << cur_anchor->get_y() << ")";
        }

        if (cur_anchor->get_y() != prev_y) //then create new vertex
        {
            double dual_point_y_coord = -1 * arrangement->y_grades[cur_anchor->get_y()]; //point-line duality requires multiplying by -1
            leftedge = arrangement->insert_vertex(leftedge, 0, dual_point_y_coord); //set leftedge to edge that will follow the new edge
            prev_y = cur_anchor->get_y(); //remember the discrete y-index
        }

        //now insert new edge at origin vertex of leftedge
        std::shared_ptr<Halfedge> new_edge = arrangement->create_edge_left(leftedge, cur_anchor);

        //remember Halfedge corresponding to this Anchor
        lines.push_back(new_edge);

        //remember relative position of this Anchor
        cur_anchor->set_position(lines.size() - 1);

        //remember line associated with this Anchor
        cur_anchor->set_line(new_edge);
    }

    //for each pair of consecutive lines, if they intersect, store the intersection
    for (unsigned i = 0; i + 1 < lines.size(); i++) {
        std::shared_ptr<Anchor> a = lines[i]->get_anchor();
        std::shared_ptr<Anchor> b = lines[i + 1]->get_anchor();
        if (a->comparable(*b)) //then the Anchors are (strongly) comparable, so we must store an intersection
            crossings.push(new Arrangement::Crossing(a, b, arrangement));

        //remember that we have now considered this intersection
        considered_pairs.insert(Anchor_pair(a, b));
    }

    // PART 2: PROCESS INTERIOR INTERSECTIONS
    //    order: x left to right; for a given x, then y low to high
    if (verbosity >= 8) {
        debug() << "PART 2: PROCESSING INTERIOR INTERSECTIONS\n";
    }

    int status_counter = 0;
    int status_interval = 10000; //controls frequency of output

    //current position of sweep line
    Arrangement::Crossing* sweep = NULL;

    while (!crossings.empty()) {
        //get the next intersection from the queue
        Arrangement::Crossing* cur = crossings.top();
        crossings.pop();

        //process the intersection
        sweep = cur;
        unsigned first_pos = cur->a->get_position(); //most recent edge in the curve corresponding to Anchor a
        unsigned last_pos = cur->b->get_position(); //most recent edge in the curve corresponding to Anchor b

        if (last_pos != first_pos + 1) {
            throw std::runtime_error("intersection between non-consecutive curves [1]: x = "
                + std::to_string(sweep->x) + ", last_pos = " + std::to_string(last_pos)
                + std::to_string(last_pos) + ", first_pos + 1 = " + std::to_string(first_pos + 1));
        }

        //find out if more than two curves intersect at this point
        while (!crossings.empty() && sweep->x_equal(crossings.top()) && (cur->b == crossings.top()->a)) {
            cur = crossings.top();
            crossings.pop();

            if (cur->b->get_position() != last_pos + 1) {
                throw std::runtime_error("intersection between non-consecutive curves [2]");
            }

            last_pos++; //last_pos = cur->b->get_position();
        }

        //compute y-coordinate of intersection
        double intersect_y = arrangement->x_grades[sweep->a->get_x()] * (sweep->x) - arrangement->y_grades[sweep->a->get_y()];

        if (verbosity >= 10) {
            debug() << "  found intersection between"
                    << (last_pos - first_pos + 1) << "edges at x =" << sweep->x << ", y =" << intersect_y;
        }

        //create new vertex
        auto new_vertex = std::make_shared<Vertex>(sweep->x, intersect_y);
        arrangement->vertices.push_back(new_vertex);

        //anchor edges to vertex and create new face(s) and edges	//TODO: check this!!!
        std::shared_ptr<Halfedge> prev_new_edge = NULL; //necessary to remember the previous new edge at each interation of the loop
        std::shared_ptr<Halfedge> first_incoming = lines[first_pos]; //necessary to remember the first incoming edge
        std::shared_ptr<Halfedge> prev_incoming = NULL; //necessary to remember the previous incoming edge at each iteration of the loop
        for (unsigned cur_pos = first_pos; cur_pos <= last_pos; cur_pos++) {
            //anchor edge to vertex
            std::shared_ptr<Halfedge> incoming = lines[cur_pos];
            incoming->get_twin()->set_origin(new_vertex);

            //create next pair of twin halfedges along the current curve (i.e. curves[incident_edges[i]] )
            std::shared_ptr<Halfedge> new_edge(new Halfedge(new_vertex, incoming->get_anchor())); //points AWAY FROM new_vertex
            arrangement->halfedges.push_back(new_edge);
            std::shared_ptr<Halfedge> new_twin(new Halfedge(NULL, incoming->get_anchor())); //points TOWARDS new_vertex
            arrangement->halfedges.push_back(new_twin);

            //update halfedge pointers
            new_edge->set_twin(new_twin);
            new_twin->set_twin(new_edge);

            if (cur_pos == first_pos) //then this is the first iteration of the loop
            {
                new_twin->set_next(lines[last_pos]->get_twin());
                lines[last_pos]->get_twin()->set_prev(new_twin);

                new_twin->set_face(lines[last_pos]->get_twin()->get_face());
            } else //then this is not the first iteration of the loop, so close up a face and create a new face
            {
                incoming->set_next(prev_incoming->get_twin());
                incoming->get_next()->set_prev(incoming);

                std::shared_ptr<Face> new_face(new Face(new_twin, arrangement->faces.size()));
                arrangement->faces.push_back(new_face);

                new_twin->set_face(new_face);
                prev_new_edge->set_face(new_face);

                new_twin->set_next(prev_new_edge);
                prev_new_edge->set_prev(new_twin);
            }

            //remember important halfedges for the next iteration of the loop
            prev_incoming = incoming;
            prev_new_edge = new_edge;

            if (cur_pos == last_pos) //then this is the last iteration of loop
            {
                new_edge->set_prev(first_incoming);
                first_incoming->set_next(new_edge);

                new_edge->set_face(first_incoming->get_face());
            }

            //update lines vector
            lines[cur_pos] = new_edge; //the portion of this vector [first_pos, last_pos] must be reversed after this loop is finished!

            //remember position of this Anchor
            new_edge->get_anchor()->set_position(last_pos - (cur_pos - first_pos));
        }

        //update lines vector: flip portion of vector [first_pos, last_pos]
        for (unsigned i = 0; i < (last_pos - first_pos + 1) / 2; i++) {
            //swap curves[first_pos + i] and curves[last_pos - i]
            std::shared_ptr<Halfedge> temp = lines[first_pos + i];
            lines[first_pos + i] = lines[last_pos - i];
            lines[last_pos - i] = temp;
        }

        //find new intersections and add them to intersections queue
        if (first_pos > 0) //then consider lower intersection
        {
            std::shared_ptr<Anchor> a = lines[first_pos - 1]->get_anchor();
            std::shared_ptr<Anchor> b = lines[first_pos]->get_anchor();

            if (considered_pairs.find(Anchor_pair(a, b)) == considered_pairs.end()
                && considered_pairs.find(Anchor_pair(b, a)) == considered_pairs.end()) //then this pair has not yet been considered
            {
                considered_pairs.insert(Anchor_pair(a, b));
                if (a->comparable(*b)) //then the Anchors are (strongly) comparable, so we have found an intersection to store
                    crossings.push(new Arrangement::Crossing(a, b, arrangement));
            }
        }

        if (last_pos + 1 < lines.size()) //then consider upper intersection
        {
            std::shared_ptr<Anchor> a = lines[last_pos]->get_anchor();
            std::shared_ptr<Anchor> b = lines[last_pos + 1]->get_anchor();

            if (considered_pairs.find(Anchor_pair(a, b)) == considered_pairs.end()
                && considered_pairs.find(Anchor_pair(b, a)) == considered_pairs.end()) //then this pair has not yet been considered
            {
                considered_pairs.insert(Anchor_pair(a, b));
                if (a->comparable(*b)) //then the Anchors are (strongly) comparable, so we have found an intersection to store
                    crossings.push(new Arrangement::Crossing(a, b, arrangement));
            }
        }

        //output status
        if (verbosity >= 8) {
            status_counter++;
            if (status_counter % status_interval == 0)
                debug() << "      processed" << status_counter << "intersections"; //TODO: adding this makes debug go into an infinite loop: <<  "sweep position =" << *sweep;
        }
    } //end while

    // PART 3: INSERT VERTICES ON RIGHT EDGE OF ARRANGEMENT AND CONNECT EDGES
    if (verbosity >= 8) {
        debug() << "PART 3: RIGHT EDGE OF THE ARRANGEMENT";
    }

    std::shared_ptr<Halfedge> rightedge = arrangement->bottomright; //need a reference halfedge along the right side of the strip
    unsigned cur_x = 0; //keep track of discrete x-coordinate of last Anchor whose line was connected to right edge (x-coordinate of Anchor is slope of line)

    //connect each line to the right edge of the arrangement (at x = INFTY)
    //    requires creating a vertex for each unique slope (i.e. Anchor x-coordinate)
    //    lines that have the same slope m are "tied together" at the same vertex, with coordinates (INFTY, Y)
    //    where Y = INFTY if m is positive, Y = -INFTY if m is negative, and Y = 0 if m is zero
    for (unsigned cur_pos = 0; cur_pos < lines.size(); cur_pos++) {
        std::shared_ptr<Halfedge> incoming = lines[cur_pos];
        std::shared_ptr<Anchor> cur_anchor = incoming->get_anchor();

        if (cur_anchor->get_x() > cur_x || cur_pos == 0) //then create a new vertex for this line
        {
            cur_x = cur_anchor->get_x();

            double Y = INFTY; //default, for lines with positive slope
            if (arrangement->x_grades[cur_x] < 0)
                Y = -1 * Y; //for lines with negative slope
            else if (arrangement->x_grades[cur_x] == 0)
                Y = 0; //for horizontal lines

            rightedge = arrangement->insert_vertex(rightedge, INFTY, Y);
        } else //no new vertex required, but update previous entry for vertical-line queries
            arrangement->vertical_line_query_list.pop_back();

        //store Halfedge for vertical-line queries
        arrangement->vertical_line_query_list.push_back(incoming->get_twin());

        //connect current line to the most-recently-inserted vertex
        std::shared_ptr<Vertex> cur_vertex = rightedge->get_origin();
        incoming->get_twin()->set_origin(cur_vertex);

        //update halfedge pointers
        incoming->set_next(rightedge->get_twin()->get_next());
        incoming->get_next()->set_prev(incoming);

        incoming->get_next()->set_face(incoming->get_face()); //only necessary if incoming->get_next() is along the right side of the strip

        incoming->get_twin()->set_prev(rightedge->get_twin());
        rightedge->get_twin()->set_next(incoming->get_twin());

        rightedge->get_twin()->set_face(incoming->get_twin()->get_face());
    }

} //end build_interior()

//computes and stores the edge weight for each anchor line
void ArrangementBuilder::find_edge_weights(Arrangement& arrangement, PersistenceUpdater& updater)
{
    std::vector<std::shared_ptr<Halfedge>> pathvec;
    std::shared_ptr<Halfedge> cur_edge = arrangement.topright;

    //find a path across all anchor lines
    while (cur_edge->get_twin() != arrangement.bottomright) //then there is another vertex to consider on the right edge
    {
        cur_edge = cur_edge->get_next();
        while (cur_edge->get_twin()->get_face() != NULL) //then we have another edge crossing to append to the path
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

} //end set_edge_weights()

//finds a pseudo-optimal path through all 2-cells of the arrangement
// path consists of a vector of Halfedges
// at each step of the path, the Halfedge points to the Anchor being crossed and the 2-cell (Face) being entered
void ArrangementBuilder::find_path(Arrangement& arrangement, std::vector<std::shared_ptr<Halfedge>>& pathvec)
{
    // PART 1: BUILD THE DUAL GRAPH OF THE ARRANGEMENT

    typedef boost::property<boost::edge_weight_t, unsigned long> EdgeWeightProperty;
    typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
        boost::no_property, EdgeWeightProperty>
        Graph; //TODO: probably listS is a better choice than vecS, but I don't know how to make the adjacency_list work with listS
    Graph dual_graph;

    // distance vector for sorting the adjacency list
    std::vector<std::vector<unsigned>> distances(arrangement.faces.size(), std::vector<unsigned>(arrangement.faces.size(), -1));
    for (size_t i = 0; i < distances.size(); ++i)
        distances.at(i).at(i) = 0;

    //loop over all arrangement.faces
    for (unsigned i = 0; i < arrangement.faces.size(); i++) {
        //consider all neighbors of this arrangement.faces
        std::shared_ptr<Halfedge> boundary = (arrangement.faces[i])->get_boundary();
        std::shared_ptr<Halfedge> current = boundary;
        do {
            //find index of neighbor
            std::shared_ptr<Face> neighbor = current->get_twin()->get_face();
            if (neighbor != NULL) {
                unsigned long j = neighbor->id();

                //if i < j, then create an (undirected) edge between these arrangement.faces
                if (i < j) {
                    boost::add_edge(i, j, current->get_anchor()->get_weight(), dual_graph);
                    distances.at(i).at(j) = current->get_anchor()->get_weight();
                    distances.at(j).at(i) = current->get_anchor()->get_weight();
                }
            }
            //move to the next neighbor
            current = current->get_next();
        } while (current != boundary);
    }

    //TESTING -- print the edges in the dual graph
    if (verbosity >= 10) {
        debug() << "EDGES IN THE DUAL GRAPH OF THE ARRANGEMENT: ";
        typedef boost::graph_traits<Graph>::edge_iterator edge_iterator;
        std::pair<edge_iterator, edge_iterator> ei = boost::edges(dual_graph);
        for (edge_iterator it = ei.first; it != ei.second; ++it)
            debug(true) << "  (" << boost::source(*it, dual_graph) << "\t, " << boost::target(*it, dual_graph) << "\t) \tweight = " << boost::get(boost::edge_weight_t(), dual_graph, *it);
    }

    // PART 2: FIND A MINIMAL SPANNING TREE

    typedef boost::graph_traits<Graph>::edge_descriptor Edge;
    std::vector<Edge> spanning_tree_edges;
    boost::kruskal_minimum_spanning_tree(dual_graph, std::back_inserter(spanning_tree_edges));

    //TESTING -- print the MST
    if (verbosity >= 10) {
        debug() << "num MST edges: " << spanning_tree_edges.size() << "\n";
        for (unsigned i = 0; i < spanning_tree_edges.size(); i++)
            debug(true) << "  (" << boost::source(spanning_tree_edges[i], dual_graph) << "\t, " << boost::target(spanning_tree_edges[i], dual_graph) << "\t) \tweight = " << boost::get(boost::edge_weight_t(), dual_graph, spanning_tree_edges[i]);
    }

    // PART 3: CONVERT THE OUTPUT OF PART 2 TO A PATH

    //organize the edges of the minimal spanning tree so that we can traverse the tree
    std::vector<std::vector<unsigned>> adjList(arrangement.faces.size(), std::vector<unsigned>()); //this will store all adjacency relationships in the spanning tree
    for (unsigned i = 0; i < spanning_tree_edges.size(); i++) {
        unsigned a = boost::source(spanning_tree_edges[i], dual_graph);
        unsigned b = boost::target(spanning_tree_edges[i], dual_graph);

        adjList.at(a).push_back(b);
        adjList.at(b).push_back(a);
    }

    //make sure to start at the proper node (2-cell)
    std::shared_ptr<Face> initial_cell = arrangement.topleft->get_twin()->get_face();
    unsigned long start = initial_cell->id();

    //store the children of each node (with initial_cell regarded as the root of the tree)
    std::vector<std::vector<unsigned>> children(arrangement.faces.size(), std::vector<unsigned>());

    // sort child nodes in decreasing order of branch weight to minimize backtracking in the path
    sortAdjacencies(adjList, distances, start, children);

    // now we can find the path
    find_subpath(arrangement, start, children, pathvec);

    //TESTING -- print the path
    if (verbosity >= 10) {
        Debug qd = debug(true);
        qd << "PATH: " << start << ", ";
        for (unsigned i = 0; i < pathvec.size(); i++) {
            unsigned long cur = pathvec[i]->get_face()->id();
            qd << "<" << pathvec[i]->get_anchor()->get_weight() << ">" << cur << ", "; //edge weight appears in angle brackets
        }
        qd << "\n";
    }
} //end find_path()

// Function to find a path through all nodes in a tree, starting at a specified node
// Iterative version: uses a stack to perform a depth-first search of the tree
// Input: tree is specified by the 2-D vector children
//   children[i] is a vector of indexes of the children of node i, in decreasing order of branch weight
//   (branch weight is total weight of all edges below a given node, plus weight of edge to parent node)
// Output: vector pathvec contains a Halfedge pointer for each step of the path
void ArrangementBuilder::find_subpath(Arrangement& arrangement, unsigned start_node, std::vector<std::vector<unsigned>>& children, std::vector<std::shared_ptr<Halfedge>>& pathvec)
{
    std::stack<unsigned> nodes; // stack for nodes as we do DFS
    nodes.push(start_node); // push node onto the node stack
    std::stack<std::shared_ptr<Halfedge>> backtrack; // stack for storing extra copy of std::shared_ptr<Halfedge>* so we don't have to recalculate when popping
    unsigned numDiscovered = 1, numNodes = children.size();

    while (numDiscovered != numNodes) // while we have not traversed the whole tree
    {
        unsigned node = nodes.top(); // let node be the current node that we are considering

        if (children[node].size() != 0) // if we have not traversed all of node's children
        {
            // find the halfedge to be traversed
            unsigned next_node = children[node].back();
            children[node].pop_back();

            std::shared_ptr<Halfedge> cur_edge = (arrangement.faces[node])->get_boundary();
            while (cur_edge->get_twin()->get_face() != arrangement.faces[next_node]) {
                cur_edge = cur_edge->get_next();

                if (cur_edge == (arrangement.faces[node])->get_boundary()) {
                    debug() << "ERROR:  cannot find edge between 2-cells " << node << " and " << next_node << "\n";
                    throw std::exception();
                }
            }
            // and push it onto pathvec
            pathvec.push_back(cur_edge->get_twin());
            // push a copy onto the backtracking stack so we don't have to search for this std::shared_ptr<Halfedge>* again when popping
            backtrack.push(cur_edge);
            // push the next node onto the stack
            nodes.push(next_node);
            // increment the discovered counter
            ++numDiscovered;
        } // end if
        else // we have traversed all of node's children
        {
            nodes.pop(); // pop node off of the node stack
            pathvec.push_back(backtrack.top()); // push the top of backtrack onto pathvec
            backtrack.pop(); // and pop that std::shared_ptr<Halfedge>* off of backtrack
        }
    }

} //end find_subpath()
