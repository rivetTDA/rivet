#include "dataselectdialog.h"
#include "ui_dataselectdialog.h"

DataSelectDialog::DataSelectDialog(VisualizationWindow* vw, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DataSelectDialog),
    vw(vw)
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
    fileName = QFileDialog::getOpenFileName(this, tr("Open Data File"), "/ima/home/mlwright/Repos","All files (*.*);;Text files (*.txt)");
    vw->setFile(fileName);
    ui->computeButton->setEnabled(true);
}

void DataSelectDialog::on_computeButton_clicked()
{
    vw->setComputationParameters(ui->homDimSpinBox->value(), ui->xbinSpinBox->value(), ui->ybinSpinBox->value(), ui->xlabelBox->text(), ui->ylabelBox->text());
    close();
}

void DataSelectDialog::on_shortcutButton1_clicked()
{
    vw->setFile("/ima/home/mlwright/Repos/RIVET/data/sample3.txt");
    ui->computeButton->setEnabled(true);
}

void DataSelectDialog::on_shortcutButton2_clicked()
{
    vw->setFile("/ima/home/mlwright/Repos/RIVET/data/circle_data_240pts_inv_density.txt");
    ui->homDimSpinBox->setValue(1);
    ui->computeButton->setEnabled(true);
    ui->xlabelBox->setText("density");
}
