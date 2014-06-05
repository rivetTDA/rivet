/****************************************************************************
* window class for a test interface for a persistence program
*
* Matthew L. Wright
* 2014
****************************************************************************/

#include <QtGui>
#include <string>
#include <math.h>
#include "pdarea.h"
#include "slicearea.h"
#include "window.h"

const int IdRole = Qt::UserRole;

Window::Window()
{
    /* items for the slice diagram */
    sliceLabel = new QLabel(tr("<b>Slice</b>"));

    double gx = 30, gy = 10, lx = 100, ly = 50; //values for testing

    sliceArea = new SliceArea(gx, gy, lx, ly);

    gcdLabel = new QLabel(tr("GCD = (30, 10)")); //FIX THIS!!!
    lcmLabel = new QLabel(tr("LCM = (100, 50)"));   //FIX THIS!!!
    dydxLabel = new QLabel(tr("DY/DX = ???"));

    angleSlider = new QSlider(Qt::Horizontal);
    angleSlider->setRange(0, 90);
    angleSpinBox = new QSpinBox;
    angleSpinBox->setRange(0, 90);
    QObject::connect(angleSpinBox, SIGNAL(valueChanged(int)), angleSlider, SLOT(setValue(int)));
    QObject::connect(angleSlider, SIGNAL(valueChanged(int)), angleSpinBox, SLOT(setValue(int)));
    QObject::connect(angleSpinBox, SIGNAL(valueChanged(int)), this, SLOT(sliceChanged()));
    angleLabel = new QLabel(tr("&Angle:"));
    angleLabel->setBuddy(angleSpinBox);

    offsetSlider = new QSlider(Qt::Horizontal);
    offsetSlider->setRange(-lx, ly);
    offsetSpinBox = new QSpinBox;
    offsetSpinBox->setRange(-lx, ly);
    QObject::connect(offsetSpinBox, SIGNAL(valueChanged(int)), offsetSlider, SLOT(setValue(int)));
    QObject::connect(offsetSlider, SIGNAL(valueChanged(int)), offsetSpinBox, SLOT(setValue(int)));
    QObject::connect(offsetSpinBox, SIGNAL(valueChanged(int)), this, SLOT(sliceChanged()));
    offsetSpinBox->setValue(round((gy+ly)/2));
    offsetLabel = new QLabel(tr("&Offset:"));
    offsetLabel->setBuddy(offsetSpinBox);

    /* items for the persistence diagram */
    pdLabel = new QLabel(tr("<b>Persistence Diagram</b>"));

    pdArea = new PDArea;

    normCoordCheckBox = new QCheckBox(tr("&Normalize Coordinates"));

    cleanDiagSpinBox = new QSpinBox;
    cleanDiagSpinBox->setRange(0,100);
    cleanDiagLabel = new QLabel(tr("Clean &Diagonal"));
    cleanDiagLabel->setBuddy(cleanDiagSpinBox);

    scaleSpinBox = new QDoubleSpinBox;
    scaleSpinBox->setRange(0,10);
    scaleSpinBox->setSingleStep(0.1);
    scaleLabel = new QLabel(tr("Current/Default &Scale"));
    scaleLabel->setBuddy(scaleSpinBox);

    fitScaleButton = new QPushButton("&Fit Scale");
    resetScaleButton = new QPushButton(tr("&Reset Scale"));


    /* the layout */
    QGridLayout *mainLayout = new QGridLayout;

 //   mainLayout->setColumnStretch(0, 1);
 //   mainLayout->setColumnStretch(3, 1);
    mainLayout->setColumnMinimumWidth(3, 20);

    mainLayout->addWidget(sliceLabel, 0, 0, 1, 3, Qt::AlignHCenter);
    mainLayout->addWidget(pdLabel, 0, 4, 1, 3, Qt::AlignHCenter);

    mainLayout->addWidget(sliceArea, 1, 0, 1, 3);
    mainLayout->addWidget(pdArea, 1, 4, 1, 3);

    mainLayout->addWidget(gcdLabel, 2, 0);
    mainLayout->addWidget(lcmLabel, 2, 1);
    mainLayout->addWidget(dydxLabel, 2, 2);

    mainLayout->addWidget(angleLabel, 3, 0);
    mainLayout->addWidget(angleSlider, 3, 1);
    mainLayout->addWidget(angleSpinBox, 3, 2);

    mainLayout->addWidget(offsetLabel, 4, 0);
    mainLayout->addWidget(offsetSlider, 4, 1);
    mainLayout->addWidget(offsetSpinBox, 4, 2);

    mainLayout->addWidget(normCoordCheckBox, 2, 5);

    mainLayout->addWidget(cleanDiagSpinBox, 3, 4);
    mainLayout->addWidget(cleanDiagLabel, 3, 5);

    mainLayout->addWidget(scaleSpinBox, 4, 4);
    mainLayout->addWidget(scaleLabel, 4, 5);

    mainLayout->addWidget(fitScaleButton, 5, 4);
    mainLayout->addWidget(resetScaleButton, 5, 5);

    setLayout(mainLayout);

    sliceChanged();

    setWindowTitle(tr("Persistence Visualization"));
}

void Window::sliceChanged()
{
    int angle = angleSpinBox->value();
    int offset = offsetSpinBox->value();

    sliceArea->setLine(angle, offset);
}
