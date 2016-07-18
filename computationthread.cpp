#include "computationthread.h"

#include "dcel/mesh.h"
#include "interface/input_manager.h"
#include "interface/input_parameters.h"
#include "math/multi_betti.h"
#include "math/simplex_tree.h"
#include "math/xi_point.h"
#include "dcel/serialization.h"
#include "dcel/mesh_message.h"

#include "interface/console_interaction.h"
#include "base_64.h"

#include <boost/archive/tmpdir.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <fstream>
#include <vector>
#include <QDebug>
#include <QTime>
#include <QProcess>
#include <QString>


ComputationThread::ComputationThread(InputParameters& params, QObject *parent) :
    QThread(parent),
    params(params),
    xi_support(),
    x_exact(),
    y_exact()
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
    QStringList args;

    args << QString::fromStdString(params.fileName)
    << QString::fromStdString(params.outputFile)
    << "-H" << QString::number(params.dim)
    << "-x" << QString::number(params.x_bins)
    << "-y" << QString::number(params.y_bins)
    << "-V" << QString::number(params.verbosity);
    auto console = RivetConsoleApp::start(args);

    if (!console->waitForStarted()) {
        qDebug() << "Error launching rivet_console:" << RivetConsoleApp::errorMessage(console->error()) ;
        return;
    }

    bool reading_xi = false;
    std::stringstream ss;
    while(console->canReadLine() || console->waitForReadyRead()) {
        QString line = console->readLine();
        qDebug().noquote() << "console: " << line;
        if (reading_xi) {
            if (line.startsWith("END XI")) {
//                    cereal::JSONInputArchive archive(ss);
//                    cereal::BinaryInputArchive archive(ss);
                XiSupportMessage message;
                {
//                    cereal::XMLInputArchive archive(ss);
                    boost::archive::text_iarchive archive(ss);
                    archive >> message;
                }
                    xi_support = message.xi_support;
                    std::vector<unsigned> dims(message.homology_dimensions.shape(),
                                                          message.homology_dimensions.shape() + message.homology_dimensions.num_dimensions());
                    assert(dims.size() == 2);
                    hom_dims.resize(boost::extents[dims[0]][dims[1]]);
                    hom_dims = message.homology_dimensions;
                qDebug() << "Received hom_dims: " << hom_dims.shape()[0] << " x " << hom_dims.shape()[1];
                    x_exact = message.x_exact;
                    y_exact = message.y_exact;
                reading_xi = false;
                emit xiSupportReady();
            } else {
                ss << line.toStdString();
            }
        } else if (line.startsWith("ARRANGEMENT: ")) {
                {
                    std::ifstream input(line.mid(QString("ARRANGEMENT: ").length()).trimmed().toStdString());
                    //TODO: better error handling on both sides
                    if (!input.is_open()) {
                        throw std::runtime_error("Could not open console arrangement file");
                    }
                    boost::archive::text_iarchive archive(input);
                    arrangement.reset(new MeshMessage());
                    archive >> *arrangement;
                }
//                qDebug() << "Mesh received: " << arrangement->x_exact.size() << " x " << arrangement->y_exact.size();
                emit arrangementReady(&*arrangement);
                console->waitForFinished();
                return;
        } else if (line.startsWith("PROGRESS ")) {
            auto progress = line.mid(QString("PROGRESS ").length()).trimmed();
            qDebug() << "***Progress is: " << progress;
            setCurrentProgress(progress.toInt());
        } else if (line.startsWith("STAGE")) {
            emit advanceProgressStage();
        } else if (line.startsWith("XI")) {
            reading_xi = true;
            ss.clear();
        }
    }

    qDebug() << "Mesh was not delivered";

}//end run()

