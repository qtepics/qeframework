/*  markupText.h
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
 This class manages a 'test' markup.
 The markup simply contains some text. The user cannot interact with it.
 It is used to present a time stamp, but could be used for any text markups.
*/

#ifndef MARKUPTEXT_H
#define MARKUPTEXT_H

#include <markupItem.h>

#include <QDebug>

class imageMarkup;

// Text markup. Used to add some text to an image, such as a timestamp
class markupText : public markupItem
{
public:

    markupText( imageMarkup* ownerIn, const bool interactiveIn, const bool reportOnMoveIn, const QString legendIn );

    void setText( QString textIn );

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
    //==================================================================

private:
    QString text;   // Text displayed
    QRect rect;     // Area of the text
};

#endif // MARKUPTEXT_H
