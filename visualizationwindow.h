#ifndef VISUALIZATIONWINDOW_H
#define VISUALIZATIONWINDOW_H

#include <QMainWindow>
#include <QtGui>

#include "dcel/mesh.h"

#include <boost/multiprecision/cpp_int.hpp>
typedef boost::multiprecision::cpp_rational exact;

#include "interface/slice_diagram.h"
class SliceDiagram;

#include "interface/persistence_diagram.h"
class PersistenceDiagram;


namespace Ui {
class VisualizationWindow;
}

class InputManager;
class Mesh;
class SimplexTree;

class VisualizationWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit VisualizationWindow(QWidget *parent = 0);
    ~VisualizationWindow();

    void setFile(QString name); //sets the name of the data file
    void setComputationParameters(int hom_dim, unsigned num_x_bins, unsigned num_y_bins, QString x_text, QString y_text);    //sets parameters for the computation
    void compute(); //begins the computation pipeline

    void set_line_parameters(double angle, double offset);

    void select_bar(unsigned index);
    void deselect_bar();
    void select_dot(unsigned index);
    void deselect_dot();

    void resizeEvent(QResizeEvent* event);
    
private slots:
    void on_angleDoubleSpinBox_valueChanged(double angle);

    void on_offsetSpinBox_valueChanged(double arg1);

    void on_xi0CheckBox_toggled(bool checked);

    void on_xi1CheckBox_toggled(bool checked);

    void on_normCoordCheckBox_clicked(bool checked);

    void on_barcodeCheckBox_clicked(bool checked);

private:
    Ui::VisualizationWindow *ui;

    //computational items
    const int verbosity;

    InputManager* im;

    QString fileName;   //name of data file
    int dim;            //dimension of homology to compute
    unsigned x_bins;    //number of bins for x-coordinate (if 0, then bins are not used for x)
    unsigned y_bins;    //number of bins for y-coordinate (if 0, then bins are not used for y)
    QString x_label;    //label for x-axis of slice diagram
    QString y_label;    //label for y-axis of slice_diagram

    std::vector<double> x_grades;
    std::vector<double> y_grades;
    SimplexTree* bifiltration;  //discrete bifiltration

    std::vector<xiPoint> xi_support;  //stores discrete coordinates of xi support points, with multiplicities

    Mesh* arrangement; //pointer to the DCEL arrangement

    //items for slice diagram
    QGraphicsScene* sliceScene;
    SliceDiagram* slice_diagram;
    bool slice_update_lock;

    //items for persistence diagram
    QGraphicsScene* pdScene;
    PersistenceDiagram* p_diagram;
    bool persistence_diagram_drawn;

    void update_persistence_diagram();

};

#endif // VISUALIZATIONWINDOW_H
