//
// Created by Bryn Keller on 7/11/16.
//

#ifndef RIVET_CONSOLE_SERIALIZATION_TESTS_H_H
#define RIVET_CONSOLE_SERIALIZATION_TESTS_H_H
#include "dcel/anchor.h"
#include "dcel/dcel.h"
#include "dcel/arrangement.h"
#include "dcel/serialization.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

template <typename T>
T round_trip(const T& thing)
{
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

#endif //RIVET_CONSOLE_SERIALIZATION_TESTS_H_H
