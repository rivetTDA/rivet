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
