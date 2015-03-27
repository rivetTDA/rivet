#ifndef __DISCRETE_BARCODE_H__
#define __DISCRETE_BARCODE_H__

#include <set>

struct DiscreteBar
{
    unsigned begin;                   //index of xiMatrixEntry of the equivalence class corresponding to the beginning of this bar
    unsigned end;                     //index of xiMatrixEntry of the equivalence class corresponding to the end of this bar
    mutable unsigned multiplicity;    //maybe this is bad style, but multiplicity is not involved in comparisons

    DiscreteBar(unsigned a, unsigned b);
    DiscreteBar(const DiscreteBar& other);

    bool operator<(const DiscreteBar other) const;
};


class DiscreteBarcode
{
    public:
        DiscreteBarcode();

        void add_bar(unsigned a, unsigned b);      //adds a bar to the barcode (updating multiplicity, if necessary)

        std::set<DiscreteBar>::iterator begin();    //returns an iterator to the first bar in the barcode
        std::set<DiscreteBar>::iterator end();      //returns an iterator to the past-the-end element of the barcode

        void print();   //for testing only

    private:
        std::set<DiscreteBar> bars;
};

#endif // __DISCRETE_BARCODE_H__
