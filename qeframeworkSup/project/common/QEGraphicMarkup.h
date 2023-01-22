/*  QEGraphicMarkup.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2014-2023 Australian Synchrotron.
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
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_GRAPHIC_MARKUP_H
#define QE_GRAPHIC_MARKUP_H

#include <qwt_plot_curve.h>
#include <QCursor>
#include <QObject>
#include <QFontMetrics>
#include <QPen>
#include <QPoint>
#include <QVariant>
#include <QEGraphicNames.h>
#include <persistanceManager.h>
#include <QEFrameworkLibraryGlobal.h>

class QEGraphic;  // differed declaration - avoid mutual header inclusion

//-----------------------------------------------------------------------------
// Base class for all QEGraphic markups.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEGraphicMarkup {
public:
   explicit QEGraphicMarkup (const QEGraphicNames::Markups markup,
                             QEGraphic* owner);
   virtual ~QEGraphicMarkup ();

   QEGraphicNames::Markups getMarkup () const;

   void setCurrentPosition (const QPointF& currentPosition);
   QPointF getCurrentPosition () const;

   void setData (const QVariant& data);
   QVariant getData () const;

   // Save/restore a markup configuration
   //
   void saveConfiguration (PMElement& parentElement);
   void restoreConfiguration (PMElement& parentElement);

   virtual void setInUse (const bool inUse);   // in use or permitted.
   virtual bool isInUse () const;

   virtual void setVisible (const bool visible);
   virtual bool isVisible () const;

   virtual void setEnabled (const bool enabled);
   virtual bool isEnabled () const;

   virtual void setSelected (const bool selected);
   virtual bool isSelected () const;

   virtual void mousePress   (const QPointF& realMousePosition, const Qt::MouseButton button);
   virtual void mouseRelease (const QPointF& realMousePosition, const Qt::MouseButton button);
   virtual void mouseMove    (const QPointF& realMousePosition);

   virtual QCursor getCursor () const;
   virtual bool isOver (const QPointF& point, int& distance) const;

   virtual void relocate ();
   void plot ();

protected:
   QEGraphic* getOwner () const;

   // Utility function available for use by concrete class isOver functions.
   // To return true the markup must be visible and the point close to here.
   //
   bool isOverHere (const QPointF& here, const QPointF& point, int& distance) const;

   // Conveniance utility function to set owner curve pen/brush and then call
   // owner plot curve function.
   //
   void plotCurve (const QEGraphicNames::DoubleVector& xData,
                   const QEGraphicNames::DoubleVector& yData);

   // Extract the font metric for a given font point size.
   //
   QFontMetrics getFontMetrics ();

   // Emits the current markup postion from the QEGraphic owner.
   //
   void emitCurrentPostion ();

   // All concrete classes must provide a means to draw a markup.
   // This is only ever called when the markup is visible.
   //
   virtual void plotMarkup () = 0;

   QPointF position;  // notional current position
   QVariant data;     // any associated data
   QPen pen;
   QBrush brush;
   QwtPlotCurve::CurveStyle curveStyle;
   QCursor cursor;
   Qt::MouseButton activationButton;
   bool inUse;
   bool visible;
   bool enabled;
   bool selected;
   const QEGraphicNames::Markups markup;   // own type indicator.
//private:
   QEGraphic* owner;
};

//-----------------------------------------------------------------------------
// Draws rectangle from origin to current (mouse position).
//
class QEGraphicAreaMarkup : public QEGraphicMarkup {
public:
   explicit QEGraphicAreaMarkup (QEGraphic* owner);

   void mousePress   (const QPointF& realMousePosition, const Qt::MouseButton button);
   void mouseRelease (const QPointF& realMousePosition, const Qt::MouseButton button);
   void mouseMove    (const QPointF& realMousePosition);

protected:
   void plotMarkup ();

private:
   bool isValidArea () const;
   QPointF origin;
};

//-----------------------------------------------------------------------------
// Draws a line from origin to current mouse position.
//
class QEGraphicLineMarkup : public QEGraphicMarkup {
public:
   explicit QEGraphicLineMarkup (QEGraphic* owner);

   QPointF getSlope () const;

   void mousePress   (const QPointF& realMousePosition, const Qt::MouseButton button);
   void mouseRelease (const QPointF& realMousePosition, const Qt::MouseButton button);
   void mouseMove    (const QPointF& realMousePosition);

protected:
   void plotMarkup ();

private:
   QPointF origin;
};


//-----------------------------------------------------------------------------
// Draws a box around the position.
//
class QEGraphicBoxMarkup : public QEGraphicMarkup {
public:
   explicit QEGraphicBoxMarkup (QEGraphic* owner);
   bool isOver (const QPointF& point, int& distance) const;
   void setSelected (const bool selected);

protected:
   void plotMarkup ();
};


//-----------------------------------------------------------------------------
// Draws crosshairs about to current (mouse) position.
//
class QEGraphicCrosshairsMarkup : public QEGraphicMarkup {
public:
   explicit QEGraphicCrosshairsMarkup (QEGraphic* owner);

   void setVisible (const bool visible);
   bool isOver (const QPointF& point, int& distance) const;

   void mousePress   (const QPointF& realMousePosition, const Qt::MouseButton button);
   void mouseRelease (const QPointF& realMousePosition, const Qt::MouseButton button);
   void mouseMove    (const QPointF& realMousePosition);

protected:
   void plotMarkup ();
};

//-----------------------------------------------------------------------------
// Draws horizontal line - like Crosshairs only 1 dimension.
//
class QEGraphicHorizontalMarkerMarkup : public QEGraphicMarkup {
public:
   explicit QEGraphicHorizontalMarkerMarkup (QEGraphicNames::Markups markup, QEGraphic* owner);

   void setVisible (const bool visible);

   // No isOver, mousePress, mouseRelease or mouseMove.
   // This is currently a display only markup.
protected:
   void plotMarkup ();
};

//-----------------------------------------------------------------------------
// Draws horizontal line - like Crosshairs only 1 dimension.
//
class QEGraphicVerticalMarkerMarkup : public QEGraphicMarkup {
public:
   explicit QEGraphicVerticalMarkerMarkup (QEGraphicNames::Markups markup, QEGraphic* owner);

   void setVisible (const bool visible);

   // No isOver, mousePress, mouseRelease or mouseMove.
   // This is currently a display only markup.
protected:
   void plotMarkup ();
};

//-----------------------------------------------------------------------------
// (Abstract) base class for QEGraphicHorizontalMarkup/QEGraphicVerticalMarkup.
//
class QEGraphicHVBaseMarkup : public QEGraphicMarkup {
public:
   explicit QEGraphicHVBaseMarkup (QEGraphicNames::Markups markup, QEGraphic* owner);

   void setInUse     (const bool inUse);
   void mousePress   (const QPointF& realMousePosition, const Qt::MouseButton button);
   void mouseRelease (const QPointF& realMousePosition, const Qt::MouseButton button);
   void mouseMove    (const QPointF& realMousePosition);

protected:
   void setColours (const unsigned int baseRGB);   // Expected 0xff0000 or 0x00ff00 or 0x0000ff
   void plotMarkup ();

   virtual void getLine (double& xmin, double& xmax, double& ymin, double& ymax) = 0;
   virtual void getShape (QPoint shape []) = 0;

   QColor brushDisabled;
   QColor brushEnabled;
   QColor brushSelected;
};


//-----------------------------------------------------------------------------
// Draws horizontal through current position.
//
class QEGraphicHorizontalMarkup : public QEGraphicHVBaseMarkup {
public:
   explicit QEGraphicHorizontalMarkup (const QEGraphicNames::Markups markup, QEGraphic* owner);
   bool isOver (const QPointF& point, int& distance) const;
   void relocate ();  // make sure we don't "lose" the markers
protected:
   void getLine (double& xmin, double& xmax, double& ymin, double& ymax);
   void getShape (QPoint shape []);
};


//-----------------------------------------------------------------------------
// Draws vertical through current position.
//
class QEGraphicVerticalMarkup : public QEGraphicHVBaseMarkup {
public:
   explicit QEGraphicVerticalMarkup (const QEGraphicNames::Markups markup, QEGraphic* owner);
   bool isOver (const QPointF& point, int& distance) const;
   void relocate ();  // make sure we don't "lose" the markers
protected:
   void getLine (double& xmin, double& xmax, double& ymin, double& ymax);
   void getShape (QPoint shape []);
};

#endif  // QE_GRAPHIC_MARKUP_H
