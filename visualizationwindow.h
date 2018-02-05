/**********************************************************************
Copyright 2014-2016 The RIVET Developers. See the COPYRIGHT file at
the top-level directory of this distribution.

This file is part of RIVET.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#ifndef VISUALIZATIONWINDOW_H
#define VISUALIZATIONWINDOW_H

//forward declarations
class Barcode;
class BarcodeTemplate;
class TemplatePoint;

#include "computationthread.h"
#include "dataselectdialog.h"
#include "dcel/arrangement_message.h"
#include "dcel/grades.h"
#include "interface/aboutmessagebox.h"
#include "interface/config_parameters.h"
#include "interface/configuredialog.h"
#include "interface/input_parameters.h"
#include "interface/persistence_diagram.h"
#include "interface/progressdialog.h"
#include "interface/slice_diagram.h"
#include <QMainWindow>
#include <QtWidgets>

#include <boost/multiprecision/cpp_int.hpp>
typedef boost::multiprecision::cpp_rational exact;
#include "boost/multi_array.hpp"
typedef boost::multi_array<unsigned, 2> unsigned_matrix;

#include <vector>
namespace Ui {
class VisualizationWindow;
}

class VisualizationWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit VisualizationWindow(InputParameters& params);
    ~VisualizationWindow();

protected:
    void showEvent(QShowEvent* event); //shows the DataSelectDialog and blocks until it is closed
    void resizeEvent(QResizeEvent*);
    void closeEvent(QCloseEvent* event);
    QString suggestedName(QString extension);

public slots:
    void start_computation(); //begins the computation pipeline
    void paint_template_points(std::shared_ptr<TemplatePointsMessage> points);
    void augmented_arrangement_ready(std::shared_ptr<ArrangementMessage> arrangement);
    void set_line_parameters(double angle, double offset);

private slots:
    void on_angleDoubleSpinBox_valueChanged(double angle);
    void on_offsetSpinBox_valueChanged(double arg1);
    void on_xi0CheckBox_toggled(bool checked);
    void on_xi1CheckBox_toggled(bool checked);
    void on_xi2CheckBox_toggled(bool checked);
    void on_normCoordCheckBox_clicked(bool checked);
    void on_barcodeCheckBox_clicked(bool checked);


    void on_BottomCornerXSpinBox_valueChanged(double x_bottom);
    void on_BottomCornerYSpinBox_valueChanged(double y_bottom);
    void on_TopCornerXSpinBox_valueChanged(double x_top);
    void on_TopCornerYSpinBox_valueChanged(double y_top);

    void on_actionExit_triggered();
    void on_actionAbout_triggered();
    void on_actionConfigure_triggered();
    void on_actionSave_persistence_diagram_as_image_triggered();
    void on_actionSave_line_selection_window_as_image_triggered();
    void on_actionSave_triggered();
    void on_actionOpen_triggered();
    void on_actionRestore_default_window_triggered();
    void on_actionBetti_number_window_triggered();
    void on_actionAutomatically_reset_line_toggled();



private:
    static const QString DEFAULT_SAVE_DIR_KEY;

    Ui::VisualizationWindow* ui;

    //data items
    const int verbosity;

    bool data_selected; //false until user selects data, then true
    bool unsaved_data; //false until augmented arrangement is computed, then true until user saves the augmented arrangement
    InputParameters& input_params; //parameters set by the user via the DataSelectDialog
    ConfigParameters config_params; //parameters that control the visualization
    DataSelectDialog ds_dialog; //dialog box that gets the input parameters




    Grades grades;

    double angle_precise; //sufficiently-precise internal value of the slice-line angle in DEGREES, necessary because QDoubleSpinBox truncates this value
    double offset_precise; //sufficiently-precise internal value of the slice-line offset, necessary because QDoubleSpinBox truncates this value

    double xmin_precise; //internal value of the coordinates of the current window
    double xmax_precise;
    double ymin_precise;
    double ymax_precise;

    double origin_x, origin_y;//the coordinates in data units of the origin on the line; i.e. the point where it intersects the box circumscribing the gradings
    double dist_to_origin; //signed distance in data units from bottom left control dot in visible window to the origin; positive if the dot is up and to the right of the origin; negative otherwise
    double slice_length;//length of the visible slice in DATA units
    bool is_visible; //whether the line is visible in the current window
    std::shared_ptr<TemplatePointsMessage> template_points; //The template points, homology dimensions, and other useful context
    std::shared_ptr<ArrangementMessage> arrangement; //pointer to the DCEL arrangement
    std::unique_ptr<Barcode> barcode; //pointer to the currently-displayed barcode

    //computation items
    ComputationThread cthread;
    ProgressDialog prog_dialog;

    //items for slice diagram
    bool line_selection_ready; //initially false, but set to true when data is in place for line selection
    SliceDiagram slice_diagram; //subclass of QGraphicsScene, contains all of the graphics elements for the line-selection diagram
    bool slice_update_lock; //true iff slice diagram is being updated; helps avoid an infinite loop

    //items for persistence diagram
    PersistenceDiagram p_diagram; //subclass of QGraphicsScene, contains all of the graphics elements for the persistence diagram
    bool persistence_diagram_drawn;

    void update_persistence_diagram(); //updates the persistence diagram and barcode after a change in the slice line
    void reset_line(); //change the line so it connects the extreme points of the current window

    void update_origin();//computes the origin of the line after a change in parameters; updates the value of dist_to_origin
    //other items
    void save_arrangement(const QString& filename);

    AboutMessageBox aboutBox; //which is better for these dialog boxes
    ConfigureDialog* configBox; // -- pointer or no pointer?
};

#endif // VISUALIZATIONWINDOW_H
