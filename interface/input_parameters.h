#ifndef INPUT_PARAMETERS_H
#define INPUT_PARAMETERS_H

#include <string>
//these parameters are set by the user via the console or the DataSelectDialog before computation can begin
struct InputParameters {
    std::string fileName;   //name of data file
    std::string shortName;  //name of data file, without path
    std::string outputFile; //name of the file where the augmented arrangement should be saved
    int dim;            //dimension of homology to compute
    unsigned x_bins;    //number of bins for x-coordinate (if 0, then bins are not used for x)
    unsigned y_bins;    //number of bins for y-coordinate (if 0, then bins are not used for y)
    std::string x_label;    //label for x-axis of slice diagram
    std::string y_label;    //label for y-axis of slice_diagram
    bool raw_data;      //true if persistence must be computed from the data; false if the data is a RIVET file
    int verbosity;      //controls the amount of console output printed
};

#endif // INPUT_PARAMETERS_H
