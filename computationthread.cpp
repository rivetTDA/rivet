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
    std::cerr << "Computation thread running" << std::endl;
    if (is_precomputed(params.fileName)) {
        load_from_file(params.fileName);
    } else {
        compute_from_file();
    }
}

bool ComputationThread::is_precomputed(const std::string &file_name)
{
    std::cerr << "Checking if precomputed" << std::endl;
    std::ifstream file(file_name);
    if (!file.is_open()) {
        throw std::runtime_error("Couldn't open " + file_name + " for reading");
    }
    std::string line;
    std::getline(file, line);
    std::cerr << line.substr(0, 5) << std::endl;
    return line.substr(0, 5) == "RIVET";
}

void ComputationThread::load_template_points_from_file(const std::string &file_name)
{
    std::ifstream file(file_name);
    if (!file.is_open()) {
        throw std::runtime_error("Couldn't open " + file_name + " for reading");
    }
    std::string type;
    std::getline(file, type);
    assert(type == "RIVET_msgpack");
    message.reset(new TemplatePointsMessage());
    std::string buffer((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());

    msgpack::unpacker pac;
    pac.reserve_buffer( buffer.size() );
    std::copy( buffer.begin(), buffer.end(), pac.buffer() );
    pac.buffer_consumed( buffer.size() );

    msgpack::object_handle oh;
    pac.next(oh);
    auto m2 = oh.get();
    m2.convert(*message);
    emit templatePointsReady(message);
}

void ComputationThread::load_from_file(const std::string &file_name)
{
    std::ifstream file(file_name);
    if (!file.is_open()) {
        throw std::runtime_error("Couldn't open " + file_name + " for reading");
    }
    std::string oldFileName = params.fileName;
    std::string oldShortName = params.shortName;
    std::string type;
    std::getline(file, type);
    assert(type == "RIVET_msgpack");
    arrangement.reset(new ArrangementMessage());
    message.reset(new TemplatePointsMessage());
    std::string buffer((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());

    msgpack::unpacker pac;
    pac.reserve_buffer( buffer.size() );
    std::copy( buffer.begin(), buffer.end(), pac.buffer() );
    pac.buffer_consumed( buffer.size() );

    msgpack::object_handle oh;
    pac.next(oh);
    auto m1 = oh.get();
    m1.convert(params);
    //We're not interested in the output file from the original invocation, and if it is non-empty,
    //the viewer will try to save the output also, which will most likely fail.
    params.outputFile = "";
    //Copy the input names from the original params, since we care what the user selected, not what previous input
    //files were called
    params.fileName = oldFileName;
    params.shortName = oldShortName;
    pac.next(oh);
    auto m2 = oh.get();
    m2.convert(*message);
    emit templatePointsReady(message);
    pac.next(oh);
    auto m3 = oh.get();
    m3.convert(*arrangement);
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
         << "msgpack"
         << "--binary";
    auto console = RivetConsoleApp::start(args);

    if (!console->waitForStarted()) {
        qDebug() << "Error launching rivet_console:" << RivetConsoleApp::errorMessage(console->error());
        return;
    }

    std::stringstream ss;
    while (console->waitForReadyRead(-1)) {
        while (console->canReadLine()) {
            QString line = console->readLine();
            qDebug().noquote() << "console: " << line;
            if (line.startsWith("XI: ")) {
                auto file_name = line.mid(QString("XI: ").length()).trimmed().toStdString();
                std::clog << "Loading file " << file_name << std::endl;
                load_template_points_from_file(file_name);
            } else if (line.startsWith("ARRANGEMENT: ")) {
                console->waitForFinished();
                auto file_name = line.mid(QString("ARRANGEMENT: ").length()).trimmed().toStdString();
                std::clog << "Loading file " << file_name << std::endl;
                load_from_file(file_name);
                return;
            } else if (line.startsWith("PROGRESS ")) {
                auto progress = line.mid(QString("PROGRESS ").length()).trimmed();
                setCurrentProgress(progress.toInt());
            } else if (line.startsWith("STAGE")) {
                emit advanceProgressStage();
            } else if (line.startsWith("STEPS_IN_STAGE")) {
                auto steps = line.mid(QString("STEPS_IN_STAGE ").length()).trimmed();
                emit setProgressMaximum(steps.toInt());
            }
        }
    }
    console->waitForFinished();

    throw std::runtime_error("Arrangement was not delivered");

} //end run()

//TODO: this is basically a copy of a method in the console. Move file handling to librivet and link?
void write_msgpack_file(QString file_name, InputParameters const& params, TemplatePointsMessage const& message, ArrangementMessage const& arrangement)
{
    std::ofstream file(file_name.toStdString(), std::ios::binary);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open " + file_name.toStdString() + " for writing.");
    }
    file << "RIVET_msgpack\n";
    msgpack::pack(file, params);
    msgpack::pack(file, message);
    msgpack::pack(file, arrangement);
    file.flush();
}
