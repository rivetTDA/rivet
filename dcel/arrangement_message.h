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
//
// Created by Bryn Keller on 7/15/16.
//

#ifndef RIVET_CONSOLE_MESH_MESSAGE_H
#define RIVET_CONSOLE_MESH_MESSAGE_H
#include "dcel/anchor.h"
#include "dcel/arrangement.h"
#include "dcel/barcode_template.h"
#include "dcel/dcel.h"
#include "type_tag.h"
#include <boost/optional.hpp>
#include <boost/serialization/split_member.hpp>

class ArrangementMessage {

public:
    ArrangementMessage(Arrangement const& arrangement);

    ArrangementMessage();

    template <class Archive>
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar& x_grades& y_grades& x_exact& y_exact& half_edges& vertices& anchors& faces& topleft& topright& bottomleft& bottomright& vertical_line_query_list;
    }

    BarcodeTemplate get_barcode_template(double degrees, double offset);

    friend bool operator==(ArrangementMessage const& left, ArrangementMessage const& right);

    Arrangement to_arrangement() const;

    bool is_empty() const;

private:
    friend class boost::serialization::access;

    typedef ID<Vertex, long, -1> VertexId;
    typedef ID<Anchor, long, -1> AnchorId;
    typedef ID<Face, long, -1> FaceId;
    typedef ID<Halfedge, long, -1> HalfedgeId;

    std::vector<double> x_grades; //floating-point values for x-grades
    std::vector<double> y_grades; //floating-point values for y-grades

    std::vector<exact> x_exact; //exact values for x-grades
    std::vector<exact> y_exact; //exact values for y-grades

    HalfedgeId topleft; //pointer to Halfedge that points down from top left corner (0,infty)
    HalfedgeId topright; //pointer to Halfedge that points down from the top right corner (infty,infty)
    HalfedgeId bottomleft; //pointer to Halfedge that points up from bottom left corner (0,-infty)
    HalfedgeId bottomright; //pointer to Halfedge that points up from bottom right corner (infty,-infty)

    std::vector<HalfedgeId> vertical_line_query_list; //stores a pointer to the rightmost Halfedge of the "top" line of each unique slope, ordered from small slopes to big slopes (each Halfedge points to Anchor and Face for vertical-line queries)

    struct AnchorM {

        unsigned x_coord; //discrete x-coordinate
        unsigned y_coord; //discrete y-coordinate

        //get_(x|y) needed to support AnchorComparator
        unsigned get_x() const { return x_coord; }
        unsigned get_y() const { return y_coord; }

        HalfedgeId dual_line; //pointer to left-most halfedge corresponding to this Anchor in the arrangement
        unsigned position; //relative position of Anchor line at sweep line, used for Bentley-Ottmann DCEL construction algorithm
        bool above_line; //true iff this Anchor is above the current slice line, used for the vineyard-update process of storing persistence data in cells of the arrangement
        unsigned long weight; //estimate of the cost of updating the RU-decomposition when crossing this anchor

        template <typename Archive>
        void serialize(Archive& ar, const unsigned int /*version*/)
        {
            ar& x_coord& y_coord& dual_line& position& above_line& weight;
        }
    };

    friend bool operator==(AnchorM const& left, AnchorM const& right);

    struct AnchorStructComparator : AnchorComparator<ArrangementMessage::AnchorM> {
    };

    struct HalfedgeM {

        VertexId origin; //pointer to the vertex from which this halfedge originates
        HalfedgeId twin; //pointer to the halfedge that, together with this halfedge, make one edge
        HalfedgeId next; //pointer to the next halfedge around the boundary of the face to the right of this halfedge
        HalfedgeId prev; //pointer to the previous halfedge around the boundary of the face to the right of this halfedge
        FaceId face; //pointer to the face to the right of this halfedge
        AnchorId anchor; //stores the coordinates of the anchor corresponding to this halfedge

        template <typename Archive>
        void serialize(Archive& ar, const unsigned int /*version*/)
        {
            ar& origin& twin& next& prev& face& anchor;
        }
    };

    friend bool operator==(HalfedgeM const& left, HalfedgeM const& right);

    struct FaceM {
        HalfedgeId boundary; //pointer to one halfedge in the boundary of this cell
        BarcodeTemplate dbc; //barcode template stored in this cell

        template <typename Archive>
        void serialize(Archive& ar, const unsigned int /*version*/)
        {
            ar& boundary& dbc;
        }
    };

    friend bool operator==(FaceM const& left, FaceM const& right);

    struct VertexM {
        HalfedgeId incident_edge; //pointer to one edge incident to this vertex
        double x; //x-coordinate of this vertex
        double y; //y-coordinate of this vertex

        template <typename Archive>
        void serialize(Archive& ar, const unsigned int /*version*/)
        {
            ar& incident_edge& x& y;
        }
    };

    friend bool operator==(VertexM const& left, VertexM const& right);
    std::vector<HalfedgeM> half_edges;
    std::vector<VertexM> vertices;
    std::vector<AnchorM> anchors;
    std::vector<FaceM> faces;

    //finds the first anchor that intersects the left edge of the arrangement at a point not less than the specified y-coordinate
    //  if no such anchor, returns nullptr
    boost::optional<AnchorM> find_least_upper_anchor(double y_coord);

    //finds the (unbounded) cell associated to dual point of the vertical line with the given x-coordinate
    //  i.e. finds the Halfedge whose Anchor x-coordinate is the largest such coordinate not larger than than x_coord; returns the Face corresponding to that Halfedge
    FaceId find_vertical_line(double x_coord);

    ////find a 2-cell containing the specified point
    FaceId find_point(double x_coord, double y_coord);

    HalfedgeM& get(HalfedgeId index);
    FaceM& get(FaceId index);
    VertexM& get(VertexId index);
    AnchorM& get(AnchorId index);
};

#endif //RIVET_CONSOLE_MESH_MESSAGE_H
