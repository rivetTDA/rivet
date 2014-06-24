#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class InputManager;
class Mesh;
class SimplexTree;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_angleSlider_valueChanged(int angle);

    void on_angleSpinBox_valueChanged(int angle);

    void on_fileButton_clicked();

    void on_computeButton_clicked();

    void on_offsetSlider_valueChanged(int value);

    void on_offsetSpinBox_valueChanged(double arg1);

private:
    Ui::MainWindow *ui;

    const int verbosity;

    InputManager* im;

    QString fileName;   //name of data file

    SimplexTree* bifiltration;  //bifiltration constructed from the data

    std::vector<std::pair<int, int> > xi_support;  //integer (relative) coordinates of xi support points

    Mesh* dcel; //pointer to the DCEL arrangement

    void draw_persistence_diagram();

};

#endif // MAINWINDOW_H
