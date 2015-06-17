#include "configuredialog.h"
#include "ui_configuredialog.h"

#include "config_parameters.h"

#include <QColorDialog>
#include <QDebug>


ConfigureDialog::ConfigureDialog(ConfigParameters& params, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigureDialog),
    config_params(params),
    xi0col(config_params.xi0color), xi1col(config_params.xi1color),
    perCol(config_params.persistenceColor), perHiCol(config_params.persistenceHighlightColor),
    lineCol(config_params.sliceLineColor), lineHiCol(config_params.sliceLineHighlightColor),
    bettiRadius(config_params.bettiDotRadius), perRadius(config_params.persistenceDotRadius)
{
    ui->setupUi(this);

  //set color selectors to current colors
    QString qss = QString("border: none; outline: none; background-color: %1;").arg(xi0col.name());
    ui->xi0colorButton->setStyleSheet(qss);
    ui->xi0colorButton->setAutoFillBackground(true);
    ui->xi0colorButton->setFlat(true);
    ui->xi0spinBox->setValue(xi0col.alpha());

    qss = QString("border: none; outline: none; background-color: %1;").arg(xi1col.name());
    ui->xi1colorButton->setStyleSheet(qss);
    ui->xi1colorButton->setAutoFillBackground(true);
    ui->xi1colorButton->setFlat(true);
    ui->xi1spinBox->setValue(xi1col.alpha());

    qss = QString("border: none; outline: none; background-color: %1;").arg(perCol.name());
    ui->persistenceColorButton->setStyleSheet(qss);
    ui->persistenceColorButton->setAutoFillBackground(true);
    ui->persistenceColorButton->setFlat(true);
    ui->persistenceSpinBox->setValue(perCol.alpha());

    qss = QString("border: none; outline: none; background-color: %1;").arg(perHiCol.name());
    ui->persistenceHighlightColorButton->setStyleSheet(qss);
    ui->persistenceHighlightColorButton->setAutoFillBackground(true);
    ui->persistenceHighlightColorButton->setFlat(true);
    ui->persistenceHighlightSpinBox->setValue(perHiCol.alpha());

    qss = QString("border: none; outline: none; background-color: %1;").arg(lineCol.name());
    ui->lineColorButton->setStyleSheet(qss);
    ui->lineColorButton->setAutoFillBackground(true);
    ui->lineColorButton->setFlat(true);
    ui->lineSpinBox->setValue(lineCol.alpha());

    qss = QString("border: none; outline: none; background-color: %1;").arg(lineHiCol.name());
    ui->lineHighlightColorButton->setStyleSheet(qss);
    ui->lineHighlightColorButton->setAutoFillBackground(true);
    ui->lineHighlightColorButton->setFlat(true);
    ui->lineHighlightSpinBox->setValue(lineHiCol.alpha());

  //set size selectors to current sizes
    ui->bettiDotSpinBox->setValue(bettiRadius);
    ui->persistenceDotSpinBox->setValue(perRadius);

}//end constructor

ConfigureDialog::~ConfigureDialog()
{
    delete ui;
}

void ConfigureDialog::on_cancelButton_clicked()
{
    close();
}

void ConfigureDialog::on_okButton_clicked()
{
    //update config_params to the selected parameters
    config_params.xi0color = xi0col;
    config_params.xi1color = xi1col;
    config_params.persistenceColor = perCol;
    config_params.persistenceHighlightColor = perHiCol;
    config_params.sliceLineColor = lineCol;
    config_params.sliceLineHighlightColor = lineHiCol;
    config_params.bettiDotRadius = bettiRadius;
    config_params.persistenceDotRadius = perRadius;

    //close the dialog box
    close();
}

void ConfigureDialog::on_xi0colorButton_clicked()
{
    QColor color = QColorDialog::getColor(config_params.xi0color, this); //, "Choose a Color", QColorDialog::ShowAlphaChannel);

    if(color.isValid())
    {
        xi0col = color;
        xi0col.setAlpha(ui->xi0spinBox->value());
        QString qss = QString("border: none; outline: none; background-color: %1;").arg(color.name());
        ui->xi0colorButton->setStyleSheet(qss);
        ui->xi0colorButton->setAutoFillBackground(true);
        ui->xi0colorButton->setFlat(true);
    }
}

void ConfigureDialog::on_xi1colorButton_clicked()
{
    QColor color = QColorDialog::getColor(config_params.xi1color, this);

    if(color.isValid())
    {
        xi1col = color;
        xi1col.setAlpha(ui->xi1spinBox->value());
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
        perCol.setAlpha(ui->persistenceSpinBox->value());
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
        perHiCol.setAlpha(ui->persistenceHighlightSpinBox->value());
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
        lineCol.setAlpha(ui->lineSpinBox->value());
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
        lineHiCol.setAlpha(ui->lineHighlightSpinBox->value());
        QString qss = QString("border: none; outline: none; background-color: %1;").arg(color.name());
        ui->lineHighlightColorButton->setStyleSheet(qss);
        ui->lineHighlightColorButton->setAutoFillBackground(true);
        ui->lineHighlightColorButton->setFlat(true);
    }
}


void ConfigureDialog::on_xi0spinBox_valueChanged(int arg1)
{
    xi0col.setAlpha(arg1);
}

void ConfigureDialog::on_xi1spinBox_valueChanged(int arg1)
{
    xi1col.setAlpha(arg1);
}

void ConfigureDialog::on_persistenceSpinBox_valueChanged(int arg1)
{
    perCol.setAlpha(arg1);
}

void ConfigureDialog::on_persistenceHighlightSpinBox_valueChanged(int arg1)
{
    perHiCol.setAlpha(arg1);
}

void ConfigureDialog::on_lineSpinBox_valueChanged(int arg1)
{
    lineCol.setAlpha(arg1);
}

void ConfigureDialog::on_lineHighlightSpinBox_valueChanged(int arg1)
{
    lineHiCol.setAlpha(arg1);
}

void ConfigureDialog::on_defaultColorsButton_clicked()
{
    ConfigParameters defaults;

    xi0col = defaults.xi0color;
    QString qss = QString("border: none; outline: none; background-color: %1;").arg(xi0col.name());
    ui->xi0colorButton->setStyleSheet(qss);
    ui->xi0spinBox->setValue(xi0col.alpha());

    xi1col = defaults.xi1color;
    qss = QString("border: none; outline: none; background-color: %1;").arg(xi1col.name());
    ui->xi1colorButton->setStyleSheet(qss);
    ui->xi1spinBox->setValue(xi1col.alpha());

    perCol = defaults.persistenceColor;
    qss = QString("border: none; outline: none; background-color: %1;").arg(perCol.name());
    ui->persistenceColorButton->setStyleSheet(qss);
    ui->persistenceSpinBox->setValue(perCol.alpha());

    perHiCol = defaults.persistenceHighlightColor;
    qss = QString("border: none; outline: none; background-color: %1;").arg(perHiCol.name());
    ui->persistenceHighlightColorButton->setStyleSheet(qss);
    ui->persistenceHighlightSpinBox->setValue(perHiCol.alpha());

    lineCol = defaults.sliceLineColor;
    qss = QString("border: none; outline: none; background-color: %1;").arg(lineCol.name());
    ui->lineColorButton->setStyleSheet(qss);
    ui->lineSpinBox->setValue(lineCol.alpha());

    lineHiCol = defaults.sliceLineHighlightColor;
    qss = QString("border: none; outline: none; background-color: %1;").arg(lineHiCol.name());
    ui->lineHighlightColorButton->setStyleSheet(qss);
    ui->lineHighlightSpinBox->setValue(lineHiCol.alpha());
}

void ConfigureDialog::on_bettiDotSpinBox_valueChanged(int arg1)
{
    bettiRadius = arg1;
}

void ConfigureDialog::on_persistenceDotSpinBox_valueChanged(int arg1)
{
    perRadius = arg1;
}

void ConfigureDialog::on_defaultSizesButton_clicked()
{
    ConfigParameters defaults;

    bettiRadius = defaults.bettiDotRadius;
    ui->bettiDotSpinBox->setValue(bettiRadius);

    perRadius = defaults.persistenceDotRadius;
    ui->persistenceDotSpinBox->setValue(perRadius);
}
