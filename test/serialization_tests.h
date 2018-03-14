//
// Created by Bryn Keller on 7/11/16.
//

#ifndef RIVET_CONSOLE_SERIALIZATION_TESTS_H_H
#define RIVET_CONSOLE_SERIALIZATION_TESTS_H_H
#include "dcel/anchor.h"
#include "dcel/arrangement.h"
#include "dcel/dcel.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

template <typename T>
T round_trip_msgpack(const T& thing)
{
    std::stringstream ss;
    msgpack::pack(ss, thing);

    std::string buff(ss.str());
    T result;
    auto oh = msgpack::unpack(buff.data(), buff.size());
    auto obj = oh.get();
    obj.convert(result);
    return result;
}


TEST_CASE("InputParameters can be roundtripped with msgpack", "[serialization - msgpack]")
{

    InputParameters params;
    params.dim = 0;
    params.outputFile = "bogus";
    params.fileName = "inbogus";
    params.outputFormat = "flergh";
    params.x_bins = 10;
    params.y_bins = 10;
    params.x_label = "x";
    params.y_label = "y";

    InputParameters result = round_trip_msgpack(params);

    REQUIRE(params.dim == result.dim);
    REQUIRE(params.fileName == result.fileName);
    REQUIRE(params.outputFile == result.outputFile);
    REQUIRE(params.outputFormat == result.outputFormat);
    REQUIRE(result.outputFormat == "flergh");
    REQUIRE(params.x_bins == result.x_bins);
    REQUIRE(params.y_bins == result.y_bins);
    REQUIRE(params.x_label == result.x_label);
    REQUIRE(params.y_label == result.y_label);

}
#endif //RIVET_CONSOLE_SERIALIZATION_TESTS_H_H
