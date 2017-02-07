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
