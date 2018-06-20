#ifndef CUSTOMSPINBOX_H
#define CUSTOMSPINBOX_H
#include <QtWidgets>

//class for spin boxes used in visualization window
class CustomSpinBox : public QDoubleSpinBox
{
public:
    explicit CustomSpinBox(QWidget *parent = 0) : QDoubleSpinBox(parent) {

    }

    //this function controls the appearance of the text
    QString textFromValue(double val) const {

        QString str = QString::number( val, 'f', decimals() );

        str.remove( QRegExp("0+$") ); // Remove any number of trailing 0's
        str.remove( QRegExp("\\.$") ); // If the last character is just a '.' then remove it

        return str;
    }
};


#endif // CUSTOMSPINBOX_H
