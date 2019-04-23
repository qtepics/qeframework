/*  QEAxisPainter.cpp
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
 *  Copyright (c) 2015,2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <math.h>

#include <QDebug>
#include <QFontMetrics>
#include <QBrush>
#include <QPen>
#include <QPoint>
#include <QPainter>

#include <alarm.h>

#include <QEPlatform.h>
#include <QECommon.h>
#include <QCaAlarmInfo.h>
#include <QEWidget.h>
#include <QEAxisPainter.h>

#define DEBUG qDebug () << "QEAxisPainter" << __LINE__ << __FUNCTION__

#define MIN_VALUE         (-1.0E+24)
#define MAX_VALUE         (+1.0E+24)
#define MIN_INTERVAL      (+1.0E-20)
#define MAX_INTERVAL      (+1.0E+23)
#define MAX_MINOR_TICKS   1000

//------------------------------------------------------------------------------
//
QEAxisPainter::QEAxisPainter (QWidget* parent) : QWidget (parent)
{
   this->setMinimumHeight (20);

   this->bandList.clear ();
   this->mModulo = 0.0;
   this->mPrecision = 1;
   this->mMinimum = 0.0;
   this->mMaximum = 10.0;
   this->mMinorInterval = 0.2;
   this->mMajorMinorRatio = 5;   // => majorInterval = 1.0
   this->mIsLogScale = false;
   this->mHasAxisLine = false;
   this->mTopLeftIndent = 20;
   this->mRightBottomIndent = 20;
   this->mGap = 2;
   this->mAutoFixedSize = false;
   this->mOrientation = Left_To_Right;
   this->mTextPosition = BelowLeft;

   for (int j = 0; j < NUMBER_OF_MARKERS; j++) {
      this->markerColour [j] =  QColor (0, 0, 0, 255);  // black
      this->markerVisible [j] = false;
      this->markerValue [j] = 0.0;
   }

   this->mPenColour = QColor (0, 0, 0, 255);  // black

   // Construct inerator based on current/default attributes.
   //
   this->iterator = new QEAxisIterator (this->mMinimum, this->mMaximum,
                                        this->mMinorInterval, this->mMajorMinorRatio,
                                        this->mIsLogScale);
}

//------------------------------------------------------------------------------
//
QEAxisPainter::~QEAxisPainter ()
{
   delete this->iterator;
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setMinimum (const double minimum)
{
   // Ensure in range
   //
   this->mMinimum = LIMIT (minimum, MIN_VALUE, MAX_VALUE);

   // Ensure consistant.
   //
   this->mMaximum = MAX (this->mMaximum, this->mMinimum + MIN_INTERVAL);

   double n = (this->mMaximum - this->mMinimum) / this->getMinorInterval ();
   if (n > MAX_MINOR_TICKS) {
      this->setMinorInterval (this->mMinorInterval * n / MAX_MINOR_TICKS);
   }

   this->iterator->reInitialise (this->mMinimum, this->mMaximum,
                                 this->mMinorInterval,
                                 this->mMajorMinorRatio, this->mIsLogScale);
   this->update ();
}

//------------------------------------------------------------------------------
//
double QEAxisPainter::getMinimum () const
{
   return this->mMinimum;
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setMaximum (const double maximum)
{
   // Ensure in range
   //
   this->mMaximum = LIMIT (maximum, MIN_VALUE, MAX_VALUE);

   // Ensure consistant.
   //
   this->mMinimum = MIN (this->mMinimum, this->mMaximum - MIN_INTERVAL);

   double n = (this->mMaximum - this->mMinimum) / this->getMinorInterval ();
   if (n > MAX_MINOR_TICKS) {
      this->setMinorInterval (this->mMinorInterval * n / MAX_MINOR_TICKS);
   }

   this->iterator->reInitialise (this->mMinimum, this->mMaximum,
                                 this->mMinorInterval,
                                 this->mMajorMinorRatio, this->mIsLogScale);
   this->update ();
}

//------------------------------------------------------------------------------
//
double QEAxisPainter::getMaximum  () const
{
   return this->mMaximum;
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setModulo (const double modulo)
{
   const double minModulo = (this->mMaximum - this->mMinimum) / 10.0;

   this->mModulo = modulo;

   // Ensure in range
   //
   if (this->mModulo > 0.0) {
      this->mModulo = MAX (this->mModulo, +minModulo);
   } else if (this->mModulo < 0.0) {
      this->mModulo = MIN (this->mModulo, -minModulo);
   }
   this->update ();
}

//------------------------------------------------------------------------------
//
double QEAxisPainter::getModulo () const
{
   return this->mModulo;
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setPrecision (const int precision)
{
   this->mPrecision = LIMIT (precision, 0, 6);
}

//------------------------------------------------------------------------------
//
int QEAxisPainter::getPrecision () const
{
   return this->mPrecision;
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setMinorInterval (const double minorInterval)
{
   // Ensure in range
   //
   double limitedMin = LIMIT (minorInterval, MIN_INTERVAL, MAX_INTERVAL);
   double dynamicMin = (this->mMaximum - this->mMinimum) / MAX_MINOR_TICKS;

   this->mMinorInterval = MAX (limitedMin, dynamicMin);

   this->iterator->reInitialise (this->mMinimum, this->mMaximum,
                                 this->mMinorInterval,
                                 this->mMajorMinorRatio, this->mIsLogScale);
   this->update ();
}

//------------------------------------------------------------------------------
//
double QEAxisPainter::getMinorInterval  () const
{
   return this->mMinorInterval;
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setMajorMinorRatio (const int majorMinorRatio)
{
   // Ensure in range
   //
   this->mMajorMinorRatio = MAX (1, majorMinorRatio);

   this->iterator->reInitialise (this->mMinimum, this->mMaximum,
                                 this->mMinorInterval,
                                 this->mMajorMinorRatio, this->mIsLogScale);
   this->update ();
}

//------------------------------------------------------------------------------
//
int QEAxisPainter::getMajorMinorRatio () const
{
   return this->mMajorMinorRatio;
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setHasAxisLine (const bool hasAxisLine)
{
   this->mHasAxisLine = hasAxisLine;
   this->update ();
}

//------------------------------------------------------------------------------
//
bool QEAxisPainter::getHasAxisLine () const
{
   return this->mHasAxisLine;
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setLogScale (const bool value)
{
   this->mIsLogScale = value;
   this->iterator->reInitialise (this->mMinimum, this->mMaximum,
                                 this->mMinorInterval,
                                 this->mMajorMinorRatio, this->mIsLogScale);
   this->update ();
}

//------------------------------------------------------------------------------
//
bool QEAxisPainter::getLogScale () const
{
   return this->mIsLogScale;
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setOrientation (const Orientations orientation)
{
   this->mOrientation = orientation;
   this->update ();
}

//------------------------------------------------------------------------------
//
QEAxisPainter::Orientations QEAxisPainter::getOrientation () const
{
   return this->mOrientation;
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setTextPosition (const TextPositions textPositions)
{
   this->mTextPosition = textPositions;
   this->update ();
}

//------------------------------------------------------------------------------
//
QEAxisPainter::TextPositions QEAxisPainter::getTextPosition () const
{
   return this->mTextPosition;
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setPenColour (const QColor colour)
{
   this->mPenColour = colour;
   this->update ();
}

//------------------------------------------------------------------------------
//
QColor QEAxisPainter::getPenColour () const
{
   return this->mPenColour;
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setIndent (const int topLeftIndent, const int rightBottomIndent)
{
   this->mTopLeftIndent = MAX (topLeftIndent, 0);
   this->mRightBottomIndent = MAX (rightBottomIndent, 0);
   this->update ();
}

//------------------------------------------------------------------------------
//
int QEAxisPainter::getTopLeftIndent  () const
{
   return this->mTopLeftIndent;
}

//------------------------------------------------------------------------------
//
int QEAxisPainter::getRightBottomIndent  () const
{
   return this->mRightBottomIndent;
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setIndent (const int indent)
{
   this->setIndent (indent, indent);
   this->update ();
}

//------------------------------------------------------------------------------
//
int QEAxisPainter::getIndent () const
{
   return (this->mTopLeftIndent + this->mRightBottomIndent) / 2;
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setGap (const int gap)
{
   this->mGap = MAX (gap, 0);
   this->update ();
}

//------------------------------------------------------------------------------
//
int QEAxisPainter::getGap  () const
{
   return this->mGap;
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setAutoFixedSize (const bool enabled)
{
   this->mAutoFixedSize = enabled;
   this->update ();
}

//------------------------------------------------------------------------------
//
bool QEAxisPainter::getAutoFixedSize () const
{
   return this->mAutoFixedSize;
}


//------------------------------------------------------------------------------
//
#define ASSERT_VALID_INDEX(index, action)  {                \
   if ((index < 0) && (index >= NUMBER_OF_MARKERS)) {       \
      action;                                               \
   }                                                        \
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setMarkerColour (const int index, const QColor& colour)
{
   ASSERT_VALID_INDEX (index, return);
   this->markerColour [index] = colour;
   this->update ();
}

//------------------------------------------------------------------------------
//
QColor QEAxisPainter::getMarkerColour (const int index) const
{
   ASSERT_VALID_INDEX (index, return QColor (0,0,0,0));
   return this->markerColour [index];
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setMarkerVisible (const int index, const bool isVisible)
{
   ASSERT_VALID_INDEX (index, return);
   this->markerVisible [index] = isVisible;
   this->update ();
}

//------------------------------------------------------------------------------
//
bool QEAxisPainter::getMarkerVisible (const int index) const
{
   ASSERT_VALID_INDEX (index, return false);
   return this->markerVisible [index];
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setMarkerValue (const int index, const double value)
{
   ASSERT_VALID_INDEX (index, return);
   this->markerValue [index] = value;
   this->update ();
}

//------------------------------------------------------------------------------
//
double QEAxisPainter::getMarkerValue (const int index) const
{
   ASSERT_VALID_INDEX (index, return 0.0);
   return this->markerValue [index];
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::setColourBandList (const QEColourBandList& bandListIn)
{
   this->bandList = bandListIn;
   this->update ();
}

//------------------------------------------------------------------------------
//
QEColourBandList QEAxisPainter::getColourBandList () const
{
   return this->bandList;
}

//------------------------------------------------------------------------------
//
bool QEAxisPainter::isLeftRight () const
{
   return (this->mOrientation == QEAxisPainter::Left_To_Right) ||
          (this->mOrientation == QEAxisPainter::Right_To_Left);
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::draw (QWidget* widget)
{
   if (!widget) return;  // sanity check

   // Tick sizes on axis
   //
   const int markerTick = 14;
   const int minorTick = 5;
   const int majorTick = 10;
   const int pointSize = widget->font ().pointSize();

   QPainter painter (widget);
   QPen pen;
   QBrush brush;
   QColor penColour;
   int width;
   int height;

   int sign;
   int x_first, x_last;
   int y_first, y_last;
   bool isMajor;
   double value;

   // Draw everything with antialiasing off.
   //
   painter.setRenderHint (QPainter::Antialiasing, false);

   // Alias/edge adjustment.
   // Note: Actual size appears to be 1 less than widget width/height.
   //
   width  = widget->geometry ().width () - 1;
   height = widget->geometry ().height () - 1;

   switch (this->mOrientation) {

      case Left_To_Right:
         sign = (this->mTextPosition == BelowLeft) ? +1 : -1;
         x_first = this->mTopLeftIndent;
         x_last  = width - this->mRightBottomIndent;
         y_first = (this->mTextPosition == BelowLeft) ? this->mGap : height - this->mGap;
         y_last  = y_first;
         break;

      case Right_To_Left:
         sign = (this->mTextPosition == BelowLeft) ? +1 : -1;
         x_first = width - this->mRightBottomIndent;
         x_last  = this->mTopLeftIndent;
         y_first = (this->mTextPosition == BelowLeft) ? this->mGap : height - this->mGap;
         y_last  = y_first;
         break;

      case Top_To_Bottom:
         sign = (this->mTextPosition == BelowLeft) ? -1 : +1;
         x_first = (this->mTextPosition == BelowLeft) ? width - this->mGap : this->mGap;
         x_last  = x_first;
         y_first = this->mTopLeftIndent;
         y_last  = height - this->mRightBottomIndent;
         break;

      case Bottom_To_Top:
         sign = (this->mTextPosition == BelowLeft) ? -1 : +1;
         x_first = (this->mTextPosition == BelowLeft) ? width - this->mGap : this->mGap;
         x_last  = x_first;
         y_first = height - this->mRightBottomIndent;
         y_last  = this->mTopLeftIndent;
         break;

      default:
         // report an error??
         //
         return;
   }

   // Draw color bands (if any)
   //
   for (int j = 0; j < this->bandList.count (); j++) {
      const QEColourBand band = this->bandList.value (j);
      double fl, gl;
      double fu, gu;
      int x1, x2;
      int y1, y2;
      QRect bandRect;

      pen.setWidth (0);
      pen.setColor (band.colour);
      painter.setPen (pen);

      brush.setColor (band.colour);
      brush.setStyle (Qt::SolidPattern);
      painter.setBrush (brush);

      fl = this->calcFraction (band.lower);
      fu =  this->calcFraction (band.upper);
      gl = 1.0 - fl;
      gu = 1.0 - fu;

      x1 = int (gl * double (x_first) +  fl * double (x_last));
      x2 = int (gu * double (x_first) +  fu * double (x_last));

      y1 = y_first;
      y2 = y1 + majorTick + 1 + pointSize + 1;

      bandRect.setTop (y1);
      bandRect.setBottom (y2);
      bandRect.setLeft (x1);
      bandRect.setRight (x2);
      painter.drawRect (bandRect);
   }

   // Draw markers
   //
   for (int j = 0; j < NUMBER_OF_MARKERS; j++) {
      if (!this->markerVisible [j]) continue;

      double mark = this->markerValue [j];
      if ((mark < this->mMinimum) || (mark > this->mMaximum)) continue;

      double f, g;
      int x, y;
      QPoint p1, p2;

      f = this->calcFraction (mark);
      g = 1.0 - f;    // co-fraction

      x = int (g * double (x_first) +  f * double (x_last));
      y = int (g * double (y_first) +  f * double (y_last));

      p1 = QPoint (x, y);
      p2 = this->isLeftRight () ? QPoint (x, y + sign*markerTick) : QPoint (x + sign*markerTick, y);

      pen.setWidth (5);
      penColour = this->markerColour [j];
      if (!this->isEnabled()) {
         penColour = QEUtilities::blandColour (penColour);
      }
      pen.setColor (penColour);
      painter.setPen (pen);
      painter.drawLine (p1, p2);
   }

   // Draw actual axis
   //
   pen.setWidth (1);
   penColour = this->mPenColour;
   if (!this->isEnabled()) {
      penColour = QEUtilities::blandColour (penColour);
   }
   pen.setColor (penColour);
   painter.setPen (pen);

   // Draw line itself if required
   //
   if (this->mHasAxisLine) {
      QPoint p1 = QPoint (x_first, y_first);
      QPoint p2 = QPoint (x_last,  y_last);
      painter.drawLine (p1, p2);
   }

   QFontMetrics fm = painter.fontMetrics ();
   int maxTextWidth = 0;
   int maxTextHeight = 0;

   for (bool ok = this->iterator->firstValue (value, isMajor, MAX_MINOR_TICKS);
        ok;  ok = this->iterator->nextValue  (value, isMajor)) {

      double f, g;
      int x, y;
      QPoint p1, p2;

      f = this->calcFraction (value);
      g = 1.0 - f;    // co-fraction

      x = int (g * double (x_first) +  f * double (x_last));
      y = int (g * double (y_first) +  f * double (y_last));

      p1 = QPoint (x, y);
      if (isMajor) {
         p2 = this->isLeftRight () ? QPoint (x, y + sign*majorTick) : QPoint (x + sign*majorTick, y);
      }  else {
         p2 = this->isLeftRight () ? QPoint (x, y + sign*minorTick) : QPoint (x + sign*minorTick, y);
      }

      painter.drawLine (p1, p2);

      if (isMajor) {
         QString vt;

         double mvalue = value;
         // Apply modulo processing if specified.
         //
         if (this->mModulo != 0.0) {
            const double n = mvalue / this->mModulo;
            if ((n < 0.0) || (n > 1.0)) {
               mvalue -= floor (n) * this->mModulo;
            }
         }

         if (this->getLogScale ()) {
            vt.setNum (mvalue, 'e', 0);
            // vt.sprintf ("%.0e", mvalue);
         } else {
            vt.setNum (mvalue, 'f', this->mPrecision);
            // vt = QString ("%1").arg (mvalue, 0, 'g', 4);
            // if (!vt.contains(".")) vt.append(".0");
         }

         maxTextWidth = MAX (maxTextWidth, fm.width (vt));
         maxTextHeight = 10;

         p2 = this->isLeftRight () ? QPoint (x, y + sign*(majorTick + 1)) :
                                     QPoint (x + sign*(majorTick + 1), y);

         this->drawAxisText (painter, p2, vt);
      }
   }

   if (this->mAutoFixedSize) {
      if(this->isLeftRight ()) {
         int requiredHeight = maxTextHeight + markerTick + this->mGap;
         if ((widget->minimumHeight () != requiredHeight) &&
             (widget->maximumHeight () != requiredHeight)) {
            widget->setFixedHeight (requiredHeight);
         }
      } else {
         int requiredWidth = maxTextWidth + markerTick + this->mGap;
         if ((widget->minimumWidth () != requiredWidth) &&
             (widget->maximumWidth () != requiredWidth)) {
            widget->setFixedWidth (requiredWidth);
         }
      }
   }
}

//------------------------------------------------------------------------------
//
void QEAxisPainter::paintEvent (QPaintEvent *)
{
   // Do we have a parent?
   // Would we even get a paint event if we didn't?
   //
   QWidget* p = qobject_cast <QWidget*>(this->parent());
   if (p) {
      this->draw (this);
   }
}

//------------------------------------------------------------------------------
//
double QEAxisPainter::calcFraction (const double x)
{
   double result;

   // Calculate the fractional scale and constrain to be in range.
   //
   if (this->getLogScale ()) {
      result = (LOG10 (x)              - LOG10 (this->mMinimum)) /
               (LOG10 (this->mMaximum) - LOG10 (this->mMinimum));
   } else {
      result = (x              - this->mMinimum) /
               (this->mMaximum - this->mMinimum);
   }
   result = LIMIT (result, 0.0, 1.0);

   return result;
}

//------------------------------------------------------------------------------
// Depending on orientation/edge, draws the text releative to nominated position.
//
void QEAxisPainter::drawAxisText (QPainter& painter, const QPoint& position,
                                  const QString& text)
{
   QFont pf (this->font ());
   painter.setFont (pf);

   QFontMetrics fm = painter.fontMetrics ();
   const int textWidth = fm.width (text);
   const int textHeight = pf.pointSize ();  // For height, pointSize seems better than fm.height ()

   int x;
   int y;

   // Find text origin, painter.drawText needs bottom left coordinates.
   //
   if (this->isLeftRight ()) {
      x = position.x () - textWidth / 2;
      y = position.y ();
      if (this->mTextPosition == BelowLeft) {
         y += textHeight;
      }

   } else {
      x = position.x ();
      if (this->mTextPosition == BelowLeft) {
         x -= textWidth;
      }
      y = position.y () + (textHeight + 1) / 2;
   }

   // Font colour same as basix axis colour.
   // If text too wide, then ensure we show most significant part.
   //
   painter.drawText (MAX (1, x), y, text);
}

// end
