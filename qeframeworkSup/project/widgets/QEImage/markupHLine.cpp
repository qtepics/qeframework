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

// This class manages an image markup consisting of a horizontal line through the entire image.
// Refer to markupItem.h for base functionality and to imageMarkup.h for
// general descrition of image markups.

#include <markupHLine.h>
#include <imageMarkup.h>

markupHLine::markupHLine( imageMarkup* ownerIn, const bool interactiveIn, const bool reportOnMoveIn, const QString legendIn ) : markupItem( ownerIn, OVER_LINE, interactiveIn, reportOnMoveIn, legendIn )
{
    y = 0;
}

void markupHLine::drawMarkup( QPainter& p )
{
    // Scale markup
    double scale = getZoomScale();
    int yScaled = y*scale;
    int width = getImageSize().width();
    int widthScaled = width*scale;

    // Draw markup
    p.drawLine( 0, yScaled, widthScaled, yScaled );

    // If single pixel thickness, draw a single handle in the middle
    if( thickness == 1 )
    {
        //!!! draw the handle in the middle of the existing view, not the entire image
        QRect handle( (widthScaled/2)-(HANDLE_SIZE/2), (yScaled)-(HANDLE_SIZE/2), HANDLE_SIZE, HANDLE_SIZE );
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
        p.drawLine( 0, yScaled-(thicknessScaled/2), widthScaled, yScaled-(thicknessScaled/2) );
        p.drawLine( 0, yScaled+(thicknessScaled/2), widthScaled, yScaled+(thicknessScaled/2) );
        pen.setStyle( Qt::SolidLine );
        p.setPen( pen );
    }

    // Draw markup legend
    drawLegend( p, QPoint( widthScaled/2, yScaled ) );
}

void markupHLine::setArea()
{
    area.setTop(    y - (thickness/2) - HANDLE_SIZE/2);
    area.setBottom( y + (thickness/2) + HANDLE_SIZE/2 );

    area.setLeft( 0 );
    area.setRight( getImageSize().width() );

    scalableArea.setTop(    y - (thickness/2) );
    scalableArea.setBottom( y + (thickness/2) );

    scalableArea.setLeft( 0 );
    scalableArea.setRight( getImageSize().width() );

    setLegendOffset( QPoint( 0, 0 ), ABOVE_RIGHT );
    addLegendArea();

    owner->markupAreasStale = true;
}

void markupHLine::startDrawing( const QPoint pos )
{
    y = pos.y();
    activeHandle = MARKUP_HANDLE_NONE;
}

void markupHLine::moveTo( const QPoint posIn )
{
    // Limit position to within the image
    QPoint limPos = limitPointToImage( posIn );

    // Move the appropriate part of the line, according to which bit the user has grabbed
    switch( activeHandle )
    {
        case MARKUP_HANDLE_NONE:
            y = limPos.y();
            break;

        case MARKUP_HANDLE_CENTER:
            thickness = abs( y-limPos.y() )*2+1;
            if( thickness > maxThickness ) thickness = maxThickness;
            break;

        default: break;
    }

    // Update the area the line now occupies
    setArea();
}

bool markupHLine::isOver( const QPoint point, QCursor* cursor )
{
    // If thickness more than one pixel, look for pointer over the main line, or the thickness lines.
    // Note, the thickness lines start life by grabbing the center handle, so the when
    // over any part of the thickness lines, the current handle is the center handle
    if( thickness > 1 )
    {
        // Check of over or near the main line up to the 'over' tolerance, but not past the thickness lines
        int mainLineTolerance = std::min( (unsigned int)OVER_TOLERANCE, thickness/2+1 );

        // If over main line...
        if( abs( point.y() - y ) <= mainLineTolerance )
        {
            activeHandle = MARKUP_HANDLE_NONE;
            *cursor = cursorForHandle( activeHandle );
            return true;
        }

        // If over top edge...
        else if( abs( point.y() - (y - int (thickness/2) )) <= OVER_TOLERANCE )
        {
            activeHandle = MARKUP_HANDLE_CENTER;
            *cursor = cursorForHandle( activeHandle );
            return true;
        }

        // If over bottom edge...
        else if( abs( point.y() - (y + int (thickness/2) )) <= OVER_TOLERANCE )
        {
            activeHandle = MARKUP_HANDLE_CENTER;
            *cursor = cursorForHandle( activeHandle );
            return true;
        }
    }

    // If thickness of one pixel only, look for pointer over the main line, or the thickness handle
    else
    {
        QPoint handle( getImageSize().width()/2, y );

        if( pointIsNear( point, handle ))
        {
            activeHandle = MARKUP_HANDLE_CENTER;
            *cursor = cursorForHandle( activeHandle );
            return true;
        }

        // If over any part of the line...
        else if( abs( point.y() - y ) <= OVER_TOLERANCE )
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

QPoint markupHLine::origin()
{
    return QPoint( 0, y );
}

// Return the cursor for each handle
QCursor markupHLine::cursorForHandle( const markupItem::markupHandles handle )
{
    switch( handle )
    {
        case MARKUP_HANDLE_CENTER: return Qt::SizeVerCursor;
        case MARKUP_HANDLE_NONE:   return defaultCursor();
        default:                   return defaultCursor();
    }
}

QPoint markupHLine::getPoint1()
{
    return origin();
}

QPoint markupHLine::getPoint2()
{
    return QPoint();
}

QCursor markupHLine::defaultCursor()
{
    return owner->getHLineCursor();
}

void markupHLine::nonInteractiveUpdate( QPoint p1, QPoint )
{
    y = p1.y();
    setArea();
}
