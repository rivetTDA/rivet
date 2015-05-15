#include "visualizationwindow.h"
#include "ui_visualizationwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>
#include <QTime>

#include "interface/input_manager.h"
#include "interface/progressdialog.h"
#include "math/simplex_tree.h"
#include "math/multi_betti.h"
#include "dcel/anchor.h"
#include "dcel/dcel.h"
#include "dcel/mesh.h"
#include "interface/barcode.h"


VisualizationWindow::VisualizationWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VisualizationWindow),
    verbosity(5), INFTY(std::numeric_limits<double>::infinity()),
    ds_dialog(input_params),
    x_grades(), y_grades(), xi_support(),
    cthread(verbosity, input_params, x_grades, y_grades, xi_support),
    line_selection_ready(false),
    slice_diagram(NULL), slice_update_lock(false),
    p_diagram(NULL), persistence_diagram_drawn(false)
{
    ui->setupUi(this);

    //set up the slice diagram scene
    sliceScene = new QGraphicsScene(this);
    ui->sliceView->setScene(sliceScene);
//    ui->sliceView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->sliceView->scale(1,-1);
    ui->sliceView->setRenderHint(QPainter::Antialiasing);

    //set up the persistence diagram scene
    pdScene = new QGraphicsScene(this);
    ui->pdView->setScene(pdScene);
    ui->pdView->scale(1,-1);
    ui->pdView->setRenderHint(QPainter::Antialiasing);

    //connect signals from ComputationThread to slots in VisualizationWindow
    QObject::connect(&cthread, &ComputationThread::xiSupportReady, this, &VisualizationWindow::paint_xi_support);
    QObject::connect(&cthread, &ComputationThread::arrangementReady, this, &VisualizationWindow::augmented_arrangement_ready);
}

VisualizationWindow::~VisualizationWindow()
{
    delete ui;
}

//start the persistent homology computation in a new thread
void VisualizationWindow::start_computation()
{
    //create the progress box
//    ProgressDialog prog_dialog;
//    prog_dialog.exec();

    //start the computation in a new thread
    cthread.compute();

}//end start_computation()

//this slot is signaled when the xi support points are ready to be drawn
void VisualizationWindow::paint_xi_support()
{
    //initialize the SliceDiagram and send xi support points
    slice_diagram = new SliceDiagram(sliceScene, this, x_grades.front(), x_grades.back(), y_grades.front(), y_grades.back(), ui->normCoordCheckBox->isChecked());
    for(std::vector<xiPoint>::iterator it = xi_support.begin(); it != xi_support.end(); ++it)
        slice_diagram->add_point(x_grades[it->x], y_grades[it->y], it->zero, it->one);
    slice_diagram->create_diagram(input_params.x_label, input_params.y_label);

    //update offset extents   //TODO: FIX THIS!!!
    ui->offsetSpinBox->setMinimum(-1*x_grades.back());
    ui->offsetSpinBox->setMaximum(y_grades.back());

    line_selection_ready = true;
}

//this slot is signaled when the agumented arrangement is ready
void VisualizationWindow::augmented_arrangement_ready(Mesh* arrangement)
{
    //receive the arrangement
    this->arrangement = arrangement;

    if(verbosity >= 2) { qDebug() << "COMPUTATION FINISHED; READY FOR INTERACTIVITY."; }

    //print arrangement info
    arrangement->print_stats();
    ui->statusBar->showMessage("computed all barcode templates");

    //TESTING: verify consistency of the arrangement
//    arrangement->test_consistency();


//    qDebug() << "zero: " << slice_diagram->get_zero();

    //inialize persistence diagram
    p_diagram = new PersistenceDiagram(pdScene, this, &(input_params.fileName), input_params.dim);
    p_diagram->resize_diagram(slice_diagram->get_slice_length(), slice_diagram->get_pd_scale());

    //get the barcode
    double degrees = ui->angleDoubleSpinBox->value();
    double offset = ui->offsetSpinBox->value();                             ///TODO: CHECK THIS!!!

    BarcodeTemplate& dbc = arrangement->get_barcode_template(degrees, offset);
    Barcode* barcode = rescale_barcode_template(dbc, degrees, offset);      ///TODO: CHECK THIS!!!

    //TESTING
    barcode->print();

    //draw the barcode
    p_diagram->draw_points(slice_diagram->get_zero(), barcode);
    slice_diagram->draw_barcode(barcode, ui->barcodeCheckBox->isChecked());

    //clean up
    delete barcode;

    persistence_diagram_drawn = true;
    ui->statusBar->showMessage("persistence diagram drawn");

}//end augmented_arrangement_ready()

void VisualizationWindow::on_angleDoubleSpinBox_valueChanged(double angle)
{
//    qDebug() << "angleDoubleSpinBox_valueChanged(); angle: " << angle << "; slice_update_lock: " << slice_update_lock;

    if(line_selection_ready && !slice_update_lock)
        slice_diagram->update_line(angle, ui->offsetSpinBox->value());

    if(persistence_diagram_drawn)
        update_persistence_diagram();
}

void VisualizationWindow::on_offsetSpinBox_valueChanged(double offset)
{
//    qDebug() << "offsetSpinBox_valueChanged(); offset: " << offset << "; slice_update_lock: " << slice_update_lock;

    if(line_selection_ready && !slice_update_lock)
        slice_diagram->update_line(ui->angleDoubleSpinBox->value(), offset);

    if(persistence_diagram_drawn)
        update_persistence_diagram();
}

void VisualizationWindow::on_normCoordCheckBox_clicked(bool checked)
{
    if(line_selection_ready)
    {
        slice_diagram->set_normalized_coords(checked);
        slice_diagram->resize_diagram();
        if(p_diagram != NULL)
            p_diagram->resize_diagram(slice_diagram->get_slice_length(), slice_diagram->get_pd_scale());
    }
}

void VisualizationWindow::on_barcodeCheckBox_clicked(bool checked)
{
    if(line_selection_ready)
        slice_diagram->toggle_barcode(checked);
}

void VisualizationWindow::on_xi0CheckBox_toggled(bool checked)
{
    if(line_selection_ready)
        slice_diagram->toggle_xi0_points(checked);
}

void VisualizationWindow::on_xi1CheckBox_toggled(bool checked)
{
    if(line_selection_ready)
        slice_diagram->toggle_xi1_points(checked);
}

void VisualizationWindow::update_persistence_diagram()
{
    //get the barcode
    double degrees = ui->angleDoubleSpinBox->value();
    double offset = ui->offsetSpinBox->value();

    BarcodeTemplate& dbc = arrangement->get_barcode_template(degrees, offset);
    Barcode* barcode = rescale_barcode_template(dbc, degrees, offset);

    //TESTING
    barcode->print();

    //draw the barcode
    p_diagram->update_diagram(slice_diagram->get_slice_length(), slice_diagram->get_pd_scale(), slice_diagram->get_zero(), barcode);
    slice_diagram->update_barcode(barcode, ui->barcodeCheckBox->isChecked());

    //clean up
    delete barcode;
}

//rescales a barcode template by projecting points onto the specified line
// NOTE: angle in DEGREES
Barcode* VisualizationWindow::rescale_barcode_template(BarcodeTemplate& dbc, double angle, double offset)
{
    Barcode* bc = new Barcode();     //NOTE: delete later!

    //loop through bars
    for(std::set<BarTemplate>::iterator it = dbc.begin(); it != dbc.end(); ++it)
    {
        xiPoint begin = xi_support[it->begin];
        double birth = project(begin, angle, offset);

        if(birth != INFTY)  //then bar exists in this rescaling
        {
            if(it->end >= xi_support.size())    //then endpoint is at infinity
            {
                bc->add_bar(birth, INFTY, it->multiplicity);
            }
            else    //then bar is finite
            {
                xiPoint end = xi_support[it->end];
                double death = project(end, angle, offset);
                bc->add_bar(birth, death, it->multiplicity);

                //testing
                if(birth > death)
                    qDebug() << "=====>>>>> ERROR: inverted bar (" << birth << "," << death << ")";
            }
        }
    }

    return bc;
}//end rescale_barcode_template()

//computes the projection of an xi support point onto the specified line
//  NOTE: returns INFTY if the point has no projection (can happen only for horizontal and vertical lines)
//  NOTE: angle in DEGREES
double VisualizationWindow::project(xiPoint& pt, double angle, double offset)
{
    if(angle == 0)  //then line is horizontal
    {
        if( y_grades[pt.y] <= offset)   //then point is below the line, so projection exists
            return x_grades[pt.x];
        else    //then no projection
            return INFTY;
    }
    else if(angle == 90)    //then line is vertical
    {
        if( x_grades[pt.x] <= -1*offset)   //then point is left of the line, so projection exists
            return y_grades[pt.y];
        else    //then no projection
            return INFTY;
    }
    //if we get here, then line is neither horizontal nor vertical
    double radians = angle * 3.14159265/180;
    double x = x_grades[pt.x];
    double y = y_grades[pt.y];

    if( y > x*tan(radians) + offset/cos(radians) )	//then point is above line
        return y/sin(radians) - offset/tan(radians); //project right

    return x/cos(radians) + offset*tan(radians); //project up
}//end project()

void VisualizationWindow::set_line_parameters(double angle, double offset)
{
    slice_update_lock = true;

//    qDebug() << "  set_line_parameters: angle = " << angle << "; offset = " << offset;

    ui->angleDoubleSpinBox->setValue(angle);
    ui->offsetSpinBox->setValue(offset);

    slice_update_lock = false;
}

void VisualizationWindow::select_bar(unsigned index)
{
    slice_diagram->select_bar(index);
}

void VisualizationWindow::deselect_bar()
{
    slice_diagram->deselect_bar(false);
}

void VisualizationWindow::select_dot(unsigned index)
{
    p_diagram->select_dot(index);
}

void VisualizationWindow::deselect_dot()
{
    p_diagram->deselect_dot(false);
}

void VisualizationWindow::showEvent(QShowEvent* event)
{
    QMainWindow::showEvent(event);
    ds_dialog.exec();               //show the DataSelectDialog box and blocks until the dialog is closed
    start_computation();            //starts the persistence calculation
}

void VisualizationWindow::resizeEvent(QResizeEvent* /*unused*/)
{
//    qDebug() << "resize event! slice_diagrma = " << slice_diagram;
    if(slice_diagram != NULL)
    {
        slice_diagram->resize_diagram();

        if(p_diagram != NULL)
            p_diagram->resize_diagram(slice_diagram->get_slice_length(), slice_diagram->get_pd_scale());
    }
}
