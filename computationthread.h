#ifndef COMPUTATIONTHREAD_H
#define COMPUTATIONTHREAD_H

//forward declarations
class InputManager;
struct InputParameters;
class Mesh;

#include "dcel/barcode_template.h"
#include "math/simplex_tree.h"
#include "math/xi_point.h"

#include <QObject>
#include <QThread>

#include <boost/multiprecision/cpp_int.hpp>
typedef boost::multiprecision::cpp_rational exact;
#include "boost/multi_array.hpp"
typedef boost::multi_array<unsigned, 2> unsigned_matrix;

#include <vector>


class ComputationThread : public QThread
{
    Q_OBJECT

    friend class InputManager;  //so that we don't have to pass all of the data structures from ComputationThread to InputManager

    public:
        ComputationThread(InputParameters& params,
                          QObject *parent = 0);
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

        std::vector<xiPoint> xi_support;
        std::shared_ptr<Mesh> arrangement;


};

#endif // COMPUTATIONTHREAD_H
