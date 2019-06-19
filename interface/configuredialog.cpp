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

#include "configuredialog.h"
#include "ui_configuredialog.h"

#include "config_parameters.h"

#include <QColorDialog>
#include <QDebug>

ConfigureDialog::ConfigureDialog(ConfigParameters& c_params, InputParameters& i_params, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ConfigureDialog)
    , config_params(c_params)
    , input_params(i_params)
    , xi0col(config_params.xi0color)
    , xi1col(config_params.xi1color)
    , xi2col(config_params.xi2color)
    , perCol(config_params.persistenceColor)
    , perHiCol(config_params.persistenceHighlightColor)
    , lineCol(config_params.sliceLineColor)
    , lineHiCol(config_params.sliceLineHighlightColor)
    , bettiRadius(config_params.bettiDotRadius)
    , perRadius(config_params.persistenceDotRadius)
    , autoDots(config_params.autoDotSize)
    , dgmFont(config_params.diagramFont)
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

    qss = QString("border: none; outline: none; background-color: %1;").arg(xi2col.name());
    ui->xi2colorButton->setStyleSheet(qss);
    ui->xi2colorButton->setAutoFillBackground(true);
    ui->xi2colorButton->setFlat(true);
    ui->xi2spinBox->setValue(xi2col.alpha());

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

    if (autoDots) {
        ui->AutoDotSizeCheckBox->setChecked(true);
        ui->bettiDotSpinBox->setEnabled(false);
        ui->bettiDotSizeLabel->setEnabled(false);
        ui->persistenceDotSpinBox->setEnabled(false);
        ui->persistenceDotSizeLabel->setEnabled(false);
    }

    ui->lineWidthSpinBox->setValue(config_params.sliceLineWidth);
    ui->barWidthSpinBox->setValue(config_params.persistenceBarWidth);
    ui->barSpacingSpinBox->setValue(config_params.persistenceBarSpace);

    //set labels to current labels
    ui->xaxisText->setText(config_params.xLabel);
    ui->yaxisText->setText(config_params.yLabel);

    //set font properties
    ui->fontSizeSpinBox->setValue(dgmFont.pointSize());
    ui->fontSample->setFont(dgmFont);

} //end constructor

ConfigureDialog::~ConfigureDialog()
{
    delete ui;
}

void ConfigureDialog::closeEvent(QCloseEvent* event)
{
    emit window_closed(); // tells visualization window to check for changes and redraw
    QDialog::closeEvent(event);
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
    config_params.xi2color = xi2col;
    config_params.persistenceColor = perCol;
    config_params.persistenceHighlightColor = perHiCol;
    config_params.sliceLineColor = lineCol;
    config_params.sliceLineHighlightColor = lineHiCol;
    config_params.bettiDotRadius = bettiRadius;
    config_params.persistenceDotRadius = perRadius;
    config_params.autoDotSize = autoDots;
    config_params.sliceLineWidth = ui->lineWidthSpinBox->value();
    config_params.persistenceBarWidth = ui->barWidthSpinBox->value();
    config_params.persistenceBarSpace = ui->barSpacingSpinBox->value();
    config_params.diagramFont = dgmFont;
    config_params.xLabel = ui->xaxisText->text();
    config_params.yLabel = ui->yaxisText->text();

    //close the dialog box
    close();
}

void ConfigureDialog::on_xi0colorButton_clicked()
{
    QColor color = QColorDialog::getColor(config_params.xi0color, this); //, "Choose a Color", QColorDialog::ShowAlphaChannel);

    if (color.isValid()) {
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

    if (color.isValid()) {
        xi1col = color;
        xi1col.setAlpha(ui->xi1spinBox->value());
        QString qss = QString("border: none; outline: none; background-color: %1;").arg(color.name());
        ui->xi1colorButton->setStyleSheet(qss);
        ui->xi1colorButton->setAutoFillBackground(true);
        ui->xi1colorButton->setFlat(true);
    }
}

void ConfigureDialog::on_xi2colorButton_clicked()
{
    QColor color = QColorDialog::getColor(config_params.xi2color, this);

    if (color.isValid()) {
        xi2col = color;
        xi2col.setAlpha(ui->xi2spinBox->value());
        QString qss = QString("border: none; outline: none; background-color: %1;").arg(color.name());
        ui->xi2colorButton->setStyleSheet(qss);
        ui->xi2colorButton->setAutoFillBackground(true);
        ui->xi2colorButton->setFlat(true);
    }
}

void ConfigureDialog::on_persistenceColorButton_clicked()
{
    QColor color = QColorDialog::getColor(config_params.persistenceColor, this);

    if (color.isValid()) {
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

    if (color.isValid()) {
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

    if (color.isValid()) {
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

    if (color.isValid()) {
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

void ConfigureDialog::on_xi2spinBox_valueChanged(int arg1)
{
    xi2col.setAlpha(arg1);
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

    xi2col = defaults.xi2color;
    qss = QString("border: none; outline: none; background-color: %1;").arg(xi2col.name());
    ui->xi2colorButton->setStyleSheet(qss);
    ui->xi2spinBox->setValue(xi2col.alpha());

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

void ConfigureDialog::on_AutoDotSizeCheckBox_clicked(bool checked)
{
    autoDots = checked;

    if (autoDots) {
        ui->bettiDotSpinBox->setEnabled(false);
        ui->bettiDotSizeLabel->setEnabled(false);
        ui->persistenceDotSpinBox->setEnabled(false);
        ui->persistenceDotSizeLabel->setEnabled(false);
    } else {
        ui->bettiDotSpinBox->setEnabled(true);
        ui->bettiDotSizeLabel->setEnabled(true);
        ui->persistenceDotSpinBox->setEnabled(true);
        ui->persistenceDotSizeLabel->setEnabled(true);
    }
}

void ConfigureDialog::on_fontSizeSpinBox_valueChanged(int arg1)
{
    dgmFont.setPointSize(arg1);
    ui->fontSample->setFont(dgmFont);
}
