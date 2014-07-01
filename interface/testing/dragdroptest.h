#ifndef DRAGDROPTEST_H
#define DRAGDROPTEST_H

#include <QMainWindow>
#include <QtCore>
#include <QtGui>

#include "mydot.h"
#include "myline.h"


namespace Ui {
class DragDropTest;
}

class DragDropTest : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit DragDropTest(QWidget *parent = 0);
    ~DragDropTest();
    
private:
    Ui::DragDropTest *ui;
    QGraphicsScene* scene;

    MyDot* dot;
    QGraphicsLineItem* lineLeft;
    QGraphicsLineItem* lineBottom;
    QGraphicsLineItem* lineDiag;
    QGraphicsTextItem* coords;

    MyLine* myline;
};

#endif // DRAGDROPTEST_H
