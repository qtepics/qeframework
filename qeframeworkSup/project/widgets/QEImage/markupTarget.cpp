/*  markupTarget.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

// This class manages an image markup consisting of a dotted crosshair across the entire image.
// Refer to markupItem.h for base functionality and to imageMarkup.h for
// general descrition of image markups.

#include <markupTarget.h>
#include <imageMarkup.h>

markupCrosshair1::markupCrosshair1( imageMarkup* ownerIn, const bool interactiveIn, const bool reportOnMoveIn, const QString legendIn ) : markupItem( ownerIn, OVER_LINE, interactiveIn, reportOnMoveIn, legendIn )
{
}

void markupCrosshair1::drawMarkup( QPainter& p )
{
    // Scale markup
    double scale = getZoomScale();
    QPoint scaledPos = QPoint( pos.x() * scale, pos.y() * scale );

    // Draw markup
    QPen pen = p.pen();
    pen.setStyle( Qt::DashLine );
    p.setPen( pen );
    p.drawLine( scaledPos.x(), 0, scaledPos.x(), getImageSize().height() );
    p.drawLine( 0, scaledPos.y(), getImageSize().width(), scaledPos.y() );
    pen.setStyle( Qt::SolidLine );
    p.setPen( pen );

    // Draw markup legend
    drawLegend( p, scaledPos );
}

void markupCrosshair1::setArea()
{
    area = QRect( QPoint(0,0), getImageSize() );

    scalableArea = area;

    setLegendOffset( QPoint( 0, 0 ), ABOVE_RIGHT );
    addLegendArea();

    owner->markupAreasStale = true;
}

void markupCrosshair1::startDrawing( const QPoint posIn )
{
    pos = posIn;
    activeHandle = MARKUP_HANDLE_NONE;
}

void markupCrosshair1::moveTo( const QPoint posIn )
{
    // Limit position to within the image
    QPoint limPos = limitPointToImage( posIn );

    pos = limPos;

    // Update the crosshair now occupies
    setArea();
}

bool markupCrosshair1::isOver( const QPoint point, QCursor* cursor )
{
    *cursor = owner->getCircleCursor();
    activeHandle = MARKUP_HANDLE_NONE;
    return ( abs( point.x() - pos.x() ) <= OVER_TOLERANCE ) || ( abs( point.y() - pos.y() ) <= OVER_TOLERANCE );
}

QPoint markupCrosshair1::origin()
{
    return pos;
}

// Return the cursor for each handle
QCursor markupCrosshair1::cursorForHandle( const markupItem::markupHandles )
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

QPoint markupCrosshair1::getPoint1()
{
    return origin();
}

QPoint markupCrosshair1::getPoint2()
{
    return QPoint();
}

QCursor markupCrosshair1::defaultCursor()
{
    return owner->getTargetCursor();
}

void markupCrosshair1::nonInteractiveUpdate( QPoint p1, QPoint, double )
{
    pos = p1;
    setArea();
}
