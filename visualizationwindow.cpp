#include "visualizationwindow.h"
#include "ui_visualizationwindow.h"

#include <QFileDialog>
#include <QMessageBox>

#include "interface/slicearea.h"    //DEPRECATED
#include "interface/pdarea.h"

#include "interface/input_manager.h"
#include "math/simplex_tree.h"
#include "math/multi_betti.h"
#include "dcel/mesh.h"
#include "dcel/persistence_data.hpp"


//#include "boost/date_time/posix_time/posix_time.hpp"
//using namespace boost::posix_time;


VisualizationWindow::VisualizationWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::VisualizationWindow),
    verbosity(2)
{
    ui->setupUi(this);

    sliceScene = new QGraphicsScene(this);
    ui->sliceView->setScene(sliceScene);
//    ui->sliceView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->sliceView->scale(1,-1);
    ui->sliceView->setRenderHint(QPainter::Antialiasing);

//    bigFont = new QFont();
//    bigFont->setPixelSize(70);



}

VisualizationWindow::~VisualizationWindow()
{
    delete ui;
}

void VisualizationWindow::on_angleSpinBox_valueChanged(int angle)
{
//    ui->sliceArea->setLine(angle, ui->offsetSpinBox->value());
//    draw_persistence_diagram();
}

void VisualizationWindow::on_offsetSpinBox_valueChanged(double offset)
{
//    ui->sliceArea->setLine(ui->angleSpinBox->value(), offset);
//    draw_persistence_diagram();
}

void VisualizationWindow::on_fileButton_clicked()    //let the user choose an input file
{
    fileName = QFileDialog::getOpenFileName(this, tr("Open Data File"), "/ima/home/mlwright/Repos","All files (*.*);;Text files (*.txt)");
    ui->statusBar->showMessage("file: "+fileName);
}

void VisualizationWindow::on_computeButton_clicked() //read the file and do the persistent homology computation
{
    //get dimension of homology to compute
    int dim = ui->homDimSpinBox->value();

    //start the input manager
    im = new InputManager(dim, verbosity);
//    const char* filestr = fileName.toStdString().data();
//    im->start(filestr);
    im->start("/ima/home/mlwright/Repos/RIVET/data/sample3.txt");

    //get the bifiltration
    bifiltration = im->get_bifiltration();

    //print simplex tree
    if(verbosity >= 4)
    {
        std::cout << "SIMPLEX TREE:\n";
        bifiltration->print();
    }

    //get data extents
    double data_xmin = bifiltration->grade_x_value(0);
    double data_xmax = bifiltration->grade_x_value(bifiltration->num_x_grades() - 1);
    double data_ymin = bifiltration->grade_y_value(0);
    double data_ymax = bifiltration->grade_y_value(bifiltration->num_y_grades() - 1);

    //print bifiltration statistics
    std::cout << "\nBIFILTRATION:\n";
    std::cout << "   Number of simplices of dimension " << dim << ": " << bifiltration->get_size(dim) << "\n";
    std::cout << "   Number of simplices of dimension " << (dim+1) << ": " << bifiltration->get_size(dim+1) << "\n";
    std::cout << "   Number of x-grades: " << bifiltration->num_x_grades() << "; values " << data_xmin << " to " << data_xmax << "\n";
    std::cout << "   Number of y-grades: " << bifiltration->num_y_grades() << "; values " << data_ymin << " to " << data_ymax << "\n";
    std::cout << "\n";

    //compute xi_0 and xi_1 at all multi-grades
    if(verbosity >= 2) { std::cout << "COMPUTING xi_0 AND xi_1 FOR HOMOLOGY DIMENSION " << dim << ":\n"; }
    MultiBetti mb(bifiltration, dim, verbosity);

    //start timer
//    ptime time_start(microsec_clock::local_time());

    //compute
    mb.compute_fast();

    //stop timer
//    ptime time_end(microsec_clock::local_time());
//    time_duration duration(time_end - time_start);

    //print computation time
//    std::cout << "\nxi_i computation took " << duration << ".\n";

    //initialize the slice diagram
    slice_diagram = new SliceDiagram(sliceScene, this, data_xmin, data_xmax, data_ymin, data_ymax, 360, 360);   //TODO: BOX SIZE SHOULD NOT BE HARD-CODED!!!!!


    //find all support points of xi_0 and xi_1              //TODO: THIS PART WILL BE IMPROVED WITH NEW ALGORITHM FOR FINDING LCMs
    for(int i=0; i < bifiltration->num_x_grades(); i++)
    {
        for(int j=0; j < bifiltration->num_y_grades(); j++)
        {
            if(mb.xi0(i,j) != 0 || mb.xi1(i,j) != 0)
            {
                xi_support.push_back(std::pair<int,int>(i,j));  //TODO: WHY IS THIS NECESSARY???
//                ui->sliceArea->addPoint(bifiltration->get_time(i), bifiltration->get_dist(j), mb.xi0(i,j), mb.xi1(i,j));

                slice_diagram->add_point(bifiltration->grade_x_value(i), bifiltration->grade_y_value(j), mb.xi0(i,j), mb.xi1(i,j));

            }
        }
    }

    //find default maximum coordinate for persistence diagram
//    double pd_max = sqrt( pow(bifiltration->get_time(max_time) - bifiltration->get_time(min_time), 2) + pow(bifiltration->get_dist(max_dist) - bifiltration->get_dist(min_dist), 2) );

    //print support points of xi_0 and xi_1
    if(verbosity >= 2)
    {
        std::cout << "  SUPPORT POINTS OF xi_0 AND xi_1: ";
        for(int i=0; i<xi_support.size(); i++)
        {
            std::cout << "(" << xi_support[i].first << "," << xi_support[i].second << "), ";

        }
        std::cout << "\n";
    }


//    ui->pdArea->setMax(pd_max);

    //draw slice diagram
    slice_diagram->create_diagram();

    ui->statusBar->showMessage("computed xi support points");


/*    //find LCMs, build decomposition of the affine Grassmannian
    if(verbosity >= 2) { std::cout << "CALCULATING LCMs AND DECOMPOSING THE STRIP:\n"; }
    dcel = new Mesh(verbosity);

    for(int i=0; i<xi_support.size(); i++)
    {
        for(int j=i+1; j<xi_support.size(); j++)
        {
            int ax = xi_support[i].first, ay = xi_support[i].second;
            int bx = xi_support[j].first, by = xi_support[j].second;

            if((ax - bx)*(ay - by) <= 0)	//then the support points are incomparable, so we have found an LCM
            {
                double t = bifiltration->get_time(std::max(ax,bx));
                double d = bifiltration->get_dist(std::max(ay,by));

                if(!dcel->contains(t,d))	//then this LCM has not been inserted yet
                {
                    if(verbosity >= 6) { std::cout << "  LCM at (" << std::max(ax,bx) << "," << std::max(ay,by) << ") => (" << t << "," << d << ") determined by (" << ax << "," << ay << ") and (" << bx << "," << by << ")\n"; }

                    dcel->add_curve(t,d);
                }
                else
                {
                    if(verbosity >= 6) { std::cout << "  LCM (" << t << "," << d << ") already inserted.\n"; }
                }
            }
        }
    }

    //print the dcel arrangement
    if(verbosity >= 2)
    {
        std::cout << "DCEL ARRANGEMENT:\n";
        dcel->print();
    }

    //do the persistence computations in each cell
    if(verbosity >= 2) { std::cout << "COMPUTING PERSISTENCE DATA FOR EACH CELL:\n"; }
    dcel->build_persistence_data(xi_support, bifiltration, dim);

    ui->statusBar->showMessage("computed persistence data");

    //update the control objects
    int min_offset = floor(-1*(bifiltration->get_time(max_time)));
    int max_offset = ceil(bifiltration->get_dist(max_dist));
    ui->offsetSlider->setMinimum(min_offset);
    ui->offsetSlider->setMaximum(max_offset);
    ui->offsetSpinBox->setMinimum(min_offset);
    ui->offsetSpinBox->setMaximum(max_offset);

    std::cout << "TEST1: bifiltration: " <<  bifiltration << "\n";

    //draw persistence diagram
    draw_persistence_diagram();
*/
}//end on_computeButton_clicked()

void VisualizationWindow::draw_persistence_diagram()
{
    double radians = (ui->angleSpinBox->value())*3.14159265359/180;   //convert to radians

    PersistenceDiagram* pdgm = dcel->get_persistence_diagram(radians, ui->offsetSpinBox->value(), xi_support, bifiltration);

    ui->pdArea->setData(pdgm->get_pairs(), pdgm->get_cycles());
    ui->pdArea->drawDiagram();
    ui->statusBar->showMessage("persistence diagram drawn");
}

void VisualizationWindow::on_scaleSpinBox_valueChanged(double arg1)
{
    ui->pdArea->setScale(arg1);
    ui->pdArea->drawDiagram();
    ui->statusBar->showMessage("persistence diagram scale changed");
}

void VisualizationWindow::on_fitScalePushButton_clicked()
{
    double curmax = ui->pdArea->fitScale();
    ui->scaleSpinBox->setValue(curmax);
    ui->pdArea->drawDiagram();
    ui->statusBar->showMessage("persistence diagram scale changed to fit");
}

void VisualizationWindow::on_resetScalePushButton_clicked()
{
    ui->scaleSpinBox->setValue(1);
    ui->statusBar->showMessage("persistence diagram scale reset");
}

void VisualizationWindow::set_line_parameters(double angle, double offset)
{
    ui->angleSpinBox->setValue(angle);
    ui->offsetSpinBox->setValue(offset);
}

