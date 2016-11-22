#include <dcel/barcode.h>
#include <vector>
#include <math/template_point.h>
#include <cassert>
#include <cmath>
#include <numerics.h>
#include "barcode_template.h"

#include "debug.h"
#include "grades.h"

BarTemplate::BarTemplate(unsigned a, unsigned b)
    : begin(a)
    , end(b)
    , multiplicity(1)
{
}

BarTemplate::BarTemplate(unsigned a, unsigned b, unsigned m)
    : begin(a)
    , end(b)
    , multiplicity(m)
{
}

BarTemplate::BarTemplate(const BarTemplate& other)
    : begin(other.begin)
    , end(other.end)
    , multiplicity(other.multiplicity)
{
}

BarTemplate::BarTemplate()
    : begin(0)
    , end(0)
    , multiplicity(0)
{
}

bool BarTemplate::operator<(const BarTemplate other) const
{
    return (begin < other.begin) || (begin == other.begin && end < other.end);
}

BarcodeTemplate::BarcodeTemplate()
    : bars()
{
}

//adds a bar to the barcode (updating multiplicity, if necessary)
void BarcodeTemplate::add_bar(unsigned a, unsigned b)
{
    //look for the bar
    BarTemplate bar(a, b);
    std::set<BarTemplate>::iterator it = bars.find(bar);

    if (it == bars.end()) //then the bar doesn't already exist, so insert it
    {
        bars.insert(bar);
    } else //then the bar already exists, so increment its multiplicity
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

    if (it == bars.end()) //then the bar doesn't already exist, so insert it
    {
        bars.insert(bar);
    } else //then the bar already exists, so increment its multiplicity
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
    debug() << "      barcode template: ";
    for (std::set<BarTemplate>::iterator it = bars.begin(); it != bars.end(); ++it) {
        BarTemplate b = *it;
        debug(true) << "(" << b.begin << "," << b.end << ")x" << b.multiplicity << ", ";
    }
}

bool operator==(BarcodeTemplate const& left, BarcodeTemplate const& right)
{
    return left.bars == right.bars;
}

bool operator==(BarTemplate const& left, BarTemplate const& right)
{
    return left.begin == right.begin
        && left.end == right.end
        && left.multiplicity == right.multiplicity;
}


//rescales a barcode template by projecting points onto the specified line
// NOTE: angle in DEGREES
std::unique_ptr<Barcode> BarcodeTemplate::rescale(double angle, double offset,
                                                  const std::vector<TemplatePoint> &template_points,
                                                  const Grades &grades)
{
    std::unique_ptr<Barcode> bc = std::unique_ptr<Barcode>(new Barcode());

    //loop through bars
    for (std::set<BarTemplate>::iterator it = this->begin(); it != this->end(); ++it) {
//        qDebug() << "BarTemplate: " << it->begin << " " << it->end;
        assert(it->begin < template_points.size());
        TemplatePoint begin = template_points[it->begin];
        double birth = project(begin, angle, offset, grades);

        if (birth != rivet::numeric::INFTY) //then bar exists in this rescaling
        {
            if (it->end >= template_points.size()) //then endpoint is at infinity
            {
                bc->add_bar(birth, rivet::numeric::INFTY, it->multiplicity);
            } else //then bar is finite
            {
                assert(it->end < template_points.size());
                TemplatePoint end = template_points[it->end];
                double death = project(end, angle, offset, grades);
                bc->add_bar(birth, death, it->multiplicity);

//                //testing
//                if (birth > death)
//                    qDebug() << "=====>>>>> ERROR: inverted bar (" << birth << "," << death << ")";
            }
        }
    }

    return bc;
} //end rescale_barcode_template()


//computes the projection of an xi support point onto the specified line
//  NOTE: returns INFTY if the point has no projection (can happen only for horizontal and vertical lines)
//  NOTE: angle in DEGREES
double BarcodeTemplate::project(const TemplatePoint& pt, double angle, double offset, const Grades &grades)
{
    if (angle == 0) //then line is horizontal
    {
        if (grades.y[pt.y] <= offset) //then point is below the line, so projection exists
            return grades.x[pt.x];
        else //then no projection
            return rivet::numeric::INFTY;
    } else if (angle == 90) //then line is vertical
    {
        if (grades.x[pt.x] <= -1 * offset) //then point is left of the line, so projection exists
            return grades.y[pt.y];
        else //then no projection
            return rivet::numeric::INFTY;
    }
    //if we get here, then line is neither horizontal nor vertical
    double radians = angle * rivet::numeric::PI / 180;
    double x = grades.x[pt.x];
    double y = grades.y[pt.y];

    if (y > x * tan(radians) + offset / cos(radians)) //then point is above line
        return y / sin(radians) - offset / tan(radians); //project right

    return x / cos(radians) + offset * tan(radians); //project up
} //end project()


