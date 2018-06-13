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

        //determine how many trailing zeros there are in the decimals shown
        int sig_figs=(val-int(val))*pow(10.0,decimals());
        int num_trailing_zeros=0;

        while(num_trailing_zeros!=decimals()&&sig_figs%10==0){
            sig_figs=sig_figs/10;
            num_trailing_zeros+=1;
        }

        return QLocale().toString(val, 'f', decimals()-num_trailing_zeros);
    }
};


#endif // CUSTOMSPINBOX_H
