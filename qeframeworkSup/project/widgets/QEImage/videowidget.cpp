/*  videowidget.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2022 Australian Synchrotron
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
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
 This class manages the low level presentation of images in a display widget and user interact with the image.
 The image is delivered as a QImage ready for display. There is no need to flip, rotate, clip, etc.
 This class manages zooming the image simply by setting the widget size as required and drawing into it. Qt then performs the scaling required.
 */

#include "videowidget.h"
#include <QPainter>
#include <QDebug>

#define DEBUG qDebug() << "videowidget"  << __LINE__ << __FUNCTION__ << "  "


#define PANNING_CURSOR Qt::CrossCursor

VideoWidget::VideoWidget(QWidget *parent) : QWidget(parent)
{
    panning = false;

    setAutoFillBackground(false);

    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::black);
    setPalette(palette);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    setMouseTracking( true );
    setCursor( getDefaultMarkupCursor() );

    setAttribute( Qt::WA_OpaquePaintEvent, true );
    update();// !!!required???
}

VideoWidget::~VideoWidget()
{
}

void VideoWidget::markupSetCursor( QCursor cursor )
{
    setCursor( cursor );
}

// Ensure we have a reference image and it is the same size as the display.
void VideoWidget::createRefImage()
{
    // Do nothing if the reference image has been set and is the correct size
    // (it will not be set initially, or after a new image has arrived in which case it is cleared)
    if( !refImage.isNull() && refImage.size() == size() )
    {
        return;
    }

    // If the current image is present and is the same size as the the video widget,
    // use the current image as the reference image.
    // (cheap - creates a shallow copy)
    if( !currentImage.isNull() && currentImage.size() == size() )
    {
        refImage = currentImage;
    }

    // If the current image is not present or the wrong size...
    else
    {
        // Create a correctly sized reference image if one does not exist or it is the wrong size.
        if( refImage.isNull() || refImage.size() != size() )
        {
            refImage = QImage( size(), QImage::Format_RGB32 );
        }

        // If the current image exists, draw it scaled into the reference image
        QPainter refPainter( &refImage );
        if( !currentImage.isNull() )
        {
            refPainter.drawImage( refImage.rect(), currentImage, currentImage.rect() );
        }

        // If the current image does not exists, blank the reference image
        else
        {
            QColor bg(0, 0, 0, 255);
            refPainter.fillRect(rect(), bg);
        }
    }
}

// The displayed image has changed, redraw it
void VideoWidget::setNewImage( QImage image, QCaDateTime& time )
{
    // Note if this is the first image update
    bool firstImage = currentImage.isNull();

    // Take a copy of the current image
    // (cheap - creates a shallow copy)
    currentImage = image;

    // Invalidate the current reference image
    refImage = QImage();

    // Note the time for markups
    setMarkupTime( time );

    // Ensure the markup system is aware of the image size
    setImageSize( currentImage.size() );

    // Ensure the markup scaling is correct.
    // The scaling is set up on the first image (here), and each resize (in the resize event)
    if( firstImage )
    {
        markupResize( getXScale() );
    }

    // Cause a repaint with the new image
    update();
}

// The markups have changed redraw them all
void VideoWidget::markupChange()
{
    QVector<QRect> areas;
    areas.append( QRect( 0, 0, width(), height() ));
    markupChange( areas );
}

// The markups have changed redraw the required parts
void VideoWidget::markupChange( QVector<QRect>& changedAreas )
{
    // Start accumulating the changed areas
    QRect nextRect = changedAreas[0];

    // For each additional area, accumulate it, or draw the
    // areas accumulated so far and start a new accumulation.
    for( int i = 1; i < changedAreas.count(); i++ )
    {
        // Determine the total pixel area if the next rectangle united with the area accumulated so far
        QRect unitedRect = nextRect.united( changedAreas[i] );
        int unitedArea = unitedRect.width() * unitedRect.height();

        // Determine the total pixel area if the next rectangle is drawn seperately
        int totalArea = nextRect.width() * nextRect.height() + changedAreas[i].width() * changedAreas[i].height();

        // If it is more efficient to draw the area accumulated so far
        // seperately from the next rectangle, then draw the area accumulated
        // so far and start a fresh accumulation.
        if( totalArea < unitedArea )
        {
            update( nextRect );
            nextRect = changedAreas[i];
        }
        // If it is more efficient to unite the area accumulated so far with the
        // next rectangle, do this
        else
        {
            nextRect = unitedRect;
        }
    }

    // Draw the last accumulated area
    update( nextRect );
}

// Manage a paint event in the video widget
void VideoWidget::paintEvent(QPaintEvent* event )
{
    // Create the reference image.
    // It may be created now if there has never been an update, which is likely
    // at creation before an image update has arrived.
    createRefImage();

    // Build a painter and only bother about the changed area
    QPainter painter(this);
    painter.setClipRect( event->rect() );

    // Update the display with the reference image.
    painter.drawImage( event->rect(), refImage, event->rect() );

    // Update any markups
    if( !currentImage.isNull() )
    {
        drawMarkups( painter, event->rect() );
    }

    // Report position for pixel info logging
    emit currentPixelInfo( pixelInfoPos );
}

// Manage a resize event
void VideoWidget::resizeEvent( QResizeEvent *event )
{
    // Ignore resizes from nothing (there are no markups and scaling calculations go weird)
    if( event->oldSize().width() <= 0 || event->oldSize().height() <= 0 )
    {
        return;
    }

    // If there is a current image, redraw it and recalculate the markup dimensions
    if( !currentImage.isNull() )
    {
        emit redraw();
    }

    // Ensure the markups match the new size
    markupResize( getXScale() );
}

// Act on a markup change
void VideoWidget::markupAction( markupIds mode,             // Markup being manipulated
                                bool complete,              // True if the user has completed an operation (for example, finished moving a markup to a new position and a write to a variable is now required)
                                bool clearing,              // True if a markup is being cleared
                                QPoint point1,              // Generic first point of the markup. for example, to left of an area, or target position
                                QPoint point2,              // Optional generic second point of the markup
                                unsigned int thickness )    // Optional thickness of the markup
{
    emit userSelection( mode,           // Markup being manipulated
                        complete,       // True if the user has completed an operation (for example, finished moving a markup to a new position and a write to a variable is now required)
                        clearing,       // True if a markup is being cleared
                        point1,         // Generic first point of the markup. for example, to left of an area, or target position
                        point2,         // Optional generic second point of the markup
                        thickness );    // Optional thickness of the markup
}

// Return a point from the displayed image as a point in the original image
QPoint VideoWidget::scalePoint( QPoint pnt )
{
    QPoint scaled;
    scaled.setX( scaleOrdinate( pnt.x() ));
    scaled.setY( scaleOrdinate( pnt.y() ));
    return scaled;
}

// Return a point from the original image as a point in the displayed image
QRect VideoWidget::scaleImageRectangle( QRect r )
{
    QRect scaled;
    scaled.setTopLeft( scaleImagePoint( r.topLeft() ));
    scaled.setBottomRight( scaleImagePoint( r.bottomRight() ));
    return scaled;
}

// Return a point from the original image as a point in the displayed image
QPoint VideoWidget::scaleImagePoint( QPoint pnt )
{
    QPoint scaled;
    scaled.setX( scaleImageOrdinate( pnt.x() ));
    scaled.setY( scaleImageOrdinate( pnt.y() ));
    return scaled;
}

// Return an ordinate from the displayed image as an ordinate in the original image
int VideoWidget::scaleOrdinate( int ord )
{
    return (int)((double)ord / getXScale());
}

// Return an ordinate from the original image as an ordinate in the displayed image
int VideoWidget::scaleImageOrdinate( int ord )
{
    return (int)((double)ord * getXScale());
}

// Return the displayed size of the current image
QSize VideoWidget::getImageSize()
{
    return currentImage.size();
}

// Return true if displaying an image
bool VideoWidget::hasCurrentImage()
{
    return !currentImage.isNull();
}

// Return the horizontal scale of the displayed image
double VideoWidget::getXScale() const
{
    // If for any reason a scale can't be determined, return scale of 1.0
    if( currentImage.isNull() || currentImage.width() == 0 || width() == 0)
        return 1.0;

    // Return the horizontal scale of the displayed image
    return (double)width() / (double)currentImage.width();
}

// Return the vertical scale of the displayed image
double VideoWidget::getYScale() const
{
    // If for any reason a scale can't be determined, return scale of 1.0
    if( currentImage.isNull() || currentImage.height() == 0 || height() == 0)
        return 1.0;

    // Return the vertical scale of the displayed image
    return (double)height() / (double)currentImage.height();
}


// The mouse has been pressed over the image
void VideoWidget::mousePressEvent( QMouseEvent* event)
{
    // Only act on left mouse button press
    if( !(event->buttons()&Qt::LeftButton) )
        return;

    // Grab the keyboard to get any 'tweak' (up/down/left/right) keys
    grabKeyboard();

    // Pass the event to the markup system. It will use it if appropriate.
    // If it doesn't use it, then start a pan if panning
    // Note, the markup system will take into account if panning.
    // When panning, the markup system will not use the event unless actually over a markup.
    if( !markupMousePressEvent( event, panning ) && panning )
    {
        setCursor( Qt::ClosedHandCursor );
        panStart = event->pos();
    }
}

// The mouse has been released over the image
void VideoWidget::mouseReleaseEvent ( QMouseEvent* event )
{
    // Release the keyboard (grabbed when mouse pressed to catch 'tweak' keys)
    releaseKeyboard();

    // Pass the event to the markup system. It will use it if appropriate.
    // If it doesn't use it, then complete panning.
    // Note, the markup system will take into account if panning.
    // When panning, the markup system will not use the event unless moving a markup.
    if( !markupMouseReleaseEvent( event, panning ) && panning )
    {
        setCursor( PANNING_CURSOR );
        emit pan( pos() );
    }
}

//Manage a mouse move event
void VideoWidget::mouseMoveEvent( QMouseEvent* event )
{
    // Report position for pixel info logging
    pixelInfoPos.setX( int ( (double)(event->pos().x()) / getXScale() ) );
    pixelInfoPos.setY( int ( (double)(event->pos().y()) / getXScale() ) );
    emit currentPixelInfo( pixelInfoPos );

    // Pass the event to the markup system. It will use it if appropriate.
    // If it doesn't use it, then pan if panning.
    // Note, the markup system will take into account if panning.
    // When panning, the markup system will not use the event unless moving a markup.
    if( !markupMouseMoveEvent( event, panning ) && panning )
    {
        if( event->buttons()&Qt::LeftButton)
        {
            // Determine a new position that will keep the same point in the image under the mouse
            QPoint newPos = pos() - ( panStart - event->pos() ) ;

            // Limit panning. Don't pan beyond the image
            QWidget* p = this->parentWidget();
            if( newPos.x() <  p->width() - width() )
                newPos.setX( p->width() - width());
            if( newPos.y() < p->height() - height() )
                newPos.setY( p->height() - height() );

            if( newPos.x() > 0 )
                newPos.setX( 0 );
            if( newPos.y() > 0 )
                newPos.setY( 0 );

            // Do the pan
            move( newPos );
        }
    }
}

// The wheel has been moved over the image
void VideoWidget::wheelEvent( QWheelEvent* event )
{
    int zoomAmount = event->angleDelta().y() / 12;
    emit zoomInOut( zoomAmount );
}

// A key has been pressed
void VideoWidget::keyPressEvent( QKeyEvent* event )
{
    // Determine what to do
    bool ignore = false;
    QPoint warp;
    switch( event->key() )
    {
        case Qt::Key_Left:  warp = QPoint( -1,  0 ); break;
        case Qt::Key_Right: warp = QPoint(  1,  0 ); break;
        case Qt::Key_Up:    warp = QPoint(  0, -1 ); break;
        case Qt::Key_Down:  warp = QPoint(  0,  1 ); break;

        default: ignore = true; break;
    }

    // If doing nothing, then do it!
    if( ignore )
    {
        return;
    }

    // If tweaking the position, then tweak away
    QCursor::setPos( QCursor::pos() + warp );
}

// The video widget handles panning.
// Return if currently panning.
bool VideoWidget::getPanning()
{
    return panning;
}

// The video widget handles panning.
// Tell the video widget it is currently panning.
void VideoWidget::setPanning( bool panningIn )
{
    panning = panningIn;
    if( panning )
    {
        setCursor( PANNING_CURSOR );
    }
}

// end
