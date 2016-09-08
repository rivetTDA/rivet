cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 11)
project (rivet_tests)
add_executable (unit_tests
        unit_tests.cpp
        ../timer.cpp
        ../math/map_matrix.cpp
        ../debug.cpp
        ../numerics.cpp
        ../computation.cpp
        ../interface/progress.cpp
        ../interface/file_writer.cpp
        ../interface/file_input_reader.cpp
        ../interface/input_manager.cpp
        ../dcel/arrangement.cpp
        ../dcel/anchor.cpp
        ../dcel/barcode_template.cpp
        ../dcel/dcel.cpp
        ../math/map_matrix.cpp
        ../math/multi_betti.cpp
        ../math/simplex_tree.cpp
        ../math/st_node.cpp
        ../math/template_point.cpp
        ../math/template_points_matrix.cpp
        ../math/index_matrix.cpp
        ../math/persistence_updater.cpp
        ../dcel/arrangement_builder.cpp
        )

include_directories("${PROJECT_SOURCE_DIR}/..")
