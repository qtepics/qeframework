/*  markupVLine.h
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
 This class manages a 'vertical slice' markup.
 The markup is drawn as a line down the image and can be dragged left or right to any location on the image.
 The markup has a handle in the center which can be used to drag the line thickness markers.
 The line thickness markers are dashed lines either side of the main marker line.
 This markup is used in the QEImage widget to identify the vertical series of pixels to generate a profile plot.
*/

#ifndef MARKUPVLINE_H
#define MARKUPVLINE_H

#include <markupItem.h>

#include <QDebug>

class imageMarkup;

// Vertical line markup used to select a vertical slice through an image
class markupVLine : public markupItem
{
public:

    markupVLine( imageMarkup* ownerIn, const bool interactiveIn, const bool reportOnMoveIn, const QString legendIn );

    //==================================================================
    // Implement base class functions - see markupItem class defition for a description of each of these methods
    void startDrawing( const QPoint pos );
    void setArea();
    void drawMarkup( QPainter& p );
    void moveTo( const QPoint pos );  // Move an item (always make it visible and highlighed)
    bool isOver( const QPoint point, QCursor* cursor );
    QPoint origin();
    QCursor cursorForHandle( const markupItem::markupHandles handle );
    QPoint getPoint1();
    QPoint getPoint2();
    QCursor defaultCursor();

    void nonInteractiveUpdate(  QPoint p1, QPoint p2, double rotation );
    //==================================================================

private:
    int x;
};


#endif // MARKUPVLINE_H
