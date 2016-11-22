#ifndef __BARCODE_H__
#define __BARCODE_H__

#include <set>

struct MultiBar {
    double birth; //coordinate where this bar begins
    double death; //coordinate where this bar ends
    unsigned multiplicity; //multiplicity of this bar

    MultiBar(double b, double d, unsigned m);
    MultiBar(const MultiBar& other);

    bool operator<(const MultiBar other) const;
};

class Barcode {
public:
    Barcode();

    void add_bar(double b, double d, unsigned m); //adds a bar to the barcode

    std::multiset<MultiBar>::const_iterator begin() const; //returns an iterator to the first bar in the barcode
    std::multiset<MultiBar>::const_iterator end() const; //returns an iterator to the pst-the-end element the barcode
    unsigned size() const; //returns the number of multibars in the barcode

    void print() const; //for testing only

private:
    std::multiset<MultiBar> bars; //must be a multiset because the comparison operator for MultiBars might not establish a total order
};

#endif // __BARCODE_H__
