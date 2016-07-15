#include "interface/input_parameters.h"
#include "visualizationwindow.h"

#include <QApplication>
#include <QScopedPointer>

#include <iostream>


QCoreApplication* createApplication(int &argc, char *argv[])
{
    for(int i = 1; i < argc; ++i)
        if(!qstrcmp(argv[i], "-n") || !qstrcmp(argv[i], "--no-gui"))
            return new QCoreApplication(argc, argv);
    return new QApplication(argc, argv);
}


int main(int argc, char *argv[])
{
    InputParameters params;   //parameter values stored here

    //create the appropriate application object
    QScopedPointer<QCoreApplication> app(createApplication(argc, argv));
  //  QScopedPointer<QApplication> app(new QApplication(argc, argv));
    QCoreApplication::setApplicationName("RIVET");
    QCoreApplication::setApplicationVersion("0.4");

    //define the command line options
    QCommandLineParser parser;
    parser.setApplicationDescription(QCoreApplication::translate("main", "RIVET: Rank Invariant Visualization and Exploration Tool"));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("input_file", QCoreApplication::translate("main", "Input file"));

    QCommandLineOption noGUIOption(QStringList() << "n" << "no-gui", QCoreApplication::translate("main", "Run RIVET in the console, without a graphical interface."));
    parser.addOption(noGUIOption);

    QCommandLineOption homOption(QStringList() << "H" << "homology", QCoreApplication::translate("main", "Dimension of homology to compute."), QCoreApplication::translate("main", "dimension"), "0");
    parser.addOption(homOption);

    QCommandLineOption outputOption(QStringList() << "o" << "output", QCoreApplication::translate("main", "Output file where augmented arrangement will be saved."), QCoreApplication::translate("main", "filename"), "");
    parser.addOption(outputOption);

    QCommandLineOption verbosityOption(QStringList() << "V" << "verbosity", QCoreApplication::translate("main", "Verbosity level: 0 (no console output) to 10 (lots of output)."), QCoreApplication::translate("main", "integer"), "2");
    parser.addOption(verbosityOption);

    QCommandLineOption xbinOption(QStringList() << "x" << "xbins", QCoreApplication::translate("main", "Number of bins in the x-direction."), QCoreApplication::translate("main", "integer"), "0");
    parser.addOption(xbinOption);

    QCommandLineOption ybinOption(QStringList() << "y" << "ybins", QCoreApplication::translate("main", "Number of bins in the y-direction."), QCoreApplication::translate("main", "integer"), "0");
    parser.addOption(ybinOption);

    //parse the command line options
    parser.process(*app);

    QStringList args = parser.positionalArguments();
    if(args.size() > 0)
      params.fileName = args.at(0).toUtf8().constData();
    params.outputFile = parser.value(outputOption).toUtf8().constData();
    params.dim = parser.value(homOption).toInt();
    params.verbosity = parser.value(verbosityOption).toInt();
    params.x_bins = parser.value(xbinOption).toInt();
    params.y_bins = parser.value(ybinOption).toInt();

    //now run RIVET
    if (qobject_cast<QApplication *>(app.data()))   // start GUI version
    {
        VisualizationWindow v_window(params);
        v_window.show();

        QApplication* ap = qobject_cast<QApplication *>(app.data());
        return ap->exec();
    }

}//end main()
