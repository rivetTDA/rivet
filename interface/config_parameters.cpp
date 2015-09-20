#include "config_parameters.h"

//default values are set here
ConfigParameters::ConfigParameters() :
    xi0color(0, 255, 0, 100),                       //green semi-transparent, for xi_0 support dots
    xi1color(255, 0, 0, 100),                       //red semi-transparent, for xi_1 support dots
    xi2color(255, 255, 0, 100),                     //yellow semi-transparent, for xi_2 support dots
    persistenceColor(160, 0, 200, 127),             //purple semi-transparent, for persistence bars and dots
    persistenceHighlightColor(255, 140, 0, 150),    //orange semi-transparent, for highlighting part of the slice line
    sliceLineColor(0, 0, 255, 150),                 //blue semi-transparent, for slice line
    sliceLineHighlightColor(0, 200, 200, 150)   ,   //cyan semi-transparent, for highlighting the slice line on click-and-drag
    bettiDotRadius(5),                              //radius of dot representing xi_0 = 1 or xi_1 = 1
    persistenceDotRadius(5)                         //radius of dot representing one homology class in persistence diagram
{ }

