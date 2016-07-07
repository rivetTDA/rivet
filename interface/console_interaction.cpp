//
// Created by Bryn Keller on 7/7/16.
//

#include "console_interaction.h"
void RivetConsoleApp::errorMessage(QProcess::ProcessError error) {

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
