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
    x_grades(x_grades), x_exact(x_exact), y_grades(y_grades), y_exact(y_exact),
    xi_support(xi_support),
    bifiltration(NULL),
    verbosity(verbosity)
{ }

ComputationThread::~ComputationThread()
{ }

void ComputationThread::compute()
{
    start();
}

//this function does the work
void ComputationThread::run()
{
  //STAGES 1 and 2: INPUT DATA AND CREATE BIFILTRATION

    QTime timer;    //for timing the computations

    //create the SimplexTree
    bifiltration = new SimplexTree(params.dim, verbosity);

    //get the data via the InputManager
    InputManager im(this);      //NOTE: InputManager will fill the vectors x_grades, x_exact, y_grades, and y_exact
    im.start();                 //   If the input file is raw data, then InputManager will also build the bifiltration.
                                //   If the input file is a RIVET data file, then InputManager will fill xi_support and barcode templates, but bifiltration will remain NULL.

    //print bifiltration statistics
    if(verbosity >= 2 && params.raw_data)
    {
        qDebug() << "\nBIFILTRATION:";
        qDebug() << "   Number of simplices of dimension" << params.dim << ":" << bifiltration->get_size(params.dim);
        qDebug() << "   Number of simplices of dimension" << (params.dim + 1) << ":" << bifiltration->get_size(params.dim + 1);
        qDebug() << "   Number of x-grades:" << x_grades.size() << "; values" << x_grades.front() << "to" << x_grades.back();
        qDebug() << "   Number of y-grades:" << y_grades.size() << "; values" << y_grades.front() << "to" << y_grades.back() << "\n";
    }
    if(verbosity >= 8)
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

    emit advanceProgressStage(); //update progress box to stage 3

    if(params.raw_data)    //then the user selected a raw data file, and we need to do persistence calculations
    {
      //STAGE 3: COMPUTE MULTIGRADED BETTI NUMBERS

        //compute xi_0 and xi_1 at all multi-grades
        if(verbosity >= 2) { qDebug() << "COMPUTING xi_0 AND xi_1 FOR HOMOLOGY DIMENSION " << params.dim << ":"; }
        MultiBetti mb(bifiltration, params.dim, verbosity);

        timer.start();
        mb.compute_fast(this);
        qDebug() << "  --> xi_i computation took" << timer.elapsed() << "milliseconds";

        //store the xi support points
        mb.store_support_points(xi_support);

        emit xiSupportReady();          //signal that xi support points are ready for visualization
        emit advanceProgressStage();    //update progress box to stage 4


      //STAGES 4 and 5: BUILD THE LINE ARRANGEMENT AND COMPUTE BARCODE TEMPLATES

        //build the arrangement
        if(verbosity >= 2) { qDebug() << "CALCULATING ANCHORS AND BUILDING THE DCEL ARRANGEMENT"; }

        timer.start();
        arrangement = new Mesh(x_grades, x_exact, y_grades, y_exact, verbosity);    //NOTE: delete later!
        arrangement->build_arrangement(mb, xi_support, this);     ///TODO: update this -- does not need to store list of xi support points in xi_support
        //NOTE: this also computes and stores barcode templates in the arrangement

        qDebug() << "   building the line arrangement and computing all barcode templates took" << timer.elapsed() << "milliseconds";

        //send (a pointer to) the arrangement back to the VisualizationWindow
        emit arrangementReady(arrangement);

        //delete the SimplexTree
        delete bifiltration;
    }
    else    //then the user selected a RIVET file with pre-computed persistence information, and we just need to re-build the arrangement
    {
      //STAGE 3: MULTIGRADED BETTI NUMBERS ALREADY COMPUTED

        if(verbosity >= 2) { qDebug() << "INPUT FINISHED: xi support points ready"; }
        emit xiSupportReady();          //signal that xi support points are ready for visualization
        emit advanceProgressStage();    //update progress box to stage 4


      //STAGES 4 and 5: RE-BUILD THE AUGMENTED ARRANGEMENT

        if(verbosity >= 2) { qDebug() << "RE-BUILDING THE AUGMENTED ARRANGEMENT"; }

        timer.start();

        arrangement = new Mesh(x_grades, x_exact, y_grades, y_exact, verbosity);    //NOTE: delete later!
        arrangement->build_arrangement(xi_support, barcode_templates, this);

        qDebug() << "   re-building the augmented arrangement took" << timer.elapsed() << "milliseconds";

        //send (a pointer to) the arrangement back to the VisualizationWindow
        emit arrangementReady(arrangement);
    }
}//end run()
