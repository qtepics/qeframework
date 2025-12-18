/*  markupItem.cpp
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

// Refer to imageMarkup.h for general module description

#include <markupItem.h>
#include <imageMarkup.h>
#include <QAction>

//===========================================================================
// Generic markup base class

// Construct a markup item
markupItem::markupItem( imageMarkup* ownerIn, const isOverOptions /*unused*/, const bool interactiveIn, const bool reportOnMoveIn, const QString legendIn )
{
    interactive = interactiveIn;
    reportOnMove = reportOnMoveIn;
    visible = false;
    activeHandle = MARKUP_HANDLE_NONE;
    owner = ownerIn;
    color = QColor( 0, 255, 0 ); // green
    thickness = 1;
    maxThickness = THICKNESS_MAX;

    setLegend( legendIn );

}

markupItem::~markupItem()
{
}

bool markupItem::pointIsNear( QPoint p1, QPoint p2 )
{
    double scale = getZoomScale();
    QPoint p1Scaled = QPoint( p1.x()*scale, p1.y()*scale );
    QPoint p2Scaled = QPoint( p2.x()*scale, p2.y()*scale );
    return QPoint( p1Scaled - p2Scaled ).manhattanLength() < OVER_TOLERANCE;
}

// Draw the item
void markupItem::drawMarkupItem( QPainter& p )
{
    QPen pen( color );
    pen.setStyle( Qt::SolidLine );
    p.setPen( pen );
    drawMarkup( p );
}

void markupItem::setColor( QColor colorIn )
{
    color = colorIn;
}

QColor markupItem::getColor()
{
    return color;
}

// Set the string used to notate the markup (and the calculate its size)
void markupItem::setLegend( const QString legendIn )
{
    legend = legendIn;
    legendSize = owner->legendFontMetrics->size( Qt::TextSingleLine, legend );
}

// Return the string used to notate the markup
const QString markupItem::getLegend()
{
    return legend;
}

// Return the size of the string used to notate the markup
const QSize markupItem:: getLegendSize()
{
    return legendSize;
}

// Returns true if legend text is present
bool markupItem::hasLegend()
{
    return !(legend.isEmpty());
}

// Extend the markup area to include the area occupied by the legend, if any
void markupItem::addLegendArea()
{
    if( hasLegend() )
    {
        QRect legendArea;
        legendArea.setSize( getLegendSize() );
        QPoint x = scalableArea.topLeft()+legendOffset;
        legendArea.moveTo( x );//area.topLeft()+legendOffset );
        area = area.united( legendArea );
    }
}

// Returns the text drawing origin of the legend
const QPoint markupItem::getLegendTextOrigin( QPoint posScaled )
{
    QPoint textOrigin = posScaled + legendOffset;
    textOrigin.setY( textOrigin.y() + owner->legendFontMetrics->ascent() );
    return textOrigin;
}

// Sets the top left position of the rectangle enclosing the legend relative to the markup origin.
// Note, it's up to the markup as to what its origin is. For a vertical line it is the
// X coordintate of the line excluding thickness and Y center of the image.
// For a region markup it is the top left of the region, excluding handles.
void markupItem::setLegendOffset( QPoint offset, legendJustification just )
{
    legendOffset = offset;

    // Position the legend around the position requested according to the justification
    switch( just )
    {
        case ABOVE_RIGHT:
        default:
            legendOffset.setY( legendOffset.y() - owner->legendFontMetrics->height() );
            break;

        case BELOW_LEFT:
            legendOffset.setX( legendOffset.x() - legendSize.width() );

        case BELOW_RIGHT:
            // legendPos is correct as is
            break;
    }

}

// Returns the last drawn legend position
const QPoint markupItem::getLegendOffset()
{
    return legendOffset;
}

void markupItem::drawLegend( QPainter& p, QPoint posScaled )
{
    p.setFont( owner->legendFont );
    p.drawText( getLegendTextOrigin( posScaled ), getLegend() );
}

// Limit a given point to the image
QPoint markupItem::limitPointToImage( const QPoint pos )
{
    // Limit X
    QPoint retPos = pos;
    if( retPos.x() < 0 )
    {
        retPos.setX( 0 );
    }
    else
    {
        int w = getImageSize().width();
        if( retPos.x() > w ) retPos.setX( w-1 );
    }

    // Limit Y
    if( retPos.y() < 0 )
    {
        retPos.setY( 0 );
    }
    else
    {
        int h = getImageSize().height();
        if( retPos.y() > h ) retPos.setY( h-1 );
    }

    // Return limited point
    return retPos;
}

// Set the line thickness of a markup
void markupItem::setThickness( const unsigned int thicknessIn )
{
    // Update the thickness
    thickness = thicknessIn;

    // Update the area the line now occupies
    setArea();
}

unsigned int  markupItem::getThickness()
{
    return thickness;
}

QSize markupItem::getImageSize()
{
    return owner->getImageSize();
}

double markupItem::getZoomScale()
{
    return owner->getZoomScale();
}
