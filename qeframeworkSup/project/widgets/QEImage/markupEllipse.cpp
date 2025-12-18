/*  markupEllipse.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2014-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

// This class manages an image markup consisting of a 'ellipse' rectangle.
// Refer to markupItem.h for base functionality and to imageMarkup.h for
// general descrition of image markups.

#include <markupEllipse.h>
#include <imageMarkup.h>
#include <QDebug>

#define DEBUG qDebug() << "markupEllipse"  << __LINE__ << __FUNCTION__ << "  "

markupEllipse::markupEllipse( imageMarkup* ownerIn, const bool interactiveIn,
                              const bool reportOnMoveIn, const QString legendIn )
    : markupItem( ownerIn, OVER_AREA, interactiveIn, reportOnMoveIn, legendIn )
{
    rotation = 0.0;
}

void markupEllipse::drawMarkup( QPainter& p )
{
    // Scale markup
    QRect scaledRect = rect;
    double scale = getZoomScale();
    scaledRect.moveTo( rect.x() * scale, rect.y() * scale );

    scaledRect.setWidth( rect.width() * scale );
    scaledRect.setHeight( rect.height() * scale );

    // Setup markup rotation transform
    const QPointF c = scaledRect.center();
    const QTransform transform =
            QTransform()
            .translate (+c.x(),+c.y())
            .rotate(rotation)
            .translate (-c.x(),-c.y());

    p.setTransform( transform );

    // Draw markup with rotation
    p.drawEllipse( scaledRect );

    p.resetTransform();

    // Draw markup legend
    drawLegend( p, scaledRect.topLeft() );
}

void markupEllipse::setArea()
{
    area = rect;
    scalableArea = area;

    // Sanity check - rect should never be non-normallized.
    // Note, drawing a non normalized QRect and a normalized QRect will not draw the same pixels!
    if( rect.width() < 0 || rect.height() < 0 )
    {
        qDebug() << "Error, markupEllipse::setArea() rect has negative dimensions" << rect;
    }

    setLegendOffset( QPoint( 0, 0 ), ABOVE_RIGHT );
    addLegendArea();

    owner->markupAreasStale = true;
}

void markupEllipse::startDrawing( const QPoint pos )
{
    rect.setBottomLeft( pos );
    rect.setTopRight( pos );
    activeHandle = MARKUP_HANDLE_NONE;
}

void markupEllipse::moveTo( const QPoint posIn )
{
    // Limit position to within the image
    QPoint limPos = limitPointToImage( posIn );

    rect.translate( limPos - owner->grabOffset );

    // Update the area the ellipse now occupies
    setArea();
}

bool markupEllipse::isOver( const QPoint point, QCursor* cursor )
{
    *cursor = Qt::OpenHandCursor;
    activeHandle = MARKUP_HANDLE_NONE;
    return rect.contains( point );
}

QPoint markupEllipse::origin()
{
    return rect.topLeft();
}

// Return the cursor for each handle
QCursor markupEllipse::cursorForHandle( const markupItem::markupHandles /*handle*/ )
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

QPoint markupEllipse::getPoint1()
{
    return rect.topLeft();
}

QPoint markupEllipse::getPoint2()
{
    return rect.bottomRight();
}

QCursor markupEllipse::defaultCursor()
{
    return Qt::CrossCursor;
}

void markupEllipse::nonInteractiveUpdate( QPoint p1, QPoint p2, double rotationIn )
{
    rect.setTopLeft( p1 );
    rect.setBottomRight( p2 );
    rotation = rotationIn;
    setArea();
}

// end
