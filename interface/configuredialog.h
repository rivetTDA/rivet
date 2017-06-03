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

#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

struct ConfigParameters;

#include "input_parameters.h"

#include <QColor>
#include <QDialog>
#include <QFont>
#include <QString>

namespace Ui {
class ConfigureDialog;
}

class ConfigureDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConfigureDialog(ConfigParameters& c_params, InputParameters& i_params, QWidget* parent = 0);
    ~ConfigureDialog();

private slots:
    void on_cancelButton_clicked();
    void on_okButton_clicked();

    void on_xi0colorButton_clicked();
    void on_xi1colorButton_clicked();
    void on_xi2colorButton_clicked();
    void on_persistenceColorButton_clicked();
    void on_persistenceHighlightColorButton_clicked();
    void on_lineColorButton_clicked();
    void on_lineHighlightColorButton_clicked();

    void on_xi0spinBox_valueChanged(int arg1);
    void on_xi1spinBox_valueChanged(int arg1);
    void on_xi2spinBox_valueChanged(int arg1);
    void on_persistenceSpinBox_valueChanged(int arg1);
    void on_persistenceHighlightSpinBox_valueChanged(int arg1);
    void on_lineSpinBox_valueChanged(int arg1);
    void on_lineHighlightSpinBox_valueChanged(int arg1);

    void on_defaultColorsButton_clicked();

    void on_bettiDotSpinBox_valueChanged(int arg1);
    void on_persistenceDotSpinBox_valueChanged(int arg1);

    void on_xaxisText_editingFinished();

    void on_yaxisText_editingFinished();

    void on_AutoDotSizeCheckBox_clicked(bool checked);

    void on_fontSizeSpinBox_valueChanged(int arg1);

private:
    Ui::ConfigureDialog* ui;
    ConfigParameters& config_params;
    InputParameters& input_params;

    //data structures to store selected configuration until user clicks the OK button
    QColor xi0col;
    QColor xi1col;
    QColor xi2col;
    QColor perCol;
    QColor perHiCol;
    QColor lineCol;
    QColor lineHiCol;
    int bettiRadius;
    int perRadius;
    bool autoDots;
    QString xlabel;
    QString ylabel;
    QFont dgmFont;
};

#endif // CONFIGUREDIALOG_H
