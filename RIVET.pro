#-------------------------------------------------
#
# Project created by QtCreator 2014-06-24T12:13:18
#
#-------------------------------------------------

macx {
  QMAKE_CXXFLAGS+="-g -gdwarf-2 -ftemplate-depth=1024 "
  QMAKE_POST_LINK='/usr/bin/dsymutil RIVET.app/Contents/MacOS/RIVET -o RIVET.app/Contents/MacOS/RIVET.dsym'
}

CONFIG += c++11 debug

QT       += core gui \
		widgets

TARGET = RIVET
TEMPLATE = app

QMAKE_LIBDIR += /usr/local/lib #TODO: figure out how to generalize
LIBS        += -lboost_serialization

SOURCES	+= main.cpp                         \
		visualizationwindow.cpp             \
		dataselectdialog.cpp                \
		dcel/dcel.cpp                       \
		dcel/arrangement.cpp                       \
		interface/control_dot.cpp           \
		#interface/input_manager.cpp         \
		interface/persistence_bar.cpp       \
		interface/persistence_diagram.cpp   \
		interface/persistence_dot.cpp       \
		interface/slice_diagram.cpp         \
		interface/slice_line.cpp            \
	    math/bool_array.cpp                 \
		math/index_matrix.cpp               \
		math/map_matrix.cpp                 \
		#math/multi_betti.cpp                \
		dcel/barcode.cpp               \
		dcel/barcode_template.cpp           \
		dcel/anchor.cpp                     \
		dcel/arrangement_message.cpp               \
		dcel/grades.cpp                     \
		#math/persistence_updater.cpp        \
		math/template_points_matrix.cpp          \
		math/template_point.cpp                   \
		interface/progressdialog.cpp        \
		computationthread.cpp               \
		interface/aboutmessagebox.cpp       \
		interface/configuredialog.cpp       \
		interface/config_parameters.cpp     \
		interface/file_input_reader.cpp \
    #driver.cpp \
    interface/file_writer.cpp \
    debug.cpp \
    timer.cpp \
    interface/console_interaction.cpp \
    numerics.cpp \


HEADERS  += visualizationwindow.h			\
		dataselectdialog.h					\
		dcel/dcel.h							\
		dcel/arrangement.h							\
		interface/control_dot.h				\
		interface/input_manager.h			\
		interface/persistence_bar.h			\
		interface/persistence_diagram.h		\
		interface/persistence_dot.h			\
		interface/slice_diagram.h			\
		interface/slice_line.h				\
		math/bool_array.h                 \
		math/index_matrix.h					\
		math/map_matrix.h					\
		math/multi_betti.h					\
		dcel/barcode.h	    				\
		dcel/barcode_template.h				\
		dcel/anchor.h						\
		dcel/grades.h                       \
		math/persistence_updater.h			\
		math/template_points_matrix.h			\
		math/template_point.h \
    interface/progressdialog.h \
    computationthread.h \
    interface/input_parameters.h \
    interface/aboutmessagebox.h \
    interface/configuredialog.h \
    interface/config_parameters.h \
    interface/file_input_reader.h \
    #driver.h \
    interface/file_writer.h \
    cutgraph.h \
    dcel/serialization.h \
    interface/console_interaction.h \
    numerics.h \

FORMS   += visualizationwindow.ui			\
		dataselectdialog.ui \
    interface/progressdialog.ui \
    interface/aboutmessagebox.ui \
    interface/configuredialog.ui
