/**********************************************************************
Copyright 2014-2016 The RIVET Devlopers. See the COPYRIGHT file at
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

#ifndef TEMPLATE_POINT_H
#define TEMPLATE_POINT_H

//class to store xi points, to help send data from the Arrangement to the VisualizationWindow
class TemplatePoint {
public:
    unsigned x, y; //coordinates (discrete)
    int zero, one, two; //multiplicities of xi_0, xi_1, and xi_2 at this point ---- TODO: maybe should be unsigned?

    TemplatePoint(unsigned xc, unsigned yc, int m0, int m1, int m2);
    TemplatePoint(); //for serialization

    friend bool operator==(TemplatePoint const& left, TemplatePoint const& right);

    template <class Archive>
    void serialize(Archive& archive, const unsigned int /*version*/)
    {
        archive& x& y& zero& one& two;
    }
};

#endif // TEMPLATE_POINT_H
