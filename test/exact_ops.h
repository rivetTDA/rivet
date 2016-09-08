//
// Created by Bryn Keller on 6/21/16.
//

#ifndef RIVET_CONSOLE_EXACT_OPS_H
#define RIVET_CONSOLE_EXACT_OPS_H

#endif //RIVET_CONSOLE_EXACT_OPS_H

#include "catch.hpp"
#include "numerics.h"
#include <iostream>
#include <vector>

TEST_CASE("Exact parser parses 12.34", "[Exact]")
{

    exact v = rivet::numeric::str_to_exact("12.34");
    REQUIRE(v == exact(1234, 100));
}

TEST_CASE("Exact parser parses 765", "[Exact]")
{

    exact v = rivet::numeric::str_to_exact("765");
    REQUIRE(v == exact(765));
}

TEST_CASE("Exact parser parses -10.8421", "[Exact]")
{

    exact v = rivet::numeric::str_to_exact("-10.8421");
    REQUIRE(v == exact(-108421, 10000));
}
