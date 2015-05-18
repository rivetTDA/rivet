#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

#include <QDialog>

namespace Ui {
class ConfigureDialog;
}

class ConfigureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigureDialog(QWidget *parent = 0);
    ~ConfigureDialog();

private slots:
    void on_cancelButton_clicked();

    void on_testButton_clicked();

private:
    Ui::ConfigureDialog *ui;
};

#endif // CONFIGUREDIALOG_H
