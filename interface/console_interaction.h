//
// Created by Bryn Keller on 7/7/16.
//

#ifndef RIVET_CONSOLE_CONSOLE_INTERACTION_H
#define RIVET_CONSOLE_CONSOLE_INTERACTION_H
#include <QCoreApplication>
#include <QProcess>
#include <QString>
#include <memory>

class RivetConsoleApp {
public:
    static QString errorMessage(QProcess::ProcessError error);
    static std::unique_ptr<QProcess> start(QStringList args);
};

#endif //RIVET_CONSOLE_CONSOLE_INTERACTION_H
