#ifndef COMPUTATIONTHREAD_H
#define COMPUTATIONTHREAD_H

//forward declarations
struct InputParameters;
class Mesh;

#include "math/xi_point.h"

#include <QObject>
#include <QThread>

#include <boost/multiprecision/cpp_int.hpp>
typedef boost::multiprecision::cpp_rational exact;

#include <vector>


class ComputationThread : public QThread
{
    Q_OBJECT

    public:
        ComputationThread(int verbosity, InputParameters& params, std::vector<double>& x_grades, std::vector<exact>& x_exact, std::vector<double>& y_grades, std::vector<exact>& y_exact, std::vector<xiPoint>& xi_support, QObject *parent = 0);
        ~ComputationThread();

        void compute();

    signals:
        void advanceProgressStage();
        void setProgressMaximum(unsigned max);
        void setCurrentProgress(unsigned current);
        void xiSupportReady();
        void arrangementReady(Mesh* arrangement);

    protected:
        void run() Q_DECL_OVERRIDE;

    private:
        InputParameters& params;

        std::vector<double>& x_grades;
        std::vector<exact>& x_exact;
        std::vector<double>& y_grades;
        std::vector<exact>& y_exact;

        std::vector<xiPoint>& xi_support;

        Mesh* arrangement;      //TODO: should this be a pointer?

        const int verbosity;
};

#endif // COMPUTATIONTHREAD_H
