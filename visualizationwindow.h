#ifndef VISUALIZATIONWINDOW_H
#define VISUALIZATIONWINDOW_H

//forward declarations
class Barcode;
class BarcodeTemplate;
class TemplatePoint;

#include "computationthread.h"
#include "dataselectdialog.h"
#include "dcel/arrangement_message.h"
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

public slots:
    void start_computation(); //begins the computation pipeline
    void paint_template_points();
    void augmented_arrangement_ready(ArrangementMessage* arrangement);
    void set_line_parameters(double angle, double offset);

private slots:
    void on_angleDoubleSpinBox_valueChanged(double angle);
    void on_offsetSpinBox_valueChanged(double arg1);
    void on_xi0CheckBox_toggled(bool checked);
    void on_xi1CheckBox_toggled(bool checked);
    void on_xi2CheckBox_toggled(bool checked);
    void on_normCoordCheckBox_clicked(bool checked);
    void on_barcodeCheckBox_clicked(bool checked);

    void on_actionExit_triggered();
    void on_actionAbout_triggered();
    void on_actionConfigure_triggered();
    void on_actionSave_persistence_diagram_as_image_triggered();
    void on_actionSave_line_selection_window_as_image_triggered();
    void on_actionSave_triggered();
    void on_actionOpen_triggered();

private:
    Ui::VisualizationWindow* ui;

    //data items
    const int verbosity;
    const double INFTY;
    const double PI;

    bool data_selected; //false until user selects data, then true
    bool unsaved_data; //false until augmented arrangement is computed, then true until user saves the augmented arrangement
    InputParameters& input_params; //parameters set by the user via the DataSelectDialog
    ConfigParameters config_params; //parameters that control the visualization
    DataSelectDialog ds_dialog; //dialog box that gets the input parameters

    std::vector<exact> x_exact;
    std::vector<exact> y_exact;
    std::vector<double> y_grades;
    std::vector<double> x_grades;
    std::vector<TemplatePoint> template_points; //stores discrete coordinates of xi support points, with multiplicities
    unsigned_matrix homology_dimensions; //stores the dimension of homology at each grade

    double angle_precise; //sufficiently-precise internal value of the slice-line angle in DEGREES, necessary because QDoubleSpinBox truncates this value
    double offset_precise; //sufficiently-precise internal value of the slice-line offset, necessary because QDoubleSpinBox truncates this value

    ArrangementMessage* arrangement; //pointer to the DCEL arrangement
    Barcode* barcode; //pointer to the currently-displayed barcode

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

    Barcode* rescale_barcode_template(BarcodeTemplate& dbc, double angle, double offset);
    double project(TemplatePoint& pt, double angle, double offset);

    //computes the projection of the lower-left corner of the line-selection window onto the specified line
    // TESTING AS REPLACEMENT FOR SliceDiagram::get_zero()
    double project_zero(double angle, double offset);

    //other items
    void save_arrangement(const QString& filename);

    AboutMessageBox aboutBox; //which is better for these dialog boxes
    ConfigureDialog* configBox; // -- pointer or no pointer?
    void copy_fields_from_cthread();
};

#endif // VISUALIZATIONWINDOW_H
