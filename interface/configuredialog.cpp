#include "configuredialog.h"
#include "ui_configuredialog.h"

#include <QColorDialog>
#include <QDebug>
#include <QPalette>

ConfigureDialog::ConfigureDialog(ConfigParameters& params, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigureDialog),
    config_params(params),
    xi0col(config_params.xi0color), xi1col(config_params.xi1color),
    perCol(config_params.persistenceColor), perHiCol(config_params.persistenceHighlightColor),
    lineCol(config_params.sliceLineColor), lineHiCol(config_params.sliceLineHighlightColor)
{
    ui->setupUi(this);

    QString qss = QString("border: none; outline: none; background-color: %1;").arg(xi0col.name());
    ui->xi0colorButton->setStyleSheet(qss);
    ui->xi0colorButton->setAutoFillBackground(true);
    ui->xi0colorButton->setFlat(true);

    qss = QString("border: none; outline: none; background-color: %1;").arg(xi1col.name());
    ui->xi1colorButton->setStyleSheet(qss);
    ui->xi1colorButton->setAutoFillBackground(true);
    ui->xi1colorButton->setFlat(true);

    qss = QString("border: none; outline: none; background-color: %1;").arg(perCol.name());
    ui->persistenceColorButton->setStyleSheet(qss);
    ui->persistenceColorButton->setAutoFillBackground(true);
    ui->persistenceColorButton->setFlat(true);

    qss = QString("border: none; outline: none; background-color: %1;").arg(perHiCol.name());
    ui->persistenceHighlightColorButton->setStyleSheet(qss);
    ui->persistenceHighlightColorButton->setAutoFillBackground(true);
    ui->persistenceHighlightColorButton->setFlat(true);

    qss = QString("border: none; outline: none; background-color: %1;").arg(lineCol.name());
    ui->lineColorButton->setStyleSheet(qss);
    ui->lineColorButton->setAutoFillBackground(true);
    ui->lineColorButton->setFlat(true);

    qss = QString("border: none; outline: none; background-color: %1;").arg(lineHiCol.name());
    ui->lineHighlightColorButton->setStyleSheet(qss);
    ui->lineHighlightColorButton->setAutoFillBackground(true);
    ui->lineHighlightColorButton->setFlat(true);
}

ConfigureDialog::~ConfigureDialog()
{
    qDebug() << "deleting ConfigureDialog";
    delete ui;
}

void ConfigureDialog::on_cancelButton_clicked()
{
    close();
}

void ConfigureDialog::on_okButton_clicked()
{
    //save selected configuration
    config_params.xi0color.setRgb(xi0col.red(), xi0col.green(), xi0col.blue(), config_params.xi0color.alpha());
    config_params.xi1color.setRgb(xi1col.red(), xi1col.green(), xi1col.blue(), config_params.xi1color.alpha());
    config_params.persistenceColor.setRgb(perCol.red(), perCol.green(), perCol.blue(), config_params.persistenceColor.alpha());
    config_params.persistenceHighlightColor.setRgb(perHiCol.red(), perHiCol.green(), perHiCol.blue(), config_params.persistenceHighlightColor.alpha());
    config_params.sliceLineColor.setRgb(lineCol.red(), lineCol.green(), lineCol.blue(), config_params.sliceLineColor.alpha());
    config_params.sliceLineHighlightColor.setRgb(lineHiCol.red(), lineHiCol.green(), lineHiCol.blue(), config_params.sliceLineHighlightColor.alpha());

    //redraw the diagrams
    emit configuration_changed();
    ///TODO: FINISH THIS
    ///   also think about how to best set opacity of colors

    //close the dialog box
    close();
}

void ConfigureDialog::on_xi0colorButton_clicked()
{
    QColor color = QColorDialog::getColor(config_params.xi0color, this); //, "Choose a Color", QColorDialog::ShowAlphaChannel);

    if(color.isValid())
    {
        xi0col = color;
        QString qss = QString("border: none; outline: none; background-color: %1;").arg(color.name());
        ui->xi0colorButton->setStyleSheet(qss);
        ui->xi0colorButton->setAutoFillBackground(true);
        ui->xi0colorButton->setFlat(true);
    }

//    QPalette pal = ui->xi0_color_widget->palette();
//    pal.setColor(QPalette::Window, color);
//    ui->xi0_color_widget->setPalette(pal);

}

void ConfigureDialog::on_xi1colorButton_clicked()
{
    QColor color = QColorDialog::getColor(config_params.xi1color, this);

    if(color.isValid())
    {
        xi1col = color;
        QString qss = QString("border: none; outline: none; background-color: %1;").arg(color.name());
        ui->xi1colorButton->setStyleSheet(qss);
        ui->xi1colorButton->setAutoFillBackground(true);
        ui->xi1colorButton->setFlat(true);
    }
}

void ConfigureDialog::on_persistenceColorButton_clicked()
{
    QColor color = QColorDialog::getColor(config_params.persistenceColor, this);

    if(color.isValid())
    {
        perCol = color;
        QString qss = QString("border: none; outline: none; background-color: %1;").arg(color.name());
        ui->persistenceColorButton->setStyleSheet(qss);
        ui->persistenceColorButton->setAutoFillBackground(true);
        ui->persistenceColorButton->setFlat(true);
    }
}

void ConfigureDialog::on_persistenceHighlightColorButton_clicked()
{
    QColor color = QColorDialog::getColor(config_params.persistenceHighlightColor, this);

    if(color.isValid())
    {
        perHiCol = color;
        QString qss = QString("border: none; outline: none; background-color: %1;").arg(color.name());
        ui->persistenceHighlightColorButton->setStyleSheet(qss);
        ui->persistenceHighlightColorButton->setAutoFillBackground(true);
        ui->persistenceHighlightColorButton->setFlat(true);
    }
}

void ConfigureDialog::on_lineColorButton_clicked()
{
    QColor color = QColorDialog::getColor(config_params.sliceLineColor, this);

    if(color.isValid())
    {
        lineCol = color;
        QString qss = QString("border: none; outline: none; background-color: %1;").arg(color.name());
        ui->lineColorButton->setStyleSheet(qss);
        ui->lineColorButton->setAutoFillBackground(true);
        ui->lineColorButton->setFlat(true);
    }
}

void ConfigureDialog::on_lineHighlightColorButton_clicked()
{
    QColor color = QColorDialog::getColor(config_params.sliceLineHighlightColor, this);

    if(color.isValid())
    {
        lineHiCol = color;
        QString qss = QString("border: none; outline: none; background-color: %1;").arg(color.name());
        ui->lineHighlightColorButton->setStyleSheet(qss);
        ui->lineHighlightColorButton->setAutoFillBackground(true);
        ui->lineHighlightColorButton->setFlat(true);
    }
}

