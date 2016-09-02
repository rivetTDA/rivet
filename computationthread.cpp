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
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <QDebug>
#include <QTime>
#include <QProcess>
#include <QString>
#include <QDir>

ComputationThread::ComputationThread(InputParameters& params, QObject *parent) :
    QThread(parent),
    params(params),
    xi_support(),
    x_exact(),
    y_exact(),
    x_label(),
    y_label()
{ }

ComputationThread::~ComputationThread()
{ }

void ComputationThread::compute()
{
    start();
}

//this function does the work
void ComputationThread::run() {
    if (is_precomputed(params.fileName)) {
        load_from_file();
    } else {
        compute_from_file();
    }
}

bool ComputationThread::is_precomputed(std::string file_name) {
    std::ifstream file(file_name);
    if (!file.is_open()) {
        throw std::runtime_error("Couldn't open " + file_name + " for reading");
    }
    std::string line;
    std::getline(file, line);
    return line == "RIVET_1";
}


void ComputationThread::load_from_file() {
    std::ifstream file(params.fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Couldn't open " + params.fileName + " for reading");
    }
    std::string type;
    std::getline(file, type);
    assert(type == "RIVET_1");
    boost::archive::binary_iarchive archive(file);

    arrangement.reset(new MeshMessage());
    archive >> params;
    archive >> message;
    unpack_message_fields();
    emit xiSupportReady();
    archive >> *arrangement;
    emit arrangementReady(&*arrangement);
}

//TODO: Probably better to not have to unpack all these properties into fields of computationthread.
void ComputationThread::unpack_message_fields() {

    xi_support = message.xi_support;
    std::vector<unsigned> dims(message.homology_dimensions.shape(),
                               message.homology_dimensions.shape() + message.homology_dimensions.num_dimensions());
    assert(dims.size() == 2);
    hom_dims.resize(boost::extents[dims[0]][dims[1]]);
    hom_dims = message.homology_dimensions;
    qDebug() << "Received hom_dims: " << hom_dims.shape()[0] << " x " << hom_dims.shape()[1];
    for(int i = 0; i < hom_dims.shape()[0]; i++) {
        auto row = qDebug();
        for (int j = 0; j < hom_dims.shape()[1]; j++) {
            row << hom_dims[i][j];
        }
    }
    x_exact = message.x_exact;
    y_exact = message.y_exact;
    x_label = QString::fromStdString(message.x_label);
    y_label = QString::fromStdString(message.y_label);
}

void ComputationThread::compute_from_file() {
    QStringList args;

    args << QString::fromStdString(params.fileName)
    << QDir(QCoreApplication::applicationDirPath()).filePath("rivet_arrangement_temp")
    << "-H" << QString::number(params.dim)
    << "-x" << QString::number(params.x_bins)
    << "-y" << QString::number(params.y_bins)
    << "-V" << QString::number(params.verbosity)
            << "-f" << "R1";
    auto console = RivetConsoleApp::start(args);

    if (!console->waitForStarted()) {
        qDebug() << "Error launching rivet_console:" << RivetConsoleApp::errorMessage(console->error()) ;
        return;
    }

    bool reading_xi = false;
    std::stringstream ss;
    while(console->canReadLine() || console->waitForReadyRead(-1)) {
        QString line = console->readLine();
        qDebug().noquote() << "console: " << line;
        if (reading_xi) {
            if (line.startsWith("END XI")) {
                {
                    boost::archive::text_iarchive archive(ss);
                    archive >> message;
                }
                unpack_message_fields();
                reading_xi = false;
                emit xiSupportReady();
            } else {
                ss << line.toStdString();
            }
        } else if (line.startsWith("ARRANGEMENT: ")) {
                {
                    console->waitForFinished();
                    std::ifstream input(line.mid(QString("ARRANGEMENT: ").length()).trimmed().toStdString());
                    if (!input.is_open()) {
                        throw std::runtime_error("Could not open console arrangement file");
                    }
                    std::string type;
                    std::getline(input, type);
                    if (type != "RIVET_1") {
                        throw std::runtime_error("Unsupported file format");
                    }
                    qDebug() << "ComputationThread::compute_from_file() : checkpoint A -- xi_support.size() = " << xi_support.size();
                    boost::archive::binary_iarchive archive(input);
                    arrangement.reset(new MeshMessage());
                    InputParameters p;
                    archive >> p;
                    archive >> message;
                    unpack_message_fields();
                    archive >> *arrangement;
                    qDebug() << "ComputationThread::compute_from_file() : checkpoint B -- xi_support.size() = " << xi_support.size();
                }
//                qDebug() << "Mesh received: " << arrangement->x_exact.size() << " x " << arrangement->y_exact.size();
            emit arrangementReady(&*arrangement);
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

//TODO: this doesn't really belong here, look for a better place.
//NOTE this is a copy of a function in the console
//It has to be here because we get duplicate symbol errors when we use binary_oarchive in a different compilation
//unit in addition to this one.
void write_boost_file(QString file_name, InputParameters const &params, XiSupportMessage const &message, MeshMessage const &mesh) {
    std::ofstream file(file_name.toStdString(), std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open " + file_name.toStdString() + " for writing");
    }
    file << "RIVET_1\n";
    boost::archive::binary_oarchive oarchive(file);
    oarchive & params & message & mesh;
    file.flush();
}
