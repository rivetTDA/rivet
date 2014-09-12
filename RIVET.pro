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
    math/multi_betti.cpp \
    math/map_matrix.cpp \
    math/st_node.cpp \
    math/simplex_tree.cpp \
    dcel/mesh.cpp \
    interface/input_manager.cpp \
    interface/slice_diagram.cpp \
    interface/control_dot.cpp \
    interface/slice_line.cpp \
    interface/persistence_diagram.cpp \
    interface/persistence_dot.cpp \
    dataselectdialog.cpp \
    math/persistence_data.cpp \
    interface/persistence_bar.cpp \
    dcel/cell_persistence_data.cpp \
    dcel/lcm.cpp \
    dcel/dcel.cpp \
    math/index_matrix.cpp \
    math/point.cpp

HEADERS  += visualizationwindow.h \
    dcel/mesh.h \
    interface/input_manager.h \
    math/st_node.h \
    math/simplex_tree.h \
    math/point.h \
    math/multi_betti.h \
    math/map_matrix.h \
    interface/slice_diagram.h \
    interface/control_dot.h \
    math/index_matrix.h \
    interface/slice_line.h \
    interface/persistence_diagram.h \
    interface/persistence_dot.h \
    dataselectdialog.h \
    math/persistence_data.h \
    interface/persistence_bar.h \
    dcel/cell_persistence_data.h \
    dcel/lcm.h \
    dcel/dcel.h

FORMS    += visualizationwindow.ui \
    dataselectdialog.ui
