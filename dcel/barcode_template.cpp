#include "barcode_template.h"

#include <iostream>


BarTemplate::BarTemplate(unsigned a, unsigned b) :
    begin(a), end(b), multiplicity(1)
{ }

BarTemplate::BarTemplate(const BarTemplate& other) :
    begin(other.begin), end(other.end), multiplicity(other.multiplicity)
{ }

bool BarTemplate::operator<(const BarTemplate other) const
{
    return (begin < other.begin) || (begin == other.begin && end < other.end);
}



BarcodeTemplate::BarcodeTemplate()
{ }

//adds a bar to the barcode (updating multiplicity, if necessary)
void BarcodeTemplate::add_bar(unsigned a, unsigned b)
{
    //look for the bar
    BarTemplate bar(a,b);
    std::set<BarTemplate>::iterator it = bars.find(bar);

    if(it == bars.end())    //then the bar doesn't already exist, so insert it
    {
        bars.insert(bar);
    }
    else    //then the bar already exists, so increment its multiplicity
    {
        (*it).multiplicity++;
    }
}

//returns an iterator to the first bar in the barcode
std::set<BarTemplate>::iterator BarcodeTemplate::begin()
{
    return bars.begin();
}

//returns an iterator to the past-the-end element of the barcode
std::set<BarTemplate>::iterator BarcodeTemplate::end()
{
    return bars.end();
}

//for testing only
void BarcodeTemplate::print()
{
    for(std::set<BarTemplate>::iterator it = bars.begin(); it != bars.end(); ++it)
    {
        BarTemplate b = *it;
        std::cout << "(" << b.begin << "," << b.end << ")x" << b.multiplicity << ", ";
    }
    std::cout << "\n";
}


