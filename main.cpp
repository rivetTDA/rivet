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
    QCoreApplication::setApplicationVersion("0.4");

    //define the command line options
    QCommandLineParser parser;
    parser.setApplicationDescription(
            QCoreApplication::translate("main",
                                        "RIVET: Rank Invariant Visualization and Exploration Tool\n"
            "This is the RIVET viewer (GUI) application.\n"
    "There is also a command line tool called rivet_console.\n\n"
            "for more information, see the RIVET website at https://rivet.online"));
    auto helpOption = parser.addHelpOption();
    auto versionOption = parser.addVersionOption();
    QCommandLineOption verbosityOption(QStringList() << "V"
                                                     << "verbosity",
        QCoreApplication::translate("main", "Verbosity level: 0 (no console output) to 10 (lots of output)."), QCoreApplication::translate("main", "integer"), "2");
    parser.addOption(verbosityOption);

    //parse the command line options
    parser.process(app);

    params.dim = 0;
    params.verbosity = parser.value(verbosityOption).toInt();
    params.x_bins = 0;
    params.y_bins = 0;

    qRegisterMetaType<std::shared_ptr<ArrangementMessage>>();
    qRegisterMetaType<std::shared_ptr<TemplatePointsMessage>>();

    //now run RIVET
    if (!(parser.isSet(helpOption) || parser.isSet(versionOption))) {
        VisualizationWindow v_window(params);
        v_window.show();

        return app.exec();
    }

} //end main()
