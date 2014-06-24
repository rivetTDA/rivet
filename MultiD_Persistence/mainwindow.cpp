#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>

#include "slicearea.h"
#include "pdarea.h"

#include "../input_manager.h"
#include "../simplex_tree.h"
#include "../multi_betti.h"
#include "../dcel/mesh.h"
#include "../dcel/persistence_data.hpp"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    verbosity(6)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_angleSlider_valueChanged(int angle)
{
    ui->angleSpinBox->setValue(angle);
}

void MainWindow::on_angleSpinBox_valueChanged(int angle)
{
    ui->angleSlider->setValue(angle);
    ui->sliceArea->setLine(angle, ui->offsetSpinBox->value());
    draw_persistence_diagram();
}

void MainWindow::on_offsetSlider_valueChanged(int offset)
{
    ui->offsetSpinBox->setValue(offset);
}

void MainWindow::on_offsetSpinBox_valueChanged(double offset)
{
//    ui->offsetSlider->setValue(offset);
    ui->sliceArea->setLine(ui->angleSpinBox->value(), offset);
    draw_persistence_diagram();
}

void MainWindow::on_fileButton_clicked()    //let the user choose an input file
{
    fileName = QFileDialog::getOpenFileName(this, tr("Open Data File"), "/ima/home/mlwright/Repos","All files (*.*);;Text files (*.txt)");
    ui->statusBar->showMessage("file: "+fileName);
}

void MainWindow::on_computeButton_clicked() //read the file and do the persistent homology computation
{
    //start the input manager
    im = new InputManager(verbosity);
    const char* filestr = fileName.toStdString().data();
    im->start(filestr);

    //get the bifiltration
//    SimplexTree* bifiltration = im.get_bifiltration();
    bifiltration = im->get_bifiltration();

    //print simplex tree
    if(verbosity >= 4)
    {
        std::cout << "SIMPLEX TREE:\n";
        bifiltration->print();
    }

    //compute xi_0 and xi_1 at ALL multi-indexes
    int dim = ui->homDimSpinBox->value();
    if(verbosity >= 2) { std::cout << "COMPUTING xi_0 AND xi_1 FOR HOMOLOGY DIMENSION " << dim << ":\n"; }
    MultiBetti mb(bifiltration, dim, verbosity);
    mb.compute_all_xi();

    //print xi_0 and xi_1
    if(verbosity >= 4)
    {
        std::cout << "COMPUTATION FINISHED:\n";

        //build column labels for output
        std::string col_labels = "        dist ";
        std::string hline = "    --------";
        for(int j=0; j<(*bifiltration).get_num_dists(); j++)
        {
            std::ostringstream oss;
            oss << j;
            col_labels += oss.str() + "  ";
            hline += "---";
        }
        col_labels += "\n" + hline + "\n";

        //output xi_0
        std::cout << "  VALUES OF xi_0 for dimension " << dim << ":\n";
        std::cout << col_labels;
        for(int i=0; i<(*bifiltration).get_num_times(); i++)
        {
            std::cout << "    time " << i << " | ";
            for(int j=0; j<(*bifiltration).get_num_dists(); j++)
            {
                std::cout << mb.xi0(i,j) << "  ";
            }
            std::cout << "\n";
        }

        //output xi_1
        std::cout << "\n  VALUES OF xi_1 for dimension " << dim << ":\n";
        std::cout << col_labels;
        for(int i=0; i<(*bifiltration).get_num_times(); i++)
        {
            std::cout << "    time " << i << " | ";
            for(int j=0; j<(*bifiltration).get_num_dists(); j++)
            {
                std::cout << mb.xi1(i,j) << "  ";
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }

    //find all support points of xi_0 and xi_1
//    std::vector<std::pair<int, int> > xi_support;  //integer (relative) coordinates of xi support points
    int min_time = bifiltration->get_num_times();  //min (relative) time coordinate
    int max_time = 0;                              //max (relative) time coordinate
    int min_dist = bifiltration->get_num_dists();  //min (relative) distance coordinate
    int max_dist = 0;                              //max (relative) distance coordinate

    for(int i=0; i < bifiltration->get_num_times(); i++)
    {
        for(int j=0; j < bifiltration->get_num_dists(); j++)
        {
            if(mb.xi0(i,j) != 0 || mb.xi1(i,j) != 0)
            {
                xi_support.push_back(std::pair<int,int>(i,j));
                ui->sliceArea->addPoint(bifiltration->get_time(i), bifiltration->get_dist(j), mb.xi0(i,j), mb.xi1(i,j));

                if(i < min_time)
                    min_time = i;
                if(i > max_time)
                    max_time = i;
                if(j < min_dist)
                    min_dist = j;
                if(j > max_dist)
                    max_dist = j;
            }
        }
    }

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

    //draw support points of xi_0 and xi_1
    ui->sliceArea->setExtents(bifiltration->get_time(min_time), bifiltration->get_time(max_time), bifiltration->get_dist(min_dist), bifiltration->get_dist(max_dist));
    ui->sliceArea->update();
    ui->statusBar->showMessage("computed xi support points");


    //find LCMs, build decomposition of the affine Grassmannian
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


}//end on_computeButton_clicked()

void MainWindow::draw_persistence_diagram()
{
    double radians = (ui->angleSpinBox->value())*3.14159265359/180;   //convert to radians

    PersistenceDiagram* pdgm = dcel->get_persistence_diagram(radians, ui->offsetSpinBox->value(), xi_support, bifiltration);

    ui->pdArea->setData(pdgm->get_pairs(), pdgm->get_cycles());
    ui->pdArea->drawDiagram();
    ui->statusBar->showMessage("persistence diagram drawn");
}
