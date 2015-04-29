#include "visualizationwindow.h"
#include "ui_visualizationwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

#include "interface/input_manager.h"
#include "math/simplex_tree.h"
#include "math/multi_betti.h"
#include "dcel/lcm.h"
#include "dcel/dcel.h"
#include "dcel/mesh.h"
#include "interface/barcode.h"

#include "boost/date_time/posix_time/posix_time.hpp"
using namespace boost::posix_time;



VisualizationWindow::VisualizationWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VisualizationWindow),
    verbosity(7), INFTY(std::numeric_limits<double>::infinity()),
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
}

VisualizationWindow::~VisualizationWindow()
{
    delete ui;
}

//sets the name of the data file
void VisualizationWindow::setFile(QString name)
{
    fileName = name;
    ui->statusBar->showMessage("file: "+fileName);
}

//sets parameters for the computation
void VisualizationWindow::setComputationParameters(int hom_dim, unsigned num_x_bins, unsigned num_y_bins, QString x_text, QString y_text)
{
    dim = hom_dim;
    x_bins = num_x_bins;
    y_bins = num_y_bins;
    x_label = x_text;
    y_label = y_text;
}

//void VisualizationWindow::on_computeButton_clicked() //read the file and do the persistent homology computation
void VisualizationWindow::compute()
{
  //STEP 1: INPUT DATA AND CREATE BIFILTRATION

    //start the input manager
    im = new InputManager(dim, verbosity);
    //const char* filestr = fileName.toStdString().data();
    std::string filestr = fileName.toStdString();
    im->start(filestr, x_bins, y_bins);

    //get the data
    x_grades = im->get_x_grades();
    y_grades = im->get_y_grades();
    std::vector<exact> x_exact = im->get_x_exact();
    std::vector<exact> y_exact = im->get_y_exact();
    bifiltration = im->get_bifiltration();

    //get data extents
    double data_xmin = x_grades.front();
    double data_xmax = x_grades.back();
    double data_ymin = y_grades.front();
    double data_ymax = y_grades.back();

    //print bifiltration statistics
    std::cout << "\nBIFILTRATION:\n";
    std::cout << "   Number of simplices of dimension " << dim << ": " << bifiltration->get_size(dim) << "\n";
    std::cout << "   Number of simplices of dimension " << (dim+1) << ": " << bifiltration->get_size(dim+1) << "\n";
    std::cout << "   Number of x-grades: " << x_grades.size() << "; values " << data_xmin << " to " << data_xmax << "\n";
    std::cout << "   Number of y-grades: " << y_grades.size() << "; values " << data_ymin << " to " << data_ymax << "\n";
    std::cout << "\n";

    if(verbosity >= 6)
    {
        std::cout << "x-grades:\n";
        for(unsigned i=0; i<x_grades.size(); i++)
            std::cout << "  " << x_grades[i] << " = " << x_exact[i] << "\n";
        std::cout << "y-grades:\n";
        for(unsigned i=0; i<y_grades.size(); i++)
            std::cout << "  " << y_grades[i] << " = " << y_exact[i] << "\n";
//        std::cout << "bifiltration simplex tree:\n";
//            bifiltration->print();
    }


  //STEP 2: COMPUTE SUPPORT POINTS OF MULTI-GRADED BETTI NUMBERS

    //compute xi_0 and xi_1 at all multi-grades
    if(verbosity >= 2) { std::cout << "COMPUTING xi_0 AND xi_1 FOR HOMOLOGY DIMENSION " << dim << ":\n"; }
    MultiBetti mb(bifiltration, dim, verbosity);

    ptime time_xi_start(microsec_clock::local_time());  //start timer

    mb.compute_fast();

    ptime time_xi_end(microsec_clock::local_time());    //stop timer
    time_duration duration_xi(time_xi_end - time_xi_start);

    std::cout << "   xi_i computation took " << duration_xi << "\n";
    ui->statusBar->showMessage("computed xi support points");


  //STEP 3: BUILD THE ARRANGEMENT

    //build the arrangement
    if(verbosity >= 2) { std::cout << "CALCULATING LCMs AND BUILDING THE DCEL ARRANGEMENT\n"; }

    ptime time_dcel_start(microsec_clock::local_time());    //start timer

    arrangement = new Mesh(x_grades, x_exact, y_grades, y_exact, verbosity);
    arrangement->build_arrangement(mb, xi_support);     //also stores list of xi support points in the last argument
        //NOTE: this also computes and stores barcode templates in the arrangement

    ptime time_dcel_end(microsec_clock::local_time());      //stop timer
    time_duration duration_dcel(time_dcel_end - time_dcel_start);

    //testing:
    if(verbosity >= 4)
    {
        std::cout << "  SUPPORT POINTS OF xi_0 AND xi_1 (INTEGER VALUES): ";
        for(unsigned i=0; i<xi_support.size(); i++)
            std::cout << "(" << xi_support[i].x << "," << xi_support[i].y << "), ";
        std::cout << "\n";
    }
    if(verbosity >= 2) { std::cout << "DATA COMPUTED; READY FOR INTERACTIVITY.\n"; }

    //print arrangement info
    std::cout << "   building the arrangement and computing barcode templates took " << duration_dcel << "\n";
    arrangement->print_stats();
    ui->statusBar->showMessage("computed persistence data");

    //TESTING: verify consistency of the arrangement
//    arrangement->test_consistency();


  //STEP 4: PREPARE GRAPHICAL ELEMENTS

    //initialize the SliceDiagram and send xi support points
    slice_diagram = new SliceDiagram(sliceScene, this, data_xmin, data_xmax, data_ymin, data_ymax, ui->normCoordCheckBox->isChecked());
    for(std::vector<xiPoint>::iterator it = xi_support.begin(); it != xi_support.end(); ++it)
        slice_diagram->add_point(x_grades[it->x], y_grades[it->y], it->zero, it->one);
    slice_diagram->create_diagram(x_label, y_label);

    //update offset extents   //TODO: FIX THIS!!!
    ui->offsetSpinBox->setMinimum(-1*data_xmax);
    ui->offsetSpinBox->setMaximum(data_ymax);

//    qDebug() << "zero: " << slice_diagram->get_zero();

    //inialize persistence diagram
    p_diagram = new PersistenceDiagram(pdScene, this, &fileName, dim);
    p_diagram->resize_diagram(slice_diagram->get_slice_length(), slice_diagram->get_pd_scale());

    //get the barcode
    double degrees = ui->angleDoubleSpinBox->value();
    double offset = ui->offsetSpinBox->value();                             ///TODO: CHECK THIS!!!

    BarcodeTemplate& dbc = arrangement->get_barcode_template(degrees, offset);
    Barcode* barcode = rescale_barcode_template(dbc, degrees, offset);      ///TODO: CHECK THIS!!!

    //TESTING
    std::cout<< "RESCALED BARCODE: ";
    barcode->print();
    std::cout << "\n";

    //draw the barcode
    p_diagram->draw_points(slice_diagram->get_zero(), barcode);
    slice_diagram->draw_barcode(barcode, ui->barcodeCheckBox->isChecked());

    //clean up
    delete barcode;

    persistence_diagram_drawn = true;
    ui->statusBar->showMessage("persistence diagram drawn");
}//end on_computeButton_clicked()

void VisualizationWindow::on_angleDoubleSpinBox_valueChanged(double angle)
{
//    qDebug() << "angleDoubleSpinBox_valueChanged(); angle: " << angle << "; slice_update_lock: " << slice_update_lock;

    if(!slice_update_lock)
        slice_diagram->update_line(angle, ui->offsetSpinBox->value());

    if(persistence_diagram_drawn)
        update_persistence_diagram();
}

void VisualizationWindow::on_offsetSpinBox_valueChanged(double offset)
{
//    qDebug() << "offsetSpinBox_valueChanged(); offset: " << offset << "; slice_update_lock: " << slice_update_lock;

    if(!slice_update_lock)
        slice_diagram->update_line(ui->angleDoubleSpinBox->value(), offset);

    if(persistence_diagram_drawn)
        update_persistence_diagram();
}

void VisualizationWindow::on_normCoordCheckBox_clicked(bool checked)
{
    if(slice_diagram != NULL)
    {
        slice_diagram->set_normalized_coords(checked);
        slice_diagram->resize_diagram();
        if(p_diagram != NULL)
            p_diagram->resize_diagram(slice_diagram->get_slice_length(), slice_diagram->get_pd_scale());
    }
}

void VisualizationWindow::on_barcodeCheckBox_clicked(bool checked)
{
    if(slice_diagram != NULL)
    {
        slice_diagram->toggle_barcode(checked);
    }
}

void VisualizationWindow::on_xi0CheckBox_toggled(bool checked)
{
    slice_diagram->toggle_xi0_points(checked);
}

void VisualizationWindow::on_xi1CheckBox_toggled(bool checked)
{
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
    std::cout<< "RESCALED BARCODE: ";
    barcode->print();
    std::cout << "\n";

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
                    std::cout << "\nERROR: inverted bar (" << birth << "," << death << ")\n";
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

void VisualizationWindow::resizeEvent(QResizeEvent* event)
{
//    qDebug() << "resize event! slice_diagrma = " << slice_diagram;
    if(slice_diagram != NULL)
    {
        slice_diagram->resize_diagram();

        if(p_diagram != NULL)
            p_diagram->resize_diagram(slice_diagram->get_slice_length(), slice_diagram->get_pd_scale());
    }
}
