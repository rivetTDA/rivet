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
    QPen grayPen(Qt::gray);

    lineDiag = scene->addLine(0,0,180,180,grayPen);
    scene->addLine(0,0,100,200,grayPen);
    scene->addLine(0,0,180,90,grayPen);
    lineLeft = scene->addLine(0,0,0,200,blackPen);
    lineBottom = scene->addLine(0,0,180,0,blackPen);

    scene->addLine(0,200,180,200,blackPen);
    scene->addLine(180,0,180,200,blackPen);

    coords = scene->addText("(0,0)");
    coords->scale(1,-1);
    coords->setPos(0,-20);


    myline = new MyLine(0,180,0,200);
    scene->addItem(myline);


    dot = new MyDot(coords);
    scene->addItem(dot);

}

DragDropTest::~DragDropTest()
{
    delete ui;
}
