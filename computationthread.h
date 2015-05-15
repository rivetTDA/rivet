#ifndef COMPUTATIONTHREAD_H
#define COMPUTATIONTHREAD_H

#include <QObject>
#include <QThread>

#include "interface/input_parameters.h"
#include "math/xi_point.h"
#include "dcel/mesh.h"
class Mesh;

#include <boost/multiprecision/cpp_int.hpp>
typedef boost::multiprecision::cpp_rational exact;


class ComputationThread : public QThread
{
    Q_OBJECT

    public:
        ComputationThread(int verbosity, InputParameters& params, std::vector<double>& x_grades, std::vector<double>& y_grades, std::vector<xiPoint>& xi_support, QObject *parent = 0);
        ~ComputationThread();

        void compute();

    signals:
        void sendProgressUpdate(QString text, int percent);
        void sendProgressPercent(int percent);
        void xiSupportReady();
        void arrangementReady(Mesh* arrangement);

    protected:
        void run() Q_DECL_OVERRIDE;

    private:
        InputParameters& params;
        std::vector<double>& x_grades;
        std::vector<double>& y_grades;
        std::vector<xiPoint>& xi_support;

        Mesh* arrangement;      //TODO: should this be a pointer?

        const int verbosity;
};

#endif // COMPUTATIONTHREAD_H
