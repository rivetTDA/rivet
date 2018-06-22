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

#ifndef __BARCODE_TEMPLATE_H__
#define __BARCODE_TEMPLATE_H__

#include "barcode.h"
#include "grades.h"
#include "math/template_point.h"
#include <memory>
#include <msgpack.hpp>
#include <set>
#include <vector>
struct BarTemplate {
    unsigned begin; //index of TemplatePointsMatrixEntry of the equivalence class corresponding to the beginning of this bar
    unsigned end; //index of TemplatePointsMatrixEntry of the equivalence class corresponding to the end of this bar
    mutable unsigned multiplicity; //maybe this is bad style, but multiplicity is not involved in comparisons

    BarTemplate(unsigned a, unsigned b);
    BarTemplate(unsigned a, unsigned b, unsigned m);
    BarTemplate(const BarTemplate& other);
    BarTemplate(); // for serialization

    bool operator<(const BarTemplate other) const;

    MSGPACK_DEFINE(begin, end, multiplicity);

    friend bool operator==(BarTemplate const& left, BarTemplate const& right);
};

class BarcodeTemplate {
public:
    BarcodeTemplate();

    void add_bar(unsigned a, unsigned b); //adds a bar to the barcode template (updating multiplicity, if necessary)
    void add_bar(unsigned a, unsigned b, unsigned m); //adds a bar with multiplicity to the barcode template

    std::set<BarTemplate>::iterator begin(); //returns an iterator to the first bar in the barcode
    std::set<BarTemplate>::iterator end(); //returns an iterator to the past-the-end element of the barcode
    bool is_empty(); //returns true iff this barcode has no bars

    //rescales a barcode template by projecting points onto the line specificed by angle and offset
    //  NOTE: parametrization of the line is as in the RIVET paper
    //  NOTE: angle in DEGREES
    std::unique_ptr<Barcode> rescale(double angle, double offset,
        const std::vector<TemplatePoint>& template_points,
        const Grades& grades);

    //computes the projection of an xi support point onto the line specificed by angle and offset
    //  NOTE: parametrization of the line is as in the RIVET paper
    //  NOTE: returns INFTY if the point has no projection (can happen only for horizontal and vertical lines)
    //  NOTE: angle in DEGREES
    double project(const TemplatePoint& pt, double angle, double offset, const Grades& grades);

    void print(); //for testing only

    MSGPACK_DEFINE(bars);
    friend bool operator==(BarcodeTemplate const& left, BarcodeTemplate const& right);

private:
    std::set<BarTemplate> bars;
};

#endif // __BARCODE_TEMPLATE_H__
