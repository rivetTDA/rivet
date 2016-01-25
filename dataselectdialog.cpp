#include "dataselectdialog.h"
#include "ui_dataselectdialog.h"

#include "interface/file_input_reader.h"
#include "interface/input_parameters.h"

#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>


DataSelectDialog::DataSelectDialog(InputParameters& params, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DataSelectDialog),
    params(params)
{
    ui->setupUi(this);

    ui->xbinSpinBox->setSpecialValueText(tr("No bins"));
    ui->ybinSpinBox->setSpecialValueText(tr("No bins"));

    //set initial values
    if(!params.fileName.isEmpty())
        detect_file_type();
    ui->homDimSpinBox->setValue(params.dim);
    ui->xbinSpinBox->setValue(params.x_bins);
    ui->ybinSpinBox->setValue(params.y_bins);
}

DataSelectDialog::~DataSelectDialog()
{
    delete ui;
}

void DataSelectDialog::closeEvent(QCloseEvent* event)
{
    event->accept();
}

void DataSelectDialog::on_computeButton_clicked()
{
    params.dim = ui->homDimSpinBox->value();
    params.x_bins = ui->xbinSpinBox->value();
    params.y_bins = ui->ybinSpinBox->value();

    emit dataSelected();

    close();
}

void DataSelectDialog::on_openFileButton_clicked()
{
    //prompt user to select a file
    QString selected_file = QFileDialog::getOpenFileName(this, tr("Open Data File"), "/ima/home/mlwright/Repos","All files (*.*);;Text files (*.txt)");

    if(!selected_file.isNull())
    {
        params.fileName = selected_file;
        detect_file_type();
    }
}//end on_openFileButton_clicked()

void DataSelectDialog::detect_file_type()
{
    QFile infile(params.fileName);
    if(infile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        FileInputReader reader(infile);

        //attempt to determine the file type
        QString filetype = reader.next_line().first(); ///TODO: error handling?

        if(filetype == QString("points"))
        {
            ui->fileTypeLabel->setText("This file appears to contain point-cloud data.");
            raw_data_file_selected(infile);
        }
        else if(filetype == QString("metric"))
        {
            ui->fileTypeLabel->setText("This file appears to contain metric data.");
            raw_data_file_selected(infile);
        }
        else if(filetype == QString("bifiltration"))
        {
            ui->fileTypeLabel->setText("This file appears to contain bifiltration data.");
            raw_data_file_selected(infile);
        }
        else if(filetype == QString("RIVET_0"))
        {
            ui->fileTypeLabel->setText("This file appears to contain pre-computed RIVET data.");
            QFileInfo fileInfo(infile);
            params.shortName = fileInfo.fileName();
            ui->fileLabel->setText("Selected file: " + params.shortName);
            params.raw_data = false;
            ui->parameterFrame->setEnabled(false);
            ui->computeButton->setEnabled(true);
        }
        else    //unrecognized file type
        {
            ui->fileTypeLabel->setText("File type not recognized.");
            ui->parameterFrame->setEnabled(false);
            ui->computeButton->setEnabled(false);
        }
    }
    else    //error: unable to read file
    {
        ui->fileTypeLabel->setText("Unable to read file.");
        ui->parameterFrame->setEnabled(false);
        ui->computeButton->setEnabled(false);
    }
}//end detect_file_type()

void DataSelectDialog::raw_data_file_selected(const QFile& file)
{
    //display file name
    QFileInfo fileInfo(file);
    params.shortName = fileInfo.fileName();
    ui->fileLabel->setText("Selected file: " + params.shortName);

    //save file parameters
    params.raw_data = true;

    //activate parameter selection items
    ui->parameterFrame->setEnabled(true);
    ui->computeButton->setEnabled(true);
}

