#include "aboutmessagebox.h"
#include "ui_aboutmessagebox.h"

AboutMessageBox::AboutMessageBox(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutMessageBox)
{
    ui->setupUi(this);
}

AboutMessageBox::~AboutMessageBox()
{
    delete ui;
}

void AboutMessageBox::on_pushButton_clicked()
{
    close();
}
