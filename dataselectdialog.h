#ifndef DATASELECTDIALOG_H
#define DATASELECTDIALOG_H

#include <QDialog>
#include <QFileDialog>

class VisualizationWindow;
#include "visualizationwindow.h"

namespace Ui {
class DataSelectDialog;
}

class DataSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit DataSelectDialog(VisualizationWindow* vw, QWidget *parent = 0);
    ~DataSelectDialog();

private slots:
    void on_rawDataFileButton_clicked();

    void on_computeButton_clicked();

    void on_shortcutButton1_clicked();

    void on_shortcutButton2_clicked();

private:
    Ui::DataSelectDialog *ui;
    VisualizationWindow* vw;

    QString fileName;   //name of data file

};

#endif // DATASELECTDIALOG_H
