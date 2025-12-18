/*  markupRegion.h
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
 This class manages a 'region' markup.
 The markup is drawn as a rectangle anywhere in the image. Individual corners, individual sides, or the entire rectangle, can be dragged to any location on the image.
 The markup has a handle on each corner and each side.
 This markup is used in the QEImage widget to identify areas of the image, typically to interact with region or interest variables.
*/

#ifndef MARKUPREGION_H
#define MARKUPREGION_H

#include <markupItem.h>

#include <QDebug>

class imageMarkup;

// Region markup used to select a rectangular region of an image. Used to select an area detector region, or a scan region
class markupRegion : public markupItem
{
public:

    markupRegion( imageMarkup* ownerIn, const bool interactiveIn, const bool reportOnMoveIn, const QString legendIn );

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
    QRect rect;
};

#endif // MARKUPREGION_H
