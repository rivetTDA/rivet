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
    , slice_diagram_initialized(false)
    , degenerate_x(false)
    , degenerate_y(false)
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

    //update text items
    auto shortName = QString::fromStdString(input_params.shortName);
    this->setWindowTitle("RIVET - " + shortName);
    ui->filenameLabel->setText( QStringLiteral("Input file: ").append(shortName) );
    ui->homdimLabel->setText( QStringLiteral("Homology dimension: %1").arg(input_params.dim) );


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

    double initial_xmax, initial_ymax;

    if(!slice_diagram_initialized)
    {
        double xrev_sign=1-2*input_params.x_reverse;
        double yrev_sign=1-2*input_params.y_reverse;

        double x_step=fmax(.0001, (grades.x.back()-grades.x.front())/20); //spin box has 4 digit precision, so shouldn't have step size be less than this
        double y_step=fmax(.0001, (grades.y.back()-grades.y.front())/20);


        //note that setting a negative step is the same as setting a 0 step, according to QT docs
        ui->BottomCornerXSpinBox->setSingleStep(x_step);
        ui->TopCornerXSpinBox->setSingleStep(x_step);
        ui->BottomCornerYSpinBox->setSingleStep(y_step);
        ui->TopCornerYSpinBox->setSingleStep(y_step);



        origin_x=grades.x.front();
        origin_y=grades.y.front();

        degenerate_x=(grades.x.size()==1);
        degenerate_y=(grades.y.size()==1);

        //set the maximal values of the initial window


        if(degenerate_x&& degenerate_y){
            //doubly degenerate case-choose window scale arbitrarily
            initial_xmax=grades.x.front()+1;
            initial_ymax=grades.y.front()+1;
        }
        //in the singly degenerate case, choose the default window to be square
        else if(degenerate_x){
            initial_xmax=grades.x.front()+grades.y.back()-grades.y.front();
            initial_ymax=grades.y.back();
        }
        else if(degenerate_y){
            initial_xmax=grades.x.back();
            initial_ymax=grades.y.front()+grades.x.back()-grades.x.front();
        }
        else{
            initial_xmax=grades.x.back();
            initial_ymax=grades.y.back();
        }


        //set absolute bounds on the maximum/minimum window scroll boxes, to prevent numerical issues

        double max_x_length=10*(initial_xmax-grades.x.front());
        double max_y_length=10*(initial_ymax-grades.y.front());

        double xmin, xmax,ymin,ymax;

        if (input_params.x_reverse){
            xmin=-1*grades.x.back()-max_x_length;
            xmax=-1*grades.x.front()+max_x_length;
        }
        else{
            xmin=grades.x.front()-max_x_length;
            xmax=grades.x.back()+max_x_length;
        }
        if (input_params.y_reverse){
            ymin=-1*grades.y.back()-max_y_length;
            ymax=-1*grades.y.front()+max_y_length;
        }
        else{
            ymin=grades.y.front()-max_y_length;
            ymax=grades.y.back()+max_y_length;
        }

        ui->BottomCornerXSpinBox->setMinimum(xmin);
        ui->TopCornerXSpinBox->setMaximum(xmax);
        ui->BottomCornerXSpinBox->setMaximum(xmax);
        ui->TopCornerXSpinBox->setMinimum(xmin);
        ui->BottomCornerYSpinBox->setMinimum(ymin);
        ui->TopCornerYSpinBox->setMinimum(ymin);
        ui->BottomCornerYSpinBox->setMaximum(ymax);
        ui->TopCornerYSpinBox->setMaximum(ymax);



        //set the iniital bounds of the slice window


        //these calls to setValue will only update the internal value and then return (since slice_diagram_initialized is false)
        ui->BottomCornerXSpinBox->setValue(grades.x.front()*xrev_sign);
        ui->BottomCornerYSpinBox->setValue(grades.y.front()*yrev_sign);
        ui->TopCornerXSpinBox->setValue(initial_xmax*xrev_sign);
        ui->TopCornerYSpinBox->setValue(initial_ymax*yrev_sign);

        std::vector<QDoubleSpinBox*> spin_boxes={ui->BottomCornerXSpinBox,ui->BottomCornerYSpinBox,
                                                 ui->TopCornerXSpinBox,ui->TopCornerYSpinBox};

        for(auto spinbox: spin_boxes)
        {
            QSizePolicy sp_retain = spinbox->sizePolicy();
            sp_retain.setRetainSizeWhenHidden(true);
            spinbox->setSizePolicy(sp_retain);
            //spinbox->setEnabled(false);
        }

        slice_diagram_initialized=true;
    }

    //create the SliceDiagram
    config_params.xLabel = QString::fromStdString(template_points->x_label);
    config_params.yLabel = QString::fromStdString(template_points->y_label);
    if (!slice_diagram.is_created() && !grades.x.empty() && !grades.y.empty()) {




        slice_diagram.create_diagram(
            config_params.xLabel,
            config_params.yLabel,
            grades.x.front(), initial_xmax,
            grades.y.front(), initial_ymax,
            ui->normCoordCheckBox->isChecked(), template_points->homology_dimensions,
            input_params.x_reverse, input_params.y_reverse);
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

    //create persistence diagram
    p_diagram.create_diagram();

    //get the barcode
    BarcodeTemplate dbc = arrangement->get_barcode_template(angle_precise, offset_precise);
    barcode = dbc.rescale(angle_precise, offset_precise, template_points->template_points, grades);

    //TESTING
    barcode->print();

    if (!grades.x.empty() && !grades.y.empty()) {
        //shift the barcode so that "zero" is where the selected line crosses the bottom or left side of the viewing window
        double ll_corner = rivet::numeric::project_to_line(angle_precise, offset_precise, grades.x[0], grades.y[0]); //lower-left corner of line selection window
        barcode = barcode->shift(-1*ll_corner);

        //show slice line; necessary to do this here because successive functions use parameters of this diagram

        //draw the barcode
        p_diagram.set_barcode(*barcode);
        p_diagram.resize_diagram(slice_diagram.get_slice_length(), slice_diagram.get_pd_scale());
        //draws the diagram itself, but with incorrect parameter values
        double max_len=sqrt((grades.x.front()-grades.x.back())*(grades.x.front()-grades.x.back())+(grades.y.front()-grades.y.back())*(grades.y.front()-grades.y.back()));

        p_diagram.update_diagram(slice_diagram.get_slice_length(), slice_diagram.get_pd_scale(), 0, slice_diagram.get_line_visible(), *barcode);
        //updates the diagram with the correct parameter values



        //enable slice diagram control items
        slice_diagram.enable_slice_line();
        ui->angleLabel->setEnabled(true);
        ui->angleDoubleSpinBox->setEnabled(true);
        ui->offsetLabel->setEnabled(true);
        ui->offsetSpinBox->setEnabled(true);
        ui->barcodeCheckBox->setEnabled(true);

        //draw the slice diagram
        slice_diagram.zoom_diagram(angle_precise, offset_precise,0);

        slice_diagram.draw_barcode(*barcode, ui->barcodeCheckBox->isChecked());

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


void VisualizationWindow::on_BottomCornerXSpinBox_valueChanged(double x_bottom)
{

    xmin_precise=x_bottom*double(1-2*input_params.x_reverse);
    
    
    if(!slice_diagram_initialized)
    {
        return;
    }

    //set minimum value in other x spin box, to prevent overflow errors
    double padding=degenerate_x? .01: std::min(.01, .5*(grades.x.back()-grades.x.front()));





    if(input_params.x_reverse){
        ui->TopCornerXSpinBox->setMaximum(x_bottom-padding);
    }
    else{
        ui->TopCornerXSpinBox->setMinimum(x_bottom+padding);
    }
    update_origin();

    double step=fmax(.0001,(xmax_precise-xmin_precise)/20);
    ui->TopCornerXSpinBox->setSingleStep(step);
    ui->BottomCornerXSpinBox->setSingleStep(step);



    slice_diagram.update_BottomX(xmin_precise,dist_to_origin,is_visible);
    slice_diagram.zoom_diagram(angle_precise, offset_precise,dist_to_origin);
    //this needs to be done before calling update_persistence_diagram so that
    //slice_diagram contains the updated slice length and scale;
    //note that changing the value of xmin (or any other bound) does not change
    //the value of origin_x or origin_y, so the updated barcode sent to slice_diagram
    //in the call to update_persistence_diagram is identical to the barcode stored in slice_diagram
    //at this point in the program, therefore this call to zoom_diagram draws the barcode in the right place

    if(ui->actionAutomatically_reset_line->isChecked() && !slice_diagram.get_line_visible())
    {
        reset_line();
        update_persistence_diagram();
    }

    else if(persistence_diagram_drawn)
    {
        update_persistence_diagram(false);
    }



}

void VisualizationWindow::on_BottomCornerYSpinBox_valueChanged(double y_bottom)
{

    ymin_precise=y_bottom*double(1-2*input_params.y_reverse);
    if(!slice_diagram_initialized)
    {
        return;
    }
    double padding=degenerate_y? .01: std::min(.01, .5*(grades.y.back()-grades.y.front()));

    if(input_params.y_reverse){
        ui->TopCornerYSpinBox->setMaximum(y_bottom-padding);
    }
    else{
        ui->TopCornerYSpinBox->setMinimum(y_bottom+padding);
    }
    update_origin();
    double step=fmax(.0001,(ymax_precise-ymin_precise)/20);
    ui->TopCornerYSpinBox->setSingleStep(step);
    ui->BottomCornerYSpinBox->setSingleStep(step);

    slice_diagram.update_BottomY(ymin_precise,dist_to_origin,is_visible);
    slice_diagram.zoom_diagram(angle_precise, offset_precise,dist_to_origin);


    if(ui->actionAutomatically_reset_line->isChecked() && !slice_diagram.get_line_visible())
    {
        reset_line();
        update_persistence_diagram();
    }

    else if(persistence_diagram_drawn)
    {
        update_persistence_diagram(false);

    }




}

void VisualizationWindow::on_TopCornerXSpinBox_valueChanged(double x_top)
{

    xmax_precise=x_top*double(1-2*input_params.x_reverse);
    if(!slice_diagram_initialized)
    {
        return;
    }
    double padding=degenerate_x? .01: std::min(.01, .5*(grades.x.back()-grades.x.front()));

    if (input_params.x_reverse){
        ui->BottomCornerXSpinBox->setMinimum(x_top+padding);
    }
    else{
        ui->BottomCornerXSpinBox->setMaximum(x_top-padding);
    }
    update_origin();
    double step=fmax(.0001,(xmax_precise-xmin_precise)/20);
    ui->TopCornerXSpinBox->setSingleStep(step);
    ui->BottomCornerXSpinBox->setSingleStep(step);

    slice_diagram.update_TopX(xmax_precise,dist_to_origin,is_visible);
    slice_diagram.zoom_diagram(angle_precise, offset_precise,dist_to_origin);


    if(ui->actionAutomatically_reset_line->isChecked() && !slice_diagram.get_line_visible())
    {
        reset_line();
        update_persistence_diagram();
    }
    else if(persistence_diagram_drawn)
    {
        update_persistence_diagram(false);
    }





}

void VisualizationWindow::on_TopCornerYSpinBox_valueChanged(double y_top)
{

    ymax_precise=y_top*double(1-2*input_params.y_reverse);
    if(!slice_diagram_initialized)
    {
        return;
    }

    double padding=degenerate_y? .01: std::min(.01, .5*(grades.y.back()-grades.y.front()));

    if (input_params.y_reverse){
        ui->BottomCornerYSpinBox->setMinimum(y_top+padding);
    }
    else{
        ui->BottomCornerYSpinBox->setMaximum(y_top-padding);
    }
    update_origin();
    double step=fmax(.0001,(ymax_precise-ymin_precise)/20);
    ui->TopCornerYSpinBox->setSingleStep(step);
    ui->BottomCornerYSpinBox->setSingleStep(step);

    slice_diagram.update_TopY(ymax_precise,dist_to_origin,is_visible);
    slice_diagram.zoom_diagram(angle_precise, offset_precise,dist_to_origin);



    if(ui->actionAutomatically_reset_line->isChecked() && !slice_diagram.get_line_visible())
    {
        reset_line();
        update_persistence_diagram();
    }

    else if(persistence_diagram_drawn)
    {
        update_persistence_diagram(false);
    }



}


//TODO: update the value of dist_to_origin when these two boxes are changed
void VisualizationWindow::on_angleDoubleSpinBox_valueChanged(double angle)
{


    if (line_selection_ready && !slice_update_lock) {
        angle_precise = angle;
        update_origin();
        slice_diagram.update_line(angle_precise, ui->offsetSpinBox->value(), dist_to_origin);
        update_persistence_diagram();//updates the barcode in the slice diagram
        //note that the x and y scales of the diagram do not need to be updated, so the values
        //passed to the pd in this function will be correct
        slice_diagram.zoom_diagram(angle_precise, offset_precise,dist_to_origin); //draws the diagram

    }


}

void VisualizationWindow::on_offsetSpinBox_valueChanged(double offset)
{

    if (line_selection_ready && !slice_update_lock) {
        offset_precise = offset;
        update_origin();
        slice_diagram.update_line(ui->angleDoubleSpinBox->value(), offset_precise, dist_to_origin);
        update_persistence_diagram(); //updates the barcode in the slice diagram
        slice_diagram.zoom_diagram(angle_precise, offset_precise,dist_to_origin);//draws the diagram,with the updated barcode

    }

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
void VisualizationWindow::update_persistence_diagram(bool line_changed)
{
    if (persistence_diagram_drawn) {


        //get the barcode


        if(line_changed)
        {
            if (verbosity >= 0) {
                qDebug() << "  QUERY: angle =" << angle_precise << ", offset =" << offset_precise;
            }
            BarcodeTemplate dbc = arrangement->get_barcode_template(angle_precise, offset_precise);
            barcode = dbc.rescale(angle_precise, offset_precise, template_points->template_points, grades);
            //shift the barcode so that "zero" is where the selected line crosses the bottom or left side of the viewing window
            double ll_corner = rivet::numeric::project_to_line(angle_precise, offset_precise, origin_x, origin_y); //lower-left corner of line selection window
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
           slice_diagram.update_barcode(*barcode, ui->barcodeCheckBox->isChecked());
        }




        p_diagram.update_diagram(slice_diagram.get_slice_length(), slice_diagram.get_pd_scale(),dist_to_origin, is_visible,*barcode);
        //slice_diagram.update_barcode(*barcode, ui->barcodeCheckBox->isChecked());

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
    update_origin();

    //update UI elements (values will be truncated)
    ui->angleDoubleSpinBox->setValue(angle);//note these calls do NOT call update_origin or update_persistence diagram because slice_update_lock=true
    ui->offsetSpinBox->setValue(offset);


    update_persistence_diagram();
    slice_diagram.zoom_diagram(angle_precise, offset_precise,dist_to_origin);
    slice_update_lock = false;


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
    }
    if (persistence_diagram_drawn)
    {
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
        slice_diagram.receive_parameter_change();

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




void VisualizationWindow::on_actionRestore_default_window_triggered()
{
    //the internal values of the bounds
    //e.g. xmin is always less than xmax.
    //if xreverse=true, and the window goes from, e.g. 10 to 0
    //then orig_xmin=-10 and orig_xmax=0
    double orig_xmin=slice_diagram.get_original_xmin();
    double orig_xmax=slice_diagram.get_original_xmax();
    double orig_ymin=slice_diagram.get_original_ymin();
    double orig_ymax=slice_diagram.get_original_ymax();

    double xrev_sign=input_params.x_reverse? -1 :1;

    double yrev_sign=input_params.y_reverse? -1 :1;

    //set the values displayed in the controls
    ui->BottomCornerXSpinBox->setValue(orig_xmin*xrev_sign);
    ui->BottomCornerYSpinBox->setValue(orig_ymin*yrev_sign);
    ui->TopCornerXSpinBox->setValue(orig_xmax*xrev_sign);
    ui->TopCornerYSpinBox->setValue(orig_ymax*yrev_sign);

    xmin_precise=orig_xmin; //overwrite the value set by the call to setValue to avoid rounding errors in reset_line()
    xmax_precise=orig_xmax;
    ymin_precise=orig_ymin;
    ymax_precise=orig_ymax;

    reset_line();
    update_origin();
    slice_diagram.zoom_diagram(angle_precise, offset_precise, dist_to_origin);

    update_persistence_diagram();



}//end on_actionRestore_default_window_triggered()


//sets the window to be the smallest one containing all nonzero Betti numbers, with the slice line connecting the corners
void VisualizationWindow::on_actionBetti_number_window_triggered()
{
    if(degenerate_x|| degenerate_y){
        on_actionRestore_default_window_triggered();
        return;
    }


    double xmin=slice_diagram.get_min_supp_xi_x();
    double xmax=slice_diagram.get_max_supp_xi_x();
    double ymin=slice_diagram.get_min_supp_xi_y();
    double ymax=slice_diagram.get_max_supp_xi_y();

    double xrev_sign=input_params.x_reverse? -1 :1;

    double yrev_sign=input_params.y_reverse? -1 :1;

    ui->BottomCornerXSpinBox->setValue(xmin*xrev_sign);
    ui->BottomCornerYSpinBox->setValue(ymin*yrev_sign);
    ui->TopCornerXSpinBox->setValue(xmax*xrev_sign);
    ui->TopCornerYSpinBox->setValue(ymax*yrev_sign);

    xmin_precise=xmin; //overwrite the value set by the call to setValue to avoid rounding errors in reset_line()
    xmax_precise=xmax;
    ymin_precise=ymin;
    ymax_precise=ymax;

    reset_line();
    update_origin();
    slice_diagram.zoom_diagram(angle_precise, offset_precise, dist_to_origin);

    update_persistence_diagram();


}//end on_actionBetti_number_window_triggered()

//resets the line if it is not currently visible, and the option is set to true
void VisualizationWindow::on_actionAutomatically_reset_line_toggled()
{
    if(ui->actionAutomatically_reset_line->isChecked()&& ! slice_diagram.get_line_visible())
    {
        reset_line();
        update_persistence_diagram();
    }
}//end on_actionAutomatically_reset_line_toggled()


//changes the line so that it connects the extreme corners of the window
void VisualizationWindow::reset_line()
{

    double new_slope=(ymax_precise-ymin_precise)/(xmax_precise-xmin_precise);
    double new_angle=atan(new_slope)*180/PI;
    double new_offset=(ymin_precise-new_slope*xmin_precise)*cos(new_angle*PI/180);

    angle_precise=new_angle;
    offset_precise=new_offset;
    ui->angleDoubleSpinBox->setValue(new_angle);
    ui->offsetSpinBox->setValue(new_offset);
    is_visible=true;
    update_origin();

}//end reset_line()


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
//updates the values of origin_x/y, dist_to_origin,is_visible, slice_length,called after a change in slice line or window bounds
void VisualizationWindow::update_origin()
{
    if(angle_precise==90.0)
    {
        origin_y=grades.y[0];
        origin_x=-1.0*offset_precise;
        dist_to_origin=ymin_precise-origin_y;
        is_visible=(xmin_precise<=origin_x)&&(origin_x<=xmax_precise);

        slice_length=ymax_precise-ymin_precise;

    }
    else if(angle_precise==0.0)
    {
        origin_x=grades.x[0];
        origin_y=offset_precise;
        dist_to_origin=xmin_precise-origin_x;
        is_visible=(ymin_precise<=origin_y)&&(origin_y<=ymax_precise);
        slice_length=xmax_precise-xmin_precise;

    }


    else
    {
        double slope=tan(angle_precise*PI/180);
        double y_int=offset_precise/cos(angle_precise*PI/180);
        if(y_int+slope*grades.x[0]>grades.y[0])
        {//origin is on left side of box
            origin_x=grades.x[0];
            origin_y=y_int+slope*grades.x[0];

        }
        else
        {//origin is on bottom side of box
            origin_x=(grades.y[0]-y_int)/slope;
            origin_y=grades.y[0];
        }

        double dot_pos_x=0; //data coordinates of the bottom dot in the visible window
        double dot_pos_y=0;

        if(y_int+slope*xmin_precise>ymin_precise)
        {//bottom dot is on the left edge of the visible window
            dot_pos_x=xmin_precise;
            dot_pos_y=y_int+slope*xmin_precise;
            dist_to_origin=sqrt(pow(dot_pos_x-origin_x,2.0)+pow(dot_pos_y-origin_y,2.0));
            is_visible=(ymin_precise<=dot_pos_y)&&(dot_pos_y<=ymax_precise);
            
        }
        else
        {//bottom dot is on the bottom edge of the visible window
            dot_pos_x=(ymin_precise-y_int)/slope;
            dot_pos_y=ymin_precise;
            dist_to_origin=sqrt(pow(dot_pos_x-origin_x,2.0)+pow(dot_pos_y-origin_y,2.0));
            is_visible=(xmin_precise<=dot_pos_x)&&(dot_pos_x<=xmax_precise);
            

        }
        if(dot_pos_x<origin_x || dot_pos_y<origin_y)
        {
            dist_to_origin*=-1;
        }


        double top_corner_x=0; //the coordinates of the top corner of the line, in the visible window
        double top_corner_y=0;

        if(y_int+slope*xmax_precise>ymax_precise)
        {//then the top corner is on the top edge
            top_corner_x=(ymax_precise-y_int)/slope;
            top_corner_y=ymax_precise;

        }
        else
        {//the top corner is on the right edge
            top_corner_x=xmax_precise;
            top_corner_y=top_corner_x*slope+y_int;
        }

        slice_length=sqrt((top_corner_x-dot_pos_x)*(top_corner_x-dot_pos_x)+(top_corner_y-dot_pos_y)*(top_corner_y-dot_pos_y));
    }
}
