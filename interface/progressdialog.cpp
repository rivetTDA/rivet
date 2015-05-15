#include "progressdialog.h"
#include "ui_progressdialog.h"

#include <QDebug>

ProgressDialog::ProgressDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ProgressDialog)
{
    ui->setupUi(this);
}

ProgressDialog::~ProgressDialog()
{
    delete ui;
}

void ProgressDialog::updateProgress(QString text, int percent)
{
    ui->description->setText(text);
    ui->progressBar->setValue(percent);
}

void ProgressDialog::updatePercent(int percent)
{
    ui->progressBar->setValue(percent);
}

