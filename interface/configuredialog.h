#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

#include <QDialog>
#include <QColor>
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

signals:
    void configuration_changed();

private slots:
    void on_cancelButton_clicked();
    void on_okButton_clicked();

    void on_xi0colorButton_clicked();
    void on_xi1colorButton_clicked();
    void on_persistenceColorButton_clicked();
    void on_persistenceHighlightColorButton_clicked();
    void on_lineColorButton_clicked();
    void on_lineHighlightColorButton_clicked();


private:
    Ui::ConfigureDialog *ui;
    ConfigParameters& config_params;

    //data structures to store selected configuration until user clicks the OK button
    QColor xi0col;
    QColor xi1col;
    QColor perCol;
    QColor perHiCol;
    QColor lineCol;
    QColor lineHiCol;
};

#endif // CONFIGUREDIALOG_H
