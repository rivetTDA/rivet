#ifndef INPUT_PARAMETERS_H
#define INPUT_PARAMETERS_H

#include <QString>

//these parameters are set by the user via the DataSelectDialog before computation can begin
struct InputParameters {
    QString fileName;   //name of data file
    int dim;            //dimension of homology to compute
    unsigned x_bins;    //number of bins for x-coordinate (if 0, then bins are not used for x)
    unsigned y_bins;    //number of bins for y-coordinate (if 0, then bins are not used for y)
    QString x_label;    //label for x-axis of slice diagram
    QString y_label;    //label for y-axis of slice_diagram
};

#endif // INPUT_PARAMETERS_H
