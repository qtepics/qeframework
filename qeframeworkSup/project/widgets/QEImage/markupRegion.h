/*
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The EPICS QT Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Copyright (c) 2012 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
 This class manages a 'region' markup.
 The markup is drawn as a rectangle anywhere in the image. Individual corners, individual sides, or the entire rectangle, can be dragged to any location on the image.
 The markup has a handle on each corner and each side.
 This markup is used in the QEImage widget to identify areas of the image, typically to interact with region or interest variables.
*/

#ifndef MARKUPREGION_H
#define MARKUPREGION_H

#include <markupItem.h>

#include <QDebug>

class imageMarkup;

// Region markup used to select a rectangular region of an image. Used to select an area detector region, or a scan region
class markupRegion : public markupItem
{
public:

    markupRegion( imageMarkup* ownerIn, const bool interactiveIn, const bool reportOnMoveIn, const QString legendIn );

    //==================================================================
    // Implement base class functions - see markupItem class defition for a description of each of these methods
    void startDrawing( const QPoint pos );
    void setArea();
    void drawMarkup( QPainter& p );
    void moveTo( const QPoint pos );  // Move an item (always make it visible and highlighed)
    bool isOver( const QPoint point, QCursor* cursor );
    QPoint origin();
    QCursor cursorForHandle( const markupItem::markupHandles handle );
    QPoint getPoint1();
    QPoint getPoint2();
    QCursor defaultCursor();

    void nonInteractiveUpdate(  QPoint p1, QPoint p2, double rotation );
    //==================================================================


private:
    QRect rect;
};

#endif // MARKUPREGION_H
