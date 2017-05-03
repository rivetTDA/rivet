/**********************************************************************
Copyright 2014-2016 The RIVET Developers. See the COPYRIGHT file at
the top-level directory of this distribution.

This file is part of RIVET.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#include "computationthread.h"

#include "dcel/arrangement.h"
#include "dcel/arrangement_message.h"
#include "dcel/serialization.h"
#include "interface/input_manager.h"
#include "interface/input_parameters.h"
#include "math/multi_betti.h"
#include "math/simplex_tree.h"
#include "math/template_point.h"

#include "interface/console_interaction.h"

#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QString>
#include <QTime>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/tmpdir.hpp>
#include <fstream>
#include <string>
#include <vector>

ComputationThread::ComputationThread(InputParameters& params, QObject* parent)
    : QThread(parent)
    , message()
    , params(params)
{
}

ComputationThread::~ComputationThread()
{
}

void ComputationThread::compute()
{
    start();
}

//this function does the work
void ComputationThread::run()
{
    if (is_precomputed(params.fileName)) {
        load_from_file();
    } else {
        compute_from_file();
    }
}

bool ComputationThread::is_precomputed(std::string file_name)
{
    std::ifstream file(file_name);
    if (!file.is_open()) {
        throw std::runtime_error("Couldn't open " + file_name + " for reading");
    }
    std::string line;
    std::getline(file, line);
    return line == "RIVET_1";
}

void ComputationThread::load_from_file()
{
    std::ifstream file(params.fileName);
    if (!file.is_open()) {
        throw std::runtime_error("Couldn't open " + params.fileName + " for reading");
    }
    std::string type;
    std::getline(file, type);
    assert(type == "RIVET_1");
    boost::archive::binary_iarchive archive(file);

    arrangement.reset(new ArrangementMessage());
    message.reset(new TemplatePointsMessage());
    archive >> params;
    //We're not interested in the output file from the original invocation, and if it is non-empty,
    //the viewer will try to save the output also, which will most likely fail.
    params.outputFile = "";
    archive >> *message;
    emit templatePointsReady(message);
    archive >> *arrangement;
    emit arrangementReady(arrangement);
}

void ComputationThread::compute_from_file()
{
    QStringList args;

    args << QString::fromStdString(params.fileName)
         << QDir(QCoreApplication::applicationDirPath()).filePath("rivet_arrangement_temp")
         << "-H" << QString::number(params.dim)
         << "-x" << QString::number(params.x_bins)
         << "-y" << QString::number(params.y_bins)
         << "-V" << QString::number(params.verbosity)
         << "-f"
         << "R1"
         << "--binary";
    auto console = RivetConsoleApp::start(args);

    if (!console->waitForStarted()) {
        qDebug() << "Error launching rivet_console:" << RivetConsoleApp::errorMessage(console->error());
        return;
    }

    bool reading_xi = false;
    std::stringstream ss;
    while (console->waitForReadyRead(-1)) {
        while (console->canReadLine()) {
            QString line = console->readLine();
            qDebug().noquote() << "console: " << line;
            if (reading_xi) {
                if (line.startsWith("END XI")) {
                    {
                        message.reset(new TemplatePointsMessage());
                        boost::archive::text_iarchive archive(ss);
                        archive >> *message;
                    }
                    reading_xi = false;
                    emit templatePointsReady(message);
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
                    //qDebug() << "ComputationThread::compute_from_file() : checkpoint A -- template_points.size() = "
                    //         << message->template_points.size();
                    boost::archive::binary_iarchive archive(input);
                    message.reset(new TemplatePointsMessage());
                    arrangement.reset(new ArrangementMessage());
                    InputParameters p;
                    archive >> p;
                    archive >> *message;
                    archive >> *arrangement;
                    //qDebug() << "ComputationThread::compute_from_file() : checkpoint B -- template_points.size() = "
                    //         << message->template_points.size();
                    emit templatePointsReady(message);
                }
                //                qDebug() << "Arrangement received: " << arrangement->x_exact.size() << " x " << arrangement->y_exact.size();
                emit arrangementReady(arrangement);
                return;
            } else if (line.startsWith("PROGRESS ")) {
                auto progress = line.mid(QString("PROGRESS ").length()).trimmed();
                setCurrentProgress(progress.toInt());
            } else if (line.startsWith("STAGE")) {
                emit advanceProgressStage();
            } else if (line.startsWith("STEPS_IN_STAGE")) {
                auto steps = line.mid(QString("STEPS_IN_STAGE ").length()).trimmed();
                emit setProgressMaximum(steps.toInt());
            } else if (line.startsWith("XI")) {
                reading_xi = true;
                ss.clear();
            }
        }
    }

    throw std::runtime_error("Arrangement was not delivered");

} //end run()

//TODO: this doesn't really belong here, look for a better place.
//NOTE this is a copy of a function in the console
//It has to be here because we get duplicate symbol errors when we use binary_oarchive in a different compilation
//unit in addition to this one.
void write_boost_file(QString file_name, InputParameters const& params, TemplatePointsMessage const& message, ArrangementMessage const& arrangement)
{
    std::ofstream file(file_name.toStdString(), std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open " + file_name.toStdString() + " for writing");
    }
    file << "RIVET_1\n";
    boost::archive::binary_oarchive oarchive(file);
    oarchive& params& message& arrangement;
    file.flush();
}
