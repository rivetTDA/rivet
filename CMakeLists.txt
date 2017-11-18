cmake_minimum_required (VERSION 3.1)
project (rivet_console)
set (CMAKE_CXX_STANDARD 14)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-ftemplate-depth=1024 -Wall -Wextra -pedantic")

include(ExternalProject)

externalproject_add(
  docopt_project
  PREFIX docopt
  GIT_REPOSITORY https://github.com/docopt/docopt.cpp
  GIT_TAG a4177ccf1a6e36ebe268972732ddd456a3574f6d
  CMAKE_ARGS -D CMAKE_CXX_STANDARD=11 -D CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  INSTALL_COMMAND ""
  )

externalproject_add(
        msgpack_project
        PREFIX msgpack
        GIT_REPOSITORY https://github.com/msgpack/msgpack-c
        GIT_TAG master
        CMAKE_ARGS -D CMAKE_CXX_STANDARD=11 -D CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        INSTALL_COMMAND ""
)

find_package(Boost "1.60" COMPONENTS serialization system)

#note this must come before add_executable or it will be ignored
link_directories(${CMAKE_CURRENT_BINARY_DIR}/docopt/src/docopt_project-build)

include_directories("${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/include" ${Boost_INCLUDE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/docopt/src/docopt_project"
       "${CMAKE_CURRENT_BINARY_DIR}/msgpack/src/msgpack_project/include" )

add_definitions(-DMSGPACK_USE_BOOST)

add_library(rivet
        computation.cpp
        interface/progress.cpp
        interface/file_writer.cpp
        interface/file_input_reader.cpp
        interface/input_manager.cpp
        dcel/barcode.cpp
        dcel/arrangement.cpp
        dcel/arrangement_builder.cpp
        dcel/anchor.cpp
        dcel/barcode_template.cpp
        dcel/dcel.cpp
        dcel/arrangement_message.cpp
        math/map_matrix.cpp
        math/multi_betti.cpp
        math/simplex_tree.cpp
        math/st_node.cpp
        math/template_point.cpp
        math/template_points_matrix.cpp
        math/index_matrix.cpp
        math/persistence_updater.cpp
        numerics.cpp
        timer.cpp
        debug.cpp
        dcel/grades.cpp
        dcel/grades.h
        math/bool_array.cpp
        math/bool_array.h
        interface/c_api.cpp
        interface/c_api.h api.h api.cpp)

add_dependencies(rivet msgpack_project)
target_link_libraries(rivet ${Boost_LIBRARIES})

add_executable (rivet_console
        console.cpp
        computation.cpp
        interface/progress.cpp
        interface/file_writer.cpp
        interface/file_input_reader.cpp
        interface/input_manager.cpp
        dcel/barcode.cpp
        dcel/arrangement.cpp
        dcel/arrangement_builder.cpp
        dcel/anchor.cpp
        dcel/barcode_template.cpp
        dcel/dcel.cpp
        dcel/arrangement_message.cpp
        math/map_matrix.cpp
        math/multi_betti.cpp
        math/simplex_tree.cpp
        math/st_node.cpp
        math/template_point.cpp
        math/template_points_matrix.cpp
        math/index_matrix.cpp
        math/persistence_updater.cpp
        numerics.cpp
        timer.cpp
        debug.cpp
        dcel/grades.cpp
        dcel/grades.h
        math/bool_array.cpp
        math/bool_array.h
        api.cpp)

add_dependencies(rivet_console docopt_project msgpack_project)

target_link_libraries(rivet_console ${CMAKE_CURRENT_BINARY_DIR}/docopt/src/docopt_project-build/libdocopt_s.a ${Boost_LIBRARIES})
# TODO: Make this file run the qmake build as well, and copy the rivet_console into the same dir where the viewer is built
# TODO: make this not recompile everything we just compiled for rivet_console.
# Maybe using https://cmake.org/Wiki/CMake/Tutorials/Object_Library ?

add_executable(unit_tests
        computation.cpp
        interface/progress.cpp
        interface/file_writer.cpp
        interface/file_input_reader.cpp
        interface/input_manager.cpp
        dcel/arrangement.cpp
        dcel/arrangement_builder.cpp
        dcel/anchor.cpp
        dcel/barcode.cpp
        dcel/barcode_template.cpp
        dcel/dcel.cpp
        dcel/serialization.h
        math/map_matrix.cpp
        math/multi_betti.cpp
        math/simplex_tree.cpp
        math/st_node.cpp
        math/template_point.cpp
        math/template_points_matrix.cpp
        math/index_matrix.cpp
        math/persistence_updater.cpp
        numerics.cpp
        timer.cpp
        debug.cpp
        test/unit_tests.cpp dcel/grades.cpp dcel/grades.h math/bool_array.cpp math/bool_array.h)

add_dependencies(unit_tests msgpack_project)

include_directories("${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/include" ${Boost_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/test)

target_link_libraries(unit_tests ${Boost_LIBRARIES} )
