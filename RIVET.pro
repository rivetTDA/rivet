#-------------------------------------------------
#
# Project created by QtCreator 2014-06-24T12:13:18
#
#-------------------------------------------------

QT       += core gui

TARGET = RIVET
TEMPLATE = app


SOURCES += main.cpp\
        visualizationwindow.cpp \
    interface/slicearea.cpp \
    interface/pdarea.cpp \
    math/multi_betti.cpp \
    math/map_matrix.cpp \
    math/st_node.cpp \
    math/simplex_tree.cpp \
    dcel/mesh.cpp \
    interface/input_manager.cpp \
    interface/slice_diagram.cpp \
    interface/control_dot.cpp

HEADERS  += visualizationwindow.h \
    interface/slicearea.h \
    interface/pdarea.h \
    dcel/mesh.h \
    dcel/vertex.hpp \
    dcel/persistence_data.hpp \
    dcel/lcm_left_comparator.hpp \
    dcel/lcm_angle_comparator.hpp \
    dcel/lcm.hpp \
    dcel/halfedge.hpp \
    dcel/face.hpp \
    interface/input_manager.h \
    math/st_node.h \
    math/simplex_tree.h \
    math/simplex.h \
    math/point.h \
    math/persistence_diagram.hpp \
    math/multi_betti.h \
    math/map_matrix_node.h \
    math/map_matrix.h \
    interface/slice_diagram.h \
    interface/control_dot.h

FORMS    += visualizationwindow.ui
