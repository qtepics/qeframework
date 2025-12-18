/*  markupBeam.h
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
 This class manages a 'beam' markup.
 The markup looks like a target (but different to the 'target' markup. It can be dragged to any location in the image.
 This markup is used in the QEImage widget to interactivly mark where beam is.
*/

#ifndef MARKUPBEAM_H
#define MARKUPBEAM_H

#include <markupItem.h>

#include <QDebug>

class imageMarkup;

// Beam markup used to identify beam center in an image
class markupCrosshair2 : public markupItem
{
public:

    markupCrosshair2( imageMarkup* ownerIn, const bool interactiveIn, const bool reportOnMoveIn, const QString legendIn );

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
    QPoint pos;
    int armSize;    // Length of arms in cross hair
};

#endif // MARKUPBEAM_H
