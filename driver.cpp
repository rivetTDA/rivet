#include "driver.h"

#include "computationthread.h"
#include "math/xi_point.h"

#include <QDebug>
#include <QTimer>

#include <vector>


Driver::Driver(InputParameters& params, QObject *parent) :
    QObject(parent),
    input_params(params)
{
    //get instance of the main application
    app = QCoreApplication::instance();

}//end constructor

//run this method after the application starts
void Driver::run()
{
    //data structures for computation
    std::vector<double> x_grades;     //floating-point x-coordinates of the grades, sorted exactly
    std::vector<exact> x_exact;       //exact (e.g. rational) values of all x-grades, sorted
    std::vector<double> y_grades;     //floating-point y-coordinates of the grades
    std::vector<exact> y_exact;       //exact (e.g. rational) values of all y-grades, sorted
    std::vector<xiPoint> xi_support;  //stores discrete coordinates of xi support points, with multiplicities
    unsigned_matrix homology_dimensions;       //stores the dimension of homology at each grade

    //now do the RIVET computation
    ComputationThread cthread(input_params.verbosity, input_params, x_grades, x_exact, y_grades, y_exact, xi_support, homology_dimensions);
    cthread.compute();
    cthread.wait();

    //must call quit when complete
    quit();
}//end run()

//call this to quit the application
void Driver::quit()
{
    //do cleanup here, if necessary

    emit finished();
}

//shortly after quit is called, the CoreApplication will signal this routine
//this is a good place to delete any objects that were created in the constructor and stop any threads
void Driver::aboutToQuitApp()
{
    //stop threads
    //sleep(1); //wait for threads to stop
    //delete any objects
}
