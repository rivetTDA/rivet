#ifndef DRIVER_H
#define DRIVER_H

#include "computationthread.h"
#include "interface/input_parameters.h"
#include "dcel/mesh.h"

#include <QCoreApplication>
#include <QObject>
#include <QString>

class Driver : public QObject
{
    Q_OBJECT

    public:
        explicit Driver(InputParameters& params, QObject* parent = 0);

        void quit();    //call this to quit the application

    signals:
        void finished();    //signal to finish, connected to Application Quit

    public slots:
        void run();     //gets called from main to start everything
        void augmented_arrangement_ready(Mesh* arrangement);  //gets signaled when the agumented arrangement is ready
        void aboutToQuitApp();  //gets signal when application is about to quit

    private:
        QCoreApplication *app;

        InputParameters& input_params;
        Mesh* arrangement;

        //data structures for computation
        std::vector<double> x_grades;     //floating-point x-coordinates of the grades, sorted exactly
        std::vector<exact> x_exact;       //exact (e.g. rational) values of all x-grades, sorted
        std::vector<double> y_grades;     //floating-point y-coordinates of the grades
        std::vector<exact> y_exact;       //exact (e.g. rational) values of all y-grades, sorted
        std::vector<xiPoint> xi_support;  //stores discrete coordinates of xi support points, with multiplicities
        unsigned_matrix homology_dimensions;       //stores the dimension of homology at each grade

        //computation thread
        ComputationThread cthread;
};

#endif // DRIVER_H
