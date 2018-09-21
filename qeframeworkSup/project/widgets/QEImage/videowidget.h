/*  videowidget.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2018 Australian Synchrotron
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

#ifndef QE_VIDEO_WIDGET_H
#define QE_VIDEO_WIDGET_H

#include <QWidget>
#include <imageMarkup.h>

class VideoWidget : public QWidget, public imageMarkup
{
    Q_OBJECT
public:
    VideoWidget(QWidget *parent = 0);
    ~VideoWidget();

    void setNewImage( QImage image, QCaDateTime& time );
    void setPanning( bool panningIn );
    bool getPanning();
    QPoint scalePoint( QPoint pnt );
    int scaleOrdinate( int ord );

    QPoint scaleImagePoint( QPoint pnt );
    QRect scaleImageRectangle( QRect r );

    int scaleImageOrdinate( int ord );

    QImage getImage(){ return refImage; }
    QSize getImageSize();
    bool hasCurrentImage();                         // Return true if displaying an image
    void markupChange();                            // The markup overlay has changed, redraw them all

protected:
    void paintEvent(QPaintEvent*);

    void mousePressEvent( QMouseEvent* event);
    void mouseReleaseEvent ( QMouseEvent* event );
    void mouseMoveEvent( QMouseEvent* event );
    void wheelEvent( QWheelEvent* event );

    void keyPressEvent( QKeyEvent* event );

    void markupChange( QVector<QRect>& changedAreas );    // The markup overlay has changed, redraw part of it

    void resizeEvent( QResizeEvent *event );
    void markupSetCursor( QCursor cursor );
    void markupAction( markupIds mode, bool complete, bool clearing, QPoint point1, QPoint point2, unsigned int thickness );


signals:
    void userSelection( imageMarkup::markupIds mode, bool complete, bool clearing, QPoint point1, QPoint point2, unsigned int thickness );
    void zoomInOut( int zoomAmount );
    void currentPixelInfo( QPoint pos );
    void pan( QPoint pos );
    void redraw();

private:
    void addMarkups( QPainter& screenPainter, QVector<QRect>& changedAreas );

    QImage currentImage;              // Latest camera image
    QImage refImage;                  // Latest camera image at the same resolution as the display - used for erasing markups when they are moved
    void   createRefImage();          // Create a reference image the same size as currently being viewed.

    double getXScale() const;         // Currently only this used - markups zoom incorrectly when X stretch != Y stretch
    double getYScale() const;         // Place holder function.

    bool panning;       // If true user is dragging image with mouse (rather than performing any markup)
    QPoint panStart;

    QPoint pixelInfoPos;    // Current pixel under pointer
};

#endif // QE_VIDEO_WIDGET_H
