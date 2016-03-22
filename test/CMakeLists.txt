cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)
project (rivet_tests)
add_executable (unit_tests unit_tests.cpp ../math/map_matrix.cpp )

include_directories("${PROJECT_SOURCE_DIR}/..")
