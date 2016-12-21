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

#ifndef ABOUTMESSAGEBOX_H
#define ABOUTMESSAGEBOX_H

#include <QDialog>

namespace Ui {
class AboutMessageBox;
}

class AboutMessageBox : public QDialog {
    Q_OBJECT

public:
    explicit AboutMessageBox(QWidget* parent = 0);
    ~AboutMessageBox();

private slots:
    void on_pushButton_clicked();

private:
    Ui::AboutMessageBox* ui;
};

#endif // ABOUTMESSAGEBOX_H
