#include "configuredialog.h"
#include "ui_configuredialog.h"

#include <QColorDialog>
#include <QDebug>
#include <QPalette>

ConfigureDialog::ConfigureDialog(ConfigParameters& params, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigureDialog),
    config_params(params)
{
    ui->setupUi(this);

    QString qss = QString("border: none; outline: none; background-color: %1;").arg(config_params.xi0color.name());
    ui->xi0colorButton->setStyleSheet(qss);
    ui->xi0colorButton->setAutoFillBackground(true);
    ui->xi0colorButton->setFlat(true);

    qss = QString("border: none; outline: none; background-color: %1;").arg(config_params.xi1color.name());
    ui->xi1colorButton->setStyleSheet(qss);
    ui->xi1colorButton->setAutoFillBackground(true);
    ui->xi1colorButton->setFlat(true);

    qss = QString("border: none; outline: none; background-color: %1;").arg(config_params.persistenceColor.name());
    ui->persistenceColorButton->setStyleSheet(qss);
    ui->persistenceColorButton->setAutoFillBackground(true);
    ui->persistenceColorButton->setFlat(true);

    qss = QString("border: none; outline: none; background-color: %1;").arg(config_params.persistenceHighlightColor.name());
    ui->persistenceHighlightColorButton->setStyleSheet(qss);
    ui->persistenceHighlightColorButton->setAutoFillBackground(true);
    ui->persistenceHighlightColorButton->setFlat(true);

    qss = QString("border: none; outline: none; background-color: %1;").arg(config_params.sliceLineColor.name());
    ui->lineColorButton->setStyleSheet(qss);
    ui->lineColorButton->setAutoFillBackground(true);
    ui->lineColorButton->setFlat(true);

    ///TODO: slice line highlight color!
}

ConfigureDialog::~ConfigureDialog()
{
    delete ui;
}

void ConfigureDialog::on_cancelButton_clicked()
{
    close();
}

void ConfigureDialog::on_xi0colorButton_clicked()
{
    QColor color = QColorDialog::getColor(Qt::blue, this); //, "Choose a Color", QColorDialog::ShowAlphaChannel);

    if(color.isValid())
    {
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
    QColor color = QColorDialog::getColor(Qt::blue, this);

    if(color.isValid())
    {
        QString qss = QString("border: none; outline: none; background-color: %1;").arg(color.name());
        ui->xi1colorButton->setStyleSheet(qss);
        ui->xi1colorButton->setAutoFillBackground(true);
        ui->xi1colorButton->setFlat(true);
    }
}

void ConfigureDialog::on_persistenceColorButton_clicked()
{
    QColor color = QColorDialog::getColor(Qt::blue, this);

    if(color.isValid())
    {
        QString qss = QString("border: none; outline: none; background-color: %1;").arg(color.name());
        ui->persistenceColorButton->setStyleSheet(qss);
        ui->persistenceColorButton->setAutoFillBackground(true);
        ui->persistenceColorButton->setFlat(true);
    }
}

void ConfigureDialog::on_persistenceHighlightColorButton_clicked()
{
    QColor color = QColorDialog::getColor(Qt::blue, this);

    if(color.isValid())
    {
        QString qss = QString("border: none; outline: none; background-color: %1;").arg(color.name());
        ui->persistenceHighlightColorButton->setStyleSheet(qss);
        ui->persistenceHighlightColorButton->setAutoFillBackground(true);
        ui->persistenceHighlightColorButton->setFlat(true);
    }
}

void ConfigureDialog::on_lineColorButton_clicked()
{
    QColor color = QColorDialog::getColor(Qt::blue, this);

    if(color.isValid())
    {
        QString qss = QString("border: none; outline: none; background-color: %1;").arg(color.name());
        ui->lineColorButton->setStyleSheet(qss);
        ui->lineColorButton->setAutoFillBackground(true);
        ui->lineColorButton->setFlat(true);
    }
}
