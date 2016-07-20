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
#include "dcel/mesh_message.h"


class ComputationThread : public QThread
{
    Q_OBJECT

    friend class InputManager;  //so that we don't have to pass all of the data structures from ComputationThread to InputManager

    public:
        ComputationThread(InputParameters& params,
                          QObject *parent = 0);
        ~ComputationThread();

        void compute();

    //TODO: these really ought to be delivered via signal rather than read by other classes
        std::vector<xiPoint> xi_support;
        std::vector<exact> x_exact;
        std::vector<exact> y_exact;
        unsigned_matrix hom_dims;
    QString x_label;
    QString y_label;

    signals:
        void advanceProgressStage();
        void setProgressMaximum(unsigned max);
        void setCurrentProgress(unsigned current);
        void xiSupportReady();
        void arrangementReady(MeshMessage* arrangement);

    protected:
        void run() Q_DECL_OVERRIDE;

    private:
        InputParameters& params;

        std::shared_ptr<MeshMessage> arrangement;


};

#endif // COMPUTATIONTHREAD_H
