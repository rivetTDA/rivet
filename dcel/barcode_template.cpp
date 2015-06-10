#include "barcode_template.h"

#include <QDebug>


BarTemplate::BarTemplate(unsigned a, unsigned b) :
    begin(a), end(b), multiplicity(1)
{ }

BarTemplate::BarTemplate(unsigned a, unsigned b, unsigned m) :
    begin(a), end(b), multiplicity(m)
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
    BarTemplate bar(a, b);
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

//adds a bar with multiplicity to the barcode template
void BarcodeTemplate::add_bar(unsigned a, unsigned b, unsigned m)
{
    //look for the bar
    BarTemplate bar(a, b, m);
    std::set<BarTemplate>::iterator it = bars.find(bar);

    if(it == bars.end())    //then the bar doesn't already exist, so insert it
    {
        bars.insert(bar);
    }
    else    //then the bar already exists, so increment its multiplicity
    {
        (*it).multiplicity += m;
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

//returns true iff this barcode has no bars
bool BarcodeTemplate::is_empty()
{
    return bars.empty();
}

//for testing only
void BarcodeTemplate::print()
{
    QDebug qd = qDebug().nospace();
    qd << "      barcode template: ";
    for(std::set<BarTemplate>::iterator it = bars.begin(); it != bars.end(); ++it)
    {
        BarTemplate b = *it;
        qd << "(" << b.begin << "," << b.end << ")x" << b.multiplicity << ", ";
    }
}


