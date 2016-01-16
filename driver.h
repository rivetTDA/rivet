#ifndef DRIVER_H
#define DRIVER_H

#include "interface/input_parameters.h"

#include <QCoreApplication>
#include <QObject>
#include <QString>

class Driver : public QObject
{
    Q_OBJECT

    public:
        explicit Driver(InputParameters& params, QObject* parent = 0);

        void quit();    //call this to quit the application

    signals:
        void finished();    //signal to finish, connected to Application Quit

    public slots:
        void run();     //gets called from main to start everything
        void aboutToQuitApp();  //gets signal when application is about to quit

    private:
        QCoreApplication *app;

        InputParameters& input_params;
};

#endif // DRIVER_H
