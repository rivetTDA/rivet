#-------------------------------------------------
#
# Project created by QtCreator 2014-06-24T12:13:18
#
#-------------------------------------------------

QT       += core gui \
		widgets

CONFIG += c++11

TARGET = RIVET
TEMPLATE = app


SOURCES	+= main.cpp                         \
		visualizationwindow.cpp             \
		dataselectdialog.cpp                \
		dcel/dcel.cpp                       \
		dcel/mesh.cpp                       \
		interface/control_dot.cpp           \
		interface/input_manager.cpp         \
		interface/persistence_bar.cpp       \
		interface/persistence_diagram.cpp   \
		interface/persistence_dot.cpp       \
		interface/slice_diagram.cpp         \
		interface/slice_line.cpp            \
		math/index_matrix.cpp               \
		math/map_matrix.cpp                 \
		math/multi_betti.cpp                \
		math/simplex_tree.cpp               \
		math/st_node.cpp                    \
		interface/barcode.cpp               \
		dcel/barcode_template.cpp           \
		dcel/anchor.cpp                     \
		math/persistence_updater.cpp        \
		math/xi_support_matrix.cpp          \
		math/xi_point.cpp                   \
		interface/progressdialog.cpp        \
		computationthread.cpp               \
		interface/aboutmessagebox.cpp       \
		interface/configuredialog.cpp       \
		interface/config_parameters.cpp     \
		interface/file_input_reader.cpp \
    driver.cpp \
    interface/file_writer.cpp
    

HEADERS  += visualizationwindow.h			\
		dataselectdialog.h					\
		dcel/dcel.h							\
		dcel/mesh.h							\
		interface/control_dot.h				\
		interface/input_manager.h			\
		interface/persistence_bar.h			\
		interface/persistence_diagram.h		\
		interface/persistence_dot.h			\
		interface/slice_diagram.h			\
		interface/slice_line.h				\
		math/index_matrix.h					\
		math/map_matrix.h					\
		math/multi_betti.h					\
		math/simplex_tree.h					\
		math/st_node.h						\
		interface/barcode.h					\
		dcel/barcode_template.h				\
		dcel/anchor.h						\
		math/persistence_updater.h			\
		math/xi_support_matrix.h			\
		math/xi_point.h \
    interface/progressdialog.h \
    computationthread.h \
    interface/input_parameters.h \
    interface/aboutmessagebox.h \
    interface/configuredialog.h \
    interface/config_parameters.h \
    interface/file_input_reader.h \
    driver.h \
    interface/file_writer.h
    

FORMS   += visualizationwindow.ui			\
		dataselectdialog.ui \
    interface/progressdialog.ui \
    interface/aboutmessagebox.ui \
    interface/configuredialog.ui
