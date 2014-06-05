/****************************************************************************
* window class for a test interface for a persistence program
*
* Matthew L. Wright
* 2014
****************************************************************************/

#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>

QT_BEGIN_NAMESPACE
class QCheckBox;
class QLabel;
class QSpinBox;
class QDoubleSpinBox;
class QSlider;
class QPushButton;
QT_END_NAMESPACE
class PDArea;
class SliceArea;

class Window : public QWidget
{
    Q_OBJECT

public:
    Window();

private slots:
    void sliceChanged();

private:
    QLabel *sliceLabel;
    QLabel *pdLabel;

    SliceArea *sliceArea;
    PDArea *pdArea;

    QLabel *gcdLabel;
    QLabel *lcmLabel;
    QLabel *dydxLabel;

    QLabel *angleLabel;
    QSlider *angleSlider;
    QSpinBox *angleSpinBox;

    QLabel *offsetLabel;
    QSlider *offsetSlider;
    QSpinBox *offsetSpinBox;

    QCheckBox *normCoordCheckBox;

    QSpinBox *cleanDiagSpinBox;
    QLabel *cleanDiagLabel;

    QDoubleSpinBox *scaleSpinBox;
    QLabel *scaleLabel;

    QPushButton *fitScaleButton;
    QPushButton *resetScaleButton;

};

#endif
