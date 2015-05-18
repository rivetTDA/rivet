#ifndef CONFIG_PARAMETERS_H
#define CONFIG_PARAMETERS_H

#include <QColor>

//these parameters control the visualization and are (mostly) user-customizable through the Configure dialog box
struct ConfigParameters {
    QColor xi0color;
    QColor xi1color;
    QColor persistenceColor;
    QColor persistenceHighlightColor;
    QColor sliceLineColor;
    QColor sliceLineHighlightColor;

    ConfigParameters();
};

#endif // CONFIG_PARAMETERS_H

