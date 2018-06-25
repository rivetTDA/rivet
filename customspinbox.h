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


        QString str = QString::number(fabs(val), 'f', decimals() );
        //take absolute value so that "-0" is not immediately corrected to "0"

        //make sure there is not a zero before the decimal point for numbers in (-1,1)
        if((val>-1&& val<0) ||(val>0&& val<1)){
           str.replace("0.",".");
        }

        //add in the minus sign if the value is negative, or if the user was in the middle of typing
        //a negative value >-1 (e.g. -0.5)
        if(val<0 ||(lineEdit()->text().size()>0&& lineEdit()->text().at(0)=="-")){
            str="-"+str;
        }

        str.remove( QRegExp("0+$") ); // Remove any number of trailing 0's
        str.remove( QRegExp("\\.$") ); // If the last character is just a '.' then remove it

        return str;
    }
};


#endif // CUSTOMSPINBOX_H
