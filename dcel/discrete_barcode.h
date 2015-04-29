#ifndef __DISCRETE_BARCODE_H__
#define __DISCRETE_BARCODE_H__

#include <set>

struct BarTemplate
{
    unsigned begin;                   //index of xiMatrixEntry of the equivalence class corresponding to the beginning of this bar
    unsigned end;                     //index of xiMatrixEntry of the equivalence class corresponding to the end of this bar
    mutable unsigned multiplicity;    //maybe this is bad style, but multiplicity is not involved in comparisons

    BarTemplate(unsigned a, unsigned b);
    BarTemplate(const BarTemplate& other);

    bool operator<(const BarTemplate other) const;
};


class BarcodeTemplate
{
    public:
        BarcodeTemplate();

        void add_bar(unsigned a, unsigned b);      //adds a bar to the barcode (updating multiplicity, if necessary)

        std::set<BarTemplate>::iterator begin();    //returns an iterator to the first bar in the barcode
        std::set<BarTemplate>::iterator end();      //returns an iterator to the past-the-end element of the barcode

        void print();   //for testing only

    private:
        std::set<BarTemplate> bars;
};

#endif // __DISCRETE_BARCODE_H__
