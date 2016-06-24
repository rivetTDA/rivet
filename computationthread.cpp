#include "computationthread.h"

#include "dcel/mesh.h"
#include "interface/input_manager.h"
#include "interface/input_parameters.h"
#include "math/multi_betti.h"
#include "math/simplex_tree.h"
#include "math/xi_point.h"

#include <QDebug>
#include <QTime>
#include <QProcess>

ComputationThread::ComputationThread(InputParameters& params, QObject *parent) :
    QThread(parent),
    params(params)
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
    QProcess console;
    console.setProcessChannelModel(QProcess::MergedChannels);
    console.start("rivet_console",
                  QStringList() << params.fileName
                                << params.outputFile
                                << "-H" << params.dim
                                << "-x" << params.x_bins
                                << "-y" << params.y_bins
                                << "-V" << params.verbosity);

    if (!console.waitForStarted()) {
        QDebug() << "Error launching rivet_console";
        return;
    }

    console.waitForReadyRead();
    while(QString line = console.readLine() != nullptr) {
        QDebug() << "console: " << line;
        if (line.startsWith("PROGRESS ")) {
            QDebug() << "***Progress is: " << line;
        } else if (line.startsWith("STAGE")) {
            emit advanceProgressStage();
        } else if (line.startsWith("XI")) {
            mesh << console;
            //TODO: load xi support
            emit xiSupportReady();
        } else if (line.startsWith("ARRANGEMENT")) {
            //TODO: load arrangement
            arrangement << console;
            emit arrangementReady(arrangement);
        }
    }

    if (!console.waitForFinished()) {
        QDebug() << "Error waiting for rivet_console to finish";
        return;
    }
}//end run()

