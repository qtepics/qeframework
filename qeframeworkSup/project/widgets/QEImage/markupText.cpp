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
 *  Copyright (c) 2012, 2013 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

// This class manages an image markup consisting of some text.
// Refer to markupItem.h for base functionality and to imageMarkup.h for
// general descrition of image markups.

#include <markupText.h>
#include <imageMarkup.h>

markupText::markupText( imageMarkup* ownerIn, const bool interactiveIn, const bool reportOnMoveIn, const QString legendIn ) : markupItem( ownerIn, OVER_AREA, interactiveIn, reportOnMoveIn, legendIn )
{
}

void markupText::drawMarkup( QPainter& p )
{
    // Scale markup
    QRect scaledRect = rect;
    // Text is not scaled
    //double scale = getZoomScale();
    //scaledRect.moveTo( rect.x() * scale, rect.y() * scale );

    //scaledRect.setWidth( rect.width() * scale );
    //scaledRect.setHeight( rect.height() * scale );

    // Draw markup

    // Draw the text
    p.setFont( owner->legendFont );
    p.drawText( scaledRect, Qt::AlignLeft, text, &scaledRect );

    // Draw markup legend
    // never a legend for text drawLegend( p, ???, ABOVE_RIGHT );

}

void markupText::setText( QString textIn )
{
    text = textIn;

    // Update the area to accommodate the new text
    setArea();
}

void markupText::setArea()
{
    // Set the area to more than enough.
    // This will be trimmed to the bounding retangle of the text
    QSize textSize = owner->legendFontMetrics->size( Qt::TextSingleLine, text );

    rect = QRect( QPoint( 0, 0 ), textSize );

    area = rect;

    scalableArea = QRect( QPoint( 0, 0 ), QPoint( 0, 0 ) );

    //    setLegendPos( rect.topLeft(), ABOVE_RIGHT );  // never draw a legend for text markup
    addLegendArea();

    owner->markupAreasStale = true;

}

void markupText::startDrawing( const QPoint pos )
{
    rect.setBottomLeft( pos );
    rect.setTopRight( pos + QPoint( 50,30 ) );
    activeHandle = MARKUP_HANDLE_NONE;
}

void markupText::moveTo( const QPoint posIn )
{
    // Limit position to within the image
    QPoint limPos = limitPointToImage( posIn );

    rect.translate( limPos - owner->grabOffset );

    setArea();
}

bool markupText::isOver( const QPoint point, QCursor* cursor )
{
    *cursor = Qt::OpenHandCursor;
    activeHandle = MARKUP_HANDLE_NONE;
    return rect.contains( point );
}

QPoint markupText::origin()
{
    return rect.topLeft();
}

// Return the cursor for each handle
QCursor markupText::cursorForHandle( const markupItem::markupHandles )
{
// No special cursors for different handles
//    switch( handle )
//    {
//        case MARKUP_HANDLE_TL:   return Qt::SizeFDiagCursor;
//        case MARKUP_HANDLE_BL:   return Qt::SizeBDiagCursor;
//        ...
//        default: return defaultCursor();
//    }

    return defaultCursor();
}

QPoint markupText::getPoint1()
{
    return rect.topLeft();
}

QPoint markupText::getPoint2()
{
    return rect.bottomRight();
}

QCursor markupText::defaultCursor()
{
    return Qt::CrossCursor;
}
