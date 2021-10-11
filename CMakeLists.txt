cmake_minimum_required (VERSION 3.1)
project (rivet_console)
set (CMAKE_CXX_STANDARD 14)
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-ftemplate-depth=1024 -Wall -Wextra -pedantic -fPIC")

include(ExternalProject)

externalproject_add(
  docopt_project
  PREFIX docopt
  GIT_REPOSITORY https://github.com/docopt/docopt.cpp
  GIT_TAG 6f5de76970be94a6f1e4556d1716593100e285d2
  CMAKE_ARGS -D CMAKE_CXX_STANDARD=11 -D CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  INSTALL_COMMAND ""
  )

externalproject_add(
        msgpack_project
        PREFIX msgpack
        GIT_REPOSITORY https://github.com/msgpack/msgpack-c
        GIT_TAG cpp-3.0.1
        CMAKE_ARGS -D CMAKE_CXX_STANDARD=11 -D CMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
        INSTALL_COMMAND ""
)

find_package(Boost "1.58")

# So far, we haven’t been able to get OpenMP to work with OS X’s Clang, though this should be possible.
# See https://github.com/rivetTDA/rivet/issues/136
# For now, to avoid a distracting error message when running CMake in OS X, we use the following if statement.  
if(NOT APPLE)
	# Look for OpenMP and enable it if found.
	find_package(OpenMP)
	if (OPENMP_FOUND)
    		set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
    		set (CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
        endif()
endif()

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
        interface/data_reader.cpp
        dcel/barcode.cpp
        dcel/arrangement.cpp
        dcel/arrangement_builder.cpp
        dcel/anchor.cpp
        dcel/barcode_template.cpp
        dcel/dcel.cpp
        dcel/arrangement_message.cpp
	math/bifiltration_data.cpp
    math/distance_matrix.cpp
	math/bigraded_matrix.cpp
	math/firep.cpp
        math/map_matrix.cpp
        math/multi_betti.cpp
        math/template_point.cpp
        math/template_points_matrix.cpp
        math/index_matrix.cpp
        math/persistence_updater.cpp
        math/presentation.cpp
        numerics.cpp
        timer.cpp
        debug.cpp
        dcel/grades.cpp
        dcel/grades.h
        #math/bool_array.cpp
        #math/bool_array.h
        interface/c_api.cpp
	math/phat_mod/include/phat/representations/vector_heap_mod.h
	math/phat_mod/include/phat/helpers/misc.h
	math/phat_mod/include/phat/helpers/thread_local_storage.h
        interface/c_api.h
        api.h
        api.cpp
        )

add_dependencies(rivet msgpack_project)

add_executable (rivet_console
        console.cpp
        computation.cpp
        debug.cpp
        interface/file_writer.cpp
        interface/file_input_reader.cpp
        interface/input_manager.cpp
        interface/data_reader.cpp
	interface/progress.cpp
        numerics.cpp
        timer.cpp
        dcel/anchor.cpp
        dcel/arrangement.cpp
        dcel/arrangement_builder.cpp
        dcel/arrangement_message.cpp
	dcel/barcode.cpp
        dcel/barcode_template.cpp
        dcel/dcel.cpp
        dcel/grades.cpp
	dcel/grades.h
	math/bool_array.cpp
	math/bool_array.h
	math/bifiltration_data.cpp
    math/distance_matrix.cpp
	math/bigraded_matrix.cpp
	math/firep.cpp
     	math/index_matrix.cpp
        math/map_matrix.cpp
        math/multi_betti.cpp
        math/persistence_updater.cpp
	math/presentation.cpp
        math/template_point.cpp
        math/template_points_matrix.cpp
	math/phat_mod/include/phat/representations/vector_heap_mod.h
	math/phat_mod/include/phat/helpers/misc.h
	math/phat_mod/include/phat/helpers/thread_local_storage.h
        api.cpp
        )

add_dependencies(rivet_console docopt_project msgpack_project)

target_link_libraries(rivet_console ${CMAKE_CURRENT_BINARY_DIR}/docopt/src/docopt_project-build/libdocopt.a ${Boost_LIBRARIES})
# TODO: Make this file run the qmake build as well, and copy the rivet_console into the same dir where the viewer is built
# TODO: make this not recompile everything we just compiled for rivet_console.
# Maybe using https://cmake.org/Wiki/CMake/Tutorials/Object_Library ?

add_executable(unit_tests
        computation.cpp
        debug.cpp
        interface/file_writer.cpp
        interface/file_input_reader.cpp
        interface/input_manager.cpp
        interface/data_reader.cpp
	interface/progress.cpp
        numerics.cpp
        timer.cpp
        dcel/anchor.cpp
        dcel/arrangement.cpp
        dcel/arrangement_builder.cpp
        dcel/arrangement_message.cpp
	dcel/barcode.cpp
        dcel/barcode_template.cpp
        dcel/dcel.cpp
        dcel/grades.cpp
	dcel/grades.h
	math/bool_array.cpp
	math/bool_array.h
	math/bifiltration_data.cpp
    math/distance_matrix.cpp
	math/bigraded_matrix.cpp
	math/firep.cpp
     	math/index_matrix.cpp
        math/map_matrix.cpp
        math/multi_betti.cpp
        math/persistence_updater.cpp
	math/presentation.cpp
        math/template_point.cpp
        math/template_points_matrix.cpp
	math/phat_mod/include/phat/representations/vector_heap_mod.h
	math/phat_mod/include/phat/helpers/misc.h
	math/phat_mod/include/phat/helpers/thread_local_storage.h
        test/unit_tests.cpp
        dcel/grades.cpp
        api.cpp
)

add_dependencies(unit_tests msgpack_project)

include_directories("${PROJECT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/include" ${Boost_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/test)

target_link_libraries(unit_tests)
