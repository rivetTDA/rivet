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

#ifndef CONFIG_PARAMETERS_H
#define CONFIG_PARAMETERS_H

#include <QColor>
#include <QFont>

//these parameters control the visualization and are (mostly) user-customizable through the Configure dialog box
struct ConfigParameters {
    //colors
    QColor xi0color;
    QColor xi1color;
    QColor xi2color;
    QColor persistenceColor;
    QColor persistenceHighlightColor;
    QColor sliceLineColor;
    QColor sliceLineHighlightColor;

    //sizes
    int bettiDotRadius;
    int persistenceDotRadius;
    bool autoDotSize;
    int sliceLineWidth;
    int persistenceBarWidth;
    int persistenceBarSpace;

    //fonts
    QFont diagramFont;

    //constructor
    ConfigParameters();
};

#endif // CONFIG_PARAMETERS_H
