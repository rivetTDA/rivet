#include <QApplication>
#include "visualizationwindow.h"
#include "dataselectdialog.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //create the Visualization Window
    VisualizationWindow v_window;
    v_window.show();

    //create the DataSelectDialog
    DataSelectDialog ds_dialog(&v_window);
//    ds_dialog.setModal(true);
    ds_dialog.exec();   //this blocks until the DataSelectDialog is closed

    //do the computation and visualization
    v_window.compute();
    
    return a.exec();
}
