#include "configuredialog.h"
#include "ui_configuredialog.h"

#include <QColorDialog>
#include <QDebug>
#include <QPalette>

ConfigureDialog::ConfigureDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConfigureDialog)
{
    ui->setupUi(this);
}

ConfigureDialog::~ConfigureDialog()
{
    delete ui;
}

void ConfigureDialog::on_cancelButton_clicked()
{
    close();
}


void ConfigureDialog::on_testButton_clicked()
{
    QColor color = QColorDialog::getColor(Qt::blue, this); //, "Choose a Color", QColorDialog::ShowAlphaChannel);

    if(color.isValid())
    {
        QString qss = QString("border: none; outline: none; background-color: %1;").arg(color.name());
        ui->testButton->setStyleSheet(qss);
        ui->testButton->setAutoFillBackground(true);
        ui->testButton->setFlat(true);
        qDebug() << qss;
    }

//    QPalette pal = ui->xi0_color_widget->palette();
//    pal.setColor(QPalette::Window, color);
//    ui->xi0_color_widget->setPalette(pal);

}
