#ifndef PROGRESSDIALOG_H
#define PROGRESSDIALOG_H

#include <QDialog>

namespace Ui {
class ProgressDialog;
}

class ProgressDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ProgressDialog(QWidget *parent = 0);
    ~ProgressDialog();

public slots:
    void updateProgress(QString text, int percent);
    void updatePercent(int percent);

private:
    Ui::ProgressDialog *ui;
};

#endif // PROGRESSDIALOG_H
