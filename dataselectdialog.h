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

#ifndef DATASELECTDIALOG_H
#define DATASELECTDIALOG_H

//forward declarations
struct InputParameters;

#include <QDialog>
#include <QFile>
#include <QString>
#include <QtWidgets>

#include <map>

namespace Ui {
class DataSelectDialog;
}

class DataSelectDialog : public QDialog {
    Q_OBJECT

public:
    explicit DataSelectDialog(InputParameters& params, QWidget* parent = 0);
    ~DataSelectDialog();

signals:
    void dataSelected();

protected:
    void closeEvent(QCloseEvent* event);
    void showEvent(QShowEvent* event);

private slots:
    void on_computeButton_clicked();
    void on_openFileButton_clicked();
    void on_maxDistHelp_clicked();

    void on_filterComboBox_currentIndexChanged(int index);

    void on_functionComboBox_currentIndexChanged(int index);

private:
    Ui::DataSelectDialog* ui;

    InputParameters& params;

    void invalid_file(const QString& message);
    void detect_file_type();

    void parse_args();

    int to_skip;

    void parse_points_old();
    void parse_metric_old();
    void parse_bifiltration_old();
    void parse_firep_old();

    bool data_selected; //false until user clicks "compute" button
};

#endif // DATASELECTDIALOG_H
