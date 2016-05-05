#include "driver.h"

#include "interface/file_writer.h"
#include "math/xi_point.h"

#include <QDebug>
#include <QFile>
#include <QIODevice>
#include <QTimer>

#include <vector>


Driver::Driver(InputParameters& params, QObject *parent) :
    QObject(parent),
    input_params(params),
    cthread(input_params.verbosity, input_params, x_grades, x_exact, y_grades, y_exact, xi_support, homology_dimensions)
{
    //get instance of the main application
    app = QCoreApplication::instance();

    //connect signals and slots
    QObject::connect(&cthread, &ComputationThread::arrangementReady, this, &Driver::augmented_arrangement_ready);

}//end constructor

//run this method after the application starts
void Driver::run()
{
    //do the RIVET computation
    cthread.compute();
//    cthread.wait();
}//end run()

//this slot is signaled when the agumented arrangement is ready
void Driver::augmented_arrangement_ready(Mesh* arrangement)
{
    //receive the arrangement
    this->arrangement = arrangement;

    //TESTING: print arrangement info and verify consistency
    arrangement->print_stats();
//    arrangement->test_consistency();

    //update status
    if(input_params.verbosity >= 2) { qDebug() << "COMPUTATION FINISHED."; }

    //if an output file has been specified, then save the arrangement
    if(!input_params.outputFile.empty())
    {
      auto outputFile = QString::fromStdString(input_params.outputFile);
      QFile file(outputFile);
        if(file.open(QIODevice::ReadWrite | QIODevice::Truncate))
        {
          qDebug() << "Writing file:" << outputFile;

            FileWriter fw(input_params, arrangement, x_exact, y_exact, xi_support);
            fw.write_augmented_arrangement(file);
        }
        else
        {
            qDebug() << "Error: Unable to write file:" << outputFile;
        }
        ///TODO: error handling?
    }

    //must call quit when complete
    quit();

}//end augmented_arrangement_ready()


//call this to quit the application
void Driver::quit()
{
    //do cleanup here, if necessary
    ///TODO: anything to clean up?
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
