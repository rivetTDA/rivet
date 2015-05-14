#ifndef COMPUTATIONTHREAD_H
#define COMPUTATIONTHREAD_H

#include <QObject>
#include <QThread>

#include "interface/input_parameters.h"
#include "math/xi_point.h"

#include <boost/multiprecision/cpp_int.hpp>
typedef boost::multiprecision::cpp_rational exact;


class ComputationThread : public QThread
{
    Q_OBJECT

    public:
        ComputationThread(int verbosity, QObject *parent = 0);
        ~ComputationThread();

        void compute(InputParameters& p, std::vector<double>& xg, std::vector<double>& yg, std::vector<xiPoint>& xi);

    protected:
        void run() Q_DECL_OVERRIDE;

    private:
        InputParameters& params;
        std::vector<double>& x_grades;
        std::vector<double>& y_grades;
        std::vector<xiPoint>& xi_support;


        const int verbosity;
};

#endif // COMPUTATIONTHREAD_H
