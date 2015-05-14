#ifndef DATASELECTDIALOG_H
#define DATASELECTDIALOG_H

#include <QDialog>
#include <QFileDialog>

#include "interface/input_parameters.h"

namespace Ui {
class DataSelectDialog;
}

class DataSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DataSelectDialog(InputParameters& params, QWidget *parent = 0);
    ~DataSelectDialog();

private slots:
    void on_rawDataFileButton_clicked();

    void on_computeButton_clicked();

    void on_shortcutButton1_clicked();

    void on_shortcutButton2_clicked();

private:
    Ui::DataSelectDialog *ui;

    InputParameters& params;

    QString fileName;   //name of data file

};

#endif // DATASELECTDIALOG_H
