/*  markupText.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

// This class manages an image markup consisting of some text.
// Refer to markupItem.h for base functionality and to imageMarkup.h for
// general descrition of image markups.

#include <markupText.h>
#include <imageMarkup.h>

#define DEBUG qDebug() << "markupText"  << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
markupText::markupText( imageMarkup* ownerIn, const bool interactiveIn,
                        const bool reportOnMoveIn, const QString legendIn ) :
   markupItem( ownerIn, OVER_AREA, interactiveIn, reportOnMoveIn, legendIn )
{
}

//------------------------------------------------------------------------------
//
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

    QBrush brush;
    brush.setStyle( Qt::SolidPattern);
    brush.setColor( QColor (0, 0, 0, 96) );
    p.setBrush( brush );

    const QPen savedPen = p.pen();

    QPen pen;
    pen.setStyle( Qt::NoPen );
    p.setPen( pen );

    // Draw backgound for text
    //
    p.drawRect(scaledRect );

    // Draw the text
    p.setPen( savedPen );
    p.setFont( owner->legendFont );
    p.drawText( scaledRect, Qt::AlignLeft, text );

    // Draw markup legend
    // never a legend for text drawLegend( p, ???, ABOVE_RIGHT );
}

//------------------------------------------------------------------------------
//
void markupText::setText( QString textIn )
{
    text = QString(" %1 ").arg( textIn );

    // Update the area to accommodate the new text
    setArea();
}

//------------------------------------------------------------------------------
//
void markupText::setArea()
{
    // Set the area to more than enough.
    // This will be trimmed to the bounding retangle of the text
    //
    QSize textSize = owner->legendFontMetrics->size( Qt::TextSingleLine, text );

    rect = QRect( QPoint( 0, 0 ), textSize );

    area = rect;

    scalableArea = QRect( QPoint( 0, 0 ), QPoint( 0, 0 ) );

    //    setLegendPos( rect.topLeft(), ABOVE_RIGHT );  // never draw a legend for text markup
    addLegendArea();

    owner->markupAreasStale = true;

}

//------------------------------------------------------------------------------
//
void markupText::startDrawing( const QPoint pos )
{
    rect.setBottomLeft( pos );
    rect.setTopRight( pos + QPoint( 50,30 ) );
    activeHandle = MARKUP_HANDLE_NONE;
}

//------------------------------------------------------------------------------
//
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

//------------------------------------------------------------------------------
//
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

//------------------------------------------------------------------------------
//
QPoint markupText::getPoint1()
{
    return rect.topLeft();
}

//------------------------------------------------------------------------------
//
QPoint markupText::getPoint2()
{
    return rect.bottomRight();
}

//------------------------------------------------------------------------------
//
QCursor markupText::defaultCursor()
{
    return Qt::CrossCursor;
}

// end
