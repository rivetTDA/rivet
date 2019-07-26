//
// Created by Bryn Keller on 6/21/16.
//

#ifndef RIVET_CONSOLE_DATA_READER_TESTS_H
#define RIVET_CONSOLE_DATA_READER_TESTS_H

#endif //RIVET_CONSOLE_INPUT_MANAGER_TESTS_H

#include "catch.hpp"
#include "interface/data_reader.h"
#include "numerics.h"
#include <iostream>
#include <vector>

TEST_CASE("DataPoint parses correctly", "[InputManager]")
{
    std::vector<std::string> data{ "1.0", "-1.2", "1.12" };
    DataPoint point(data);

    REQUIRE(point.coords[0] == 1.0);
    REQUIRE(point.coords[1] == -1.2);
    REQUIRE(point.birth == exact(112, 100));
}
