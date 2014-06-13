#-------------------------------------------------
#
# Project created by QtCreator 2014-06-12T10:51:26
#
#-------------------------------------------------

QT       += core gui

TARGET = MultiD_Persistence
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    slicearea.cpp \
    pdarea.cpp

HEADERS  += mainwindow.h \
    slicearea.h \
    pdarea.h \
    ../dcel/mesh.h \
    ../dcel/mesh.hpp \
    ../st_node.hpp \
    ../st_node.h \
    ../simplex_tree.hpp \
    ../simplex_tree.h

FORMS    += mainwindow.ui
