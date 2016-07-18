#include "mesh_message.h"//
// Created by Bryn Keller on 7/15/16.
//

#include "dcel/mesh_message.h"
#include "dcel/mesh.h"
#include <boost/optional.hpp>

MeshMessage::MeshMessage(Mesh const &mesh):
            x_grades(mesh.x_grades),
            y_grades(mesh.y_grades),
            half_edges(),
            vertices(),
            anchors(),
            faces(),
            vertical_line_query_list()
    {
        //REALLY slow with all these VID, FID, etc. calls, but will do for now.
        for(auto face: mesh.faces) {
            faces.push_back(Face{HalfedgeId(mesh.HID(face->get_boundary())), face->get_barcode()});
        }
        for(auto half: mesh.halfedges) {
            half_edges.push_back(Halfedge {
                    VertexId(mesh.VID(half->get_origin())),
                    HalfedgeId(mesh.HID(half->get_twin())),
                    HalfedgeId(mesh.HID(half->get_next())),
                    HalfedgeId(mesh.HID(half->get_prev())),
                    FaceId(mesh.FID(half->get_face())),
                    AnchorId(mesh.AID(half->get_anchor()))
            });
        }
        for(auto anchor: mesh.all_anchors) {
            std::cerr << "Adding anchor: " << anchor->get_x() << ", " << anchor->get_y() << std::endl;
            anchors.push_back(Anchor{
                    anchor->get_x(),
                    anchor->get_y(),
                    HalfedgeId(mesh.HID(anchor->get_line())),
                    anchor->get_position(),
                    anchor->is_above(),
                    anchor->get_weight()
            });
        }
        assert(anchors.size() == mesh.all_anchors.size());
        for (auto vertex: mesh.vertices) {
            vertices.push_back(Vertex{
                    HalfedgeId(mesh.HID(vertex->get_incident_edge())),
                    vertex->get_x(),
                    vertex->get_y()
            });
        }

        for (auto query: mesh.vertical_line_query_list) {
            vertical_line_query_list.push_back(HalfedgeId(mesh.HID(query)));
        }

        topleft = HalfedgeId(mesh.HID(mesh.topleft));
        topright = HalfedgeId(mesh.HID(mesh.topright));
        bottomleft = HalfedgeId(mesh.HID(mesh.bottomleft));
        bottomright = HalfedgeId(mesh.HID(mesh.bottomright));
    }

    MeshMessage::MeshMessage() : half_edges(), vertices(), anchors(), faces() {}


//finds the first anchor that intersects the left edge of the arrangement at a point not less than the specified y-coordinate
//  if no such anchor, returns nullptr
    boost::optional<MeshMessage::Anchor> MeshMessage::find_least_upper_anchor(double y_coord)
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
            return boost::none;

        //if we get here, then y_grades[best] is the greatest y-grade not greater than y_coord
        //now find Anchor whose line intersects the left edge of the arrangement lowest, but not below y_grade[best]
        unsigned int zero = 0;  //disambiguate the following function call
        Anchor test{zero, best};
        auto it = std::lower_bound(anchors.begin(), anchors.end(), test, AnchorStructComparator());

        if(it == anchors.end())    //not found
        {
            return boost::none;
        }
        //else
        return *it;
    }//end find_least_upper_anchor()
//finds the (unbounded) cell associated to dual point of the vertical line with the given x-coordinate
//  i.e. finds the Halfedge whose Anchor x-coordinate is the largest such coordinate not larger than than x_coord; returns the Face corresponding to that Halfedge
    MeshMessage::FaceId MeshMessage::find_vertical_line(double x_coord)
    {
        //is there an Anchor with x-coordinate not greater than x_coord?
        if(vertical_line_query_list.size() >= 1
           && x_grades[ get(get(vertical_line_query_list[0]).anchor).x_coord ] <= x_coord)
        {
            //binary search the vertical line query list
            unsigned min = 0;
            unsigned max = vertical_line_query_list.size() - 1;
            unsigned best = 0;

            while(max >= min)
            {
                unsigned mid = (max + min)/2;
                auto test = get(get(vertical_line_query_list[mid]).anchor);

                if(x_grades[test.x_coord] <= x_coord)    //found a lower bound, but search upper subarray for a better lower bound
                {
                    best = mid;
                    min = mid + 1;
                }
                else    //search lower subarray
                    max = mid - 1;
            }

            return get(vertical_line_query_list[best]).face;
        }

        //if we get here, then either there are no Anchors or x_coord is less than the x-coordinates of all Anchors
        return get(get(bottomright).twin).face;

    }//end find_vertical_line()

////find a 2-cell containing the specified point
    MeshMessage::FaceId MeshMessage::find_point(double x_coord, double y_coord)
    {
        //start on the left edge of the arrangement, at the correct y-coordinate
        boost::optional<Anchor> start = find_least_upper_anchor(-1*y_coord);

        HalfedgeId finger; //for use in finding the cell

        if(!start)	//then starting point is in the top (unbounded) cell
        {
            finger = get(get(topleft).twin).next;   //this is the top edge of the top cell (at y=infty)
        }
        else
        {
            finger = start.get().dual_line;
        }

        FaceId cell; //will later point to the cell containing the specified point

        while(static_cast<long>(cell) < 0) //while not found
        {
            //find the edge of the current cell that crosses the horizontal line at y_coord
            VertexId next_pt = get(get(finger).next).origin;

            while(get(next_pt).y > y_coord)
            {
                finger = get(finger).next;
                next_pt = get(get(finger).next).origin;
            }

            Vertex vertex = get(next_pt);
            //now next_pt is at or below the horizontal line at y_coord
            //if (x_coord, y_coord) is to the left of crossing point, then we have found the cell; otherwise, move to the adjacent cell

            if(vertex.y == y_coord) //then next_pt is on the horizontal line
            {
                if(vertex.x >= x_coord)	//found the cell
                {
                    cell = get(finger).face;
                }
                else	//move to adjacent cell
                {
                    //find degree of vertex
                    HalfedgeId thumb = get(finger).next;
                    int deg = 1;
                    while(thumb != get(finger).twin)
                    {
                        thumb = get(get(thumb).twin).next;
                        deg++;
                    }

                    //move halfway around the vertex
                    finger = get(finger).next;
                    for(int i=0; i<deg/2; i++)
                    {
                        finger = get(get(finger).twin).next;
                    }
                }
            }
            else	//then next_pt is below the horizontal line
            {
                if(get(finger).anchor == AnchorId::invalid())	//then edge is vertical, so we have found the cell
                {
                    cell = get(finger).face;
                }
                else	//then edge is not vertical
                {
                    Anchor temp = get(get(finger).anchor);
                    double x_pos = ( y_coord + y_grades[temp.get_y()] )/x_grades[temp.get_x()];  //NOTE: division by zero never occurs because we are searching along a horizontal line, and thus we never cross horizontal lines in the arrangement

                    if(x_pos >= x_coord)	//found the cell
                    {
                        cell = get(finger).face;
                    }
                    else	//move to adjacent cell
                    {
                        finger = get(finger).twin;

                    }
                }
            }//end else
        }//end while(cell not found)
        return cell;
    }//end find_point()

//returns barcode template associated with the specified line (point)
//REQUIREMENT: 0 <= degrees <= 90

    MeshMessage::Halfedge & MeshMessage::get(HalfedgeId index) {
        return half_edges[static_cast<long>(index)];
    }
    MeshMessage::Face & MeshMessage::get(MeshMessage::FaceId index) {
        return faces[static_cast<long>(index)];
    }
    MeshMessage::Vertex & MeshMessage::get(MeshMessage::VertexId index) {
        return vertices[static_cast<long>(index)];
    }
    MeshMessage::Anchor & MeshMessage::get(MeshMessage::AnchorId index) {
        return anchors[static_cast<long>(index)];
    }

    boost::optional<BarcodeTemplate> MeshMessage::get_barcode_template(double degrees, double offset)
    {
        ///TODO: store some point/cell to seed the next query
        FaceId cell;
        if(degrees == 90) //then line is vertical
        {
            cell = find_vertical_line(-1*offset); //multiply by -1 to correct for orientation of offset
        } else if (degrees == 0) { //then line is horizontal
            auto anchor = find_least_upper_anchor(offset);

            if(anchor)
                cell = get(anchor.get().dual_line).face;
            else
                cell = get(get(topleft).twin).face;    //default
        } else {
            //else: the line is neither horizontal nor vertical
            double radians = degrees * 3.14159265 / 180;
            double slope = tan(radians);
            double intercept = offset / cos(radians);
            cell = find_point(slope, -1 * intercept);   //multiply by -1 for point-line duality
        }
        ///TODO: REPLACE THIS WITH A SEEDED SEARCH

        return get(cell).dbc;
    }//end get_barcode_template()

bool check(bool condition, std::string message) {
    if (!condition) {
        std::cerr << message;
    }
    return condition;
}

bool operator==(MeshMessage::Halfedge const & left, MeshMessage::Halfedge const & right) {
    return left.origin == right.origin
            && left.anchor == right.anchor
            && left.face == right.face
            && left.next == right.next
            && left.prev == right.prev
        && left.twin == right.twin;
}


bool operator==(MeshMessage::Vertex const & left, MeshMessage::Vertex const & right) {
    return left.incident_edge == right.incident_edge
            && left.x == right.x
            && left.y == right.y;
}

bool operator==(MeshMessage::Face const & left, MeshMessage::Face const & right) {
    return left.boundary == right.boundary
            && left.dbc == right.dbc;
}

bool operator==(MeshMessage::Anchor const & left, MeshMessage::Anchor const & right) {
    return left.above_line == right.above_line
            && left.dual_line == right.dual_line
            && left.position == right.position
            && left.weight == right.weight
            && left.x_coord == right.x_coord
            && left.y_coord == right.y_coord;
}

bool operator==(MeshMessage const & left, MeshMessage const & right) {
    bool corners = left.topright == right.topright
    && left.topleft == right.topleft
    && left.bottomleft == right.bottomleft
    && left.bottomright == right.bottomright;
    if (!check(corners, "corners")) return false;
    if (!check(left.vertical_line_query_list == right.vertical_line_query_list, "query")) return false;
    if (!check(left.vertices == right.vertices, "vertices")) return false;
    if (!check(left.anchors == right.anchors, "anchors")) return false;
    if (!check(left.faces == right.faces, "faces")) return false;
    if (!check(left.half_edges == right.half_edges, "edges")) return false;
    if (!check(left.x_grades == right.x_grades, "x_grades")) return false;
    if (!check(left.y_grades == right.y_grades, "y_grades")) return false;
    return true;
}

template <typename T, typename U, typename ID>
void set_if_valid(std::function<void(U)> func, std::vector<T> vec, ID id) {
    if (id != ID::invalid()) {
        func(vec[static_cast<long>(id)]);
    }

};
Mesh MeshMessage::to_mesh() const {
    Mesh mesh;
    //First create all the objects
    for (auto vertex : vertices) {
        mesh.vertices.push_back(std::make_shared<::Vertex>(vertex.x, vertex.y));
    }
    for (auto face : faces) {
        mesh.faces.push_back(std::make_shared<::Face>());
    }
    for (auto edge : half_edges) {
        mesh.halfedges.push_back(std::make_shared<::Halfedge>());
    }
    std::vector<std::shared_ptr<::Anchor>> temp_anchors; //For indexing, since mesh.all_anchors is a set
    for (auto anchor : anchors) {

        std::shared_ptr<::Anchor> ptr = std::make_shared<::Anchor>(anchor.x_coord, anchor.y_coord);
        temp_anchors.push_back(ptr);
        mesh.all_anchors.insert(ptr);
    }

    //Now populate all the pointers

    for (int i = 0; i < vertices.size(); i++) {
        if (vertices[i].incident_edge != HalfedgeId::invalid()) {
            mesh.vertices[i]->set_incident_edge(mesh.halfedges[static_cast<long>(vertices[i].incident_edge)]);
        }
    }
    for (int i = 0; i < faces.size(); i++) {
        auto mface = mesh.faces[i];
        auto face = faces[i];
        //TODO: this doesn't seem right, why would a face not have a boundary?
        if (faces[i].boundary != HalfedgeId::invalid()) {
            mface->set_boundary(mesh.halfedges[static_cast<long>(face.boundary)]);
        }
        if (face.dbc) {
            mface->set_barcode(face.dbc.get());
        }
    }
    for (int i = 0; i < half_edges.size(); i++) {
        ::Halfedge &edge = *(mesh.halfedges[i]);
        MeshMessage::Halfedge ref = half_edges[i];
        if (ref.face != FaceId::invalid())
            edge.set_face(mesh.faces[static_cast<long>(ref.face)]);
        if (ref.anchor != AnchorId::invalid()) {
            edge.set_anchor(temp_anchors[static_cast<long>(ref.anchor)]);
        }
        //TODO: shouldn't a halfedge always have a next?
        if (ref.next != HalfedgeId::invalid()) {
            edge.set_next(mesh.halfedges[static_cast<long>(ref.next)]);
        }
        if (ref.origin != VertexId::invalid()) {
            edge.set_origin(mesh.vertices[static_cast<long>(ref.origin)]);
        }
        if (ref.prev != HalfedgeId::invalid()) {
            edge.set_prev(mesh.halfedges[static_cast<long>(ref.prev)]);
        }
        if (ref.twin != HalfedgeId::invalid()) {
            edge.set_twin(mesh.halfedges[static_cast<long>(ref.twin)]);
        }
    }

    for (int i = 0; i < vertical_line_query_list.size(); i++) {
        mesh.vertical_line_query_list.push_back(mesh.halfedges[static_cast<long>(vertical_line_query_list[i])]);
    }

    assert(mesh.all_anchors.size() == anchors.size());
    auto it = mesh.all_anchors.begin();
    for (int i = 0; i < anchors.size(); i++) {
        ::Anchor &anchor = **it;
        MeshMessage::Anchor ref = anchors[i];
        if (ref.dual_line != HalfedgeId::invalid()) {
            std::shared_ptr<::Halfedge> edge = mesh.halfedges[static_cast<long>(ref.dual_line)];
            //TODO: why, oh why, should this reset be necessary?
            anchor.get_line().reset();
            anchor.set_line(edge);
        }
        anchor.set_position(ref.position);
        anchor.set_weight(ref.weight);
        if (it != mesh.all_anchors.end()) {
            ++it;
        }
    }
    mesh.bottomleft = mesh.halfedges[static_cast<long>(bottomleft)];
    mesh.bottomright = mesh.halfedges[static_cast<long>(bottomright)];
    mesh.topright = mesh.halfedges[static_cast<long>(topright)];
    mesh.topleft = mesh.halfedges[static_cast<long>(topleft)];

    mesh.x_grades = x_grades;
    mesh.y_grades = y_grades;
    return mesh;
}