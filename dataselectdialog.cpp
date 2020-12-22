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

#include "dataselectdialog.h"
#include "ui_dataselectdialog.h"

#include "api.h"
#include "interface/console_interaction.h"
#include "interface/file_input_reader.h"
#include "interface/input_parameters.h"
#include "interface/input_manager.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <QStringList>
#include <boost/algorithm/string.hpp>
#include <fstream>


DataSelectDialog::DataSelectDialog(InputParameters& params, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DataSelectDialog)
    , params(params)
    , data_selected(false)
{
    ui->setupUi(this);
    //set initial values
    if (!params.fileName.empty()) {
        detect_file_type();
        ui->homDimSpinBox->setValue(params.hom_degree);
        ui->xbinSpinBox->setValue(params.x_bins);
        ui->ybinSpinBox->setValue(params.y_bins);
    } else {
        ui->homDimSpinBox->setValue(0);
        ui->xbinSpinBox->setValue(10);
        ui->ybinSpinBox->setValue(10);
    }
    // infinity button
    ui->maxDistHelp->setText(QChar(0x221E));
    ui->maxDistHelp->setStyleSheet("QPushButton { font : 20px; padding-top: -4px;}"); // doesn't work:  qproperty-alignment: AlignTop;
}

DataSelectDialog::~DataSelectDialog()
{
    delete ui;
}

void DataSelectDialog::closeEvent(QCloseEvent* event)
{
    event->accept();

    if (!data_selected) {
        qobject_cast<QWidget*>(this->parent())->close();
        exit(0);
    }
}

void DataSelectDialog::showEvent(QShowEvent* event)
{
    event->accept();

    // reset the parameter frame when a new dialog is created
    // every object should be set to default

    ui->fileLabel->setText("First, select a file.");
    ui->fileTypeLabel->setText("You can start from a point cloud, finite metric space, bifiltration, FIRep, or a module invariants file.");

    ui->xAxisLabel->setText("");
    ui->yAxisLabel->setText("");
    ui->xRevCheckBox->setChecked(false);
    ui->yRevCheckBox->setChecked(false);
    ui->xRevCheckBox->setEnabled(true);
    ui->yRevCheckBox->setEnabled(true);
    ui->maxDistBox->setEnabled(true);
    ui->maxDistBox->setText("");
    ui->maxDistHelp->setToolTip("");
    ui->maxDistHelp->setEnabled(false);
    if (ui->dataTypeComboBox->findText("N/A") != -1)
        ui->dataTypeComboBox->removeItem(ui->dataTypeComboBox->findText("N/A"));
    ui->dataTypeComboBox->setCurrentIndex(0);
    ui->dataTypeComboBox->setEnabled(true);
    ui->xbinSpinBox->setValue(10);
    ui->ybinSpinBox->setValue(10);
    ui->homDimSpinBox->setSpecialValueText("");
    ui->homDimSpinBox->setEnabled(true);
    ui->homDimSpinBox->setValue(0);
    ui->parameterFrame->setEnabled(false);
    ui->computeButton->setEnabled(false);
    if (ui->filterComboBox->findText("N/A") != -1)
        ui->filterComboBox->removeItem(ui->filterComboBox->findText("N/A"));
    ui->filterComboBox->setEnabled(true);
    ui->filterComboBox->setCurrentIndex(0);
    ui->maxDistBox->setPalette(this->style()->standardPalette());
    ui->maxDistBox->setToolTip("");
    ui->functionComboBox->setEditable(true);
    ui->functionComboBox->setCurrentText("none");
    ui->functionComboBox->setEnabled(true);
    ui->parameterSpinBox->setEnabled(true);
    ui->parameterSpinBox->setValue(0.00);
    ui->parameterSpinBox->setSpecialValueText("");
    ui->parameterLabel->setText("Parameter:");

}

void DataSelectDialog::on_computeButton_clicked()
{
    // check if max_dist value is invalid
    // turns box red if it is and does not proceed
    params.md_string = ui->maxDistBox->text().toStdString();
    if (params.md_string != "N/A" && (params.md_string.length() != 3 || params.md_string != "inf")) {
        double md = atof(params.md_string.c_str());
        if (md <= 0 || md == std::numeric_limits<double>::infinity()) {
            ui->maxDistBox->setPalette(QPalette(QColor("red")));
            ui->maxDistBox->setToolTip("Distance must be a number greater than 0");
            return;
        }
    }
    
    // read in the input parameters from the dialog
    params.hom_degree = ui->homDimSpinBox->value();
    params.x_bins = ui->xbinSpinBox->value();
    params.y_bins = ui->ybinSpinBox->value();
    params.x_label = ui->xAxisLabel->text().toStdString();
    params.y_label = ui->yAxisLabel->text().toStdString();
    params.x_reverse = ui->xRevCheckBox->checkState();
    params.y_reverse = ui->yRevCheckBox->checkState();
    params.type = ui->dataTypeComboBox->currentText().toStdString();
    if (params.type != "bifiltration" && params.type != "firep" && params.type != "RIVET_msgpack")
        params.bifil = ui->filterComboBox->currentText().toStdString();
    if (params.bifil == "function") {
        params.function_type = ui->functionComboBox->currentText().toStdString();
        params.filter_param = ui->parameterSpinBox->value();
    }

    data_selected = true;

    emit dataSelected();

    close();
}

void DataSelectDialog::on_openFileButton_clicked()
{
    const QString DEFAULT_DIR_KEY("default_load_dir");

    QSettings settings;

    //prompt user to select a file
    auto selected_file = QFileDialog::getOpenFileName(
        this, tr("Open Data File"), settings.value(DEFAULT_DIR_KEY).toString(), "");

    if (!selected_file.isNull()) {

        params.fileName = selected_file.toUtf8().constData();
        QDir current_dir;
        settings.setValue(DEFAULT_DIR_KEY, current_dir.absoluteFilePath(selected_file));
        detect_file_type();

    }
} //end on_openFileButton_clicked()

void DataSelectDialog::on_maxDistHelp_clicked()
{
    ui->maxDistBox->setText("inf");
}

void DataSelectDialog::detect_file_type()
{
    // set everything to default here
    // this is necessary for the load new data feature

    ui->fileLabel->setText("First, select a file.");
    ui->fileTypeLabel->setText("You can start from a point cloud, finite metric space, bifiltration, FIRep, or a module invariants file.");

    ui->homDimSpinBox->setSpecialValueText("");
    //this turns off the special value text (i.e. zero is displayed like normal)

    ui->homDimSpinBox->setEnabled(true);
    ui->homDimSpinBox->setValue(0);

    ui->maxDistBox->setEnabled(true);
    ui->maxDistBox->setText("");
    ui->maxDistHelp->setToolTip("");
    ui->maxDistHelp->setEnabled(false);

    if (ui->dataTypeComboBox->findText("N/A") != -1)
        ui->dataTypeComboBox->removeItem(ui->dataTypeComboBox->findText("N/A"));
    ui->dataTypeComboBox->setCurrentIndex(0);
    ui->dataTypeComboBox->setEnabled(true);
    // disable bifiltration and FIRep types -- maybe unnecessary
    qobject_cast<QStandardItemModel*>(ui->dataTypeComboBox->model())->item(4)->setEnabled(true);
    qobject_cast<QStandardItemModel*>(ui->dataTypeComboBox->model())->item(5)->setEnabled(true);

    if (ui->filterComboBox->findText("N/A") != -1)
        ui->filterComboBox->removeItem(ui->filterComboBox->findText("N/A"));
    ui->filterComboBox->setCurrentIndex(0);
    ui->filterComboBox->setEnabled(true);

    ui->xbinSpinBox->setValue(10);
    ui->ybinSpinBox->setValue(10);

    ui->xRevCheckBox->setChecked(false);
    ui->yRevCheckBox->setChecked(false);

    ui->maxDistBox->setPalette(this->style()->standardPalette());
    ui->maxDistBox->setToolTip("");

    ui->functionComboBox->setEditable(true);
    ui->functionComboBox->setCurrentText("none");
    ui->functionComboBox->setEnabled(true);
    qobject_cast<QStandardItemModel*>(ui->functionComboBox->model())->item(0)->setEnabled(true);
    ui->parameterSpinBox->setEnabled(true);
    ui->parameterSpinBox->setValue(0.00);
    ui->parameterSpinBox->setSpecialValueText("");
    ui->parameterLabel->setText("Parameter:");

    // also remember to reset input parameter values
    params.x_label = "";
    params.y_label = "distance";

    params.x_reverse = false;
    params.y_reverse = false;

    params.x_bins = 10;
    params.y_bins = 10;

    params.bifil = "";
    params.new_function = false;
    params.old_function = false;

    params.type = "points";
    params.max_dist = -1;
    params.md_string = "inf";
    params.hom_degree = 0;

    params.function_type = "none";
    params.filter_param = 0;

    std::ifstream infile(params.fileName);

    if (!infile.is_open()) {
        invalid_file("Unable to read file.");
        return;
    }

    FileInputReader reader(infile);
    if (!reader.has_next_line()) {
        invalid_file("Empty file.");
        return;
    }

    // determine parameters specified in the input file
    InputManager inputManager(params);
    inputManager.start();

    // modify values in the parameter frame and dataselect dialog
    // by reading values in from the input file
    // depending on file type, some options in the parameter frame are unavailable

    QString type_string("This file appears to contain ");
    bool raw = true;
    
    ui->homDimSpinBox->setValue(params.hom_degree);
    ui->maxDistBox->setText(QString::fromStdString(params.md_string));

    if (inputManager.type_set)
        ui->dataTypeComboBox->setEnabled(false);

    if (params.type == "points") {
        type_string += "point-cloud data.";
        ui->dataTypeComboBox->setCurrentIndex(0);

        // disable bifiltration and FIRep types
        qobject_cast<QStandardItemModel*>(ui->dataTypeComboBox->model())->item(4)->setEnabled(false);
        qobject_cast<QStandardItemModel*>(ui->dataTypeComboBox->model())->item(5)->setEnabled(false);

        // disable user function values
        qobject_cast<QStandardItemModel*>(ui->functionComboBox->model())->item(0)->setEnabled(false);

        ui->xRevCheckBox->setEnabled(false);
        ui->yRevCheckBox->setEnabled(false);
    }
    else if (params.type == "points_fn") {
        type_string += "point-cloud data with function values.";
        ui->dataTypeComboBox->setCurrentIndex(1);
        ui->yRevCheckBox->setEnabled(false);
    }
    else if (params.type == "metric") {
        type_string += "metric data.";
        ui->dataTypeComboBox->setCurrentIndex(2);
        ui->xRevCheckBox->setEnabled(false);
        ui->yRevCheckBox->setEnabled(false);

        // disable user function values
        qobject_cast<QStandardItemModel*>(ui->functionComboBox->model())->item(0)->setEnabled(false);
    }
    else if (params.type == "metric_fn") {
        type_string += "metric data with function values.";
        ui->dataTypeComboBox->setCurrentIndex(3);
        ui->yRevCheckBox->setEnabled(false);
    }
    else if (params.type == "bifiltration") {
        ui->dataTypeComboBox->setCurrentIndex(4);
        ui->dataTypeComboBox->setEnabled(false);
        type_string += "bifiltration data.";
        ui->maxDistBox->setText("N/A");
        ui->maxDistBox->setEnabled(false);
        ui->filterComboBox->addItem("N/A");
        ui->filterComboBox->setCurrentIndex(ui->filterComboBox->count()-1);
        ui->filterComboBox->setEnabled(false);
        ui->functionComboBox->setEnabled(false);
        ui->xAxisLabel->setEnabled(true);
        ui->yAxisLabel->setEnabled(true);
        ui->xRevCheckBox->setEnabled(true);
        ui->xRevCheckBox->setChecked(false);
        ui->yRevCheckBox->setEnabled(true);
        ui->yRevCheckBox->setChecked(false);
    }
    else if (params.type == "firep") {
        ui->dataTypeComboBox->setCurrentIndex(5);
        ui->dataTypeComboBox->setEnabled(false);
        type_string += "free implicit representation data.";

        ui->homDimSpinBox->setSpecialValueText("N/A");
        ui->homDimSpinBox->setValue(0);
        ui->homDimSpinBox->setEnabled(false);

        ui->maxDistBox->setText("N/A");
        ui->maxDistBox->setEnabled(false);
        ui->filterComboBox->addItem("N/A");
        ui->filterComboBox->setCurrentIndex(ui->filterComboBox->count()-1);
        ui->filterComboBox->setEnabled(false);
        ui->functionComboBox->setEnabled(false);

        ui->xRevCheckBox->setEnabled(true);
        ui->xRevCheckBox->setChecked(false);
        ui->yRevCheckBox->setEnabled(true);
        ui->yRevCheckBox->setChecked(false);
    }
    else if (params.type == "RIVET_msgpack") {
        ui->dataTypeComboBox->addItem("RIVET_msgpack");
        ui->dataTypeComboBox->setCurrentIndex(ui->dataTypeComboBox->count()-1);
        ui->filterComboBox->addItem("N/A");
        ui->filterComboBox->setCurrentIndex(ui->filterComboBox->count()-1);
        ui->dataTypeComboBox->setEnabled(false);
        type_string += "pre-computed RIVET data.";
        raw = false;
    }

    ui->xAxisLabel->setText(QString::fromStdString(params.x_label));
    ui->yAxisLabel->setText(QString::fromStdString(params.y_label));

    if (params.type != "firep" && params.type != "bifiltration" && params.type != "RIVET_msgpack") {
        if (params.bifil == "degree") {
            ui->filterComboBox->setCurrentIndex(0);
            ui->xAxisLabel->setText("degree");
            ui->xAxisLabel->setEnabled(false);
            ui->xRevCheckBox->setChecked(true);
            ui->xRevCheckBox->setEnabled(false);
            ui->functionComboBox->setEditable(true);
            ui->functionComboBox->setCurrentText("none");
            ui->functionComboBox->setEnabled(false);
            ui->parameterSpinBox->setEnabled(false);
            ui->parameterSpinBox->setSpecialValueText("N/A");
            ui->parameterSpinBox->setValue(0.00);
        }
        else {
            if (params.bifil == "function")
                ui->filterComboBox->setCurrentIndex(1);
            ui->xAxisLabel->setText(QString::fromStdString(params.x_label));
            ui->xAxisLabel->setEnabled(true);
            ui->xRevCheckBox->setChecked(false);
            ui->xRevCheckBox->setEnabled(true);
            ui->functionComboBox->setEnabled(true);
            ui->functionComboBox->setEditable(false);
            ui->functionComboBox->setCurrentText(QString::fromStdString(params.function_type));
            if (params.function_type == "user" || params.function_type == "none") {
                ui->parameterSpinBox->setEnabled(false);
                ui->parameterSpinBox->setSpecialValueText("N/A");
                ui->parameterSpinBox->setValue(0.00);
            }
            else {
                params.x_reverse = true;
                if (params.function_type == "balldensity") {
                    ui->parameterLabel->setText("Radius:");
                    ui->xAxisLabel->setText("ball density");
                }
                else if (params.function_type == "gaussian") {
                    ui->parameterLabel->setText("Std Dev:");
                    ui->xAxisLabel->setText("gaussian density");
                }
                else if (params.function_type == "eccentricity") {
                    ui->parameterLabel->setText("P Norm:");
                    ui->xAxisLabel->setText("eccentricity");
                }
                ui->parameterSpinBox->setEnabled(true);
                ui->parameterSpinBox->setSpecialValueText("");
                if (params.function_type == "eccentricity" && params.filter_param == 0)
                    ui->parameterSpinBox->setValue(1.0);
                else
                    ui->parameterSpinBox->setValue(params.filter_param);
            } 
        }
    }

    if (params.x_reverse)
        ui->xRevCheckBox->setChecked(true);

    if (params.y_reverse)
        ui->yRevCheckBox->setChecked(true);

    if (params.x_bins > 0)
        ui->xbinSpinBox->setValue(params.x_bins);

    if (params.y_bins > 0)
        ui->xbinSpinBox->setValue(params.y_bins);

    if (!inputManager.type_set)
        type_string = "This file does not specify the datatype. Please select the appropriate type below.";

    ui->fileTypeLabel->setText(type_string);
    QFileInfo fileInfo(QString::fromStdString(params.fileName));
    ui->fileLabel->setText("Selected file: " + fileInfo.fileName());

    // need this for filename in visualization window
    params.shortName = fileInfo.fileName().toUtf8().constData();

    ui->parameterFrame->setEnabled(raw);

    if (ui->maxDistBox->isEnabled()) {
        ui->maxDistHelp->setToolTip("Set distance to infinity");
        ui->maxDistHelp->setEnabled(true);
    }

    ui->computeButton->setEnabled(true);
    //force black text because on Mac Qt autodefault buttons have white text when enabled,
    //so they still look like they're disabled or weird in some way.
    ui->computeButton->setStyleSheet("QPushButton { color: black; }");


} //end detect_file_type()

// TODO: Use this function to make a pop up error for invalid files
void DataSelectDialog::invalid_file(const QString& message)
{
    ui->fileLabel->setText("Please select a file.");
    ui->parameterFrame->setEnabled(false);
    ui->computeButton->setEnabled(false);
    ui->fileTypeLabel->setText(nullptr);
    QMessageBox errorBox(QMessageBox::Warning, "Error", message);
    errorBox.exec();
}

void DataSelectDialog::on_filterComboBox_currentIndexChanged(int index)
{
    // make certain options available or unavailable
    // depends on what is selected for bifiltration
    if (index == 0) {
        ui->xAxisLabel->setText("degree");
        ui->xAxisLabel->setEnabled(false);
        ui->xRevCheckBox->setChecked(true);
        ui->xRevCheckBox->setEnabled(false);
        ui->functionComboBox->setEditable(true);
        ui->functionComboBox->setCurrentText("none");
        ui->functionComboBox->setEnabled(false);
        ui->parameterSpinBox->setEnabled(false);
        ui->parameterSpinBox->setSpecialValueText("N/A");
        ui->parameterSpinBox->setValue(0.00);
    }
    else {
        ui->xAxisLabel->setText(QString::fromStdString(params.x_label));
        ui->xAxisLabel->setEnabled(true);
        ui->xRevCheckBox->setChecked(false);
        ui->xRevCheckBox->setEnabled(true);
        ui->functionComboBox->setEnabled(true);
        ui->functionComboBox->setEditable(false);
        if (!params.old_function && !params.new_function && params.function_type == "none") {
            if ((ui->dataTypeComboBox->currentText().toStdString() == "points") || (ui->dataTypeComboBox->currentText().toStdString() == "metric"))
                params.function_type = "balldensity";
            else if ((ui->dataTypeComboBox->currentText().toStdString() == "points_fn") || (ui->dataTypeComboBox->currentText().toStdString() == "metric_fn"))
                params.function_type = "user";
        }
            
        ui->functionComboBox->setCurrentText(QString::fromStdString(params.function_type));
        if (params.function_type == "user" || params.function_type == "none") {
            ui->parameterSpinBox->setEnabled(false);
            ui->parameterSpinBox->setSpecialValueText("N/A");
            ui->parameterSpinBox->setValue(0.00);
            ui->xRevCheckBox->setChecked(false);
        }
        else {
            ui->xRevCheckBox->setChecked(true);
            if (params.function_type == "balldensity") {
                ui->parameterLabel->setText("Radius:");
                ui->xAxisLabel->setText("ball density");
            }
            else if (params.function_type == "gaussian") {
                ui->parameterLabel->setText("Std Dev:");
                ui->xAxisLabel->setText("gaussian density");
            }
            else if (params.function_type == "eccentricity") {
                ui->parameterLabel->setText("P Norm:");
                ui->xAxisLabel->setText("eccentricity");
            }
            ui->parameterSpinBox->setEnabled(true);
            ui->parameterSpinBox->setSpecialValueText("");
            if ((params.function_type == "gaussian" || params.function_type == "eccentricity") && params.filter_param == 0)
                ui->parameterSpinBox->setValue(1.0);
            else
                ui->parameterSpinBox->setValue(params.filter_param);
        }
    }
}

void DataSelectDialog::on_functionComboBox_currentIndexChanged(int index)
{
    if (index == 0) {
        ui->parameterLabel->setText("Parameter:");
        ui->parameterSpinBox->setEnabled(false);
        ui->parameterSpinBox->setSpecialValueText("N/A");
        ui->parameterSpinBox->setValue(0.00);
    }
    else {
        if (index == 1) {
            ui->parameterLabel->setText("Radius:");
            ui->xAxisLabel->setText("ball density");
        }
        else if (index == 2) {
            ui->parameterLabel->setText("Std Dev:");
            ui->xAxisLabel->setText("gaussian density");
        }
        else if (index == 3) {
            ui->parameterLabel->setText("P Norm:");
            ui->xAxisLabel->setText("eccentricity");
        }
        if (index == 1 || index == 2 || index == 3) {
            ui->xRevCheckBox->setChecked(true);
        }
        else
            ui->xRevCheckBox->setChecked(false);
        ui->parameterSpinBox->setEnabled(true);
        ui->parameterSpinBox->setSpecialValueText("");
        if (index == 3 && params.filter_param == 0)
            ui->parameterSpinBox->setValue(1.0);    
        else
            ui->parameterSpinBox->setValue(params.filter_param);
    }
}

void DataSelectDialog::on_dataTypeComboBox_currentIndexChanged(int index)
{
    if (index == 0 || index == 2) {
        if (ui->functionComboBox->currentText().toStdString() == "user") {
            ui->functionComboBox->setCurrentText(QString::fromStdString("balldensity"));
            ui->xRevCheckBox->setChecked(true);
        }
        qobject_cast<QStandardItemModel*>(ui->functionComboBox->model())->item(0)->setEnabled(false);
    }
    else if (index == 1 || index == 3) {
        qobject_cast<QStandardItemModel*>(ui->functionComboBox->model())->item(0)->setEnabled(true);
        ui->xRevCheckBox->setChecked(false);
    }
}
