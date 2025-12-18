/*  videowidget.h
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

/*
 This class manages the low level presentation of images in a display widget and user interact with the image.
 The image is delivered as a QImage ready for display. There is no need to flip, rotate, clip, etc.
 This class manages zooming the image simply by setting the widget size as required and drawing into it. Qt then performs the scaling required.
 */

#ifndef QE_VIDEO_WIDGET_H
#define QE_VIDEO_WIDGET_H

#include <QWidget>
#include <imageMarkup.h>

class QEImage;  // differed

class VideoWidget : public QWidget, public imageMarkup
{
   Q_OBJECT
public:
   explicit VideoWidget(QEImage *parent);
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

   typedef void (*CustomisePaintHandlers) (QEImage* image,
                                           QPainter& painter,
                                           const QRect rect,
                                           QObject* context);

   void setCustomisePaintHandler (CustomisePaintHandlers paintExtraHandler,
                                  QObject* context = 0);
   CustomisePaintHandlers getCustomisePaintHandler () const;

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
   void markupAction( markupIds mode, bool complete, bool clearing,
                      QPoint point1, QPoint point2,
                      unsigned int thickness );


signals:
   void userSelection( imageMarkup::markupIds mode, bool complete, bool clearing,
                       QPoint point1, QPoint point2, unsigned int thickness );
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

   QEImage* owner;
   CustomisePaintHandlers paintExtraHandler;
   QObject* userContext;

   bool panning;       // If true user is dragging image with mouse (rather than performing any markup)
   QPoint panStart;

   QPoint pixelInfoPos;    // Current pixel under pointer
};

#endif // QE_VIDEO_WIDGET_H
