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

#include "config_parameters.h"

//default values are set here
ConfigParameters::ConfigParameters()
    : xi0color(0, 255, 0, 100) //green semi-transparent, for xi_0 support dots
    , xi1color(255, 0, 0, 100) //red semi-transparent, for xi_1 support dots
    , xi2color(255, 255, 0, 100) //yellow semi-transparent, for xi_2 support dots
    , persistenceColor(160, 0, 200, 127) //purple semi-transparent, for persistence bars and dots
    , persistenceHighlightColor(255, 140, 0, 150) //orange semi-transparent, for highlighting part of the slice line
    , sliceLineColor(0, 0, 255, 150) //blue semi-transparent, for slice line
    , sliceLineHighlightColor(0, 200, 200, 150) //cyan semi-transparent, for highlighting the slice line on click-and-drag
    , bettiDotRadius(5) //radius of dot representing xi_0 = 1 or xi_1 = 1
    , persistenceDotRadius(5) //radius of dot representing one homology class in persistence diagram
    , autoDotSize(true) //automatic dot sizing is initially on
    , sliceLineWidth(5) //width of selected line (pixels)
    , persistenceBarWidth(4) //width of persistence bars (pixels)
    , persistenceBarSpace(5) //space between persistence bars(pixels)
    , diagramFont() //system default font
{
    diagramFont.setPointSize(12); // SET DEFAULT FONT SIZE HERE
}
