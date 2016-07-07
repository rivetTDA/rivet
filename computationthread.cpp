#include "computationthread.h"

#define CEREAL_SERIALIZE_FUNCTION_NAME cerealize

#include "dcel/mesh.h"
#include "interface/input_manager.h"
#include "interface/input_parameters.h"
#include "math/multi_betti.h"
#include "math/simplex_tree.h"
#include "math/xi_point.h"
#include "dcel/serialization.h"
#include <cereal/archives/json.hpp>
#include <cereal/types/memory.hpp>
#include <QDebug>
#include <QTime>
#include <QProcess>
#include <QString>


ComputationThread::ComputationThread(InputParameters& params, QObject *parent) :
    QThread(parent),
    params(params),
    xi_support()
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
    console.setProcessChannelMode(QProcess::MergedChannels);
    console.start("rivet_console",
                  QStringList() << QString::fromStdString(params.fileName)
                                << QString::fromStdString(params.outputFile)
                                << "-H" << QString::number(params.dim)
                                << "-x" << QString::number(params.x_bins)
                                << "-y" << QString::number(params.y_bins)
                                << "-V" << QString::number(params.verbosity));

    if (!console.waitForStarted()) {
        qDebug() << "Error launching rivet_console:" << RivetConsoleApp::errorMessage(console.error()) ;
        return;
    }

    console.waitForReadyRead();
    while(console.canReadLine()) {
        QString line = console.readLine();
        qDebug() << "console: " << line;
        if (line.startsWith("PROGRESS ")) {
            qDebug() << "***Progress is: " << line;
        } else if (line.startsWith("STAGE")) {
            emit advanceProgressStage();
        } else if (line.startsWith("XI")) {
            //TODO: load xi support
            emit xiSupportReady();
        } else if (line.startsWith("ARRANGEMENT")) {
            //TODO: load arrangement
            std::stringstream ss;
            while(console.canReadLine()) {
                QString s = console.readLine();
                if (s.startsWith("END ARRANGEMENT")) {
                    cereal::JSONInputArchive archive(ss);
                    archive(arrangement);
                    emit arrangementReady(&*arrangement);
                    break;
                }
            }
        }
    }

    if (!console.waitForFinished()) {
        qDebug() << "Error waiting for rivet_console to finish";
        return;
    }
}//end run()

