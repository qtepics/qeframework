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

// This class manages an image markup consisting of a 'region' rectangle.
// Refer to markupItem.h for base functionality and to imageMarkup.h for
// general descrition of image markups.

#include <markupRegion.h>
#include <imageMarkup.h>

markupRegion::markupRegion( imageMarkup* ownerIn, const bool interactiveIn, const bool reportOnMoveIn, const QString legendIn ) : markupItem( ownerIn, OVER_AREA, interactiveIn, reportOnMoveIn, legendIn )
{
}

void markupRegion::drawMarkup( QPainter& p )
{
    // Scale markup
    QRect scaledRect = rect;
    double scale = getZoomScale();
    scaledRect.moveTo( rect.x() * scale, rect.y() * scale );

    scaledRect.setWidth( rect.width() * scale );
    scaledRect.setHeight( rect.height() * scale );

    // Draw markup
    p.drawRect( scaledRect );

    if(( abs(rect.size().width())  > (HANDLE_SIZE + 2) ) ||
       ( abs(rect.size().height()) > (HANDLE_SIZE + 2) ))
    {
        QRect handle( 0, 0, HANDLE_SIZE, HANDLE_SIZE );
        QPoint halfHandle( HANDLE_SIZE/2, HANDLE_SIZE/2 );

        handle.moveTo( scaledRect.topLeft() - halfHandle );
        p.drawRect( handle );

        handle.moveTo( scaledRect.topRight() - halfHandle );
        p.drawRect( handle );

        handle.moveTo( scaledRect.bottomLeft() - halfHandle );
        p.drawRect( handle );

        handle.moveTo( scaledRect.bottomRight() - halfHandle );
        p.drawRect( handle );

        handle.moveTo( QPoint( scaledRect.left(), scaledRect.top()+scaledRect.height()/2 ) - halfHandle );
        p.drawRect( handle );

        handle.moveTo( QPoint( scaledRect.right(), scaledRect.top()+scaledRect.height()/2 ) - halfHandle );
        p.drawRect( handle );

        handle.moveTo( QPoint( scaledRect.left()+scaledRect.width()/2, scaledRect.top() ) - halfHandle );
        p.drawRect( handle );

        handle.moveTo( QPoint( scaledRect.left()+scaledRect.width()/2, scaledRect.bottom() ) - halfHandle );
        p.drawRect( handle );
    }

    // Draw markup legend
    drawLegend( p, scaledRect.topLeft() );
}

void markupRegion::setArea()
{
    // Sanity check - rect should never be non-normallized.
    // Note, drawing a non normalized QRect and a normalized QRect will not draw the same pixels!
    if( rect.width() < 0 || rect.height() < 0 )
    {
        qDebug() << "Error, markupRegion::setArea() rect has negative dimensions" << rect;
    }

    // Set the scalable area of the markup (the core rectangle minus handles and legend)
    scalableArea = rect;

    // Set the overall area of the markup
    //  - add the legend to the core area
    area = rect;
    setLegendOffset( QPoint( 0, 0 ), ABOVE_RIGHT );
    addLegendArea();

    //  - add the handles to the the core area
    QRect baseArea = rect;
    baseArea.adjust( -HANDLE_SIZE, -HANDLE_SIZE, HANDLE_SIZE+1, HANDLE_SIZE+1 );

    //  - combine the core area with legend and the core area with handles
    area = area.united( baseArea );

    owner->markupAreasStale = true;
}

void markupRegion::startDrawing( const QPoint pos )
{
    rect.setBottomLeft( pos );
    rect.setTopRight( pos );
    activeHandle = MARKUP_HANDLE_BR;
}

void markupRegion::moveTo( const QPoint posIn )
{
    // Limit position to within the image
    QPoint limPos = limitPointToImage( posIn );

    // Move the appropriate part of the region, according to which bit the user has grabbed
    switch( activeHandle )
    {
        case MARKUP_HANDLE_NONE: rect.moveTo( limPos - owner->grabOffset ); break;
        case MARKUP_HANDLE_TL:   rect.setTopLeft(     limPos );     break;
        case MARKUP_HANDLE_TR:   rect.setTopRight(    limPos );     break;
        case MARKUP_HANDLE_BL:   rect.setBottomLeft(  limPos );     break;
        case MARKUP_HANDLE_BR:   rect.setBottomRight( limPos );     break;
        case MARKUP_HANDLE_T:    rect.setTop(         limPos.y() ); break;
        case MARKUP_HANDLE_B:    rect.setBottom(      limPos.y() ); break;
        case MARKUP_HANDLE_L:    rect.setLeft(        limPos.x() ); break;
        case MARKUP_HANDLE_R:    rect.setRight(       limPos.x() ); break;

        default: break;
    }

    // If the object is now mirrored, normailze it
    // (if the user has dragged the bottom above the top, or the left to the right of the right)
    bool swapped = false;
    if( rect.width() < 0 )
    {
        int left = rect.right();
        rect.setRight( rect.left() );
        rect.setLeft( left );

        switch( activeHandle )
        {
            case MARKUP_HANDLE_TL: activeHandle = MARKUP_HANDLE_TR; break;
            case MARKUP_HANDLE_TR: activeHandle = MARKUP_HANDLE_TL; break;
            case MARKUP_HANDLE_BL: activeHandle = MARKUP_HANDLE_BR; break;
            case MARKUP_HANDLE_BR: activeHandle = MARKUP_HANDLE_BL; break;
            case MARKUP_HANDLE_L:  activeHandle = MARKUP_HANDLE_R;  break;
            case MARKUP_HANDLE_R:  activeHandle = MARKUP_HANDLE_L;  break;
            default: break;
        }
        swapped = true;
    }

    if( rect.height() < 0 )
    {
        int top = rect.bottom();
        rect.setBottom( rect.top() );
        rect.setTop( top );
        switch( activeHandle )
        {
            case MARKUP_HANDLE_TL: activeHandle = MARKUP_HANDLE_BL; break;
            case MARKUP_HANDLE_TR: activeHandle = MARKUP_HANDLE_BR; break;
            case MARKUP_HANDLE_BL: activeHandle = MARKUP_HANDLE_TL; break;
            case MARKUP_HANDLE_BR: activeHandle = MARKUP_HANDLE_TR; break;
            case MARKUP_HANDLE_B:  activeHandle = MARKUP_HANDLE_T;  break;
            case MARKUP_HANDLE_T:  activeHandle = MARKUP_HANDLE_B;  break;
            default: break;
        }
        swapped = true;
    }

    // Ensure the region is still in the window.
    // The cursor point was limited to the window earlier (in call to limitPointToImage() ), but the area may not be.
    // For example, if the bottom was dragged up to the top of the window.
    if( rect.left() < 0 )
    {
        int w = rect.width();
        rect.setLeft( 0 );
        rect.setWidth( w );
    }

    if( rect.right() > getImageSize().width()-1 )
    {
        int w = rect.width();
        rect.setLeft( getImageSize().width()-1 - w );
        rect.setWidth( w );
    }

    if( rect.top() < 0 )
    {
        int h = rect.height();
        rect.setTop( 0 );
        rect.setHeight( h );
    }

    if( rect.bottom() > getImageSize().height()-1 )
    {
        int h = rect.height();
        rect.setTop( getImageSize().height() -1 - h );
        rect.setHeight( h );
    }

    // Set the cursor according to the bit we are over after manipulation
    if( swapped )
    {
        QCursor cursor = cursorForHandle( activeHandle );
        owner->markupSetCursor( cursor );
    }

    // Update the area the region now occupies
    setArea();
}

bool markupRegion::isOver( const QPoint point, QCursor* cursor )
{
    // If the point is over the left side, return 'is over' after checking the left handles
    QRect l( rect.topLeft(), QSize( 0, rect.height()) );
    l.adjust( -OVER_TOLERANCE, -OVER_TOLERANCE, OVER_TOLERANCE, OVER_TOLERANCE );
    if( l.contains( point ))
    {
        if( pointIsNear( point, rect.topLeft() ) )
        {
            activeHandle = MARKUP_HANDLE_TL;
        }
        else if( pointIsNear( point, rect.bottomLeft() ) )
        {
            activeHandle = MARKUP_HANDLE_BL;
        }
        else if( pointIsNear( point, QPoint( rect.left(), rect.top()+(rect.height()/2) )))
        {
            activeHandle = MARKUP_HANDLE_L;
        }
        else
        {
            activeHandle = MARKUP_HANDLE_NONE;
        }
        *cursor = cursorForHandle( activeHandle );
        return true;
    }

    // If the point is over the right side, return 'is over' after checking the right handles
    QRect r( rect.topRight(), QSize( 0, rect.height()) );
    r.adjust( -OVER_TOLERANCE, -OVER_TOLERANCE, OVER_TOLERANCE, OVER_TOLERANCE );
    if( r.contains( point ))
    {
        if( pointIsNear( point, rect.topRight() ) )
        {
            activeHandle = MARKUP_HANDLE_TR;
        }
        else if( pointIsNear( point, rect.bottomRight() ) )
        {
            activeHandle = MARKUP_HANDLE_BR;
        }
        else if( pointIsNear( point, QPoint( rect.right(), rect.top()+(rect.height()/2) )))
        {
            activeHandle = MARKUP_HANDLE_R;
        }
        else
        {
            activeHandle = MARKUP_HANDLE_NONE;
        }
        *cursor = cursorForHandle( activeHandle );
        return true;
    }

    // If the point is over the top side, return 'is over' after checking the top handles
    QRect t( rect.topLeft(), QSize( rect.width(), 0) );
    t.adjust( -OVER_TOLERANCE, -OVER_TOLERANCE, OVER_TOLERANCE, OVER_TOLERANCE );
    if( t.contains( point ))
    {
        if( pointIsNear( point, rect.topLeft() ) )
        {
            activeHandle = MARKUP_HANDLE_TL;
        }
        else if( pointIsNear( point, rect.topRight() ) )
        {
            activeHandle = MARKUP_HANDLE_TR;
        }
        else if( pointIsNear( point, QPoint( rect.left()+(rect.width()/2), rect.top() )))
        {
            activeHandle = MARKUP_HANDLE_T;
        }
        else
        {
            activeHandle = MARKUP_HANDLE_NONE;
        }
        *cursor = cursorForHandle( activeHandle );
        return true;
    }

    // If the point is over the bottom side, return 'is over' after checking the bottom handles
    QRect b( rect.bottomLeft(), QSize( rect.width(), 0) );
    b.adjust( -OVER_TOLERANCE, -OVER_TOLERANCE, OVER_TOLERANCE, OVER_TOLERANCE );
    if( b.contains( point ))
    {
        if( pointIsNear( point, rect.bottomLeft() ) )
        {
            activeHandle = MARKUP_HANDLE_BL;
        }
        else if( pointIsNear( point, rect.bottomRight() ) )
        {
            activeHandle = MARKUP_HANDLE_BR;
        }
        else if( pointIsNear( point, QPoint( rect.left()+(rect.width()/2), rect.bottom() )))
        {
            activeHandle = MARKUP_HANDLE_B;
        }
        else
        {
            activeHandle = MARKUP_HANDLE_NONE;
        }
        *cursor = cursorForHandle( activeHandle );
        return true;
    }

    // Not over
    activeHandle = MARKUP_HANDLE_NONE;
    return false;
}

QPoint markupRegion::origin()
{
    return rect.topLeft();
}

// Return the cursor for each handle
QCursor markupRegion::cursorForHandle( const markupItem::markupHandles handle )
{
    switch( handle )
    {
        case MARKUP_HANDLE_TL:   return Qt::SizeFDiagCursor;
        case MARKUP_HANDLE_BL:   return Qt::SizeBDiagCursor;
        case MARKUP_HANDLE_TR:   return Qt::SizeBDiagCursor;
        case MARKUP_HANDLE_BR:   return Qt::SizeFDiagCursor;
        case MARKUP_HANDLE_L:    return Qt::SizeHorCursor;
        case MARKUP_HANDLE_R:    return Qt::SizeHorCursor;
        case MARKUP_HANDLE_T:    return Qt::SizeVerCursor;
        case MARKUP_HANDLE_B:    return Qt::SizeVerCursor;
        case MARKUP_HANDLE_NONE: return Qt::OpenHandCursor;
        default: return Qt::SizeAllCursor;
    }

}

QPoint markupRegion::getPoint1()
{
    return rect.topLeft();
}

QPoint markupRegion::getPoint2()
{
    return rect.bottomRight();
}

QCursor markupRegion::defaultCursor()
{
    return owner->getRegionCursor();
}

void markupRegion::nonInteractiveUpdate( QPoint p1, QPoint p2 )
{
    // Set the region area from p1 and p2
    rect.setTopLeft( p1 );
    rect.setBottomRight( p2 );

    // The point p2 was calculated from the region variables by adding
    // the width to the X position and the height to the Y position.
    // This gives an area (rect) with a width and height 1 pixel too large
    // so reduce the width and height by 1 (while ensuring they don't become negative)
    int width = rect.width();
    int height = rect.height();

    if( width < 1 )
    {
        width = 1;
    }

    if( height < 1 )
    {
        height = 1;
    }

    rect.setWidth( width-1 );
    rect.setHeight( height-1 );

    // Set the overall region area
    setArea();
}
