/*  markupEllipse.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2014-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/*
 This class manages a 'ellipse' markup.
 The markup is drawn as an ellipse anywhere in the image.
 The markup is not interactive.
 The markup has no interaction handles.
 This markup is used in the QEImage widget to and is (initially) intended to
 reflect variables defining a centroid.
*/

#ifndef QE_MARKUP_ELLIPSE_H
#define QE_MARKUP_ELLIPSE_H

#include <markupItem.h>

#include <QDebug>

class imageMarkup;

// Ellipse markup used to indicate a centroid.
class markupEllipse : public markupItem
{
public:

    markupEllipse( imageMarkup* ownerIn, const bool interactiveIn, const bool reportOnMoveIn, const QString legendIn );

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
    QRect rect; // Rectangle enclosing the ellipse
    double rotation;  // Angle of rotation of the ellipse - clockwise in degrees
};

#endif // QE_MARKUP_ELLIPSE_H
