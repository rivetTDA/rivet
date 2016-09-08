#ifndef XI_POINT_H
#define XI_POINT_H

//class to store xi points, to help send data from the Mesh to the VisualizationWindow
class xiPoint {
public:
    unsigned x, y; //coordinates (discrete)
    int zero, one, two; //multiplicities of xi_0, xi_1, and xi_2 at this point ---- TODO: maybe should be unsigned?

    xiPoint(unsigned xc, unsigned yc, int m0, int m1, int m2);
    xiPoint(); //for serialization

    friend bool operator==(xiPoint const& left, xiPoint const& right);

    template <class Archive>
    void serialize(Archive& archive, const unsigned int version)
    {
        archive& x& y& zero& one& two;
    }
};

#endif // XI_POINT_H
