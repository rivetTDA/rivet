#ifndef CONFIGUREDIALOG_H
#define CONFIGUREDIALOG_H

struct ConfigParameters;

#include <QDialog>
#include <QColor>


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
    void on_okButton_clicked();

    void on_xi0colorButton_clicked();
    void on_xi1colorButton_clicked();
    void on_xi2colorButton_clicked();
    void on_persistenceColorButton_clicked();
    void on_persistenceHighlightColorButton_clicked();
    void on_lineColorButton_clicked();
    void on_lineHighlightColorButton_clicked();


    void on_xi0spinBox_valueChanged(int arg1);
    void on_xi1spinBox_valueChanged(int arg1);
    void on_xi2spinBox_valueChanged(int arg1);
    void on_persistenceSpinBox_valueChanged(int arg1);
    void on_persistenceHighlightSpinBox_valueChanged(int arg1);
    void on_lineSpinBox_valueChanged(int arg1);
    void on_lineHighlightSpinBox_valueChanged(int arg1);

    void on_defaultColorsButton_clicked();

    void on_bettiDotSpinBox_valueChanged(int arg1);
    void on_persistenceDotSpinBox_valueChanged(int arg1);

    void on_defaultSizesButton_clicked();

private:
    Ui::ConfigureDialog *ui;
    ConfigParameters& config_params;

    //data structures to store selected configuration until user clicks the OK button
    QColor xi0col;
    QColor xi1col;
    QColor xi2col;
    QColor perCol;
    QColor perHiCol;
    QColor lineCol;
    QColor lineHiCol;
    int bettiRadius;
    int perRadius;
};

#endif // CONFIGUREDIALOG_H
