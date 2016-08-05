#ifndef INPUT_PARAMETERS_H
#define INPUT_PARAMETERS_H

#include <string>
//TODO: this class currently conflates 3 things: command line arguments, file load dialog arguments, and viewer configuration state

//these parameters are set by the user via the console or the DataSelectDialog before computation can begin
struct InputParameters {
    std::string fileName;   //name of data file
    std::string shortName;  //name of data file, without path
    std::string outputFile; //name of the file where the augmented arrangement should be saved
    int dim;            //dimension of homology to compute
    unsigned x_bins;    //number of bins for x-coordinate (if 0, then bins are not used for x)
    unsigned y_bins;    //number of bins for y-coordinate (if 0, then bins are not used for y)
    int verbosity;      //controls the amount of console output printed
    std::string x_label; //used by configuration dialog
    std::string y_label; //used by configuration dialog
    std::string outputFormat; // Supported values: R0, R1

    template<typename Archive>
            void serialize(Archive &ar, const unsigned int &version) {
        ar & fileName & shortName & outputFile & dim & x_bins & y_bins & verbosity & x_label & y_label & outputFormat;
    }
};

#endif // INPUT_PARAMETERS_H
