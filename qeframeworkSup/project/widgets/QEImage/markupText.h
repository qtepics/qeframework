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
 This class manages a 'test' markup.
 The markup simply contains some text. The user cannot interact with it.
 It is used to present a time stamp, but could be used for any text markups.
*/

#ifndef MARKUPTEXT_H
#define MARKUPTEXT_H

#include <markupItem.h>

#include <QDebug>

class imageMarkup;

// Text markup. Used to add some text to an image, such as a timestamp
class markupText : public markupItem
{
public:

    markupText( imageMarkup* ownerIn, const bool interactiveIn, const bool reportOnMoveIn, const QString legendIn );

    void setText( QString textIn );

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
    //==================================================================

private:
    QString text;   // Text displayed
    QRect rect;     // Area of the text
};

#endif // MARKUPTEXT_H
