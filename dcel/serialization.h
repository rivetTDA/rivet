//
// Created by Bryn Keller on 7/6/16.
//

#ifndef RIVET_CONSOLE_SERIALIZATION_H
#define RIVET_CONSOLE_SERIALIZATION_H

#include <cereal/cereal.hpp>
#include <cereal/types/set.hpp>
#include <cereal/types/memory.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/list.hpp>
#include <cereal/types/string.hpp>
#include <cereal/types/common.hpp>
#include "numerics.h"
#include "anchor.h"
#include "dcel.h"
#include "math/xi_point.h"
#include "math/xi_support_matrix.h"
#include "barcode_template.h"

namespace boost {
    namespace multiprecision {
        template<class Archive>
        void load(Archive &ar, exact &num) {
            std::string rep;
            ar(rep);
            num.assign(rep);
        }

        template<class Archive>
        void save(Archive &ar, exact const &num ) {
            std::stringstream ss;
            ss << num;
            ar(ss.str());
        }
    }
}

template <class Archive>
void Vertex::cerealize(Archive &ar) {
    ar(incident_edge, x, y);
}

template <class Archive>
void Halfedge::cerealize(Archive &ar) {
    ar(origin, twin, next, prev, face, anchor);
}

template<class Archive>
void Face::cerealize(Archive & ar) {
    ar(boundary, dbc, visited);
}

template <class Archive>
void Anchor::cerealize(Archive &ar) {
    ar(x_coord, y_coord, entry, dual_line, position, above_line, weight);
}

template <class Archive>
void cerealize(Archive &ar, xiMatrixEntry &x) {
    ar(x.x, x.y, x.index, x.down, x.left, x.low_simplices, x.high_simplices, x.low_count, x.high_count,
       x.low_index, x.high_index);
}

template <class Archive>
void cerealize(Archive &ar, Multigrade &m) {
    ar(m.num_cols, m.simplex_index, m.x, m.y);
}

template <class Archive>
void save_exacts(Archive &ar, std::vector<exact> const &vector) {
    ar( cereal::make_size_tag( static_cast<cereal::size_type>(vector.size()) ) ); // number of elements
    for(auto && v : vector)
        ar( v );
}
template <class Archive>
void load_exacts(Archive &ar, std::vector<exact> &vector) {
    ar( cereal::make_size_tag( static_cast<cereal::size_type>(vector.size()) ) ); // number of elements
    for(auto & v : vector)
        ar( v );
}

template <class Archive>
void Mesh::load(Archive &archive) {
    load_exacts(archive, x_exact);
    load_exacts(archive, y_exact);
archive( x_grades, y_grades, vertices, halfedges, faces,
        verbosity, topleft, topright, bottomleft, bottomright, vertical_line_query_list
);
}

template <class Archive>
void Mesh::save(Archive &archive) const {
    save_exacts(archive, x_exact);
    save_exacts(archive, y_exact);
    archive( x_grades, y_grades, vertices, halfedges, faces,
             verbosity, topleft, topright, bottomleft, bottomright, vertical_line_query_list
    );
    archive( x_exact, y_exact, x_grades, y_grades, vertices, halfedges, faces,
             verbosity, topleft, topright, bottomleft, bottomright, vertical_line_query_list
    );
}



#endif //RIVET_CONSOLE_SERIALIZATION_H
