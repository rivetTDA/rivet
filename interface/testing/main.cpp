#include <QtGui/QApplication>
#include "dragdroptest.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    DragDropTest w;
    w.show();
    
    return a.exec();
}
