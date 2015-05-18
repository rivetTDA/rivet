#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

#include <QDialog>

#include "config_parameters.h"

namespace Ui {
class ConfigureDialog;
}

class ConfigureDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ConfigureDialog(ConfigParameters& params, QWidget *parent = 0);
    ~ConfigureDialog();

private slots:
    void on_cancelButton_clicked();

    void on_xi0colorButton_clicked();

    void on_xi1colorButton_clicked();

    void on_persistenceColorButton_clicked();

    void on_persistenceHighlightColorButton_clicked();

    void on_lineColorButton_clicked();

private:
    Ui::ConfigureDialog *ui;
    ConfigParameters& config_params;
};

#endif // CONFIGUREDIALOG_H
