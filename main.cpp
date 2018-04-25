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

#include "api.h"
#include "interface/input_parameters.h"
#include "visualizationwindow.h"

#include "dcel/arrangement_message.h"
#include "dcel/dcel.h"
#include <QApplication>
#include <QMetaType>
#include <QScopedPointer>
#include <iostream>

Q_DECLARE_SMART_POINTER_METATYPE(std::shared_ptr)
Q_DECLARE_METATYPE(TemplatePointsMessage)
Q_DECLARE_METATYPE(ArrangementMessage)
Q_DECLARE_METATYPE(std::shared_ptr<TemplatePointsMessage>)
Q_DECLARE_METATYPE(std::shared_ptr<ArrangementMessage>)

int main(int argc, char* argv[])
{
    InputParameters params; //parameter values stored here

    //create the appropriate application object
    QApplication app(argc, argv);
    QCoreApplication::setApplicationName("RIVET");
    QCoreApplication::setOrganizationName("RIVET developers");
    QCoreApplication::setOrganizationDomain("rivet.online");
    QCoreApplication::setApplicationVersion("0.4");

    //define the command line options
    QCommandLineParser parser;
    parser.setApplicationDescription(
        QCoreApplication::translate("main",
            "RIVET: Rank Invariant Visualization and Exploration Tool\n"
            "This is the RIVET viewer (GUI) application.\n"
            "There is also a command line tool called rivet_console.\n\n"
            "For more information, see the RIVET website at https://rivet.online."));
    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    QCommandLineOption verbosityOption(QStringList() << "V"
                                                     << "verbosity",
        QCoreApplication::translate("main", "Verbosity level: 0 (no console output) to 10 (lots of output)."),
        QCoreApplication::translate("main", "integer"), "2");
    parser.addOption(verbosityOption);

    //parse the command line options
    parser.process(app);

    params.dim = 0;
    params.verbosity = parser.value(verbosityOption).toInt();
    params.x_bins = 0;
    params.y_bins = 0;
    params.x_reverse=false;
    params.y_reverse=false;

    qRegisterMetaType<std::shared_ptr<ArrangementMessage>>();
    qRegisterMetaType<std::shared_ptr<TemplatePointsMessage>>();

    //now run RIVET
    if (!(parser.isSet(helpOption) || parser.isSet(versionOption))) {
        VisualizationWindow v_window(params);
        v_window.show();

        return app.exec();
    }

} //end main()
