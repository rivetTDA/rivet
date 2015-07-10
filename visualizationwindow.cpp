#include "visualizationwindow.h"
#include "ui_visualizationwindow.h"

#include "dcel/barcode_template.h"
#include "dcel/mesh.h"
#include "interface/barcode.h"
#include "interface/config_parameters.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QTime>

#include <algorithm>
#include <sstream>


VisualizationWindow::VisualizationWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VisualizationWindow),
    verbosity(5), INFTY(std::numeric_limits<double>::infinity()), PI(3.14159265358979323846),
    data_selected(false),
    input_params(), config_params(),
    ds_dialog(input_params),
    x_grades(), x_exact(), y_grades(), y_exact(), xi_support(), homology_dimensions(),
    angle_precise(0), offset_precise(0),
    cthread(verbosity, input_params, x_grades, x_exact, y_grades, y_exact, xi_support, homology_dimensions),
    line_selection_ready(false), slice_diagram(&config_params, x_grades, y_grades, this), slice_update_lock(false),
    p_diagram(&config_params, this), persistence_diagram_drawn(false)
{
    ui->setupUi(this);

    //set up the slice diagram
    ui->sliceView->setScene(&slice_diagram);
//    ui->sliceView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->sliceView->scale(1,-1);
    ui->sliceView->setRenderHint(QPainter::Antialiasing);

    //set up the persistence diagram scene
    ui->pdView->setScene(&p_diagram);
    ui->pdView->scale(1,-1);
    ui->pdView->setRenderHint(QPainter::Antialiasing);

    //connect signal from DataSelectDialog to start the computation
    QObject::connect(&ds_dialog, &DataSelectDialog::dataSelected, this, &VisualizationWindow::start_computation);

    //connect signals from ComputationThread to slots in VisualizationWindow
    QObject::connect(&cthread, &ComputationThread::advanceProgressStage, &prog_dialog, &ProgressDialog::advanceToNextStage);
    QObject::connect(&cthread, &ComputationThread::setProgressMaximum, &prog_dialog, &ProgressDialog::setStageMaximum);
    QObject::connect(&cthread, &ComputationThread::setCurrentProgress, &prog_dialog, &ProgressDialog::updateProgress);
    QObject::connect(&cthread, &ComputationThread::xiSupportReady, this, &VisualizationWindow::paint_xi_support);
    QObject::connect(&cthread, &ComputationThread::arrangementReady, this, &VisualizationWindow::augmented_arrangement_ready);
    QObject::connect(&cthread, &ComputationThread::finished, &prog_dialog, &ProgressDialog::setComputationFinished);

    //connect signals and slots for the diagrams
    QObject::connect(&slice_diagram, &SliceDiagram::set_line_control_elements, this, &VisualizationWindow::set_line_parameters);
    QObject::connect(&slice_diagram, &SliceDiagram::persistence_bar_selected, &p_diagram, &PersistenceDiagram::receive_dot_selection);
    QObject::connect(&slice_diagram, &SliceDiagram::persistence_bar_deselected, &p_diagram, &PersistenceDiagram::receive_dot_deselection);
    QObject::connect(&p_diagram, &PersistenceDiagram::persistence_dot_selected, &slice_diagram, &SliceDiagram::receive_bar_selection);
    QObject::connect(&p_diagram, &PersistenceDiagram::persistence_dot_deselected, &slice_diagram, &SliceDiagram::receive_bar_deselection);

    //connect other signals and slots
    QObject::connect(&prog_dialog, &ProgressDialog::stopComputation, &cthread, &ComputationThread::terminate);  ///TODO: don't use QThread::terminate()! modify ComputationThread so that it can stop gracefully and clean up after itself

}

VisualizationWindow::~VisualizationWindow()
{
    delete ui;
}

//slot that starts the persistent homology computation in a new thread
void VisualizationWindow::start_computation()
{
    data_selected = true;

    //show the progress box
    prog_dialog.show();
    prog_dialog.activateWindow();
    prog_dialog.raise();

    //start the computation in a new thread
    cthread.compute();

}//end start_computation()

//this slot is signaled when the xi support points are ready to be drawn
void VisualizationWindow::paint_xi_support()
{
    //send xi support points to the SliceDiagram
    for(std::vector<xiPoint>::iterator it = xi_support.begin(); it != xi_support.end(); ++it)
        slice_diagram.add_point(x_grades[it->x], y_grades[it->y], it->zero, it->one);

    //create the SliceDiagram
    slice_diagram.create_diagram(input_params.x_label, input_params.y_label, x_grades.front(), x_grades.back(), y_grades.front(), y_grades.back(), ui->normCoordCheckBox->isChecked(), homology_dimensions);

    //enable control items
    ui->xi0CheckBox->setEnabled(true);
    ui->xi1CheckBox->setEnabled(true);
    ui->normCoordCheckBox->setEnabled(true);
    ui->angleLabel->setEnabled(true);
    ui->angleDoubleSpinBox->setEnabled(true);
    ui->offsetLabel->setEnabled(true);
    ui->offsetSpinBox->setEnabled(true);

    //update offset extents
    ///TODO: maybe these extents should be updated dynamically, based on the slope of the slice line
    ui->offsetSpinBox->setMinimum( std::min(-1*x_grades.back(), y_grades.front()) );
    ui->offsetSpinBox->setMaximum( std::max(y_grades.back(), -1*x_grades.front()) );

    //update status
    line_selection_ready = true;
    ui->statusBar->showMessage("multigraded Betti number visualization ready");
}

//this slot is signaled when the agumented arrangement is ready
void VisualizationWindow::augmented_arrangement_ready(Mesh* arrangement)
{
    qDebug() << "VisualizationWindow::augmented_arrangement_ready()";

    //receive the arrangement
    this->arrangement = arrangement;

    //TESTING: print arrangement info and verify consistency
    arrangement->print_stats();
//    arrangement->test_consistency();

    //inialize persistence diagram
    p_diagram.create_diagram(input_params.shortName, input_params.dim);
    p_diagram.resize_diagram(slice_diagram.get_slice_length(), slice_diagram.get_pd_scale());

    //get the barcode
    BarcodeTemplate& dbc = arrangement->get_barcode_template(angle_precise, offset_precise);
    Barcode* barcode = rescale_barcode_template(dbc, angle_precise, offset_precise);      ///TODO: CHECK THIS!!!

    //TESTING
    barcode->print();

    //draw the barcode
    double zero_coord = project_zero(angle_precise, offset_precise);
    p_diagram.draw_dots(zero_coord, barcode);
    slice_diagram.draw_barcode(barcode, zero_coord, ui->barcodeCheckBox->isChecked());

    //clean up
    delete barcode;

    //enable control items
    ui->barcodeCheckBox->setEnabled(true);

    //update status
    if(verbosity >= 2) { qDebug() << "COMPUTATION FINISHED; READY FOR INTERACTIVITY."; }
    persistence_diagram_drawn = true;
    ui->statusBar->showMessage("ready for interactive barcode exploration");

}//end augmented_arrangement_ready()

void VisualizationWindow::on_angleDoubleSpinBox_valueChanged(double angle)
{
    if(line_selection_ready && !slice_update_lock)
    {
        angle_precise = angle;
        slice_diagram.update_line(angle_precise, ui->offsetSpinBox->value());
    }

    update_persistence_diagram();
}

void VisualizationWindow::on_offsetSpinBox_valueChanged(double offset)
{
    if(line_selection_ready && !slice_update_lock)
    {
        offset_precise = offset;
        slice_diagram.update_line(ui->angleDoubleSpinBox->value(), offset_precise);
    }

    update_persistence_diagram();
}

void VisualizationWindow::on_normCoordCheckBox_clicked(bool checked)
{
    if(line_selection_ready)
    {
        slice_diagram.set_normalized_coords(checked);
        slice_diagram.resize_diagram();
        if(persistence_diagram_drawn)
            p_diagram.resize_diagram(slice_diagram.get_slice_length(), slice_diagram.get_pd_scale());
    }
}

void VisualizationWindow::on_barcodeCheckBox_clicked(bool checked)
{
    if(line_selection_ready)
        slice_diagram.toggle_barcode(checked);
}

void VisualizationWindow::on_xi0CheckBox_toggled(bool checked)
{
    if(line_selection_ready)
        slice_diagram.toggle_xi0_points(checked);
}

void VisualizationWindow::on_xi1CheckBox_toggled(bool checked)
{
    if(line_selection_ready)
        slice_diagram.toggle_xi1_points(checked);
}

//updates the persistence diagram and barcode after a change in the slice line
void VisualizationWindow::update_persistence_diagram()
{
    if(persistence_diagram_drawn)
    {
        //get the barcode
        BarcodeTemplate& dbc = arrangement->get_barcode_template(angle_precise, offset_precise);
        Barcode* barcode = rescale_barcode_template(dbc, angle_precise, offset_precise);

        //TESTING
        dbc.print();
        barcode->print();
        double zero_coord = project_zero(angle_precise, offset_precise);

        //draw the barcode
        p_diagram.update_diagram(slice_diagram.get_slice_length(), slice_diagram.get_pd_scale(), zero_coord, barcode);
        slice_diagram.update_barcode(barcode, zero_coord, ui->barcodeCheckBox->isChecked());

        //clean up
        delete barcode;
    }
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
//                qDebug() << "   ===>>> (" << it->begin << ",inf) |---> (" << birth << ",inf)";
                bc->add_bar(birth, INFTY, it->multiplicity);
            }
            else    //then bar is finite
            {
                xiPoint end = xi_support[it->end];
                double death = project(end, angle, offset);
//                qDebug() << "   ===>>> (" << it->begin << "," << it->end << ") |---> (" << birth << "," << death << ")";
                bc->add_bar(birth, death, it->multiplicity);

                //testing
                if(birth > death)
                    qDebug() << "=====>>>>> ERROR: inverted bar (" << birth << "," << death << ")";
            }
        }
//        else
//            qDebug() << "   ===>>> (" << it->begin << "," << it->end << ") DOES NOT EXIST IN THIS PROJECTION";
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
    double radians = angle*PI/180;
    double x = x_grades[pt.x];
    double y = y_grades[pt.y];

    if( y > x*tan(radians) + offset/cos(radians) )	//then point is above line
        return y/sin(radians) - offset/tan(radians); //project right

    return x/cos(radians) + offset*tan(radians); //project up
}//end project()

//computes the projection of the lower-left corner of the line-selection window onto the specified line
/// TESTING AS REPLACEMENT FOR SliceDiagram::get_zero()
double VisualizationWindow::project_zero(double angle, double offset)
{
    if(angle == 0)  //then line is horizontal
        return x_grades[0];

    if(angle == 90)    //then line is vertical
        return y_grades[0];

    //if we get here, then line is neither horizontal nor vertical
    double radians = angle*PI/180;
    double x = x_grades[0];
    double y = y_grades[0];

    if( y > x*tan(radians) + offset/cos(radians) )	//then point is above line
        return y/sin(radians) - offset/tan(radians); //project right

    return x/cos(radians) + offset*tan(radians); //project up
}//end project_zero()

void VisualizationWindow::set_line_parameters(double angle, double offset)
{
    slice_update_lock = true;

    //correct for slight numerical errors that the interface might introduce
    if(angle < 0 && angle > -45)
        angle = 0;
    if(angle > 90 || angle < -40)
        angle = 90;

    //store values internally
    angle_precise = angle;
    offset_precise = offset;

    //update UI elements (values will be truncated)
    ui->angleDoubleSpinBox->setValue(angle);
    ui->offsetSpinBox->setValue(offset);

    slice_update_lock = false;

    update_persistence_diagram();
}

void VisualizationWindow::showEvent(QShowEvent* event)
{
    if(!data_selected)
    {
        QMainWindow::showEvent(event);
        ds_dialog.exec();               //show the DataSelectDialog box and blocks until the dialog is closed
    }
}

void VisualizationWindow::resizeEvent(QResizeEvent* /*unused*/)
{
//    qDebug() << "resize event! slice_diagrma = " << slice_diagram;
    if(line_selection_ready)
    {
        slice_diagram.resize_diagram();

        if(persistence_diagram_drawn)
            p_diagram.resize_diagram(slice_diagram.get_slice_length(), slice_diagram.get_pd_scale());
    }
}

void VisualizationWindow::closeEvent(QCloseEvent* event)
{
    QMessageBox::StandardButton reallyExit;
    reallyExit = QMessageBox::question(this, "Exit?", "Are you sure you want to exit? All unsaved data will be lost!", QMessageBox::Yes|QMessageBox::No);

    if(reallyExit == QMessageBox::Yes)
    {
        event->accept();
        qDebug() << "User has closed RIVET.";
    }
    else
        event->ignore();
}


void VisualizationWindow::on_actionExit_triggered()
{
    close();
}

void VisualizationWindow::on_actionAbout_triggered()
{
    aboutBox.show();
}

void VisualizationWindow::on_actionConfigure_triggered()
{
    configBox = new ConfigureDialog(config_params, this);
    configBox->exec();

    if(line_selection_ready)
    {
        slice_diagram.receive_parameter_change();

        if(persistence_diagram_drawn)
            p_diagram.receive_parameter_change();
    }

    delete configBox;
}

void VisualizationWindow::on_actionSave_persistence_diagram_as_image_triggered()
{
    QString fileName= QFileDialog::getSaveFileName(this, "Export persistence diagram as image", QCoreApplication::applicationDirPath(), "PNG Image (*.png)");
    if (!fileName.isNull())
    {
        QPixmap pixMap = ui->pdView->grab();
        pixMap.save(fileName, "PNG");
    }
    ///TODO: error handling?
}

void VisualizationWindow::on_actionSave_line_selection_window_as_image_triggered()
{
    QString fileName= QFileDialog::getSaveFileName(this, "Export line selection window as image", QCoreApplication::applicationDirPath(), "PNG Image (*.png)");
    if (!fileName.isNull())
    {
        QPixmap pixMap = ui->sliceView->grab();
        pixMap.save(fileName, "PNG");
    }
    ///TODO: error handling?
}

void VisualizationWindow::on_actionSave_triggered()
{
    QString fileName= QFileDialog::getSaveFileName(this, "Save computed data", QCoreApplication::applicationDirPath(), "Text File (*.txt)");
    if (!fileName.isNull())
    {
        QFile file(fileName);
        if(file.open(QIODevice::ReadWrite | QIODevice::Truncate))
        {
            QTextStream stream(&file);

            //write header info, in comment form
            stream << "# augmented arrangement data" << endl;
            stream << "# computed by RIVET from the input file " << input_params.fileName << endl;
            stream << "# homology dimension: " << input_params.dim << endl;
            stream << "# bins: " << input_params.x_bins << " " << input_params.y_bins << endl;
            stream << "# file created at: " << QDateTime::currentDateTime().toString() << endl << endl;

            //write parameters
            stream << "RIVET_0" << endl;
            stream << input_params.dim << endl;
            stream << input_params.x_label << endl;
            stream << input_params.y_label << endl << endl;

            //write x-grades
            stream << "x-grades" << endl;
            for(std::vector<exact>::iterator it = x_exact.begin(); it != x_exact.end(); ++it)
            {
                std::ostringstream oss;
                oss << *it;
                stream << QString::fromStdString(oss.str()) << endl;
            }
            stream << endl;

            //write y-grades
            stream << "y-grades" << endl;
            for(std::vector<exact>::iterator it = y_exact.begin(); it != y_exact.end(); ++it)
            {
                std::ostringstream oss;
                oss << *it;
                stream << QString::fromStdString(oss.str()) << endl;
            }
            stream << endl;

            //write values of the multigraded Betti numbers
            stream << "xi values" << endl;
            for(std::vector<xiPoint>::iterator it = xi_support.begin(); it != xi_support.end(); ++it)
            {
                xiPoint p = *it;
                stream << p.x << " " << p.y << " " << p.zero << " " << p.one << endl;
            }
            stream << endl;

            //write barcode templates
            stream << "barcode templates" << endl;
            for(unsigned i = 0; i < arrangement->num_faces(); i++)
            {
                BarcodeTemplate& bc = arrangement->get_barcode_template(i);
                if(bc.is_empty())
                {
                    stream << "-";  //this denotes an empty barcode (necessary because FileInputReader ignores white space)
                }
                else
                {
                    for(std::set<BarTemplate>::iterator it = bc.begin(); it != bc.end(); ++it)
                    {
                        stream << it->begin << ",";
                        if(it->end == (unsigned) -1)    //then the bar ends at infinity, but we just write "i"
                            stream << "i";
                        else
                            stream << it->end;
                        stream << "," << it->multiplicity << " ";
                    }
                }
                stream << endl;
            }

            file.close();   //close the file -- this might not be necessary
        }
        else
        {
            QMessageBox errorBox(QMessageBox::Warning, "Error", "Unable to write file.");
            errorBox.exec();
        }

    }
    ///TODO: error handling?
}//end on_actionSave_triggered()

void VisualizationWindow::on_actionOpen_triggered()
{
    ///TODO: get user confirmation and clear the existing data structures

    QMessageBox msgBox;
    msgBox.setText("This feature is not implemented yet.");
    msgBox.exec();


    ///TODO: open the data select dialog box and load new data
}//end on_actionOpen_triggered()
