/*  markupItem.h
 *
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
 *  Copyright (c) 2012,2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
 This class is the base class for markups that are overlayed of an image, such as region of interest, line, graticule, time and date, etc.
 The class also handles generic user interaction with the markups, such as creation and draging.
 The class works at the display resolution of the image, but also understands the actual resolution of
 the underlying image and can describe markups in terms of the underlying image.
*/

#ifndef QE_IMAGE_MARKUP_ITEM_H
#define QE_IMAGE_MARKUP_ITEM_H

#include <QPainter>

/* markups managing their own data is not implemented yet
#include <VariableNameManager.h>
#include <QCaObject.h>
*/

// Allowable distance in pixels from object which will still be considered 'over'
#define OVER_TOLERANCE 6
#define HANDLE_SIZE 6

// Profile thickness selection maximum
#define THICKNESS_MAX 51

class imageMarkup;

// Generic markup item.
// Each type of markup (line, area, etc) is based on this class.
class markupItem      // markups managing their own data is not implemented yet : public VariableNameManager
{
protected:
    enum isOverOptions{ OVER_LINE, OVER_BORDER, OVER_AREA }; // test required to determine if pointer is over the object
    markupItem( imageMarkup* ownerIn, const isOverOptions over, const bool interactiveIn, const bool reportOnMoveIn, const QString legendIn );
public:
    virtual ~markupItem();

public:
    enum markupHandles { MARKUP_HANDLE_NONE, // Over a markup, but not over any specific handle of the markup
                         MARKUP_HANDLE_START, MARKUP_HANDLE_END, MARKUP_HANDLE_CENTER,  // Lines
                         MARKUP_HANDLE_TL, MARKUP_HANDLE_TR, MARKUP_HANDLE_BL, MARKUP_HANDLE_BR, // Area corners
                         MARKUP_HANDLE_T, MARKUP_HANDLE_B, MARKUP_HANDLE_L, MARKUP_HANDLE_R };   // Area sides
    void drawMarkupItem( QPainter& p );
    QSize getImageSize();


    virtual QPoint origin()=0;                                                      // Origin of the markup, for example, the center of a target
    virtual void   moveTo( const QPoint pos )=0;                                    // Move an item (always make it visible and highlighed)
    virtual void   startDrawing( const QPoint pos ) = 0;                            // Action to take when the user starts drawing a markup, such as note the starting point of a line
    virtual bool   isOver( const QPoint point, QCursor* cursor )=0;                 // Returns true if the cursor is over a markup. Generally, 'over' means near the
                                                                                    // visual parts of the markup. For example in the center of a rectangle is not 'over' near
                                                                                    // the lines of the rectangle is 'over'.
    virtual QCursor cursorForHandle( const markupItem::markupHandles handle ) = 0;  // Returns the cursor for the specified handle. For example, horizontal arrows when over a rectangle
                                                                                    // left or right side handle
    virtual QPoint       getPoint1()=0;                                             // Return the first point of the markup (starting point for a line, top left corner for a rectangle, etc)
    virtual QPoint       getPoint2()=0;                                             // Return the second point of the markup (end point for a line, bottom right corner for a rectangle, etc)
    virtual QCursor      defaultCursor()=0;                                         // Return the default cursor for the markup.
    virtual void         nonInteractiveUpdate( QPoint, QPoint ) {}                  // Only implemented by those objects that are updated by data such as region of interest

    void          setThickness( const unsigned int thicknessIn );                   // Set the thickness of a markup where relevent.
    unsigned int  getThickness();

    void          setLegend( const QString legendIn );                              // Set the string used to notate the markup (and the calculate its size)
    const QString getLegend();                                                      // Return the string used to notate the markup

    void setColor( QColor colorIn );
    QColor        getColor();   // Return the colour used for this markup

    QRect         area;         // Area (in original image) object occupies, used for repainting,
                                //  and actual object coordinates where appropriate.
    QRect         scalableArea; // Area in original image that is scaled when drawn in display image.
                                //  This should be a part of 'area'. For example, when a region is drawn, the
                                //  actual region is scaled, but the handles on the sides and corners are not scaled.
    bool          visible;      // Object is visible to the user
    bool          interactive;  // Object can be moved by the user
    bool          reportOnMove; // Movements reported (not just on move completion)
    QColor        color;        // Color markup is drawn in

protected:
    markupHandles activeHandle;                         // The current handle the user is over
    virtual void  setArea()=0;                          // Update the total rectangular area occupied by the markup
    virtual void  drawMarkup( QPainter& p )=0;          // Draw the markup
    bool          pointIsNear( QPoint p1, QPoint p );   // Returns true of point p1 is close to point p


    imageMarkup*  owner;                                // Class containing this markup instance

    const QSize getLegendSize();                        // Return the size of the string used to notate the markup
    void addLegendArea();                               // Add the legend area to the markup area

    enum   legendJustification{ ABOVE_RIGHT, BELOW_LEFT, BELOW_RIGHT }; // Options for positioning the legend
    const  QPoint getLegendTextOrigin( QPoint posScaled );              // Returns the text drawing origin of the legend
    void   setLegendOffset( QPoint offset, legendJustification just );  // Sets the top left position of the rectangle enclosing the legend, relative to the markup's origin
    const  QPoint getLegendOffset();                                    // Returns the legend position, relative to the markup's origin
    void   drawLegend( QPainter& p, QPoint posScaled );                 // Draw the legend beside the markup
    QPoint limitPointToImage( const QPoint pos );                       // Return the input point limited to the image area

    unsigned int thickness;                             // Selected line thickness
    unsigned int maxThickness;                          // Maximum line thickness. Changes according to current zoom

    double getZoomScale();

/* Not implemented yet
// Markup items may make their own data connections in the same way QEWidgets do.
// The variables below support this and are very similar to those in QEWidget
// which is the base class for all QE widgets.
protected:
    void setNumVariables( unsigned int numVariablesIn );    // Set the number of variables that will stream data updates to the widget. Default of 1 if not called.

    bool subscribe;                                         // Flag if data updates should be requested

    qcaobject::QCaObject* createConnection( unsigned int variableIndex );       // Create a CA connection. Return a QCaObject if successfull

    virtual qcaobject::QCaObject* createQcaItem( unsigned int variableIndex );  // Function to create a appropriate superclass of QCaObject to stream data updates
    virtual void establishConnection( unsigned int variableIndex );             // Create a CA connection and initiates updates if required
    virtual void activated();                                                   // Do any post-all-widgets-constructed stuff
*/


private:
    QString      legend;                                // Text displayed beside markup
    QSize        legendSize;                            // Size of legend (according to legend font)
    bool         hasLegend();                           // Returns true if legend text is present
    QPoint       legendOffset;                          // Last drawn legend position
};

#endif // QE_IMAGE_MARKUP_ITEM_H
