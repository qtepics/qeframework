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

// Refer to imageMarkup.h for general module description


#include <imageMarkup.h>

#include <markupTarget.h>
#include <markupBeam.h>
#include <markupHLine.h>
#include <markupVLine.h>
#include <markupLine.h>
#include <markupRegion.h>
#include <markupText.h>
#include <markupEllipse.h>
#include <QEImageMarkupThickness.h>

#include <imageContextMenu.h>
#include <imageMarkupLegendSetText.h>


// Constructor
imageMarkup::imageMarkup()
{
    zoomScale = 1.0;

    mode = MARKUP_ID_NONE;
    activeItem = MARKUP_ID_NONE;

    // set up the font used for notations (and time)
    legendFont = QFont("Courier", 12);
    legendFontMetrics = new QFontMetrics( legendFont );

    items.resize(MARKUP_ID_COUNT );
    items[MARKUP_ID_H1_SLICE]   = new markupHLine(  this, true,  true, "slice 1" );
    items[MARKUP_ID_H2_SLICE]   = new markupHLine(  this, true,  true, "slice 2" );
    items[MARKUP_ID_H3_SLICE]   = new markupHLine(  this, true,  true, "slice 3" );
    items[MARKUP_ID_H4_SLICE]   = new markupHLine(  this, true,  true, "slice 4" );
    items[MARKUP_ID_H5_SLICE]   = new markupHLine(  this, true,  true, "slice 5" );
    items[MARKUP_ID_V1_SLICE]   = new markupVLine(  this, true,  true, "slice 1" );
    items[MARKUP_ID_V2_SLICE]   = new markupVLine(  this, true,  true, "slice 2" );
    items[MARKUP_ID_V3_SLICE]   = new markupVLine(  this, true,  true, "slice 3" );
    items[MARKUP_ID_V4_SLICE]   = new markupVLine(  this, true,  true, "slice 4" );
    items[MARKUP_ID_V5_SLICE]   = new markupVLine(  this, true,  true, "slice 5" );
    items[MARKUP_ID_LINE]      = new markupLine(   this, true,  true, "profile" );
    items[MARKUP_ID_REGION1]   = new markupRegion( this, true,  true, "region 1" );
    items[MARKUP_ID_REGION2]   = new markupRegion( this, true,  true, "region 2" );
    items[MARKUP_ID_REGION3]   = new markupRegion( this, true,  true, "region 3" );
    items[MARKUP_ID_REGION4]   = new markupRegion( this, true,  true, "region 4" );
    items[MARKUP_ID_TARGET]    = new markupCrosshair1( this, true,  true, "target" );
    items[MARKUP_ID_BEAM]      = new markupCrosshair2(   this, true,  true, "beam" );
    items[MARKUP_ID_TIMESTAMP] = new markupText(   this, false, false, "" );
    items[MARKUP_ID_ELLIPSE]   = new markupEllipse(this, false, false, "Centroid" );

    targetMarkupOption = CROSSHAIR1;
    beamMarkupOption   = CROSSHAIR2;

    markupAreasStale = true;

    // Create circle cursor used for target and beam
    QPixmap circlePixmap = QPixmap( ":/qe/image/circleCursor.png" );
    circleCursor = QCursor( circlePixmap );

    // Create target cursor used for target and beam
    QPixmap targetPixmap = QPixmap( ":/qe/image/targetCursor.png" );
    targetCursor = QCursor( targetPixmap );

    // Create red vertical line cursor used for vertical slice
    QPixmap vSlicePixmap = QPixmap( ":/qe/image/vLineCursor.png" );
    vLineCursor = QCursor( vSlicePixmap );

    // Create green horizontal line cursor used for horizontal slice
    QPixmap hSlicePixmap = QPixmap( ":/qe/image/hLineCursor.png" );
    hLineCursor = QCursor( hSlicePixmap );

    // Create yellow cross cursor used for line profile
    QPixmap profilePixmap = QPixmap( ":/qe/image/lineCursor.png" );
    lineCursor = QCursor( profilePixmap );

    // Create purple cross cursor used for area selection
    QPixmap areaPixmap = QPixmap( ":/qe/image/regionCursor.png" );
    regionCursor = QCursor( areaPixmap );

    // Don't show time on image by default
    showTime = false;

    // Create and setup the markup display menu
//    mdMenu = new markupDisplayMenu();
}

// Destructor
imageMarkup::~imageMarkup()
{
}

// Get the current markup mode - (what is the user doing? selecting an area? drawing a line?)
imageMarkup::markupIds imageMarkup::getMode()
{
    return mode;
}

// Set the current markup mode - (what is the user doing? selecting an area? drawing a line?)
void imageMarkup::setMode( markupIds modeIn )
{
    mode = modeIn;
}

//===========================================================================

void imageMarkup::setMarkupTime( QCaDateTime& time )
{
    if( showTime )
    {
        markupText* timeDate = (markupText*)items[MARKUP_ID_TIMESTAMP];
        timeDate->setText( time.text().left( 23 ) );

    // No need to notify of a markup change as the time is only ever set
    // when a new image arrives and all markups will be redrawn if visible
    }
}

// Set if time should be shown.
// Time is a markup that the user doesn't interact with. It is just displayed, or not
void imageMarkup::setShowTime( bool showTimeIn )
{
    showTime = showTimeIn;

    markupItem* item = items[MARKUP_ID_TIMESTAMP];

    item->visible = showTimeIn;

    // Notify a markup has changed
    QVector<QRect> changedAreas;
    changedAreas.append( scaleArea( item->area, item->scalableArea ) );
    markupChange( changedAreas );
}

// Get if the time is currently being displayed
bool imageMarkup::getShowTime()
{
    return showTime;
}

//===========================================================================

// The image has changed, redraw the markups if any
void imageMarkup::drawMarkups( QPainter& p, const QRect& rect )
{
    // Scale the region to draw to a region in the original image
    QRect originalRect( rect.left()   / zoomScale,
                        rect.top()    / zoomScale,
                        rect.width()  / zoomScale,
                        rect.height() / zoomScale );

    int n = items.count();
    for( int i = 0; i < n; i ++ )
    {
        markupItem* item = items[i];
        // If the markup is being displayed and in the area of interest, draw it.
        if( item->visible && originalRect.intersects( item->area ))
        {
            item->drawMarkupItem( p );
        }
    }
}

//===========================================================================

// User pressed a mouse button
bool imageMarkup::markupMousePressEvent(QMouseEvent *event, bool panning)
{
    // Only act on left mouse button press
    if( !(event->buttons()&Qt::LeftButton ))
        return false;

    // scale the event pos to a point in the original image
    QPoint pos;
    pos.setX( event->pos().x() / zoomScale );
    pos.setY( event->pos().y() / zoomScale );

    // Determine if the user clicked over an interactive, visible item,
    // and if so, make the first item found the active item
    setActiveItem( pos );

    // If in panning mode, then we will not take over the event unless we are over an active item
    // Note, buttonDown is cleared so there is no context of any sort of markup action in progress
    if( panning && activeItem == MARKUP_ID_NONE )
    {
        buttonDown = false;
        return false;
    }

    // Keep track of button state
    buttonDown = true;

    // If not over an item, start creating a new item
    // move the appropriate item to the point clicked on
    if( activeItem == MARKUP_ID_NONE )
    {
        bool pointAndClick = true;
        switch( mode )
        {
            default:
            case MARKUP_ID_NONE:
                break;

            case MARKUP_ID_H1_SLICE:
            case MARKUP_ID_H2_SLICE:
            case MARKUP_ID_H3_SLICE:
            case MARKUP_ID_H4_SLICE:
            case MARKUP_ID_H5_SLICE:
            case MARKUP_ID_V1_SLICE:
            case MARKUP_ID_V2_SLICE:
            case MARKUP_ID_V3_SLICE:
            case MARKUP_ID_V4_SLICE:
            case MARKUP_ID_V5_SLICE:
            case MARKUP_ID_TARGET:
            case MARKUP_ID_BEAM:
                activeItem = mode;
                pointAndClick = true;
                break;

            case MARKUP_ID_LINE:
            case MARKUP_ID_REGION1:
            case MARKUP_ID_REGION2:
            case MARKUP_ID_REGION3:
            case MARKUP_ID_REGION4:
                activeItem = mode;
                pointAndClick = false;
                break;
        }
        if( activeItem != MARKUP_ID_NONE )
        {
            // Some items are point-and-click items. They don't require the user to drag to select where the item is.
            // Typical point-and-click items are vertical lines and horizontal lines (traversing the entire window)
            // Other item are point-press-drag-release items as they require the user to drag to select where the item is.
            // A typical point-press-drag-release item is an area or a line.
            //
            // For a point-and-click item, just redraw it where the user clicks
            if( pointAndClick )
            {
                redrawActiveItemHere( pos );
            }
            // For a point-press-drag-release, erase it if visible,
            // and start the process of draging from the current position
            else
            {
                // If item was visible, ensure original location is redrawn to erase it
                if( items[activeItem]->visible )
                {
                    QVector<QRect> changedAreas;
                    changedAreas.append( scaleArea( items[activeItem]->area, items[activeItem]->scalableArea ) );
                    markupChange( changedAreas );
                }
                items[activeItem]->startDrawing( pos );

                // Set the cursor according to the bit we are over after creation
                QCursor cursor;
                if( items[activeItem]->isOver( pos, &cursor ) )
                {
                    markupSetCursor( cursor );
                }
            }
        }
    }

    // Return indicating the event was dealt with
    return true;
}

// Manage the markups as the mouse moves
bool imageMarkup::markupMouseMoveEvent( QMouseEvent* event, bool /*panning*/ )
{
    // scale the event pos to a point in the original image
    QPoint pos;
    pos.setX( event->pos().x() / zoomScale );
    pos.setY( event->pos().y() / zoomScale );

    // If no button is down, ensure the cursor reflects what it is over
    // (once the button is pressed, this doesn't need to be assesed again)
    if( !buttonDown )
    {
        // If the pointer is over a visible item, set the cursor to suit the item
        int i;
        int n = items.count();
        for( i = 0; i < n; i++ )
        {
            QCursor specificCursor;
            if( items[i]->interactive && items[i]->visible && items[i]->isOver( pos, &specificCursor ) )
            {
                markupSetCursor( specificCursor );
                break;
            }
        }

        // If not over any item, set the default markup cursor
        if( i == n /*loop completed without finding an item under the cursor*/ )
        {
            markupSetCursor( getDefaultMarkupCursor() );
        }

        return false;
    }

    // If the user has the button down, redraw the item in its new position or shape.
    if( buttonDown && activeItem != MARKUP_ID_NONE )
    {
        redrawActiveItemHere( pos );

        // If there is an active item and action is required on move, then report the move
        if( activeItem != MARKUP_ID_NONE && items[activeItem]->reportOnMove )
        {
            markupItem* item = items[activeItem];
            markupAction( getActionMode(), false, false, item->getPoint1(), item->getPoint2(), item->getThickness() );
        }

        // Return indicating the event was appropriated for markup purposes
        return true;
    }

    return false;
}

// The mouse has been released over the image
bool imageMarkup::markupMouseReleaseEvent ( QMouseEvent*, bool panning  )
{
    // If panning, and we havn't noted a button down for the purposes of image markup, then don't take over this release event
    // (If buttonDown is true then we have already appropriated the button down/move/release for markup purposes)
    if( panning && !buttonDown )
    {
        return false;
    }

    // Determine if an action is now complete
    bool complete;
    switch( activeItem )
    {
        case MARKUP_ID_REGION1:
        case MARKUP_ID_REGION2:
        case MARKUP_ID_REGION3:
        case MARKUP_ID_REGION4:
            complete = true;
            break;

        default:
            complete = false;
            break;
    }

    // If there is an active item, take action
    if( activeItem != MARKUP_ID_NONE )
    {
        markupItem* item = items[activeItem];
        markupAction( getActionMode(), complete, false, item->getPoint1(), item->getPoint2(), item->getThickness() );
    }

    // Flag there is no longer an active item
    activeItem = MARKUP_ID_NONE;
    buttonDown = false;

    // Return indicating the event was appropriated for markup purposes
    return true;
}

//===========================================================================

// A region of interest value has changed.
// Update a markup if required
void imageMarkup::markupRegionValueChange( int areaIndex, QRect area, bool displayMarkups )
{
    int region;
    switch( areaIndex )
    {
        case 0:
        default: region = MARKUP_ID_REGION1; break;
        case 1:  region = MARKUP_ID_REGION2; break;
        case 2:  region = MARKUP_ID_REGION3; break;
        case 3:  region = MARKUP_ID_REGION4; break;
    }

    markupValueChange( region, displayMarkups, area.topLeft(), area.bottomRight() );
}

// Horizontal slice 1 value has changed.
// Update the markup
void imageMarkup::markupH1ProfileChange( int y, bool displayMarkups )
{
    markupValueChange( MARKUP_ID_H1_SLICE, displayMarkups, QPoint( 0, y ) );
}

// Horizontal slice 2 value has changed.
// Update the markup
void imageMarkup::markupH2ProfileChange( int y, bool displayMarkups )
{
    markupValueChange( MARKUP_ID_H2_SLICE, displayMarkups, QPoint( 0, y ) );
}

// Horizontal slice 3 value has changed.
// Update the markup
void imageMarkup::markupH3ProfileChange( int y, bool displayMarkups )
{
    markupValueChange( MARKUP_ID_H3_SLICE, displayMarkups, QPoint( 0, y ) );
}

// Horizontal slice 4 value has changed.
// Update the markup
void imageMarkup::markupH4ProfileChange( int y, bool displayMarkups )
{
    markupValueChange( MARKUP_ID_H4_SLICE, displayMarkups, QPoint( 0, y ) );
}

// Horizontal slice 5 value has changed.
// Update the markup
void imageMarkup::markupH5ProfileChange( int y, bool displayMarkups )
{
    markupValueChange( MARKUP_ID_H5_SLICE, displayMarkups, QPoint( 0, y ) );
}

// Vertical slice 1 value has changed.
// Update the markup
void imageMarkup::markupV1ProfileChange( int x, bool displayMarkups )
{
    markupValueChange( MARKUP_ID_V1_SLICE, displayMarkups, QPoint( x, 0 ) );
}

// Vertical slice 2 value has changed.
// Update the markup
void imageMarkup::markupV2ProfileChange( int x, bool displayMarkups )
{
    markupValueChange( MARKUP_ID_V2_SLICE, displayMarkups, QPoint( x, 0 ) );
}

// Vertical slice 3 value has changed.
// Update the markup
void imageMarkup::markupV3ProfileChange( int x, bool displayMarkups )
{
    markupValueChange( MARKUP_ID_V3_SLICE, displayMarkups, QPoint( x, 0 ) );
}

// Vertical slice 4 value has changed.
// Update the markup
void imageMarkup::markupV4ProfileChange( int x, bool displayMarkups )
{
    markupValueChange( MARKUP_ID_V4_SLICE, displayMarkups, QPoint( x, 0 ) );
}

// Vertical slice 5 value has changed.
// Update the markup
void imageMarkup::markupV5ProfileChange( int x, bool displayMarkups )
{
    markupValueChange( MARKUP_ID_V5_SLICE, displayMarkups, QPoint( x, 0 ) );
}

// An arbitrary line profile value has changed.
// Update the markup
void imageMarkup::markupLineProfileChange( QPoint start, QPoint end, bool displayMarkups )
{
    markupValueChange( MARKUP_ID_LINE, displayMarkups, start, end );
}

// An ellipse markup value has changed.
// Update the markup
void imageMarkup::markupEllipseValueChange( QPoint start, QPoint end, bool displayMarkups )
{
    markupValueChange( MARKUP_ID_ELLIPSE, displayMarkups, start, end );
}

// A target value has changed.
// Update markup if required
void imageMarkup::markupTargetValueChange( QPoint point, bool displayMarkups )
{
    markupValueChange( MARKUP_ID_TARGET, displayMarkups, point );
}

// A beam position value has changed.
// Update markup if required
void imageMarkup::markupBeamValueChange( QPoint point, bool displayMarkups )
{
    markupValueChange( MARKUP_ID_BEAM, displayMarkups, point );
}

// A markup related value has changed.
// Update any markup if required.
void imageMarkup::markupValueChange( int markup, bool displayMarkups, QPoint p1, QPoint p2 )
{
    // If the markup is active (being dragged, for instance) then don't fiddle with it.
    if( markup == activeItem )
    {
        return;
    }

    // If markup should now be visible, set it visible
    if( displayMarkups )
    {
        items[markup]->visible = true;
    }

    // Initial area to update
    QVector<QRect> changedAreas;
    bool isVisible =  items[markup]->visible;
    if( isVisible )
    {
        changedAreas.append( scaleArea( items[markup]->area, items[markup]->scalableArea ) );
    }

    // Update the markup
    items[markup]->nonInteractiveUpdate( p1, p2 );

    // Extend the area to update and update it
    changedAreas.append( scaleArea( items[markup]->area, items[markup]->scalableArea ) );
    markupChange( changedAreas );
}

// Return the mode according to the active item.
// Note, this is not the mode as set by setMode(). The mode as set by setMode()
// is what happens when a user initiates action in a part of the display not
// occupied by a markup.
// This mode is related to an existing markup being manipulated.
// For example, if the current mode set by setMode() is MARKUP_MODE_AREA
// (select and area) but the user has draged the profile line the mode
// returned by this method is MARKUP_MODE_LINE
imageMarkup::markupIds imageMarkup::getActionMode()
{
    switch( activeItem )
    {
        case MARKUP_ID_H1_SLICE:
        case MARKUP_ID_H2_SLICE:
        case MARKUP_ID_H3_SLICE:
        case MARKUP_ID_H4_SLICE:
        case MARKUP_ID_H5_SLICE:
        case MARKUP_ID_V1_SLICE:
        case MARKUP_ID_V2_SLICE:
        case MARKUP_ID_V3_SLICE:
        case MARKUP_ID_V4_SLICE:
        case MARKUP_ID_V5_SLICE:
        case MARKUP_ID_LINE:
        case MARKUP_ID_REGION1:
        case MARKUP_ID_REGION2:
        case MARKUP_ID_REGION3:
        case MARKUP_ID_REGION4:
        case MARKUP_ID_TARGET:
        case MARKUP_ID_BEAM:
            return activeItem;

        default:
            return MARKUP_ID_NONE;
    }
}

// Return the default markup cursor (to be displayed when not over any particular markup)
QCursor imageMarkup::getDefaultMarkupCursor()
{
    if( mode < MARKUP_ID_COUNT )
    {
        return items[mode]->defaultCursor();
    }
    else
    {
        return Qt::CrossCursor;
    }
}

// The active item has moved to a new position. Redraw it.
void imageMarkup::redrawActiveItemHere( QPoint pos )
{
    // Do nothing if no active item
    if( activeItem == MARKUP_ID_NONE )
        return;

    // Area to update
    // !!! This is currently just one rectangle that encloses the erased and redrawn object.
    // !!! It could (should?) be a region that includes a single rect for mostly hoz and vert lines,
    // !!! four rects for the four sides of an area, and a number of rectangles that efficiently
    // !!! allows redrawing of diagonal lines
    QVector<QRect> changedAreas;

    // Ensure item will be erased, move, then ensure it will be redrawn
    if( items[activeItem]->visible )
    {
        changedAreas.append( scaleArea( items[activeItem]->area, items[activeItem]->scalableArea ) );
    }

    items[activeItem]->moveTo( pos );
    items[activeItem]->visible = true;

    // Extend the changed areas to include the item's new area and notify markups require redrawing
    changedAreas.append( scaleArea( items[activeItem]->area, items[activeItem]->scalableArea ) );
    markupChange( changedAreas );
}

void imageMarkup::setImageSize( const QSize& imageSizeIn )
{
    imageSize = imageSizeIn;
}

// The image size has changes, or the viewport has been zoomed
void imageMarkup::markupResize( const double zoomScaleIn )
{
    // Determine scaling that will be applied to the markups.
    zoomScale = (zoomScaleIn!=0)?zoomScaleIn:1.0;

    // Area to update
    QVector<QRect> changedAreas;

    // Rescale and redraw any visible markups
    // Also act on all visible markups. This is required as the new viewport coordinates will need to be retranslated according to the new viewport size.
    // Note, the results will often be identical, but not always, as the new viewport coordinates may not translate to the same pixels in the original image.
    int n = items.count();
    for( int i = 0; i < n; i ++ )
    {
        markupItem* item = items[i];

        // Ensure the area the markup occupied will be cleared
        if( item->visible )
        {
            changedAreas.append( scaleArea( item->area, item->scalableArea ) );
        }

        // If the markup is being displayed, redraw it, and act on its 'new' position
        if( item->visible )
        {
            changedAreas.append( scaleArea( item->area, item->scalableArea ) );

//            markupAction( (markupIds)i, false, false, item->getPoint1(), item->getPoint2(), item->getThickness() );
        }
    }

    // Notify the change
    if( changedAreas.count() )
    {
        markupChange( changedAreas );
    }
}

// Return true if there are any markups visible.
// Used for more efficiency when updating the image.
bool imageMarkup::anyVisibleMarkups()
{
    int n = items.count();
    for( int i = 0; i < n; i ++ )
    {
        if( items[i]->visible )
        {
            return true;
        }
    }
    return false;
}

// Return true if a specified markup is visible.
bool imageMarkup::isMarkupVisible( markupIds mode )
{
    return items[mode]->visible;
}

// Set the legend for a given mode.
// For example, area 1 markup might be called 'ROI 1'
void imageMarkup::setMarkupLegend( markupIds mode, QString legendIn )
{
    // Do nothing if mode is invalid
    if( mode < 0 || mode >= MARKUP_ID_NONE )
    {
        return ;
    }

    // Save the new markup legend
    items[mode]->setLegend( legendIn );

    // If the item is visible, redraw it with the new legend
    QVector<QRect> changedAreas;
    if( items[mode]->visible )
    {
        changedAreas.append( scaleArea( items[mode]->area, items[mode]->scalableArea ) );
        markupChange( changedAreas );
    }
}

// Return the legend for a given mode.
QString imageMarkup::getMarkupLegend( markupIds mode )
{
    // Do nothing if mode is invalid
    if( mode < 0 || mode >= MARKUP_ID_NONE )
    {
        return QString();
    }

    // Return the markup legend
    return items[mode]->getLegend();
}

// Set the color for a given mode.
// For example, please draw area selection rectangles in green.
void imageMarkup::setMarkupColor( markupIds mode, QColor markupColorIn )
{
    // Do nothing if mode is invalid
    if( mode < 0 || mode >= MARKUP_ID_NONE )
    {
        return ;
    }

    // Save the new markup color
    items[mode]->setColor( markupColorIn );

    // If the item is visible, redraw it in the new color
    QVector<QRect> changedAreas;
    if( items[mode]->visible )
    {
        changedAreas.append( scaleArea( items[mode]->area, items[mode]->scalableArea ) );
        markupChange( changedAreas );
    }

}

QColor imageMarkup::getMarkupColor( markupIds mode )
{
    // Return a valid deault color mode is invalid
    if( mode < 0 || mode >= MARKUP_ID_NONE )
    {
        return QColor( 127, 127, 127 );
    }

    // Return the markup color
    return items[mode]->color;
}

// Return the circle cursor use by the image markup system
QCursor imageMarkup::getCircleCursor()
{
    return circleCursor;
}

// Return the target cursor use by the image markup system
QCursor imageMarkup::getTargetCursor()
{
    return targetCursor;
}

// Return the vertical slice cursor use by the image markup system
QCursor imageMarkup::getVLineCursor()
{
    return vLineCursor;
}

// Return the horizontal slice cursor use by the image markup system
QCursor imageMarkup::getHLineCursor()
{
    return hLineCursor;
}

// Return the line profile cursor use by the image markup system
QCursor imageMarkup::getLineCursor()
{
    return lineCursor;
}

// Return the area selection cursor use by the image markup system
QCursor imageMarkup::getRegionCursor()
{
    return regionCursor;
}

// Show the markup context menu if required.
// Do nothing and return false if nothing to do, for example, the position is not over a markup item
// If required, present the menu, act on the user selection, then return true
//
// This method currently populates a imageContextMenu with one 'clear' option.
// Refer to  QEImage::showImageContextMenu() to see how imageContextMenu can be populated with checkable, and non checkable items, and sub menus
bool imageMarkup::showMarkupMenu( const QPoint& pos, const QPoint& globalPos )
{
    // Scale position to position in original item
    QPoint scaledPos = QPoint( pos.x()/zoomScale, pos.y()/zoomScale );

    // Determine if the user clicked over an interactive, visible item,
    // and if so, make the first item found the active item
    setActiveItem( scaledPos );

    // If not over an item, do nothing.
    if( activeItem == MARKUP_ID_NONE )
        return false;

    imageContextMenu menu;

    //                      Title                            checkable  checked                 option
    menu.addMenuItem(       "Hide",                          false,     false,                  imageContextMenu::ICM_CLEAR_MARKUP             );
    menu.addMenuItem(       "Set Legend",                    false,     false,                  imageContextMenu::ICM_SET_LEGEND             );

    // If any thickness, add thickness options (zero means item has no concept of thickness)
    if( items[activeItem]->getThickness() )
    {
        menu.addMenuItem(       "Single Pixel Line Thickness",   false,     false,                  imageContextMenu::ICM_THICKNESS_ONE_MARKUP     );
        menu.addMenuItem(       "Select Line Thickness",         false,     false,                  imageContextMenu::ICM_THICKNESS_SELECT_MARKUP  );
    }

    // Present the menu
    imageContextMenu::imageContextMenuOptions option;
    bool checked;
    menu.getContextMenuOption( globalPos, &option, &checked );

    // Act on the menu selection
    switch( option )
    {
        default:
        case imageContextMenu::ICM_NONE:
            break;

        case imageContextMenu::ICM_CLEAR_MARKUP:
            clearMarkup( activeItem );
            break;

        case imageContextMenu::ICM_SET_LEGEND:
            {
                imageMarkupLegendSetText legendEditor( items[activeItem]->getLegend() );
                if( legendEditor.exec() == QDialog::Accepted )
                {
                    items[activeItem]->setLegend( legendEditor.getLegend() );
                }
            }
            break;

        case imageContextMenu::ICM_THICKNESS_ONE_MARKUP:
            setSinglePixelThickness( activeItem );
            break;

        case imageContextMenu::ICM_THICKNESS_SELECT_MARKUP:
            // Get a new thickness from the user
            QEImageMarkupThickness thicknessDialog;
            thicknessDialog.setThickness( items[activeItem]->getThickness() );
            thicknessDialog.exec();
            setThickness( activeItem, thicknessDialog.getThickness() );
            break;
    }

    // Indicate markup menu has been presented
    return true;
}

// Determine if the user clicked over an interactive, visible item,
// and if so, make the first item found the active item
void imageMarkup::setActiveItem( const QPoint& pos )
{
    activeItem = MARKUP_ID_NONE;
    int n = items.count();
    for( int i = 0; i < n; i++ )
    {
        QCursor cursor;
        if( items[i]->interactive && items[i]->visible && items[i]->isOver( pos, &cursor ) )
        {
            activeItem = (markupIds)i;
            grabOffset = pos - items[i]->origin();
            break;
        }
    }
}

// show or hide a markup
void imageMarkup::displayMarkup( markupIds markupId, bool state )
{
    if( state )
    {
        showMarkup( markupId );
    }
    else
    {
        clearMarkup( markupId );
    }
}


// Hide a markup
void imageMarkup::clearMarkup( markupIds markupId )
{
    items[markupId]->visible = false;
    QVector<QRect> changedAreas;
    changedAreas.append( scaleArea( items[markupId]->area, items[markupId]->scalableArea ) );
    // Redraw the now hidden item
    markupChange( changedAreas );

    // Take the appropriate user action for a markup being hidden
    markupAction( markupId, false, true, QPoint(), QPoint(), 0 );

    // If the hidden markup was the current markup being manipulated, change to 'no current markup'
    if( activeItem == markupId )
    {
        activeItem = MARKUP_ID_NONE;
    }
}

// Reveal a markup
// (Only if it has any size)
void imageMarkup::showMarkup( markupIds markupId )
{
    markupItem* item = items[markupId];
    item->visible = true;

    QRect area = item->area;
    if( ( item->getPoint1().x() != item->getPoint2().x() ) ||
        ( item->getPoint1().y() != item->getPoint2().y() ) )
    {
        QVector<QRect> changedAreas;
        changedAreas.append( scaleArea( area, item->scalableArea ) );

        // Redraw the now visible item
        markupChange( changedAreas );

        // Take the appropriate user action for a markup being shown
        markupAction( markupId, false, false, item->getPoint1(), item->getPoint2(), item->getThickness() );
    }
}

// Set a markup to signel pixel thickness.
// Initially used for profile line markups.
void imageMarkup::setSinglePixelThickness( markupIds markupId )
{
    markupItem* item = items[markupId];

    // Include the area of the item before its thickness changes
    QVector<QRect> changedAreas;
    changedAreas.append( item->area );

    // set the thickness of the item
    item->setThickness( 1 );

    // Apply the markup change if visible
    if( item->visible )
    {
        // Include the area of the item after its thickness has changed
        changedAreas.append( scaleArea( item->area, item->scalableArea ) );

        // Repaint
        markupChange( changedAreas );

        // Use the changed markup
        markupAction( markupId, false, false, item->getPoint1(), item->getPoint2(), item->getThickness() );
    }
}

// Set a markup to a thickness.
// Initially used for profile line markups.
void imageMarkup::setThickness( markupIds markupId, unsigned int newThickness )
{
    markupItem* item = items[markupId];

    // Update the item if the thickness has changed
    if( newThickness != item->getThickness() )
    {
        // Change the thickness
        QVector<QRect> changedAreas;
        item->setThickness( newThickness );

        // Apply the markup change if visible
        if( item->visible )
        {
            // Include the area of the item after its thickness has changed
            changedAreas.append( scaleArea( item->area, item->scalableArea ) );

            // Repaint
            markupChange( changedAreas );

            // Use the changed markup
            markupAction( markupId, false, false, item->getPoint1(), item->getPoint2(), item->getThickness() );
        }
    }
}

// Target markup options
imageMarkup::beamAndTargetOptions imageMarkup::getTargetOption()
{
    return targetMarkupOption;
}

void imageMarkup::setTargetOption( beamAndTargetOptions option )
{
    targetMarkupOption = option;
    setBeamOrTargetOption( MARKUP_ID_TARGET, targetMarkupOption );
}

// Beam markup options
imageMarkup::beamAndTargetOptions imageMarkup::getBeamOption()
{
    return beamMarkupOption;
}

void imageMarkup::setBeamOption( beamAndTargetOptions option )
{
    beamMarkupOption = option;
    setBeamOrTargetOption( MARKUP_ID_BEAM, beamMarkupOption );
}

// Set the beam or traget markup option (which style of crosshaor to display)
// Note, if the markup is displayed, this will not re-display it in the new form
// This is OK if the property is only edited in designer. If the property is set
// while the widget is in use, then perhaps the visibility of the markup should
// be checked before deleting, other attributes noted, and then the new markup
// displayed correctly
void imageMarkup::setBeamOrTargetOption( markupIds item, beamAndTargetOptions option )
{
    // Note attributes of the current markup
    QString legend = items[item]->getLegend();
    QColor color = items[item]->getColor();

    // Delete the current markup
    delete items[item];

    // Create the new markup
    switch( option )
    {
        case CROSSHAIR1:
            items[item] = new markupCrosshair1( this, true,  true, legend );
            break;

        case CROSSHAIR2:
            items[item] = new markupCrosshair2( this, true,  true, legend );
            break;
    }

    // Restore the attributes
    items[item]->setColor( color );
}

// Return the area of a markup zoomed to the display image.
// Note, there is generally a part of a markup that is scaled (for example the main area of a region markup)
// and a part of the markup that is not scaled (for example, the handles on the corners and sides of a region markup, or a legend)
// 'scaledArea' represent the area to be scaled within the overall 'area' of the markup (using coordinates in the original image).
QRect imageMarkup::scaleArea( QRect area, QRect scaledArea )
{
    return QRect( scaledArea.left()   * zoomScale - (scaledArea.left() - area.left()),
                  scaledArea.top()    * zoomScale - (scaledArea.top() - area.top()),
                  scaledArea.width()  * zoomScale + (area.width() - scaledArea.width() + 1 ),
                  scaledArea.height() * zoomScale + (area.height() - scaledArea.height()) + 1 );
}
