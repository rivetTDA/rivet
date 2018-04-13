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

#include "progressdialog.h"
#include "ui_progressdialog.h"

#include <QCloseEvent>
#include <QDebug>
#include <QMessageBox>
#include <QtWidgets>

ProgressDialog::ProgressDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ProgressDialog)
    , current_stage(1)
    , stage_maximum(100)
    , computation_finished(false)
{
    ui->setupUi(this);

    stage_progress.push_back(0);
    stage_progress.push_back(5); //we'll say that when the file is read we are 5% done,
    stage_progress.push_back(15); // and when the bifiltration is built we are 15% done,
    stage_progress.push_back(65); // and when the Betti numbers are computed we are 65% done
    stage_progress.push_back(80); // and when the line arrangement is built we are 80% done
    stage_progress.push_back(100); // and when the barcode templates are computed we are 100% done
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::advanceToNextStage()
{
    QLabel* prevLabel = getLabel(current_stage);
    QFont font = prevLabel->font();
    font.setBold(false);
    prevLabel->setFont(font);

    current_stage++;
    
    //TODO: Why is this here?  --Mike.
    //stage_maximum = 100;

    QLabel* nextLabel = getLabel(current_stage);
    
    font.setBold(true);
    nextLabel->setFont(font);
    nextLabel->setEnabled(true);
    
    ui->progressBar->setValue(stage_progress[current_stage-1]);
}

void ProgressDialog::setStageMaximum(unsigned max)
{
    stage_maximum = max;
}

void ProgressDialog::updateProgress(unsigned current)
{
    double stage_percent = ((double)current) / stage_maximum;
    int value = (int)(stage_percent * (stage_progress[current_stage] - stage_progress[current_stage - 1]) + stage_progress[current_stage - 1]);
    ui->progressBar->setValue(value);
}

void ProgressDialog::setComputationFinished()
{
    computation_finished = true;
    QThread::msleep(200); //seems to prevent ProgressDialog from sticking around after it is supposed to have been closed
    done(0);
}

void ProgressDialog::closeEvent(QCloseEvent* event)
{
    if (!computation_finished)
        event->ignore();
    else {
        event->accept();
        QWidget::closeEvent(event);
    }
}

QLabel* ProgressDialog::getLabel(unsigned i)
{
    if (i == 1)
        return ui->step1description;
    else if (i == 2)
        return ui->step2description;
    else if (i == 3)
        return ui->step3description;
    else if (i == 4)
        return ui->step4description;
    else
        return ui->step5description;
}

void ProgressDialog::on_stopButton_clicked()
{
    QMessageBox::StandardButton reallyStop;
    reallyStop = QMessageBox::question(this, "Stop computation?", "Are you sure you want to stop the computation?", QMessageBox::Yes | QMessageBox::No);

    if (reallyStop == QMessageBox::Yes) {
        emit stopComputation();
        computation_finished = true;
        close();
        qDebug() << "COMPUTATION INTERRUPTED BY USER";
        qobject_cast<QWidget*>(this->parent())->close();
    }
}
