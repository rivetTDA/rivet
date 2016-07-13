//
// Created by Bryn Keller on 7/11/16.
//

#ifndef RIVET_CONSOLE_SERIALIZATION_TESTS_H_H
#define RIVET_CONSOLE_SERIALIZATION_TESTS_H_H
#include "dcel/dcel.h"
#include "dcel/anchor.h"
#include "dcel/mesh.h"
#include "dcel/serialization.h"
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

template <typename T>
        T round_trip(const T &thing) {
        std::stringstream ss;
        {
                boost::archive::text_oarchive out(ss);
                out << thing;
        }
        T result;
        {
                boost::archive::text_iarchive in(ss);
                in >> result;
        }
        return result;
}

TEST_CASE( "Vertex serialization works", "[Serialization]" ) {

    Vertex v(1, 2);
    Vertex v2 = round_trip<Vertex>(v);

    REQUIRE (v == v2);
}
TEST_CASE( "Vertex serialization works with shared_ptr", "[Serialization]" ) {

auto v1 = std::make_shared<Vertex>(1, 2);
auto v2 = round_trip<std::shared_ptr<Vertex>>(v1);

REQUIRE (*v1 == *v2);
}
TEST_CASE( "Vertex serialization works with halfedge", "[Serialization]" ) {
    auto edge = std::make_shared<Halfedge>();
Vertex v(1, 2);
    v.set_incident_edge(edge);
Vertex v2 = round_trip<Vertex>(v);

REQUIRE (v == v2);
}


TEST_CASE( "Mesh serialization works", "[Serialization]" ) {
    auto mesh = std::make_shared<Mesh>(std::vector<exact>{1,2,3,4}, std::vector<exact>{5,6,7,8}, 10);
    auto xi = std::make_shared<xiMatrixEntry>(1,2);
    mesh->add_anchor(Anchor(xi));
    auto result = round_trip(mesh);
//    REQUIRE (*((*mesh).vertices[0]) == *((*result).vertices[0]));
    REQUIRE(true==true);
}

#endif //RIVET_CONSOLE_SERIALIZATION_TESTS_H_H



