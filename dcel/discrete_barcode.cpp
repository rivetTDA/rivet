#include "discrete_barcode.h"

#include <iostream>


DiscreteBar::DiscreteBar(unsigned a, unsigned b) :
    begin(a), end(b), multiplicity(1)
{ }

DiscreteBar::DiscreteBar(const DiscreteBar& other) :
    begin(other.begin), end(other.end), multiplicity(other.multiplicity)
{ }

bool DiscreteBar::operator<(const DiscreteBar other) const
{
    return (begin < other.begin) || (begin == other.begin && end < other.end);
}



DiscreteBarcode::DiscreteBarcode()
{ }

//adds a bar to the barcode (updating multiplicity, if necessary)
void DiscreteBarcode::add_bar(unsigned a, unsigned b)
{
    //look for the bar
    DiscreteBar bar(a,b);
    std::set<DiscreteBar>::iterator it = bars.find(bar);

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
std::set<DiscreteBar>::iterator DiscreteBarcode::begin()
{
    return bars.begin();
}

//returns an iterator to the past-the-end element of the barcode
std::set<DiscreteBar>::iterator DiscreteBarcode::end()
{
    return bars.end();
}

//for testing only
void DiscreteBarcode::print()
{
    for(std::set<DiscreteBar>::iterator it = bars.begin(); it != bars.end(); ++it)
    {
        DiscreteBar b = *it;
        std::cout << "(" << b.begin << "," << b.end << ")x" << b.multiplicity << ", ";
    }
    std::cout << "\n";
}


