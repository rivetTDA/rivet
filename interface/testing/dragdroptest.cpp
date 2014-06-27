#include "dragdroptest.h"
#include "ui_dragdroptest.h"


DragDropTest::DragDropTest(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::DragDropTest)
{
    ui->setupUi(this);

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->scale(1,-1);

    QBrush redBrush(Qt::red);
    QBrush blueBrush(Qt::blue);
    QPen blackPen(Qt::black);
    blackPen.setWidth(2);

    lineLeft = scene->addLine(0,0,0,200,blackPen);
    lineBottom = scene->addLine(0,0,100,0,blackPen);

    coords = scene->addText("(0,0)");
    coords->scale(1,-1);
    coords->setPos(0,-20);

    dot = new MyDot(coords);
    scene->addItem(dot);

}

DragDropTest::~DragDropTest()
{
    delete ui;
}
