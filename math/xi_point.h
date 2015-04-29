#ifndef XI_POINT_H
#define XI_POINT_H

//class to store xi points, to help send data from the Mesh to the VisualizationWindow
class xiPoint
{
public:
    unsigned x, y;  //coordinates (discrete)
    int zero, one;  //multiplicities of xi_0 and xi_1 at this point ---- TODO: maybe should be unsigned?

    xiPoint(unsigned xc, unsigned yc, int m0, int m1);
};
#endif // XI_POINT_H
