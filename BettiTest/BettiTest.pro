TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp \
    input_manager.cpp \
    math/simplex_tree.cpp \
    math/map_matrix.cpp \
    math/st_node.cpp \
    math/index_matrix.cpp \
    math/multi_betti.cpp

HEADERS += \
    input_manager.h \
    math/simplex_tree.h \
    math/map_matrix.h \
    math/index_matrix.h \
    math/st_node.h \
    math/multi_betti.h

