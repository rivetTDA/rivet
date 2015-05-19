#include <QApplication>
#include "visualizationwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    VisualizationWindow v_window;
    v_window.show();
    
    return a.exec();
}
