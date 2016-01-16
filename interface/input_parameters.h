#ifndef INPUT_PARAMETERS_H
#define INPUT_PARAMETERS_H

#include <QString>

//these parameters are set by the user via the console or the DataSelectDialog before computation can begin
struct InputParameters {
    QString fileName;   //name of data file
    QString shortName;  //name of data file, without path
    QString outputFile; //name of the file where the augmented arrangement should be saved
    int dim;            //dimension of homology to compute
    unsigned x_bins;    //number of bins for x-coordinate (if 0, then bins are not used for x)
    unsigned y_bins;    //number of bins for y-coordinate (if 0, then bins are not used for y)
    QString x_label;    //label for x-axis of slice diagram
    QString y_label;    //label for y-axis of slice_diagram
    bool raw_data;      //true if persistence must be computed from the data; false if the data is a RIVET file
    int verbosity;      //controls the amount of console output printed
};

#endif // INPUT_PARAMETERS_H
