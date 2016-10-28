#include "visualizationwindow.h"
#include "ui_visualizationwindow.h"

#include "dcel/arrangement_message.h"
#include "dcel/barcode_template.h"
#include "interface/barcode.h"
#include "interface/config_parameters.h"
#include "interface/file_writer.h"
#include "numerics.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTime>

#include <algorithm>
#include <fstream>
#include <sstream>

VisualizationWindow::VisualizationWindow(InputParameters& params)
    : QMainWindow()
    , ui(new Ui::VisualizationWindow)
    , verbosity(params.verbosity)
    , INFTY(std::numeric_limits<double>::infinity())
    , PI(3.14159265358979323846)
    , data_selected(false)
    , unsaved_data(false)
    , input_params(params)
    , config_params()
    , ds_dialog(input_params, this)
    , x_grades()
    , x_exact()
    , y_grades()
    , y_exact()
    , template_points()
    , homology_dimensions()
    , angle_precise(0)
    , offset_precise(0)
    , cthread(input_params)
    , prog_dialog(this)
    , line_selection_ready(false)
    , slice_diagram(&config_params, x_grades, y_grades, this)
    , slice_update_lock(false)
    , p_diagram(&config_params, this)
    , persistence_diagram_drawn(false)
{
    ui->setupUi(this);

    //set up the slice diagram
    ui->sliceView->setScene(&slice_diagram);
    //    ui->sliceView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->sliceView->scale(1, -1);
    ui->sliceView->setRenderHint(QPainter::Antialiasing);

    //set up the persistence diagram scene
    ui->pdView->setScene(&p_diagram);
    ui->pdView->scale(1, -1);
    ui->pdView->setRenderHint(QPainter::Antialiasing);

    //connect signal from DataSelectDialog to start the computation
    QObject::connect(&ds_dialog, &DataSelectDialog::dataSelected, this, &VisualizationWindow::start_computation);

    //connect signals from ComputationThread to slots in VisualizationWindow
    QObject::connect(&cthread, &ComputationThread::advanceProgressStage, &prog_dialog, &ProgressDialog::advanceToNextStage);
    QObject::connect(&cthread, &ComputationThread::setProgressMaximum, &prog_dialog, &ProgressDialog::setStageMaximum);
    QObject::connect(&cthread, &ComputationThread::setCurrentProgress, &prog_dialog, &ProgressDialog::updateProgress);
    QObject::connect(&cthread, &ComputationThread::templatePointsReady, this, &VisualizationWindow::paint_template_points);
    QObject::connect(&cthread, &ComputationThread::arrangementReady, this, &VisualizationWindow::augmented_arrangement_ready);
    QObject::connect(&cthread, &ComputationThread::finished, &prog_dialog, &ProgressDialog::setComputationFinished);

    //connect signals and slots for the diagrams
    QObject::connect(&slice_diagram, &SliceDiagram::set_line_control_elements, this, &VisualizationWindow::set_line_parameters);
    QObject::connect(&slice_diagram, &SliceDiagram::persistence_bar_selected, &p_diagram, &PersistenceDiagram::receive_dot_selection);
    QObject::connect(&slice_diagram, &SliceDiagram::persistence_bar_deselected, &p_diagram, &PersistenceDiagram::receive_dot_deselection);
    QObject::connect(&p_diagram, &PersistenceDiagram::persistence_dot_selected, &slice_diagram, &SliceDiagram::receive_bar_selection);
    QObject::connect(&p_diagram, &PersistenceDiagram::persistence_dot_secondary_selection, &slice_diagram, &SliceDiagram::receive_bar_secondary_selection);
    QObject::connect(&p_diagram, &PersistenceDiagram::persistence_dot_deselected, &slice_diagram, &SliceDiagram::receive_bar_deselection);

    //connect other signals and slots
    QObject::connect(&prog_dialog, &ProgressDialog::stopComputation, &cthread, &ComputationThread::terminate); ///TODO: don't use QThread::terminate()! modify ComputationThread so that it can stop gracefully and clean up after itself
}

VisualizationWindow::~VisualizationWindow()
{
    delete barcode;
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

} //end start_computation()

void VisualizationWindow::copy_fields_from_cthread()
{
    //TODO: this dataflow is still odd, could use more attention.
    template_points = cthread.template_points;
    x_exact = cthread.x_exact;
    y_exact = cthread.y_exact;
    x_grades = rivet::numeric::to_doubles(x_exact);
    y_grades = rivet::numeric::to_doubles(y_exact);
    homology_dimensions.resize(std::vector<unsigned>(cthread.hom_dims.shape(), cthread.hom_dims.shape() + cthread.hom_dims.num_dimensions()));
    homology_dimensions = cthread.hom_dims;
}

//this slot is signaled when the xi support points are ready to be drawn
void VisualizationWindow::paint_template_points()
{
    //First load our local copies of the data
    copy_fields_from_cthread();
    //send xi support points to the SliceDiagram
    for (std::vector<TemplatePoint>::iterator it = template_points.begin(); it != template_points.end(); ++it)
        slice_diagram.add_point(x_grades[it->x], y_grades[it->y], it->zero, it->one, it->two);

    //create the SliceDiagram
    slice_diagram.create_diagram(cthread.x_label,
        cthread.y_label,
        x_grades.front(), x_grades.back(),
        y_grades.front(), y_grades.back(),
        ui->normCoordCheckBox->isChecked(), homology_dimensions);

    //enable control items
    ui->BettiLabel->setEnabled(true);
    ui->xi0CheckBox->setEnabled(true);
    ui->xi1CheckBox->setEnabled(true);
    ui->xi2CheckBox->setEnabled(true);
    ui->normCoordCheckBox->setEnabled(true);
    ui->angleLabel->setEnabled(true);
    ui->angleDoubleSpinBox->setEnabled(true);
    ui->offsetLabel->setEnabled(true);
    ui->offsetSpinBox->setEnabled(true);

    //update offset extents
    ///TODO: maybe these extents should be updated dynamically, based on the slope of the slice line
    ui->offsetSpinBox->setMinimum(std::min(-1 * x_grades.back(), y_grades.front()));
    ui->offsetSpinBox->setMaximum(std::max(y_grades.back(), -1 * x_grades.front()));

    //update status
    line_selection_ready = true;
    ui->statusBar->showMessage("multigraded Betti number visualization ready");
}

//this slot is signaled when the agumented arrangement is ready
void VisualizationWindow::augmented_arrangement_ready(ArrangementMessage* arrangement)
{
    //First load our local copies of the data
    copy_fields_from_cthread();

    //receive the arrangement
    this->arrangement = arrangement;

    //TESTING: print arrangement info and verify consistency
    //    arrangement->print_stats();
    //    arrangement->test_consistency();

    //inialize persistence diagram
    p_diagram.create_diagram(QString::fromStdString(input_params.shortName), input_params.dim);

    //get the barcode
    BarcodeTemplate dbc = arrangement->get_barcode_template(angle_precise, offset_precise);
    barcode = rescale_barcode_template(dbc, angle_precise, offset_precise);

    //TESTING
    barcode->print();

    //draw the barcode
    double zero_coord = project_zero(angle_precise, offset_precise);
    p_diagram.set_barcode(zero_coord, barcode);
    p_diagram.resize_diagram(slice_diagram.get_slice_length(), slice_diagram.get_pd_scale());

    slice_diagram.draw_barcode(barcode, zero_coord, ui->barcodeCheckBox->isChecked());

    //enable control items
    ui->barcodeCheckBox->setEnabled(true);

    //update status
    if (verbosity >= 2) {
        qDebug() << "COMPUTATION FINISHED; READY FOR INTERACTIVITY.";
    }
    persistence_diagram_drawn = true;
    ui->statusBar->showMessage("ready for interactive barcode exploration");

    //if an output file has been specified, then save the arrangement
    if (!input_params.outputFile.empty())
        save_arrangement(QString::fromStdString(input_params.outputFile));
    //TODO: we don't have file reading tools here anymore, so we don't know what kind of file it was
    //Have to rely on console to either a) always save (to tmp file if needed), or b) tell us filetype in the output.
    //    else if(input_params.raw_data)
    //        unsaved_data = true;

} //end augmented_arrangement_ready()

void VisualizationWindow::on_angleDoubleSpinBox_valueChanged(double angle)
{
    if (line_selection_ready && !slice_update_lock) {
        angle_precise = angle;
        slice_diagram.update_line(angle_precise, ui->offsetSpinBox->value());
    }

    update_persistence_diagram();
}

void VisualizationWindow::on_offsetSpinBox_valueChanged(double offset)
{
    if (line_selection_ready && !slice_update_lock) {
        offset_precise = offset;
        slice_diagram.update_line(ui->angleDoubleSpinBox->value(), offset_precise);
    }

    update_persistence_diagram();
}

void VisualizationWindow::on_normCoordCheckBox_clicked(bool checked)
{
    if (line_selection_ready) {
        slice_diagram.set_normalized_coords(checked);
        slice_diagram.resize_diagram();
        if (persistence_diagram_drawn)
            p_diagram.resize_diagram(slice_diagram.get_slice_length(), slice_diagram.get_pd_scale());
    }
}

void VisualizationWindow::on_barcodeCheckBox_clicked(bool checked)
{
    if (line_selection_ready)
        slice_diagram.toggle_barcode(checked);
}

void VisualizationWindow::on_xi0CheckBox_toggled(bool checked)
{
    if (line_selection_ready)
        slice_diagram.toggle_xi0_points(checked);
}

void VisualizationWindow::on_xi1CheckBox_toggled(bool checked)
{
    if (line_selection_ready)
        slice_diagram.toggle_xi1_points(checked);
}

void VisualizationWindow::on_xi2CheckBox_toggled(bool checked)
{
    if (line_selection_ready)
        slice_diagram.toggle_xi2_points(checked);
}

//updates the persistence diagram and barcode after a change in the slice line
void VisualizationWindow::update_persistence_diagram()
{
    if (persistence_diagram_drawn) {
        //get the barcode
        qDebug() << "  QUERY: angle =" << angle_precise << ", offset =" << offset_precise;
        BarcodeTemplate dbc = arrangement->get_barcode_template(angle_precise, offset_precise);
        if (barcode != NULL) //clean up the old barcode
            delete barcode;
        barcode = rescale_barcode_template(dbc, angle_precise, offset_precise);

        //TESTING
        qDebug() << "  XI SUPPORT VECTOR:";
        for (unsigned i = 0; i < template_points.size(); i++) {
            TemplatePoint p = template_points[i];
            qDebug().nospace() << "    [" << i << "]: (" << p.x << "," << p.y << ") --> (" << x_grades[p.x] << "," << y_grades[p.y] << ")";
        }
        dbc.print();
        barcode->print();

        double zero_coord = project_zero(angle_precise, offset_precise);

        //draw the barcode
        p_diagram.update_diagram(slice_diagram.get_slice_length(), slice_diagram.get_pd_scale(), zero_coord, barcode);
        slice_diagram.update_barcode(barcode, zero_coord, ui->barcodeCheckBox->isChecked());
    }
}

//rescales a barcode template by projecting points onto the specified line
// NOTE: angle in DEGREES
Barcode* VisualizationWindow::rescale_barcode_template(BarcodeTemplate& dbc, double angle, double offset)
{
    Barcode* bc = new Barcode(); //NOTE: delete later!

    //loop through bars
    for (std::set<BarTemplate>::iterator it = dbc.begin(); it != dbc.end(); ++it) {
        qDebug() << "BarTemplate: " << it->begin << " " << it->end;
        assert(it->begin < template_points.size());
        TemplatePoint begin = template_points[it->begin];
        double birth = project(begin, angle, offset);

        if (birth != INFTY) //then bar exists in this rescaling
        {
            if (it->end >= template_points.size()) //then endpoint is at infinity
            {
                //                qDebug() << "   ===>   (" << it->begin << ", inf) |---> (" << birth << ", inf)";
                bc->add_bar(birth, INFTY, it->multiplicity);
            } else //then bar is finite
            {
                assert(it->end < template_points.size());
                TemplatePoint end = template_points[it->end];
                double death = project(end, angle, offset);
                //                qDebug() << "   ===>>> (" << it->begin << "," << it->end << ") |---> (" << birth << "," << death << ")";
                bc->add_bar(birth, death, it->multiplicity);

                //testing
                if (birth > death)
                    qDebug() << "=====>>>>> ERROR: inverted bar (" << birth << "," << death << ")";
            }
        }
        //        else
        //            qDebug() << "   ===>>> (" << it->begin << "," << it->end << ") DOES NOT EXIST IN THIS PROJECTION";
    }

    return bc;
} //end rescale_barcode_template()

//computes the projection of an xi support point onto the specified line
//  NOTE: returns INFTY if the point has no projection (can happen only for horizontal and vertical lines)
//  NOTE: angle in DEGREES
double VisualizationWindow::project(TemplatePoint& pt, double angle, double offset)
{
    if (angle == 0) //then line is horizontal
    {
        if (y_grades[pt.y] <= offset) //then point is below the line, so projection exists
            return x_grades[pt.x];
        else //then no projection
            return INFTY;
    } else if (angle == 90) //then line is vertical
    {
        if (x_grades[pt.x] <= -1 * offset) //then point is left of the line, so projection exists
            return y_grades[pt.y];
        else //then no projection
            return INFTY;
    }
    //if we get here, then line is neither horizontal nor vertical
    double radians = angle * PI / 180;
    double x = x_grades[pt.x];
    double y = y_grades[pt.y];

    if (y > x * tan(radians) + offset / cos(radians)) //then point is above line
        return y / sin(radians) - offset / tan(radians); //project right

    return x / cos(radians) + offset * tan(radians); //project up
} //end project()

//computes the projection of the lower-left corner of the line-selection window onto the specified line
/// TESTING AS REPLACEMENT FOR SliceDiagram::get_zero()
double VisualizationWindow::project_zero(double angle, double offset)
{
    if (angle == 0) //then line is horizontal
        return x_grades[0];

    if (angle == 90) //then line is vertical
        return y_grades[0];

    //if we get here, then line is neither horizontal nor vertical
    double radians = angle * PI / 180;
    double x = x_grades[0];
    double y = y_grades[0];

    if (y > x * tan(radians) + offset / cos(radians)) //then point is above line
        return y / sin(radians) - offset / tan(radians); //project right

    return x / cos(radians) + offset * tan(radians); //project up
} //end project_zero()

void VisualizationWindow::set_line_parameters(double angle, double offset)
{
    slice_update_lock = true;

    //correct for slight numerical errors that the interface might introduce
    if (angle < 0 && angle > -45)
        angle = 0;
    if (angle > 90 || angle < -40)
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
    QMainWindow::showEvent(event);
    if (!data_selected) {
        ds_dialog.show();
    }
}

void VisualizationWindow::resizeEvent(QResizeEvent* /*unused*/)
{
    if (line_selection_ready) {
        slice_diagram.resize_diagram();

        if (persistence_diagram_drawn)
            p_diagram.resize_diagram(slice_diagram.get_slice_length(), slice_diagram.get_pd_scale());
    }
}

void VisualizationWindow::closeEvent(QCloseEvent* event)
{
    if (unsaved_data) {
        QMessageBox::StandardButton reallyExit;
        reallyExit = QMessageBox::question(this, "Exit?", "Are you sure you want to exit? Your augmented arrangement has not been saved and will be lost!", QMessageBox::Yes | QMessageBox::No);

        if (reallyExit == QMessageBox::Yes) {
            event->accept();
            //        qDebug() << "User has closed RIVET.";
        } else
            event->ignore();
    } else {
        event->accept();
    }
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
    configBox = new ConfigureDialog(config_params, input_params, this);
    configBox->exec();

    if (line_selection_ready) {
        slice_diagram.receive_parameter_change(cthread.x_label, cthread.y_label);

        if (persistence_diagram_drawn)
            p_diagram.receive_parameter_change();
    }

    delete configBox;
}

void VisualizationWindow::on_actionSave_persistence_diagram_as_image_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export persistence diagram as image", QCoreApplication::applicationDirPath(), "PNG Image (*.png)");
    if (!fileName.isNull()) {
        QPixmap pixMap = ui->pdView->grab();
        pixMap.save(fileName, "PNG");
    }
    ///TODO: error handling?
}

void VisualizationWindow::on_actionSave_line_selection_window_as_image_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export line selection window as image", QCoreApplication::applicationDirPath(), "PNG Image (*.png)");
    if (!fileName.isNull()) {
        QPixmap pixMap = ui->sliceView->grab();
        pixMap.save(fileName, "PNG");
    }
    ///TODO: error handling?
}

void VisualizationWindow::on_actionSave_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Save computed data", QCoreApplication::applicationDirPath());
    if (!fileName.isNull()) {
        save_arrangement(fileName);
    }
    ///TODO: error handling?
} //end on_actionSave_triggered()

void VisualizationWindow::save_arrangement(const QString& filename)
{
    try {
        write_boost_file(filename, input_params, cthread.message, *arrangement);
    } catch (std::exception& e) {
        QMessageBox errorBox(QMessageBox::Warning, "Error",
            QString("Unable to write file: ").append(filename).append(": ").append(e.what()));
        errorBox.exec();
    }
} //end save_arrangement()

void VisualizationWindow::on_actionOpen_triggered()
{
    ///TODO: get user confirmation and clear the existing data structures

    QMessageBox msgBox;
    msgBox.setText("This feature is not implemented yet.");
    msgBox.exec();

    ///TODO: open the data select dialog box and load new data
} //end on_actionOpen_triggered()
