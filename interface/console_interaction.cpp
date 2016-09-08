//
// Created by Bryn Keller on 7/7/16.
//

#include "console_interaction.h"

QString RivetConsoleApp::errorMessage(QProcess::ProcessError error)
{

    switch (error) {
    case QProcess::FailedToStart:
        return ("Error launching rivet_console");
    case QProcess::Crashed:
        return ("Rivet console crashed");
    case QProcess::Timedout:
        return ("Rivet console timed out");
    case QProcess::WriteError:
        return ("Couldn't write to rivet console");
    case QProcess::ReadError:
        return ("Couldn't read from Rivet console");
    default:
        return ("Error communicating with Rivet console");
    }
}

std::unique_ptr<QProcess> RivetConsoleApp::start(QStringList args)
{
    std::unique_ptr<QProcess> console(new QProcess());
    console->setProcessChannelMode(QProcess::MergedChannels);
    console->setWorkingDirectory(QCoreApplication::applicationDirPath());
    console->start("./rivet_console", args);
    return console;
}
