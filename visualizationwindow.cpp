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

#include "visualizationwindow.h"
#include "ui_visualizationwindow.h"

#include "dcel/arrangement_message.h"
#include "dcel/barcode.h"
#include "dcel/barcode_template.h"
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

const QString VisualizationWindow::DEFAULT_SAVE_DIR_KEY("default_save_dir");

VisualizationWindow::VisualizationWindow(InputParameters& params)
    : QMainWindow()
    , ui(new Ui::VisualizationWindow)
    , verbosity(params.verbosity)
    , data_selected(false)
    , unsaved_data(false)
    , input_params(params)
    , config_params()
    , ds_dialog(input_params, this)
    , grades()
    , angle_precise(0)
    , offset_precise(0)
    , template_points()
    , cthread(input_params)
    , prog_dialog(this)
    , line_selection_ready(false)
    , slice_diagram(&config_params, grades.x, grades.y, this)
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

//this slot is signaled when the xi support points are ready to be drawn
void VisualizationWindow::paint_template_points(std::shared_ptr<TemplatePointsMessage> points)
{
    qDebug() << "VisualizationWindow: Received template points";

    template_points = points;

    //first load our local copies of the data
    grades = Grades(template_points->x_exact, template_points->y_exact);

    //send xi support points to the SliceDiagram
    slice_diagram.clear_points();
    for (auto point : template_points->template_points)
        slice_diagram.add_point(grades.x[point.x], grades.y[point.y], point.zero, point.one, point.two);

    //create the SliceDiagram
    if (!slice_diagram.is_created() && !grades.x.empty() && !grades.y.empty()) {
        slice_diagram.create_diagram(
            QString::fromStdString(template_points->x_label),
            QString::fromStdString(template_points->y_label),
            grades.x.front(), grades.x.back(),
            grades.y.front(), grades.y.back(),
            ui->normCoordCheckBox->isChecked(), template_points->homology_dimensions);
    }

    //enable control items
    ui->BettiLabel->setEnabled(true);
    ui->xi0CheckBox->setEnabled(true);
    ui->xi1CheckBox->setEnabled(true);
    ui->xi2CheckBox->setEnabled(true);
    ui->normCoordCheckBox->setEnabled(true);

    //update offset extents
    ///TODO: maybe these extents should be updated dynamically, based on the slope of the slice line
    ui->offsetSpinBox->setMinimum(grades.min_offset());
    ui->offsetSpinBox->setMaximum(grades.max_offset());

    //update status
    line_selection_ready = true;
    ui->statusBar->showMessage("bigraded Betti number visualization ready");
}

//this slot is signaled when the augmented arrangement is ready
void VisualizationWindow::augmented_arrangement_ready(std::shared_ptr<ArrangementMessage> arrangement)
{
    //receive the arrangement
    this->arrangement = arrangement;

    if(arrangement->is_empty()) { //e.g. the arrangement contains only Betti numbers and no barcode templates
        return;
    }

    //TESTING: print arrangement info and verify consistency
    //    arrangement->print_stats();
    //    arrangement->test_consistency();
    auto shortName = QString::fromStdString(input_params.shortName);

    this->setWindowTitle("RIVET - " + shortName);

    //inialize persistence diagram
    p_diagram.create_diagram(shortName, input_params.dim);

    //get the barcode
    BarcodeTemplate dbc = arrangement->get_barcode_template(angle_precise, offset_precise);
    barcode = dbc.rescale(angle_precise, offset_precise, template_points->template_points, grades);

    //TESTING
    barcode->print();

    if (!grades.x.empty() && !grades.y.empty()) {
        //shift the barcode so that "zero" is where the selected line crosses the bottom or left side of the viewing window
    	double ll_corner = rivet::numeric::project_to_line(angle_precise, offset_precise, grades.x[0], grades.y[0]); //lower-left corner of line selection window
    	barcode = barcode->shift(-1*ll_corner);

    	//draw the barcode
        p_diagram.set_barcode(*barcode);
        p_diagram.resize_diagram(slice_diagram.get_slice_length(), slice_diagram.get_pd_scale());

        slice_diagram.draw_barcode(*barcode, ui->barcodeCheckBox->isChecked());

        //enable slice diagram control items
        slice_diagram.enable_slice_line();
        ui->angleLabel->setEnabled(true);
        ui->angleDoubleSpinBox->setEnabled(true);
        ui->offsetLabel->setEnabled(true);
        ui->offsetSpinBox->setEnabled(true);
        ui->barcodeCheckBox->setEnabled(true);

        //update status
        if (verbosity >= 2) {
            qDebug() << "COMPUTATION FINISHED; READY FOR INTERACTIVITY.";
        }
        persistence_diagram_drawn = true;
        ui->statusBar->showMessage("ready for interactive barcode exploration");

        //Enable save menu item
        ui->actionSave->setEnabled(true);
        //if an output file has been specified, then save the arrangement
        if (!input_params.outputFile.empty())
            save_arrangement(QString::fromStdString(input_params.outputFile));
        //TODO: we don't have file reading tools here anymore, so we don't know what kind of file it was
        //Have to rely on console to either a) always save (to tmp file if needed), or b) tell us filetype in the output.
        //    else if(input_params.raw_data)
        //        unsaved_data = true;
    }
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
        if (verbosity >= 0) {
            qDebug() << "  QUERY: angle =" << angle_precise << ", offset =" << offset_precise;
        }
        BarcodeTemplate dbc = arrangement->get_barcode_template(angle_precise, offset_precise);
        barcode = dbc.rescale(angle_precise, offset_precise, template_points->template_points, grades);

        //shift the barcode so that "zero" is where the selected line crosses the bottom or left side of the viewing window
    	double ll_corner = rivet::numeric::project_to_line(angle_precise, offset_precise, grades.x[0], grades.y[0]); //lower-left corner of line selection window
    	barcode = barcode->shift(-1*ll_corner);

        //TESTING
        //qDebug() << "  XI SUPPORT VECTOR:";
        //for (unsigned i = 0; i < template_points->template_points.size(); i++) {
        //    TemplatePoint p = template_points->template_points[i];
        //    qDebug().nospace() << "    [" << i << "]: (" << p.x << "," << p.y << ") --> (" << grades.x[p.x] << "," << grades.y[p.y] << ")";
        //}
        if (verbosity >= 0) {
            dbc.print();
            barcode->print();
        }

        //draw the barcode
        p_diagram.update_diagram(slice_diagram.get_slice_length(), slice_diagram.get_pd_scale(), *barcode);
        slice_diagram.update_barcode(*barcode, ui->barcodeCheckBox->isChecked());
    }
}

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
        ds_dialog.exec(); //show();
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
        slice_diagram.receive_parameter_change(QString::fromStdString(template_points->x_label),
            QString::fromStdString(template_points->y_label));

        if (persistence_diagram_drawn)
            p_diagram.receive_parameter_change();
    }

    delete configBox;
}

void VisualizationWindow::on_actionSave_persistence_diagram_as_image_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, "Export persistence diagram as image",
        suggestedName("persist_offset_" + QString::number(offset_precise)
            + "_angle_" + QString::number(angle_precise) + ".png"),
        "PNG Image (*.png)");
    if (!fileName.isNull()) {
        QSettings settings;
        settings.setValue(DEFAULT_SAVE_DIR_KEY, QFileInfo(fileName).absolutePath());
        QPixmap pixMap = ui->pdView->grab();
        pixMap.save(fileName, "PNG");
    }
    ///TODO: error handling?
}

void VisualizationWindow::on_actionSave_line_selection_window_as_image_triggered()
{

    QString fileName = QFileDialog::getSaveFileName(this, "Export line selection window as image",
        suggestedName("line_offset_" + QString::number(offset_precise)
            + "_angle_" + QString::number(angle_precise) + ".png"),
        "PNG Image (*.png)");
    if (!fileName.isNull()) {
        QSettings settings;
        settings.setValue(DEFAULT_SAVE_DIR_KEY, QFileInfo(fileName).absolutePath());
        QPixmap pixMap = ui->sliceView->grab();
        pixMap.save(fileName, "PNG");
    }
    ///TODO: error handling?
}

void VisualizationWindow::on_actionSave_triggered()
{

    QString fileName = QFileDialog::getSaveFileName(this, "Save computed data", suggestedName("rivet"));
    if (!fileName.isNull()) {
        QSettings settings;
        settings.setValue(DEFAULT_SAVE_DIR_KEY, QFileInfo(fileName).absolutePath());
        save_arrangement(fileName);
    }
    ///TODO: error handling?
} //end on_actionSave_triggered()

void VisualizationWindow::save_arrangement(const QString& filename)
{
    try {
        write_boost_file(filename, input_params, *template_points, *arrangement);
    } catch (std::exception& e) {
        QMessageBox errorBox(QMessageBox::Warning, "Error",
            QString("Unable to save arrangement: ").append(filename).append(": ").append(e.what()));
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

QString VisualizationWindow::suggestedName(QString extension)
{
    QSettings settings;
    auto name = QString::fromStdString(input_params.fileName + ".H"
                    + std::to_string(input_params.dim)
                    + "_" + std::to_string(input_params.x_bins)
                    + "_" + std::to_string(input_params.y_bins)
                    + ".")
        + extension;
    auto suggested = QDir(settings.value(DEFAULT_SAVE_DIR_KEY).toString()).filePath(name);
    return suggested;
}