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
 This class manages an arbitrary 'line' markup.
 The markup is drawn as a line anywhere in the image. Either end, or the entire line, can be dragged to any location on the image.
 The markup has a handle at each end to allow the user to drag one end only, and a handle in the center which can be used to drag the line thickness markers.
 The line thickness markers are dashed lines either side of the main marker line.
 This markup is used in the QEImage widget to identify an arbitrary series of pixels to generate a profile plot.
*/

#ifndef MARKUPLINE_H
#define MARKUPLINE_H

#include <markupItem.h>

#include <QDebug>

class imageMarkup;

// Line markup used to select an arbiraty angle slice through an image
class markupLine : public markupItem
{
public:
    markupLine( imageMarkup* ownerIn, const bool interactiveIn, const bool reportOnMoveIn, const QString legendIn );

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

    void nonInteractiveUpdate( QPoint p1, QPoint p2 );
    //==================================================================

private:
    QPoint start;
    QPoint end;
    bool isOverLine( const QPoint point, const QPoint lineStart, const QPoint lineEnd );
};


#endif // MARKUPLINE_H
