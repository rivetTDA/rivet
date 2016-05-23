cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)
project (rivet_console)

include(ExternalProject)

#Is it helpful to download and install this automatically, or should we add it to the
#requirements people have to install for themselves?
externalproject_add(
  docopt_project
  PREFIX docopt
  GIT_REPOSITORY https://github.com/docopt/docopt.cpp
  GIT_TAG a4177ccf1a6e36ebe268972732ddd456a3574f6d
  CMAKE_ARGS -D CMAKE_CXX_STANDARD=11 -D CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  )

#note this must come before add_executable or it will be ignored
link_directories(${CMAKE_CURRENT_BINARY_DIR}/docopt/src/docopt_project-build)

include_directories("${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/include")

add_executable (rivet_console console.cpp computation.cpp
        dcel/mesh.cpp
        interface/progress.cpp interface/file_writer.cpp
        interface/file_input_reader.cpp
        interface/input_manager.cpp
        dcel/anchor.cpp
        dcel/barcode_template.cpp
        dcel/dcel.cpp
        math/map_matrix.cpp
        math/multi_betti.cpp
        math/simplex_tree.cpp
        math/st_node.cpp
        math/xi_point.cpp
        math/xi_support_matrix.cpp
        math/index_matrix.cpp
        math/persistence_updater.cpp timer.cpp timer.h debug.cpp)

add_dependencies(rivet_console docopt_project)

target_link_libraries(rivet_console docopt_s)

