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

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QProcess>
#include <QSettings>
#include <QStringList>
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
        ui->homDimSpinBox->setValue(params.dim);
        ui->xbinSpinBox->setValue(params.x_bins);
        ui->ybinSpinBox->setValue(params.y_bins);
    } else {
        ui->homDimSpinBox->setValue(0);
        ui->xbinSpinBox->setValue(10);
        ui->ybinSpinBox->setValue(10);
    }


}

DataSelectDialog::~DataSelectDialog()
{
    delete ui;
}

void DataSelectDialog::closeEvent(QCloseEvent* event)
{
    event->accept();

    if (!data_selected)
        qobject_cast<QWidget*>(this->parent())->close();
}

void DataSelectDialog::on_computeButton_clicked()
{
    params.dim = ui->homDimSpinBox->value();
    params.x_bins = ui->xbinSpinBox->value();
    params.y_bins = ui->ybinSpinBox->value();
    

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

void DataSelectDialog::detect_file_type()
{
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

    auto line = reader.next_line().first;
    if (line[0] == "RIVET_1") {
        //TODO: It would be nice to support RIVET_1 in the console like other file types
        // rather than having this special case here
        ui->fileTypeLabel->setText("This file appears to contain pre-computed RIVET data");
        QFileInfo fileInfo(QString::fromStdString(params.fileName));
        ui->fileLabel->setText("Selected file: " + fileInfo.fileName());
        //TODO: this updating of the params will need to happen in console also, need to refactor
        params.shortName = fileInfo.fileName().toUtf8().constData();
        ui->parameterFrame->setEnabled(false);
    } else {
        QStringList args;
        args.append(QString::fromStdString(params.fileName));
        args.append("--identify");
        auto console = RivetConsoleApp::start(args);

        if (!console->waitForStarted()) {
            invalid_file(RivetConsoleApp::errorMessage(console->error()));
            return;
        }

        bool raw = false;
        bool function=true;
        QString partial("");
        auto error_header_len = QString("INPUT ERROR: ").length();
        auto error_footer_len = QString(" :END").length();
        qDebug() << "Reading from console";
        while (console->canReadLine() || console->waitForReadyRead()) {
            QString line = console->readLine();
            line = line.trimmed();
            if (line.length() == 0)
                continue;
            qDebug() << line;
            if (line.startsWith("RAW DATA: ")) {
                raw = line.contains("1");
            } else if (line.startsWith("INPUT ERROR: ")) {
                if (line.endsWith(":END")) {
                    line = line.mid(error_header_len, line.length() - (error_footer_len + error_header_len));
                    invalid_file(line);
                    break;
                } else {
                    qDebug() << "Partial:" << line;
                    partial = line;
                }
            } else if (line.startsWith("FILE TYPE DESCRIPTION: ")) {

                ui->fileTypeLabel->setText("This file appears to contain " + line.mid(QString("FILE TYPE DESCRIPTION: ").length()).trimmed() + ".");
                QFileInfo fileInfo(QString::fromStdString(params.fileName));
                ui->fileLabel->setText("Selected file: " + fileInfo.fileName());

                //TODO: this updating of the params will need to happen in console also, need to refactor
                QString file_des=line.mid(QString("FILE TYPE DESCRIPTION: ").length()).trimmed();
                
                //TODO: this updating of the params will need to happen in console also, need to refactor

                
                
                params.shortName = fileInfo.fileName().toUtf8().constData();

                //firep data does not have a homology dimension
                if(file_des=="free implicit representation data"){

                    ui->homDimSpinBox->setSpecialValueText("N/A");
                    //the spinbox will show the special value text when the value is the minimum value (i.e. zero)

                    ui->homDimSpinBox->setValue(0);
                    ui->homDimSpinBox->setEnabled(false);
                }
                else if(!ui->homDimSpinBox->isEnabled()){
                    //if an firep file was previously selected, and the new file is not an firep

                    ui->homDimSpinBox->setSpecialValueText("");
                    //this turns off the special value text (i.e. zero is displayed like normal)

                    ui->homDimSpinBox->setEnabled(true);
                    ui->homDimSpinBox->setValue(0);
                }

            }
            
           
            else if (partial.length() != 0) {
                if (line.endsWith(":END")) {
                    line = partial + line;
                    line = line.mid(error_header_len, line.length() - (error_footer_len + error_header_len));
                    invalid_file(line);
                    break;
                } else {
                    partial += line;
                }
            }
        }
        ui->parameterFrame->setEnabled(raw);
    }
    
    ui->computeButton->setEnabled(true);
    //force black text because on Mac Qt autodefault buttons have white text when enabled,
    //so they still look like they're disabled or weird in some way.
    ui->computeButton->setStyleSheet("QPushButton { color: black; }");

} //end detect_file_type()

void DataSelectDialog::invalid_file(const QString& message)
{
    ui->fileLabel->setText("Please select a file.");
    ui->parameterFrame->setEnabled(false);
    ui->computeButton->setEnabled(false);
    ui->fileTypeLabel->setText(nullptr);
    QMessageBox errorBox(QMessageBox::Warning, "Error", message);
    errorBox.exec();
}
