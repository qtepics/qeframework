/*  QEHistogram.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at
 *  the Australian Synchrotron.
 *
 *  Copyright (c) 2014-2023  Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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

#include "QEHistogram.h"
#include <QDebug>
#include <QBrush>
#include <QMouseEvent>
#include <QPen>
#include <QRandomGenerator>

#include <QECommon.h>
#include <QEScaling.h>
#include <QECommon.h>
#include <QEDisplayRanges.h>

#define DEBUG qDebug () << "QEHistogram" << __LINE__ << __FUNCTION__ << "  "

// Useful orientation selection macros.
//
#define HORIZONTAL       (this->mOrientation == Qt::Horizontal)
#define SELECT(ho, vo)   (HORIZONTAL ? (ho) : (vo))

// Magic null values - use NaN ?
// 'Unlikely' to occur and can be exactly represented as a double.
// A bit 'naughty' mixing control and data, but it's pragmatic.
//
static const double  NO_DATA_VALUE = -1073741824.0;
static const QColor  NO_COLOUR_VALUE = QColor (3, 1, 4, 2);   // Pi colour ;-)

static const double  MINIMUM_SPAN  = +1.0E-18;
static const int     MAX_CAPACITY  = 100000;

//------------------------------------------------------------------------------
//
static bool isNullDataValue (const double x) {
   double d = ABS (x - NO_DATA_VALUE);
   return (d <= 0.001);
}

//------------------------------------------------------------------------------
//
static bool isNullColourValue (const QColor& x) {
   return x == NO_COLOUR_VALUE;
}


//------------------------------------------------------------------------------
//
QEHistogram::QEHistogram (QWidget *parent) : QFrame (parent)
{
   // Set default property values
   // Super class....
   //
   this->setMinimumSize (80, 40);
   this->setFrameShape (QFrame::Panel);
   this->setFrameShadow (QFrame::Plain);

   // And local properties.
   //
   this->mBackgroundColour = QColor (224, 224, 224);   // pale gray
   this->mSecondBgColour   = QColor (200, 212, 224);   // blueish pale gray
   this->mBarColour = QColor (55, 155, 255);           // blue
   this->mDrawAxies = true;
   this->mDrawBorder = true;
   this->mAutoScale = false;
   this->mAutoBarGapWidths = false;
   this->mShowScale = true;
   this->mShowGrid = true;
   this->mShowSecondBg = false;
   this->mLogScale = false;

   this->mGap = 3;                // 0 .. 10
   this->mBarWidth = 8;           // 1 .. 80
   this->mMargin = 3;             // 0 .. 20
   this->mBaseLine = 0.0;
   this->mMinimum = 0.0;
   this->mMaximum = 10.0;
   this->mSecondBgSize = 5;
   this->mTestSize = 0;

   // Create internal widgets
   //
   this->layoutA = NULL;
   this->layoutB = NULL;

   this->histogramAxisPlusArea = new QWidget (this);
   this->axisPainter = new QEAxisPainter (this->histogramAxisPlusArea);
   this->histogramArea = new QWidget (this->histogramAxisPlusArea);
   this->scrollbar = new QScrollBar (this);

   this->axisPainter->setAutoFixedSize (true);  // but does not do auto indent.
   this->axisPainter->setIndent (6, 6);
   this->axisPainter->setGap (0);
   this->axisPainter->setHasAxisLine (true);

   this->histogramArea->setMouseTracking (true);
   this->histogramArea->installEventFilter (this);

   this->scrollbar->setRange (0, 0);

   // Setting the orientation will create the needed layouts and add widgets to
   // those layouts. We force first call to do an acutal update by setting the
   // orientation state to a non-default value.
   //
   this->mOrientation = Qt::Vertical;
   this->setOrientation (Qt::Horizontal);

   this->dataArray.clear ();
   this->dataArray.reserve (100);
   this->numberDisplayed = 0;
   this->firstDisplayed = 0;
   this->lastEmittedIndex = -2;  // not -1.

   // Do this only once, not in paintEvent as it causes another paint event.
   //
   this->histogramArea->setAutoFillBackground (false);
   this->histogramArea->setBackgroundRole (QPalette::NoRole);

   QObject::connect (this->scrollbar, SIGNAL (valueChanged (int)),
                     this,     SLOT (scrollBarValueChanged (int)));
}


//------------------------------------------------------------------------------
//  Define default size for this widget class.
//
QSize QEHistogram::sizeHint () const
{
   return QSize (160, 80);
}

//------------------------------------------------------------------------------
//
void QEHistogram::clearValue (const int index)
{
   this->setValue (index, NO_DATA_VALUE);
}

//------------------------------------------------------------------------------
//
void QEHistogram::clearColour (const int index)
{
   this->setColour (index, NO_COLOUR_VALUE);
}

//------------------------------------------------------------------------------
//
void QEHistogram::clear ()
{
   this->firstDisplayed = 0;
   this->dataArray.clear ();
   this->colourArray.clear ();
   this->update ();
}

//------------------------------------------------------------------------------
//
void QEHistogram::setColour (const int index, const QColor& value)
{
   if (index >= 0 && index < MAX_CAPACITY) {     // sanity check
      while (this->colourArray.count () < index + 1) {
         this->colourArray.append (NO_COLOUR_VALUE);
      }

      this->colourArray [index] = value;

      while ((this->colourArray.count () > 0) &&
             (isNullColourValue (this->colourArray.last ()))) {
         this->colourArray.remove (this->colourArray.count () - 1);
      }

      this->update ();
   }
}

//------------------------------------------------------------------------------
//
void QEHistogram::setValue (const int index, const double value)
{
   if (index >= 0 && index < MAX_CAPACITY) {     // sanity check
      while (this->dataArray.count () < index + 1) {
         this->dataArray.append (NO_DATA_VALUE);
      }

      this->dataArray [index] = value;

      while ((this->dataArray.count () > 0) &&
             (isNullDataValue (this->dataArray.last ()))) {
         this->dataArray.remove (this->dataArray.count () - 1);
      }

      this->update ();
   }
}

//------------------------------------------------------------------------------
//
int QEHistogram::count () const
{
   return this->dataArray.count ();
}

//------------------------------------------------------------------------------
//
double QEHistogram::value (const int index) const
{
   return this->dataArray.value (index, 0.0);
}

//------------------------------------------------------------------------------
//
void QEHistogram::setValues (const DataArray& values)
{
   this->dataArray = values;
   this->update ();
}

//------------------------------------------------------------------------------
//
QEHistogram::DataArray QEHistogram::values () const
{
   return this->dataArray;
}

//------------------------------------------------------------------------------
//
QColor QEHistogram::getPaintColour (const int index) const
{
   QColor result;

   result = this->colourArray.value (index, NO_COLOUR_VALUE);
   if (isNullColourValue (result)) {
      result = this->mBarColour;
   }
   return result;
}

//------------------------------------------------------------------------------
//
int QEHistogram::scrollMaximum () const
{
   return MAX (0, this->dataArray.count () - this->numberDisplayed);
}

//------------------------------------------------------------------------------
//
void QEHistogram::scrollBarValueChanged (int value)
{
   this->firstDisplayed = LIMIT (value, 0, (MAX_CAPACITY - 1));
   this->update ();
}

//------------------------------------------------------------------------------
//
int QEHistogram::firstBarTopLeft () const
{
   return SELECT (this->paintArea.left (), this->paintArea.top ());
}

//------------------------------------------------------------------------------
//
QRect QEHistogram::fullBarRect  (const int position) const
{
   const int fullWidth = this->useBarWidth + this->useGap + 1;   // also full height
   int top;
   int left;
   int right;
   int bottom;

   // paintArea defines overall paint area.
   //
   if (HORIZONTAL) {
      top = this->paintArea.top ();
      bottom = this->paintArea.bottom ();

      left = this->firstBarTopLeft () + (fullWidth * position);
      right = left + this->useBarWidth;
   } else {
      left = this->paintArea.left ();
      right = this->paintArea.right ();
      top = this->firstBarTopLeft () + (fullWidth * position);
      bottom = top + this->useBarWidth;
   }

   QRect result;
   result.setTop (top);
   result.setLeft (left);
   result.setBottom (bottom);
   result.setRight (right);

   return result;
}

//------------------------------------------------------------------------------
// Note: this function takes firstDisplayed into account, whereas the fullBarRect
// function does not.
//
QRect QEHistogram::backgroundAreaRect (const int groupIndex) const
{
   const int position = groupIndex * this->mSecondBgSize - this->firstDisplayed;
   const int fullWidth = this->useBarWidth + this->useGap + 1;   // also full height
   int top;
   int left;
   int right;
   int bottom;

   // paintArea defines overall paint area.
   //
   if (HORIZONTAL) {
      top = this->paintArea.top ();
      bottom = this->paintArea.bottom ();

      left = this->firstBarTopLeft () + (fullWidth * position) - (this->useGap / 2);
      right = left + (fullWidth * this->mSecondBgSize);
   } else {
      left = this->paintArea.left ();
      right = this->paintArea.right ();
      top = this->firstBarTopLeft () + (fullWidth * position) - (this->useGap / 2);
      bottom = top + (fullWidth * this->mSecondBgSize);
   }

   QRect result;
   result.setTop (top);
   result.setLeft (left);
   result.setBottom (bottom);
   result.setRight (right);

   return result;
}

//------------------------------------------------------------------------------
//
int QEHistogram::indexOfHistogramAreaPosition (const int x, const int y) const
{
   const int han = SELECT (x, y);

   const int guess = (han - this->firstBarTopLeft ()) /
                      MAX (1, this->useBarWidth + this->useGap + 1);

   // Add +/- 2 - very conservative.
   //
   const int lower = MAX (guess - 2, 0);
   const int upper = MIN (guess + 2, this->numberDisplayed - 1);

   int result = -1;

   for (int j = lower; j <= upper; j++) {
      QRect jbar = this->fullBarRect (j);

      if (x >= jbar.left () && x <= jbar.right () &&
          y >= jbar.top ()  && y <= jbar.bottom ()) {
         // found it.
         //
         result = j + this->firstDisplayed;
         if (result >= this->count ()) result = -1;
         break;
      }
   }

   return result;
}

//------------------------------------------------------------------------------
//
int QEHistogram::indexOfHistogramAreaPosition (const QPoint& p) const
{
   return this->indexOfHistogramAreaPosition (p.x (), p.y ());
}

//------------------------------------------------------------------------------
//
int QEHistogram::indexOfPosition (const int x, const int y) const
{
   // Convert from histogram co-ordinates (which is what the external world sees)
   // to local internal histogramArea widget co-ordinates.
   //
   const QPoint golbalPos = this->mapToGlobal (QPoint (x, y));
   const QPoint localPos = this->histogramArea->mapFromGlobal (golbalPos);

   return this->indexOfHistogramAreaPosition (localPos);
}

//------------------------------------------------------------------------------
//
int QEHistogram::indexOfPosition (const QPoint& p) const
{
   return this->indexOfPosition (p.x (), p.y ());
}

//------------------------------------------------------------------------------
//
QRect QEHistogram::positionOfIndex (const int index) const
{
   const QRect temp = this->fullBarRect (index - this->firstDisplayed);

   // Convert from internal histogramArea widget co-ordinates to histogram
   // co-ordinates (which is what the external world sees)
   //
   const QPoint topLeft     = this->mapFromGlobal (this->histogramArea->mapToGlobal (temp.topLeft ()));
   const QPoint bottomRight = this->mapFromGlobal (this->histogramArea->mapToGlobal (temp.bottomRight ()));

   return QRect (topLeft, bottomRight);
}

//------------------------------------------------------------------------------
//
void QEHistogram::paintSecondaryBackground (QPainter& painter) const
{
   QBrush brush;
   QPen pen;

   if (!this->mShowSecondBg) return;  // not required.

   brush.setStyle (Qt::SolidPattern);
   brush.setColor (this->mSecondBgColour);
   painter.setBrush (brush);

   pen.setStyle (Qt::SolidLine);
   pen.setWidth (1);
   pen.setColor (this->mSecondBgColour);
   painter.setPen (pen);

   // Only the 'odd' groups have the alternative back ground.
   //
   const int axisOffset = QEScaling::scale (4);
   const int finishBottomRight = SELECT (this->paintArea.right (),
                                         this->paintArea.bottom () - axisOffset);

   int first;
   first = this->firstDisplayed / this->mSecondBgSize;
   if (first %2 == 0) first--;  // must be odd and round down.
   first = MAX (1, first);

   int last = 1199;   // +infinity sanity check
   for (int j = first; j <= last; j += 2) {
      QRect bgArea = this->backgroundAreaRect (j);

      if (HORIZONTAL) {
         if (bgArea.left () >= finishBottomRight) break;    // Off to the side
      } else {
         if (bgArea.top ()  >= finishBottomRight) break;    // Off to the side
      }
      painter.drawRect (bgArea);
   }
}

//------------------------------------------------------------------------------
//
bool QEHistogram::paintItem (QPainter& painter,
                             const int position,
                             const int valueIndex) const
{
   const int axisOffset = QEScaling::scale (4);
   const int finishBottomRight = SELECT (this->paintArea.right (),
                                         this->paintArea.bottom () - axisOffset);
   QRect bar;
   double value;
   double base;
   double baseLineFraction;
   double valueFraction;
   int topRight;
   int bottomLeft;
   QColor colour;
   QColor boarderColour;
   QBrush brush;
   QPen pen;

   bar = this->fullBarRect (position);

   if (HORIZONTAL) {
      if (bar.left () >= finishBottomRight) return false;   // Off to the side
      if (bar.right () > finishBottomRight) {
         bar.setRight (finishBottomRight);                  // Truncate
         if (bar.width () < 5) return false;                // Tooo small!!
      }
   } else {
      if (bar.top () >= finishBottomRight) return false;    // Off to the side
      if (bar.bottom () > finishBottomRight) {
         bar.setBottom (finishBottomRight);                 // Truncate
         if (bar.height () < 5) return false;               // Tooo small!!
      }
   }

   value = this->dataArray.value (valueIndex, NO_DATA_VALUE);
   base = this->mBaseLine;

   // Is value invalid, i.e. un-defined BUT still in paint area?
   //
   if (isNullDataValue (value)) return true;

   if (this->mLogScale) {
      value = LOG10 (value);
      base = LOG10 (base);
   }

   valueFraction = (value             - this->drawMinimum) /
                   (this->drawMaximum - this->drawMinimum);
   valueFraction = LIMIT (valueFraction, 0.0, 1.0);

   baseLineFraction = (base              - this->drawMinimum) /
                      (this->drawMaximum - this->drawMinimum);
   baseLineFraction = LIMIT (baseLineFraction, 0.0, 1.0);

   if (HORIZONTAL) {
      // Top based on fraction which in turn based on value.
      // Note: top increases as value/fraction decreases.
      //
      topRight   = bar.bottom () - (int) (valueFraction * bar.height ());
      bottomLeft = bar.bottom () - (int) (baseLineFraction * bar.height ());

      bar.setBottom (bottomLeft);
      bar.setTop (topRight);
   } else {
      // Ditto
      topRight   = bar.left () + (int) (valueFraction * bar.width ());
      bottomLeft  = bar.left () + (int) (baseLineFraction * bar.width ());

      bar.setLeft (bottomLeft);
      bar.setRight (topRight);
   }

   // All good to go - set up colour.
   //
   colour = this->getPaintColour (valueIndex);
   boarderColour = QEUtilities::darkColour (colour);

   if (!this->isEnabled ()) {
      colour = QEUtilities::blandColour (colour);
      boarderColour = QEUtilities::blandColour (boarderColour);
   }

   brush.setStyle (Qt::SolidPattern);
   brush.setColor (colour);
   painter.setBrush (brush);

   pen.setStyle (Qt::SolidLine);
   pen.setWidth (1);
   if (this->mDrawBorder) {
      // Use darker version of the color for the boarder.
      //
      pen.setColor (boarderColour);
   } else {
      pen.setColor (colour);
   }
   painter.setPen (pen);

   painter.drawRect (bar);
   return true;
}

//------------------------------------------------------------------------------
// Ensure text is generated consistantly.
//
QString QEHistogram::coordinateText (const double value) const
{
   QString result;

   if (this->mLogScale) {
      // Scale, this given value, is logged, must unlog it.
      //
      result = QString ("%1").arg (EXP10 (value), 0, 'e', 0);
   } else {
      result = QString ("%1").arg (value, 0, 'f', this->axisPainter->getPrecision());
   }
   return result;
}

//------------------------------------------------------------------------------
// TODO - get this inform from axis painter.
//
int QEHistogram::maxPaintTextWidth (QPainter& painter) const
{
   int result;

   if (this->mShowScale) {
      // yes - scale present. Find required text width.
      //
      QFontMetrics fm = painter.fontMetrics ();
      result = 1;
      for (int j = 0; true; j++) {
         double value = this->drawMinimum + (j*this->drawMajor);
         if (value > this->drawMaximum) break;
         if (j > 1000) break;  // sainity check

         QString text = this->coordinateText (value);
         int w = fm.horizontalAdvance (text);
         if (result < w) result = w;
      }
   } else {
      // no scale - use minimal "text" width.
      //
      result = 0;
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEHistogram::paintGrid (QPainter& painter, const QColor& penColour) const
{
   const int axisOffset = QEScaling::scale (4);

   QPen pen;
   double value;
   double fraction;

   pen.setColor (penColour);
   pen.setWidth (1);
   pen.setStyle (Qt::DashLine);
   painter.setPen (pen);

   for (int j = 0; true; j++) {
      int x = 0;
      int y = 0;
      value = this->drawMinimum + (j*this->drawMajor);
      if (value > this->drawMaximum) break;
      if (j > 1000) break;  // sainity check

      fraction = (value             - this->drawMinimum) /
                 (this->drawMaximum - this->drawMinimum);

      // Same idea as we used in paintItem.
      //
      if (HORIZONTAL) {
         y = this->paintArea.bottom () - (int) (fraction * this->paintArea.height ());
         if (this->mShowGrid && (j > 0)) {
            painter.drawLine (this->paintArea.left () - axisOffset, y,
                              this->paintArea.right(),              y);
         }
      } else {
         x = this->paintArea.left () + (int) (fraction * this->paintArea.width ());
         if (this->mShowGrid && (j > 0)) {
            painter.drawLine (x, this->paintArea.top () - axisOffset,
                              x, this->paintArea.bottom ());
         }
      }
   }

   if (this->mDrawAxies) {
      pen.setWidth (1);
      pen.setStyle (Qt::SolidLine);
      painter.setPen (pen);

      if (HORIZONTAL) {
         painter.drawLine (this->paintArea.left () - axisOffset, this->paintArea.bottom () + axisOffset,
                           this->paintArea.right (),             this->paintArea.bottom () + axisOffset);

      } else {
         painter.drawLine (this->paintArea.left () - axisOffset, this->paintArea.top (),
                           this->paintArea.left () - axisOffset, this->paintArea.bottom () + axisOffset);
      }
   }
}

//------------------------------------------------------------------------------
//
void QEHistogram::paintAllItems ()
{
   const int numberGrid = 5;   // approx number of grid lines.

   // Use back ground colour to define style.
   //
   const QColor bgc = this->getBackgroundColour ();

   // Only apply style on change as this causes a new paint event.
   // Maybe we just just paint a rectangle of the appropriate colour.
   //
   QString ownStyle = QEUtilities::colourToStyle (bgc);
   if (this->styleSheet() != ownStyle) {
      this->setStyleSheet (ownStyle);
   }

   // Determine font/grid pen colour and apply to axis painter
   //
   QColor penColour = QEUtilities::fontColour (bgc); // black/white
   if (!this->isEnabled ()) {
      penColour = QEUtilities::blandColour (penColour);   // washed/greyed out
   }
   this->axisPainter->setPenColour (penColour);

   // Draw everything with anti-aliasing off.
   //
   QPainter painter (this->histogramArea);
   painter.setRenderHint (QPainter::Antialiasing, false);

   double useMinimum = this->mMinimum;
   double useMaximum = this->mMaximum;
   if (this->mAutoScale) {
      // Auto scale is true - find min amd max values.
      //
      bool foundValue = false;
      double searchMinimum = +1.0E25;
      double searchMaximum = -1.0E25;
      for (int j = 0; j < this->dataArray.count (); j++) {
         double v = this->dataArray.at (j);
         if (isNullDataValue (v)) continue;
         searchMinimum  = MIN (v, searchMinimum);
         searchMaximum  = MAX (v, searchMaximum);
         foundValue = true;
      }
      if (foundValue) {
         useMinimum  = searchMinimum;
         useMaximum  = searchMaximum;
      }
   }

   // Do not allow ultra small spans, which will occur when autoscaling
   // a histogram with a single value, or set of identical values.
   //
   double l1 = 1.0e-6 * ABS (useMinimum);
   double l2 = 1.0e-6 * ABS (useMaximum);
   double useMinSpan = MAX (MINIMUM_SPAN, MAX (l1, l2));

   if ((useMaximum - useMinimum) < useMinSpan) {
      double midway = (useMaximum + useMinimum)/2.0;
      useMinimum = midway - useMinSpan/2.0;
      useMaximum = midway + useMinSpan/2.0;
   }

   // Now calc draw min max  - log of min / max if necessary.
   //
   QEDisplayRanges displayRange;
   displayRange.setRange (useMinimum, useMaximum);

   if (this->mLogScale) {
      displayRange.adjustLogMinMax (this->drawMinimum, this->drawMaximum, this->drawMajor);
      // We use, and this store,  the log of these values when using the log scale.
      // drawMajor already reflects the scale scale and is typicaly 1 (as in 1 decade).
      //
      this->axisPainter->setLogScale (true);
      this->axisPainter->setMinimum (this->drawMinimum);
      this->axisPainter->setMaximum (this->drawMaximum);
      this->axisPainter->setMajorMinorRatio (1);

      this->drawMinimum = LOG10 (this->drawMinimum);
      this->drawMaximum = LOG10 (this->drawMaximum);

   } else {
      displayRange.adjustMinMax (numberGrid, true,
                                 this->drawMinimum, this->drawMaximum, this->drawMajor);
      this->axisPainter->setLogScale (false);
      this->axisPainter->setMinimum (this->drawMinimum);
      this->axisPainter->setMaximum (this->drawMaximum);
      this->axisPainter->setMinorInterval (this->drawMajor / 5.0);
      this->axisPainter->setMajorMinorRatio (5);
   }

   // Define actual historgram draw area ...
   //
   const int extra = QEScaling::scale (2);
   const int axisOffset = QEScaling::scale (4);

   QRect histAreaGeo = this->histogramArea->geometry ();
   QFont ownFont (this->axisPainter->font ());
   int halfPointSize = (ownFont.pointSize () + 1) / 2;
   int halfTextWidth = (this->maxPaintTextWidth (painter) + 1) / 2;

   if (HORIZONTAL) {
      this->paintArea.setTop (halfPointSize + 1);
      this->paintArea.setBottom (histAreaGeo.height () - halfPointSize - axisOffset);
      this->paintArea.setLeft (extra);
      this->paintArea.setRight (histAreaGeo.width ());
      this->axisPainter->setIndent (halfPointSize, halfPointSize + extra);
   } else {
      this->paintArea.setTop (0);
      this->paintArea.setBottom (histAreaGeo.height ());
      this->paintArea.setLeft (halfTextWidth);
      this->paintArea.setRight (histAreaGeo.width () - halfTextWidth - extra);
      this->axisPainter->setIndent (halfTextWidth, halfTextWidth);
   }

   // Do grid and axis - note this might tweak useMinimum/useMaximum.
   //
   this->paintSecondaryBackground (painter);
   this->paintGrid (painter, penColour);

   this->useGap = this->mGap;
   this->useBarWidth = this->mBarWidth;
   if (this->mAutoBarGapWidths) {
      const int n = this->dataArray.count ();

      if (n <= 1) {
         this->useGap = 0;
         this->useBarWidth = this->paintArea.width ();
      } else {
         const int markSpace = 6;
         // For large n itemWidth is essentially paintArea.width / n
         // For small n, this accounts for n bars and n-1 gaps.
         //
         int itemWidth = ((markSpace + 1) * paintArea.width ()) / ((markSpace + 1)*n - 1);
         if (itemWidth < 3) itemWidth = 3;
         this->useGap = itemWidth / markSpace;
         // There is an implicit +1 in the fullBarRect function.
         this->useBarWidth = MAX (1, itemWidth - this->useGap - 1);
      }
   }

   // Maximum number of items that could be drawn.
   //
   const int maxDrawable = this->dataArray.count () - this->firstDisplayed;

   this->numberDisplayed = 0;
   for (int posnIndex = 0; posnIndex < maxDrawable; posnIndex++) {
      int dataIndex = this->firstDisplayed + posnIndex;
      bool painted = this->paintItem (painter, posnIndex, dataIndex);
      if (painted) {
         this->numberDisplayed = posnIndex + 1;
      } else {
         break;
      }
   }

   // Lastly ...
   //
   const int max = this->scrollMaximum ();
   this->scrollbar->setRange (0, max);
   this->scrollbar->setVisible (max > 0);
}

//------------------------------------------------------------------------------
//
void QEHistogram::onMouseIndexChanged (const int) { }

//------------------------------------------------------------------------------
//
void QEHistogram::onMouseIndexPressed (const int, const Qt::MouseButton) { }


//------------------------------------------------------------------------------
//
bool QEHistogram::eventFilter (QObject *obj, QEvent* event)
{
   const QEvent::Type type = event->type ();
   bool result = false;

   switch (type) {

      case QEvent::MouseMove:
         if (obj == this->histogramArea) {
            const QMouseEvent* mouseEvent = static_cast<QMouseEvent *> (event);
            const QPoint pos = mouseEvent->pos ();
            const int index = this->indexOfHistogramAreaPosition (pos);
            if (this->lastEmittedIndex != index) {
               this->onMouseIndexChanged (index);
               emit this->mouseIndexChanged (index);
               this->lastEmittedIndex = index;
            }
            result = true; // event has been handled.
         }
         break;

      case QEvent::MouseButtonPress:
         if (obj == this->histogramArea) {
            const QMouseEvent* mouseEvent = static_cast<QMouseEvent *> (event);
            const QPoint pos = mouseEvent->pos ();
            const int index = this->indexOfHistogramAreaPosition (pos);
            if (index >= 0) {
               Qt::MouseButton button = mouseEvent->button();
               this->onMouseIndexPressed (index, button);
               emit this->mouseIndexPressed (index, button);
            }
            result = true; // event has been handled.
         }
         break;

      case QEvent::Leave:
         if (obj == this->histogramArea) {
            int index = -1;    // by definition
            if (this->lastEmittedIndex != index) {
               emit this->mouseIndexChanged (index);
               this->lastEmittedIndex = index;
            }
            result = true; // event has been handled.
         }
         break;

      case QEvent::Paint:
         if (obj == this->histogramArea) {
            this->paintAllItems ();
            result = true;  // event has been handled
         }
         break;

      case QEvent::FontChange:
         if (obj == this) {
            // Font must be mapped to the internal axisPainter
            //
            if (this->axisPainter) {
               this->axisPainter->setFont (this->font ());
               this->axisPainter->update ();
            }
         }
         break;

      default:
         result = false;   // event has not been handled
         break;
   }

   if (!result) {
      // event not handled - call parent.
      result = QFrame::eventFilter (obj, event);
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEHistogram::createTestData ()
{
   QRandomGenerator rg;

   this->clear ();

   for (int j = 0; j < this->mTestSize; j++) {
      double f;
      double v;
      QColor c;

      // Do special for first and last.
      //
      if (j == 0 || j == this->mTestSize - 1) {
         v = this->mMaximum;
      } else {
         f = rg.generateDouble();
         v = f * (this->mMaximum - this->mMinimum) +  this->mMinimum;
      }

      f = rg.generateDouble();
      c.setHsl (int (f * 360.0), 255, 128);

      this->dataArray << v;
      this->colourArray << c;
   }
}

//==============================================================================
// Property functions (standard)
//==============================================================================
//
// Standard propery access macro.
//
#define PROPERTY_ACCESS(type, name, convert, extra)          \
                                                             \
void QEHistogram::set##name (const type value) {             \
   type temp;                                                \
   temp = convert;                                           \
   if (this->m##name != temp) {                              \
      this->m##name = temp;                                  \
      extra;                                                 \
      this->update ();                                       \
   }                                                         \
}                                                            \
                                                             \
type QEHistogram::get##name () const {                       \
   return this->m##name;                                     \
}

#define NO_EXTRA

PROPERTY_ACCESS (int,    BarWidth,         LIMIT (value, 1, 120),                                 NO_EXTRA)
PROPERTY_ACCESS (int,    Gap,              LIMIT (value, 0, 20),                                  NO_EXTRA)
PROPERTY_ACCESS (int,    Margin,           LIMIT (value, 0, 20),                                  this->layoutA->setContentsMargins (this->mMargin, this->mMargin, 
                                                                                                                                     this->mMargin, this->mMargin))
PROPERTY_ACCESS (double, Minimum,          LIMIT (value, -1.0E20, this->mMaximum - MINIMUM_SPAN), this->mAutoScale = false)
PROPERTY_ACCESS (double, Maximum,          LIMIT (value, this->mMinimum + MINIMUM_SPAN, +1.0E40), this->mAutoScale = false)
PROPERTY_ACCESS (double, BaseLine,         value,                                                 NO_EXTRA)
PROPERTY_ACCESS (bool,   AutoScale,        value,                                                 NO_EXTRA)
PROPERTY_ACCESS (bool,   AutoBarGapWidths, value,                                                 NO_EXTRA)
PROPERTY_ACCESS (bool,   ShowScale,        value,                                                 this->axisPainter->setVisible (this->mShowScale))
PROPERTY_ACCESS (bool,   ShowGrid,         value,                                                 NO_EXTRA)
PROPERTY_ACCESS (bool,   LogScale,         value,                                                 NO_EXTRA)
PROPERTY_ACCESS (bool,   DrawAxies,        value,                                                 this->axisPainter->setHasAxisLine (this->mDrawAxies);)
PROPERTY_ACCESS (bool,   DrawBorder,       value,                                                 NO_EXTRA)
PROPERTY_ACCESS (QColor, BackgroundColour, value,                                                 NO_EXTRA)
PROPERTY_ACCESS (QColor, SecondBgColour,   value,                                                 NO_EXTRA)
PROPERTY_ACCESS (int,    SecondBgSize,     LIMIT (value, 1, 100),                                 NO_EXTRA)
PROPERTY_ACCESS (bool,   ShowSecondBg,     value,                                                 NO_EXTRA)
PROPERTY_ACCESS (QColor, BarColour,        value,                                                 NO_EXTRA)
PROPERTY_ACCESS (int,    TestSize,         LIMIT (value, 0, MAX_CAPACITY),                        this->createTestData ())

#undef PROPERTY_ACCESS

//------------------------------------------------------------------------------
// Specific property handlers,
//
void QEHistogram::setOrientation (const Qt::Orientation orientation)
{
   if (this->mOrientation != orientation) {
      this->mOrientation = orientation;

      // Deconstruct.
      //
      if (this->layoutA) {
         this->layoutA->removeWidget (this->histogramAxisPlusArea);
         this->layoutA->removeWidget (this->scrollbar);

         delete this->layoutA;
         this->layoutA = NULL;
      }

      if (this->layoutB) {
         this->layoutB->removeWidget (this->axisPainter);
         this->layoutB->removeWidget (this->histogramArea);

         delete this->layoutB;
         this->layoutB = NULL;
      }

      // Reconstruct
      //
      this->layoutB = SELECT ((QBoxLayout*) new QHBoxLayout (this->histogramAxisPlusArea),
                              (QBoxLayout*) new QVBoxLayout (this->histogramAxisPlusArea));

      this->layoutB->setContentsMargins (0, 0, 0, 0);
      this->layoutB->setSpacing (0);

      this->axisPainter->setOrientation (SELECT (QEAxisPainter::Bottom_To_Top,
                                                 QEAxisPainter::Left_To_Right));

      if (HORIZONTAL) {
         this->axisPainter->setFixedWidth (60);
         this->axisPainter->setMaximumHeight (QWIDGETSIZE_MAX);
      } else {
         this->axisPainter->setFixedHeight (30);
         this->axisPainter->setMaximumWidth (QWIDGETSIZE_MAX);
      }
      this->layoutB->addWidget (SELECT (this->axisPainter, this->histogramArea));
      this->layoutB->addWidget (SELECT (this->histogramArea, this->axisPainter));


      this->layoutA = SELECT ((QBoxLayout*) new QVBoxLayout (this),
                              (QBoxLayout*) new QHBoxLayout (this));

      this->layoutA->setContentsMargins (this->mMargin, this->mMargin, this->mMargin, this->mMargin);
      this->layoutA->setSpacing (2);

      this->scrollbar->setOrientation (orientation);
      this->layoutA->addWidget (SELECT (this->histogramAxisPlusArea, this->scrollbar));
      this->layoutA->addWidget (SELECT (this->scrollbar, this->histogramAxisPlusArea));

      this->update ();
   }
}

//------------------------------------------------------------------------------
//
Qt::Orientation QEHistogram::getOrientation () const
{
   return this->mOrientation;
}

//------------------------------------------------------------------------------
//
void QEHistogram::setAxisPrecision (const int precision)
{
   this->axisPainter->setPrecision (precision);
   this->update ();
}

//------------------------------------------------------------------------------
//
int QEHistogram::getAxisPrecision () const
{
   return this->axisPainter->getPrecision ();
}

// end
