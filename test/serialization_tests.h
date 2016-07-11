//
// Created by Bryn Keller on 7/11/16.
//

#ifndef RIVET_CONSOLE_SERIALIZATION_TESTS_H_H
#define RIVET_CONSOLE_SERIALIZATION_TESTS_H_H

#endif //RIVET_CONSOLE_SERIALIZATION_TESTS_H_H
#include "base_64.h"

TEST_CASE( "Base64 encoding works", "[Serialization]" ) {

std::string test = "This is a test. This is only a test.";
std::string result = encode64(test);
std::string decoded = decode64(result);

REQUIRE (test == decoded);
}


