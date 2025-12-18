/*  markupBeam.cpp
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

// This class manages an image markup consisting of a small solid crosshair.
// Refer to markupItem.h for base functionality and to imageMarkup.h for
// general descrition of image markups.

#include <markupBeam.h>
#include <imageMarkup.h>

markupCrosshair2::markupCrosshair2( imageMarkup* ownerIn, const bool interactiveIn, const bool reportOnMoveIn, const QString legendIn ) : markupItem( ownerIn, OVER_LINE, interactiveIn, reportOnMoveIn, legendIn )
{
    // Size of cross hair
    armSize = 20;
}

void markupCrosshair2::drawMarkup( QPainter& p )
{
    // Scale markup
    double scale = getZoomScale();
    QPoint scaledPos = QPoint( pos.x() * scale, pos.y() * scale );

    // Draw markup
    p.drawLine( scaledPos.x()-1, scaledPos.y()+1, scaledPos.x()-1,       scaledPos.y()+armSize );
    p.drawLine( scaledPos.x()+1, scaledPos.y()+1, scaledPos.x()+1,       scaledPos.y()+armSize );

    p.drawLine( scaledPos.x()-1, scaledPos.y()-1, scaledPos.x()-1,       scaledPos.y()-armSize );
    p.drawLine( scaledPos.x()+1, scaledPos.y()-1, scaledPos.x()+1,       scaledPos.y()-armSize );

    p.drawLine( scaledPos.x()+1, scaledPos.y()-1, scaledPos.x()+armSize, scaledPos.y()-1 );
    p.drawLine( scaledPos.x()+1, scaledPos.y()+1, scaledPos.x()+armSize, scaledPos.y()+1 );

    p.drawLine( scaledPos.x()-1, scaledPos.y()-1, scaledPos.x()-armSize, scaledPos.y()-1 );
    p.drawLine( scaledPos.x()-1, scaledPos.y()+1, scaledPos.x()-armSize, scaledPos.y()+1 );

    // Draw markup legend
    drawLegend( p, scaledPos );
}

void markupCrosshair2::setArea()
{
    area = QRect( pos.x()-armSize, pos.y()-armSize, armSize*2+1, armSize*2+1 );
    scalableArea = QRect( pos.x(), pos.y(), 0, 0 );

//    area.setLeft  ( pos.x()-armSize );
//    area.setRight ( pos.x()+armSize );
//    area.setTop   ( pos.y()-armSize );
//    area.setBottom( pos.y()+armSize );

    setLegendOffset( QPoint( 0, 0 ), BELOW_LEFT );
    addLegendArea();

    owner->markupAreasStale = true;
}

void markupCrosshair2::startDrawing( const QPoint posIn )
{
    pos = posIn;
    activeHandle = MARKUP_HANDLE_NONE;
}

void markupCrosshair2::moveTo( const QPoint posIn )
{
    // Limit position to within the image
    QPoint limPos = limitPointToImage( posIn );

    pos = limPos;

    // Update the crosshair now occupies
    setArea();
}

bool markupCrosshair2::isOver( const QPoint point, QCursor* cursor )
{
    *cursor = owner->getCircleCursor();
    activeHandle = MARKUP_HANDLE_NONE;
    return ((( abs( point.x() - pos.x() ) <= OVER_TOLERANCE ) &&
             ( abs( point.y() - pos.y() ) <= (armSize+OVER_TOLERANCE) )) ||
            (( abs( point.y() - pos.y() ) <= OVER_TOLERANCE ) &&
             ( abs( point.x() - pos.x() ) <= (armSize+OVER_TOLERANCE) )));
}

QPoint markupCrosshair2::origin()
{
    return pos;
}

// Return the cursor for each handle
QCursor markupCrosshair2::cursorForHandle( const markupItem::markupHandles )
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

QPoint markupCrosshair2::getPoint1()
{
    return origin();
}

QPoint markupCrosshair2::getPoint2()
{
    return QPoint();
}

QCursor markupCrosshair2::defaultCursor()
{
    return owner->getTargetCursor();
}

void markupCrosshair2::nonInteractiveUpdate( QPoint p1, QPoint, double )
{
    pos = p1;
    setArea();
}
