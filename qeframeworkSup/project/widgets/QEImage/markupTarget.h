/*  markupTarget.h
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
 This class manages a 'target' markup.
 The markup looks like a target (but different to the 'beam' markup. It can be dragged to any location in the image.
 This markup is used in the QEImage widget to interactivly mark where a sample is.
*/

#ifndef MARKUPTARGET_H
#define MARKUPTARGET_H

#include <markupItem.h>

#include <QDebug>

class imageMarkup;

// Target markup used to identify a target point on a sample
class markupCrosshair1 : public markupItem
{
public:
    markupCrosshair1( imageMarkup* ownerIn, const bool interactiveIn, const bool reportOnMoveIn, const QString legendIn );

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
    QPoint pos; // Center of target
};

#endif // MARKUPTARGET_H
