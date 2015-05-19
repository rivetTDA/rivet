#include "dataselectdialog.h"
#include "ui_dataselectdialog.h"

#include "interface/input_parameters.h"

#include <QFileDialog>


DataSelectDialog::DataSelectDialog(InputParameters& params, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DataSelectDialog),
    params(params)
{
    ui->setupUi(this);

    ui->xbinSpinBox->setSpecialValueText(tr("No bins"));
    ui->ybinSpinBox->setSpecialValueText(tr("No bins"));
}

DataSelectDialog::~DataSelectDialog()
{
    delete ui;
}

void DataSelectDialog::on_rawDataFileButton_clicked()
{
    params.fileName = QFileDialog::getOpenFileName(this, tr("Open Data File"), "/ima/home/mlwright/Repos","All files (*.*);;Text files (*.txt)");
    ui->computeButton->setEnabled(true);
}

void DataSelectDialog::on_computeButton_clicked()
{
    params.dim = ui->homDimSpinBox->value();
    params.x_bins = ui->xbinSpinBox->value();
    params.y_bins = ui->ybinSpinBox->value();
    params.x_label = ui->xlabelBox->text();
    params.y_label = ui->ylabelBox->text();
    close();
}


//THE FOLLOWING ARE FOR TESTING ONLY
void DataSelectDialog::on_shortcutButton1_clicked()
{
    params.fileName = "/ima/home/mlwright/Repos/RIVET/data/sample3.txt";
    ui->computeButton->setEnabled(true);
}

void DataSelectDialog::on_shortcutButton2_clicked()
{
    params.fileName = "/ima/home/mlwright/Repos/RIVET/data/circle_data_240pts_inv_density.txt";
    ui->homDimSpinBox->setValue(1);
    ui->computeButton->setEnabled(true);
    ui->xlabelBox->setText("codensity");
}
