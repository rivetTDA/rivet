/**********************************************************************
Copyright 2014-2016 The RIVET Devlopers. See the COPYRIGHT file at
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

#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>
#include <QLabel>

#include <vector>

namespace Ui {
class ProgressDialog;
}

class ProgressDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget* parent = 0);
    ~ProgressDialog();

public slots:
    void advanceToNextStage();
    void setStageMaximum(unsigned max);
    void updateProgress(unsigned current);
    void setComputationFinished();

protected:
    void closeEvent(QCloseEvent*);

signals:
    void stopComputation();

private slots:
    void on_stopButton_clicked();

private:
    Ui::ProgressDialog* ui;
    std::vector<unsigned> stage_progress;
    unsigned current_stage;
    unsigned stage_maximum;
    bool computation_finished;

    QLabel* getLabel(unsigned i);
};

#endif // PROGRESSDIALOG_H
