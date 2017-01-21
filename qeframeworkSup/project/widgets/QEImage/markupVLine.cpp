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

// This class manages an image markup consisting of a vertical line through the entire image
// Refer to markupItem.h for base functionality and to imageMarkup.h for
// general descrition of image markups.

#include <markupVLine.h>
#include <imageMarkup.h>

markupVLine::markupVLine( imageMarkup* ownerIn, const bool interactiveIn, const bool reportOnMoveIn, const QString legendIn ) : markupItem( ownerIn, OVER_LINE, interactiveIn, reportOnMoveIn, legendIn )
{
    x = 0;
}

void markupVLine::drawMarkup( QPainter& p )
{
    // Scale markup
    double scale = getZoomScale();
    int xScaled = x*scale;
    int height = getImageSize().height();
    int heightScaled = height*scale;

    // Draw markup
    p.drawLine( xScaled, 0, xScaled, heightScaled );

    // If single pixel thickness, draw a single handle in the middle
    if( thickness == 1 )
    {
        //!!! draw the handle in the middle of the existing view, not the entire image
        QRect handle( xScaled-(HANDLE_SIZE/2), (heightScaled/2)-(HANDLE_SIZE/2), HANDLE_SIZE, HANDLE_SIZE );
        p.drawRect( handle );
    }

    // If thickness more than one pixel, draw the thickness borders (dotted lines either side of the main line)
    // and draw two handles, one on each border
    else
    {
        int thicknessScaled = thickness*scale;
        QPen pen = p.pen();
        pen.setStyle( Qt::DashLine );
        p.setPen( pen );
        p.drawLine( xScaled-(thicknessScaled/2), 0, xScaled-(thicknessScaled/2), heightScaled );
        p.drawLine( xScaled+(thicknessScaled/2), 0, xScaled+(thicknessScaled/2), heightScaled );
        pen.setStyle( Qt::SolidLine );
        p.setPen( pen );
    }

    // Draw markup legend
    drawLegend( p, QPoint(xScaled, heightScaled/2) );
}

void markupVLine::setArea()
{
    area.setLeft(  x - (thickness/2) - HANDLE_SIZE/2 );
    area.setRight( x + (thickness/2) + HANDLE_SIZE/2 );

    area.setTop( 0 );
    area.setBottom( getImageSize().height() );

    scalableArea.setLeft(  x - (thickness/2) );
    scalableArea.setRight( x + (thickness/2) );

    scalableArea.setTop( 0 );
    scalableArea.setBottom( getImageSize().height() );

    setLegendOffset( QPoint( 0, 0 ), ABOVE_RIGHT );
    addLegendArea();

    owner->markupAreasStale = true;
}

void markupVLine::startDrawing( const QPoint pos )
{
    x = pos.x();
    activeHandle = MARKUP_HANDLE_NONE;
}

void markupVLine::moveTo( const QPoint posIn )
{
    // Limit position to within the image
    QPoint limPos = limitPointToImage( posIn );

    // Move the appropriate part of the line, according to which bit the user has grabbed
    switch( activeHandle )
    {
        case MARKUP_HANDLE_NONE:
            x = limPos.x();
            break;

        case MARKUP_HANDLE_CENTER:
            thickness = abs( x-limPos.x() )*2+1;
            if( thickness > maxThickness ) thickness = maxThickness;
            break;

        default: break;
    }

    // Update the area the line now occupies
    setArea();
}

bool markupVLine::isOver( const QPoint point, QCursor* cursor )
{
    // If thickness more than one pixel, look for pointer over the main line, or the thickness lines.
    // Note, the thickness lines start life by grabbing the center handle, so the when
    // over any part of the thickness lines, the current handle is the center handle
    if( thickness > 1 )
    {
        // Check of over or near the main line up to the 'over' tolerance, but not past the thickness lines
        int mainLineTolerance = std::min( (unsigned int)OVER_TOLERANCE, thickness/2+1 );

        // If over main line...
        if( abs( point.x() - x ) <= mainLineTolerance )
        {
            activeHandle = MARKUP_HANDLE_NONE;
            *cursor = cursorForHandle( activeHandle );
            return true;
        }

        // If over left edge...
        else if( abs( point.x() - (x - int (thickness/2) )) <= OVER_TOLERANCE )
        {
            activeHandle = MARKUP_HANDLE_CENTER;
            *cursor = cursorForHandle( activeHandle );
            return true;
        }

        // If over right edge...
        else if( abs( point.x() - (x + int (thickness/2) )) <= OVER_TOLERANCE )
        {
            activeHandle = MARKUP_HANDLE_CENTER;
            *cursor = cursorForHandle( activeHandle );
            return true;
        }
    }

    // If thickness of one pixel only, look for pointer over the main line, or the thickness handle
    else
    {
        QPoint handle( x, getImageSize().height()/2 );

        if( pointIsNear( point, handle ))
        {
            activeHandle = MARKUP_HANDLE_CENTER;
            *cursor = cursorForHandle( activeHandle );
            return true;
        }

        // If over any part of the line...
        else if( abs( point.x() - x ) <= OVER_TOLERANCE )
        {
            activeHandle = MARKUP_HANDLE_NONE;
            *cursor = cursorForHandle( activeHandle );
            return true;
        }

    }

    // Not over
    activeHandle = MARKUP_HANDLE_NONE;
    return false;
}

QPoint markupVLine::origin()
{
    return QPoint( x, 0 );
}

// Return the cursor for each handle
QCursor markupVLine::cursorForHandle( const markupItem::markupHandles handle )
{
    switch( handle )
    {
        case MARKUP_HANDLE_CENTER: return Qt::SizeHorCursor;
        case MARKUP_HANDLE_NONE:   return defaultCursor();
        default:                   return defaultCursor();
    }
}

QPoint markupVLine::getPoint1()
{
    return origin();
}

QPoint markupVLine::getPoint2()
{
    return QPoint();
}

QCursor markupVLine::defaultCursor()
{
    return owner->getVLineCursor();
}

void markupVLine::nonInteractiveUpdate( QPoint p1, QPoint )
{
    x = p1.x();
    setArea();
}
