#include "computationthread.h"

#include "dcel/mesh.h"
#include "interface/input_manager.h"
#include "interface/input_parameters.h"
#include "math/multi_betti.h"
#include "math/simplex_tree.h"
#include "math/xi_point.h"

#include <QDebug>
#include <QTime>


ComputationThread::ComputationThread(int verbosity, InputParameters& params, std::vector<double>& x_grades, std::vector<exact>& x_exact, std::vector<double>& y_grades, std::vector<exact>& y_exact, std::vector<xiPoint>& xi_support, QObject *parent) :
    QThread(parent),
    params(params),
    x_grades(x_grades), x_exact(x_exact), y_grades(y_grades), y_exact(y_exact), xi_support(xi_support),
    verbosity(verbosity)
{ }

ComputationThread::~ComputationThread()
{ }

void ComputationThread::compute()
{
    //do I need to check anything here???

    start();
}

//this function does the work
void ComputationThread::run()
{
  //STAGES 1 and 2: INPUT DATA AND CREATE BIFILTRATION

    //local data elements
    SimplexTree bifiltration(params.dim, verbosity);
    QTime timer;    //for timing the computations

    //get the data via the InputManager
    InputManager im(params.dim, x_grades, x_exact, y_grades, y_exact, bifiltration, verbosity);     //NOTE: InputManager will fill the vectors x_grades, x_exact, y_grades, and y_exact, and also build the bifiltration
    std::string filestr = params.fileName.toStdString();
    im.start(filestr, params.x_bins, params.y_bins, this);

    //print bifiltration statistics
    if(verbosity >= 2)
    {
        qDebug() << "\nBIFILTRATION:";
        qDebug() << "   Number of simplices of dimension" << params.dim << ":" << bifiltration.get_size(params.dim);
        qDebug() << "   Number of simplices of dimension" << (params.dim + 1) << ":" << bifiltration.get_size(params.dim + 1);
        qDebug() << "   Number of x-grades:" << x_grades.size() << "; values" << x_grades.front() << "to" << x_grades.back();
        qDebug() << "   Number of y-grades:" << y_grades.size() << "; values" << y_grades.front() << "to" << y_grades.back() << "\n";
    }
    if(verbosity >= 10)
    {
      qDebug() << "x-grades:";
      for(unsigned i=0; i<x_grades.size(); i++)
      {
          std::ostringstream oss;
          oss << x_exact[i];
          qDebug() << "  " << x_grades[i] << "=" << oss.str().data();
      }
      qDebug() << "y-grades:";
      for(unsigned i=0; i<y_grades.size(); i++)
      {
          std::ostringstream oss;
          oss << y_exact[i];
          qDebug() << "  " << y_grades[i] << "=" << oss.str().data();
      }
    }


  //STAGE 3: COMPUTE MULTIGRADED BETTI NUMBERS

    emit advanceProgressStage(); //update progress box to stage 3

    //compute xi_0 and xi_1 at all multi-grades
    if(verbosity >= 2) { qDebug() << "COMPUTING xi_0 AND xi_1 FOR HOMOLOGY DIMENSION " << params.dim << ":"; }
    MultiBetti mb(bifiltration, params.dim, verbosity);

    timer.start();
    mb.compute_fast(this);
    qDebug() << "  --> xi_i computation took" << timer.elapsed() << "milliseconds";

    //store the xi support points
    mb.store_support_points(xi_support);

    //signal that xi support points are ready for visualization
    emit xiSupportReady();


  //STAGES 4 and 5: BUILD THE LINE ARRANGEMENT AND COMPUTE BARCODE TEMPLATES

    emit advanceProgressStage(); //update progress box to stage 4

    //build the arrangement
    if(verbosity >= 2) { qDebug() << "CALCULATING ANCHORS AND BUILDING THE DCEL ARRANGEMENT"; }

    timer.start();
    arrangement = new Mesh(x_grades, x_exact, y_grades, y_exact, verbosity);    //NOTE: delete later!
    arrangement->build_arrangement(mb, xi_support, this);     //also stores list of xi support points in the last argument
      //NOTE: this also computes and stores barcode templates in the arrangement

    qDebug() << "   building the line arrangement and computing all barcode templates took" << timer.elapsed() << "milliseconds";

    //send (a pointer to) the arrangement back to the VisualizationWindow
    emit arrangementReady(arrangement);

}//end run()
