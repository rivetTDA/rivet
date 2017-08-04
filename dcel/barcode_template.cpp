/**********************************************************************
Copyright 2014-2016 The RIVET Developers. See the COPYRIGHT file at
the top-level directory of this distribution.

This file is part of RIVET.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
**********************************************************************/

#include "barcode_template.h"
#include <cassert>
#include <cmath>
#include <dcel/barcode.h>
#include <map>
#include <math/template_point.h>
#include <numerics.h>
#include <vector>

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

//rescales a barcode template by projecting points onto the line specificed by angle and offset
// NOTE: parametrization of the line is as in the RIVET paper
// NOTE: angle in DEGREES
std::unique_ptr<Barcode> BarcodeTemplate::rescale(double angle, double offset,
    const std::vector<TemplatePoint>& template_points,
    const Grades& grades)
{
    std::unique_ptr<Barcode> bc = std::unique_ptr<Barcode>(new Barcode());

    std::map<unsigned, unsigned> infinite_bars; //used for combining infinite bars (only necessary for vertical or horizontal lines)

    //loop through bars
    for (std::set<BarTemplate>::iterator it = this->begin(); it != this->end(); ++it) {
        assert(it->begin < template_points.size());
        TemplatePoint begin = template_points[it->begin];
        double birth = project(begin, angle, offset, grades);

        if (birth != rivet::numeric::INFTY) { //then bar exists in this rescaling
            if (it->end >= template_points.size()) { //then endpoint is at infinity
                if (angle == 0 || angle == 90) { //then add bar to the list of infinite bars, since we may need to combine bars
                    std::map<unsigned, unsigned>::iterator ibit = infinite_bars.find(it->begin);
                    if (ibit == infinite_bars.end()) { //add a new item
                        infinite_bars.insert(std::pair<unsigned, unsigned>(it->begin, it->multiplicity));
                    } else { //increment the multiplicity
                        ibit->second += it->multiplicity;
                    }
                } else { //then add the bar to the barcode -- no combining will be necessary
                    bc->add_bar(birth, rivet::numeric::INFTY, it->multiplicity);
                }
            } else { //then compute endpoint of bar (may still be infinite, but only for for horizontal or vertical lines)
                assert(it->end < template_points.size());
                TemplatePoint end = template_points[it->end];
                double death = project(end, angle, offset, grades);
                if (death == rivet::numeric::INFTY) { //add bar to the list of infinite bars
                    std::map<unsigned, unsigned>::iterator ibit = infinite_bars.find(it->begin);
                    if (ibit == infinite_bars.end()) { //add a new item
                        infinite_bars.insert(std::pair<unsigned, unsigned>(it->begin, it->multiplicity));
                    } else { //increment the multiplicity
                        ibit->second += it->multiplicity;
                    }
                } else { //then add (finite) bar to the barcode
                    bc->add_bar(birth, death, it->multiplicity);
                }
            }
        }
    }

    //if the line is vertical or horizontal, we must now add the infinite bars
    if (angle == 0 || angle == 90) {
        for (std::map<unsigned, unsigned>::iterator it = infinite_bars.begin(); it != infinite_bars.end(); ++it) {
            double birth = project(template_points[it->first], angle, offset, grades);
            bc->add_bar(birth, rivet::numeric::INFTY, it->second);
        }
    }

    return bc;
} //end rescale_barcode_template()

//computes the projection of an xi support point onto the line specificed by angle and offset
//  NOTE: parametrization of the line is as in the RIVET paper
//  NOTE: returns INFTY if the point has no projection (can happen only for horizontal and vertical lines)
//  NOTE: angle in DEGREES
double BarcodeTemplate::project(const TemplatePoint& pt, double angle, double offset, const Grades& grades)
{
    if (angle == 0) { //then line is horizontal
        if (grades.y[pt.y] <= offset) { //then point is below the line, so projection exists
            return grades.x[pt.x];
        } //else: no projection
        return rivet::numeric::INFTY;
    } else if (angle == 90) { //then line is vertical
        if (grades.x[pt.x] <= -1 * offset) { //then point is left of the line, so projection exists
            return grades.y[pt.y];
        } //else: no projection
        return rivet::numeric::INFTY;
    }
    
    //if we get here, then line is neither horizontal nor vertical
    double radians = angle * rivet::numeric::PI / 180;
    double x = grades.x[pt.x];
    double y = grades.y[pt.y];

    double yL = x * tan(radians) + offset / cos(radians); // the point (x, yL) is on the line

    if (y >= yL) { //then point is above line, so project to the right
        if (offset >= 0) {
            return (y * cos(radians) - offset) / (sin(radians) * cos(radians));
        } //else
        return y / sin(radians);
    } //else: point is below the line, so project up
    if (offset >= 0) {
        return x / cos(radians);
    } //else 
    return yL / sin(radians);

} //end project()
