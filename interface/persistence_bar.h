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

#ifndef PERSISTENCE_BAR_H
#define PERSISTENCE_BAR_H

//forward declarations
struct ConfigParameters;
class SliceDiagram;

//#include <QDebug>
#include <QGraphicsItem>
#include <QPainter>

class PersistenceBar : public QGraphicsItem {
public:
    PersistenceBar(SliceDiagram* s_diagram, ConfigParameters* params, double unscaled_start, double unscaled_end, unsigned index);

    QRectF boundingRect() const;
    QPainterPath shape() const;

    void paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*);

    void set_line(double start_x, double start_y, double end_x, double end_y);
    void set_width(int bar_width);

    void select();
    void select_secondary();
    void deselect();

    double get_start(); //returns the unscaled x-coordinate associated with this bar
    double get_end(); //returns the unscaled y-coordinate associated with this bar
    unsigned get_index(); //returns the index of this bar (e.g. to send to the PersistenceDiagram for highlighting effects)

protected:
    void hoverEnterEvent(QGraphicsSceneHoverEvent*);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent*);
    void mousePressEvent(QGraphicsSceneMouseEvent*);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent*);

private:
    SliceDiagram* sdgm;
    ConfigParameters* config_params;

    double start; //unscaled start coordinate (projection units)
    double end; //unscaled end coordinate (projection units)
    unsigned class_index; //index of the dot corresponding to this bar in the Barcode object

    bool pressed;
    bool secondary;
    bool hover;
    double dx; //horizontal length of line (pixels)
    double dy; //vertical length of line (pixels)
    int width; //width of the bar (pixels)
};

#endif // PERSISTENCE_BAR_H
