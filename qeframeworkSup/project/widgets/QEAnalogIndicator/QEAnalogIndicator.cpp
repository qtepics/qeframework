/*  QEAnalogIndicator.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2011-2019 Australian Synchrotron
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

/*
  This class is a analog version of the Qt progress bar widget.
 */

#include <QEAnalogIndicator.h>

#include <QDebug>
#include <QFontMetrics>
#include <QBrush>
#include <QPen>
#include <QPoint>
#include <QPolygon>
#include <QECommon.h>

#define  _USE_MATH_DEFINES
#include <math.h>

#define DEBUG qDebug () "QEAnalogIndicator" << __LINE__ << __FUNCTION__ << "  "

#define MINIMUM_SPAN        0.000001
#define RADIANS_PER_DEGREE  (M_PI / 180.0)


//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QEAnalogIndicator::QEAnalogIndicator (QWidget *parent) : QWidget (parent)
{
   this->mBorderColour      = QColor (  0,   0,  96);   // dark blue
   this->mBackgroundColour  = QColor (220, 220, 220);   // light gray
   this->mForegroundColour  = QColor ( 55, 155, 255);   // blue
   this->mFontColour        = QColor (0,     0,   0);   // black

   this->mMinimum = 0.0;
   this->mMaximum = 100.0;
   this->mMinorInterval = 4.0;
   this->mMajorMinorRatio = 5;   // => major = 20
   this->mLogScaleInterval = 1;

   this->mOrientation = Left_To_Right;
   this->mMode = Bar;
   this->mShowText = true;
   this->mShowScale = false;
   this->mLogScale = false;
   this->mValue = 0.0;
   this->mCentreAngle = 0;
   this->mSpanAngle = 180;
   this->mIsActive = true;

   // Do thsi only once, not on paintEvent as it caises another paint event.
   //
   this->setAutoFillBackground (false);
   this->setBackgroundRole (QPalette::NoRole);
}


/* ---------------------------------------------------------------------------
 *  Define default size for this widget class.
 */
QSize QEAnalogIndicator::sizeHint () const
{
   return QSize (48, 16);
}

//------------------------------------------------------------------------------
//
double QEAnalogIndicator::calcFraction (const double x)
{
   double result;

   // Calculate the fractional scale and constrain to be in range.
   //
   if (this->getLogScale ()) {
      result = (LOG10 (x)              - LOG10 (this->mMinimum)) /
               (LOG10 (this->mMaximum) - LOG10 (this->mMinimum));
   } else {
      result = (x - this->mMinimum) /
               (this->mMaximum - this->mMinimum);
   }
   result = LIMIT (result, 0.0, 1.0);

   return result;
}

//------------------------------------------------------------------------------
//
bool QEAnalogIndicator::firstValue (int & itc, double & value, bool & isMajor)
{
   double real;
   bool result;

   if (this->getLogScale ()) {
      real = 9.0 * LOG10 (this->mMinimum);
   } else {
      real = this->mMinimum / this->getMinorInterval ();
   }

   // Use floor to round down and - 0.5 to mitigate any rounding effects.
   // Subtract an addition -1 to ensure first call to nextValue returns a
   // value no greater than the first required value.
   //
   itc = int (floor (real) - 0.5) - 1;

   result = this->nextValue (itc, value, isMajor);
   while (result && (value < this->mMinimum)) {
      result = this->nextValue (itc, value, isMajor);
   }
   return result;
}

//------------------------------------------------------------------------------
//
bool QEAnalogIndicator::nextValue  (int & itc, double & value, bool & isMajor)
{
   const int fs = 9;

   int d;
   int f;

   itc++;
   if (this->getLogScale ()) {
      // Ensure round down towards -infinity (as opposed to 0)
      // (Oh how I wish C/C++ has a proper "mod" operator).
      //
      d = itc / fs;
      if ((fs * d) > itc) d--;
      f = itc -(fs * d);
      value = (1.0 + f) * pow (10.0, d);
      if (f == 0) {
         // Is an exact power of 10 - test for being major.
         //
         isMajor = ((d % this->getLogScaleInterval ()) == 0);
      } else {
         // Is not an exact power of 10 - canot be major.
         //
         isMajor = false;
      }
   } else {
      value = itc * this->getMinorInterval ();
      isMajor = ((itc % this->mMajorMinorRatio) == 0);
   }
   return (value <= this->mMaximum);
}

//------------------------------------------------------------------------------
//
QColor QEAnalogIndicator::getBorderPaintColour () const
{
   return (this->isEnabled() && this->getIsActive ())
         ? this->mBorderColour : QEUtilities::blandColour (this->mBorderColour);
}

//------------------------------------------------------------------------------
//
QColor QEAnalogIndicator::getBackgroundPaintColour () const
{
   return (this->isEnabled() && this->getIsActive ())
         ? this->mBackgroundColour : QEUtilities::blandColour (this->mBackgroundColour);
}

//------------------------------------------------------------------------------
//
QColor QEAnalogIndicator::getForegroundPaintColour () const
{
   return (this->isEnabled() && this->getIsActive ())
         ? this->mForegroundColour : QEUtilities::blandColour (this->mForegroundColour);
}


//------------------------------------------------------------------------------
//
QColor QEAnalogIndicator::getFontPaintColour () const
{
   return (this->isEnabled() && this->getIsActive ())
         ? this->mFontColour : QEUtilities::blandColour (this->mFontColour);
}

//------------------------------------------------------------------------------
//
bool QEAnalogIndicator::isLeftRight () const
{
   return (this->mOrientation == Left_To_Right) || (this->mOrientation == Right_To_Left);
}


//------------------------------------------------------------------------------
//
void QEAnalogIndicator::drawAxis  (QPainter & painter, QRect & axis)
{
   QPen pen;
   QBrush brush;

   int x_first, x_last;
   int y_first, y_last;

   int j;
   bool ok;
   bool isMajor;
   double value;
   BandList bandList;

   switch (this->mOrientation) {

      case Left_To_Right:
         x_first = axis.left ();
         x_last  = axis.right ();
         y_first = axis.top ();
         y_last  = axis.top ();
         break;

      case Top_To_Bottom:
         x_first = axis.left ();
         x_last  = axis.left ();
         y_first = axis.top ();
         y_last  = axis.bottom ();
         break;

      case Right_To_Left:
         x_first = axis.right ();
         x_last  = axis.left ();
         y_first = axis.top ();
         y_last  = axis.top ();
         break;

      case Bottom_To_Top:
         x_first = axis.left ();
         x_last  = axis.left ();
         y_first = axis.bottom ();
         y_last  = axis.top ();
         break;

      default:
         // report an error??
         //
         return;
   }

   // Note: this is a dispatching call.
   //
   bandList = this->getBandList ();
   for (j = 0; j < bandList.count (); j++) {
      Band band = bandList.at (j);
      double fl;
      double fu;
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

      if (this->isLeftRight ()) {
         x1 = int (x_first + (fl * double (x_last - x_first + 1)));
         x2 = int (x_first + (fu * double (x_last - x_first + 1)));

         y1 = axis.top () + 1;
         y2 = y1 + 4;
      } else {
         y1 = int (y_first + (fl * double (y_last - y_first + 1)));
         y2 = int (y_first + (fu * double (y_last - y_first + 1)));

         x1 = axis.left () + 1;
         x2 = x1 + 4;
      }

      bandRect.setTop (y1);
      bandRect.setBottom (y2);
      bandRect.setLeft (x1);
      bandRect.setRight (x2);
      painter.drawRect (bandRect);
   }

   pen.setWidth (1);
   pen.setColor (this->getFontPaintColour ());
   painter.setPen (pen);

   for (ok = this->firstValue (j, value, isMajor); ok;
        ok = this->nextValue  (j, value, isMajor)) {

      // Tick sizes on axis
      //
      const int minorTick = 5;
      const int majorTick = 10;
      const int pointSize = 7;

      double f;
      int x, y;
      QPoint p1, p2;

      f = this->calcFraction (value);

      x = int (x_first + (f * double (x_last - x_first + 1)));
      y = int (y_first + (f * double (y_last - y_first + 1)));

      p1 = QPoint (x, y);
      if (isMajor) {
         p2 = this->isLeftRight () ? QPoint (x, y + majorTick) : QPoint (x + majorTick, y);
      }  else {
         p2 = this->isLeftRight () ? QPoint (x, y + minorTick) : QPoint (x + minorTick, y);
      }

      painter.drawLine (p1, p2);

      if (isMajor) {
         QString vt;

         if (this->getLogScale () ) {
            vt.sprintf ("%.0e", value);
         } else {
            vt.sprintf ("%.1f", value);
         }

         p2 = this->isLeftRight () ? QPoint (x, y + majorTick + 1) : QPoint (x + majorTick + 1, y);
         this->drawAxisText (painter, p2, vt, pointSize);
      }
   }
}

//------------------------------------------------------------------------------
//
void QEAnalogIndicator::drawOutline (QPainter & painter, QRect & outline)
{
   QPen pen;
   QBrush brush;

   pen.setWidth (1);
   pen.setColor (this->getBorderPaintColour ());
   painter.setPen (pen);

   brush.setStyle (Qt::SolidPattern);
   brush.setColor (this->getBackgroundPaintColour ());
   painter.setBrush (brush);

   painter.drawRect (outline);
}

//------------------------------------------------------------------------------
//
void QEAnalogIndicator::drawBar (QPainter & painter, QRect &area,
                                 const double fraction)
{
   int temp;
   QPen pen;
   QBrush brush;

   QRect barRect (area);

   switch (this->mOrientation) {

      case Left_To_Right:
         // Convert fractions back to pixels.
         //
         temp = int (fraction * (area.right () - area.left ()));
         barRect.setRight (area.left () + temp);
         break;

      case Top_To_Bottom:
         temp = int (fraction * (area.bottom () - area.top ()));
         barRect.setBottom (area.top () + temp);
         break;

      case Right_To_Left:
         temp = int (fraction * (area.right () - area.left ()));
         barRect.setLeft (area.right () - temp);
         break;

      case Bottom_To_Top:
         temp = int (fraction * (area.bottom () - area.top ()));
         barRect.setTop (area.bottom () - temp);
         break;

      default:
         // report an error??
         //
         return;
   }

   // barRect and paint it.
   //
   pen.setColor (this->getForegroundPaintColour ());
   pen.setWidth (1);
   painter.setPen (pen);

   brush.setStyle (Qt::SolidPattern);
   brush.setColor (this->getForegroundPaintColour() );
   painter.setBrush (brush);

   painter.drawRect (barRect);
}

//------------------------------------------------------------------------------
//
void QEAnalogIndicator::drawMarker (QPainter & painter, QRect &area, const double fraction)
{
   int span;
   int cx, cy;
   int temp;
   int t;
   int l;
   int b;
   int r;

   QPen pen;
   QBrush brush;
   QPolygon polygon;


   // Calcualate "diamond" span
   //
   if (this->isLeftRight ()) {
      span =  (area.bottom () - area.top ()) / 8;
   } else {
      span =  (area.right () - area.left ()) / 8;
   }
   span = MAX (span, 4);


   // Centre co-ordinates.
   //
   cx = (area.left () + area.right ()) / 2;
   cy = ( area.top () + area.bottom () ) / 2;

   t = area.top ();
   l = area.left ();
   b = area.bottom ();
   r = area.right ();

   switch (this->mOrientation) {

      case Left_To_Right:
         // Convert fractions back to pixels.
         //
         temp = int (fraction * (area.right () - area.left ()));
         cx = area.left () + temp;
         l = cx - span;
         r = cx + span;
         break;

      case Top_To_Bottom:
         temp = int (fraction * (area.bottom () - area.top ()));
         cy = area.top () + temp;
         t = cy - span;
         b = cy + span;
         break;

      case Right_To_Left:
         temp = int (fraction * (area.right () - area.left ()));
         cx = area.right () - temp;
         l = cx - span;
         r = cx + span;
         break;

      case Bottom_To_Top:
         temp = int (fraction * (area.bottom () - area.top ()));
         cy = area.bottom () - temp;
         t = cy - span;
         b = cy + span;
         break;

      default:
         // report an error??
         //
         return;
   }

   // Create "diamond" polygon
   //
   polygon << QPoint (l, cy);
   polygon << QPoint (cx, t);
   polygon << QPoint (r, cy);
   polygon << QPoint (cx, b);

   pen.setColor (this->getForegroundPaintColour ());
   pen.setWidth (1);
   painter.setPen (pen);

   brush.setStyle (Qt::SolidPattern);
   brush.setColor (this->getForegroundPaintColour ());
   painter.setBrush (brush);

   painter.drawPolygon (polygon);
}

//------------------------------------------------------------------------------
//
void QEAnalogIndicator::drawMeter (QPainter & painter, QRect &area, const double fraction)
{
   // Macro function to create a point based on centre positon, radius and direction (s, c).
   //
   #define RPOINT(f)  QPoint (int (centre_x + f*radius*s),  int (centre_y - f*radius*c))

   const int width_span = area.right () - area.left ();
   const int height_span = area.bottom () - area.top ();

   double centre_x, centre_y;
   double radius;
   double s, c;
   QPen pen;
   QBrush brush;
   int j;
   double lowerAngle;
   double upperAngle;
   double angle;
   double minS, maxS, minC, maxC;
   double gap;
   QRect dialRect;
   BandList bandList;
   bool ok;
   bool isMajor;
   double value;
   double f;
   QPoint p1, p2;

   // Working in degrees.
   //
   lowerAngle = this->mCentreAngle - this->mSpanAngle / 2.0;
   upperAngle = this->mCentreAngle + this->mSpanAngle / 2.0;

   // Find min and max sin/cosine so that we can find optimum centre.
   //
   minS = maxS = minC = maxC = 0.0;
   angle = lowerAngle;
   while (true) {
      s = sin (angle * RADIANS_PER_DEGREE);
      c = cos (angle * RADIANS_PER_DEGREE);

      minS = MIN (minS, s);
      maxS = MAX (maxS, s);
      minC = MIN (minC, c);
      maxC = MAX (maxC, c);

      if (angle >= upperAngle) break;

      // Determine next angle.
      //
      if      (angle < -360.0) { angle = -360.0; }
      else if (angle < -270.0) { angle = -270.0; }
      else if (angle < -180.0) { angle = -180.0; }
      else if (angle < -90.0)  { angle = -90.0;  }
      else if (angle < 0.0)    { angle =  0.0;   }
      else if (angle < 90.0)   { angle =  90.0;  }
      else if (angle < 180.0)  { angle = 180.0;  }
      else if (angle < 270.0)  { angle = 270.0;  }
      else if (angle < 360.0)  { angle = 360.0;  }

      angle = MIN (angle, upperAngle);
   }

   // Determine centre.
   //
   gap  = 6.0;     // gap around edge

   f = (-minS) / (maxS - minS);
   centre_x = gap + f * (width_span - 2.0 * gap);

   f = (+maxC) / (maxC - minC);
   centre_y = 6.0 + f * (height_span - 2.0 * gap);

   radius = MIN (width_span, height_span);
   if (maxS > 0) radius = MIN (radius, ((area.right () - gap)  - centre_x) /maxS);
   if (minS < 0) radius = MIN (radius, ((area.left () + gap)   - centre_x) /minS);
   if (maxC > 0) radius = MIN (radius, (centre_y -    (area.top () + gap)) /maxC);
   if (minC < 0) radius = MIN (radius, (centre_y - (area.bottom () - gap)) /minC);


   // Draw band/colour based annulus.  We do this as two sets of drawPie
   // calls: one set with full radius and one with 97% radius.
   //
   dialRect.setLeft   (int (centre_x - radius));
   dialRect.setRight  (int (centre_x + radius));
   dialRect.setTop    (int (centre_y - radius));
   dialRect.setBottom (int (centre_y + radius));

   // Note: this is a dispatching call.
   //
   bandList = this->getBandList ();
   for (j = 0; j < bandList.count (); j++) {
      Band band = bandList.at (j);
      double fl;
      double fu;
      double al;
      double au;
      int startAngle;
      int spanAngle;

      pen.setWidth (0);
      pen.setColor (band.colour);
      painter.setPen (pen);

      brush.setColor (band.colour);
      brush.setStyle (Qt::SolidPattern);
      painter.setBrush (brush);

      fl = this->calcFraction (band.lower);
      fu =  this->calcFraction (band.upper);

      al = lowerAngle +  fl * (upperAngle - lowerAngle);
      au = lowerAngle +  fu * (upperAngle - lowerAngle);

      // The startAngle and spanAngle must be specified in 1/16th of a degree,
      // i.e. a full circle equals 5760 (16 * 360). Positive values for the
      // angles mean counter-clockwise while negative values mean the clockwise
      // direction. Zero degrees is at the 3 o'clock position.
      //
      startAngle = int ((90.0 - au) * 16.0);
      if (startAngle < 0) {
         startAngle +=  5760;
      }
      spanAngle = int ((au - al) * 16.0);
      if (spanAngle < 0) {
         spanAngle +=  5760;
      }

      painter.drawPie (dialRect, startAngle, spanAngle);
   }

   if (bandList.count () > 0) {
      int startAngle;
      int spanAngle;

      dialRect.setLeft   (int (centre_x - 0.97 * radius));
      dialRect.setRight  (int (centre_x + 0.97 * radius));
      dialRect.setTop    (int (centre_y - 0.97 * radius));
      dialRect.setBottom (int (centre_y + 0.97 * radius));

      pen.setWidth (0);
      pen.setColor (this->getBackgroundPaintColour ());
      painter.setPen (pen);

      brush.setColor (this->getBackgroundPaintColour ());
      brush.setStyle (Qt::SolidPattern);
      painter.setBrush (brush);

      startAngle = int ((90.0 - upperAngle) * 16.0) - 1;
      if (startAngle < 0) {
         startAngle +=  5760;
      }
      spanAngle = int ((upperAngle - lowerAngle) * 16.0) + 4;
      if (spanAngle < 0) {
         spanAngle +=  5760;
      }

      painter.drawPie (dialRect, startAngle, spanAngle);
   }

   // Draw axis
   //
   pen.setWidth (1);
   pen.setColor (this->getFontPaintColour ());
   painter.setPen (pen);

   painter.setRenderHint (QPainter::Antialiasing, true);

   // Iterate over interval values.
   //
   for (ok = this->firstValue (j, value, isMajor); ok;
        ok = this->nextValue  (j, value, isMajor)) {

      f = this->calcFraction (value);

      angle = lowerAngle +  f * (upperAngle - lowerAngle);
      s = sin (angle * RADIANS_PER_DEGREE);
      c = cos (angle * RADIANS_PER_DEGREE);

      if (isMajor) {
         p1 = RPOINT (0.94);
      } else {
         p1 = RPOINT (0.97);
      }
      p2 = RPOINT (1.0);

      painter.drawLine (p1, p2);

      if (isMajor) {
         QString vt;

         if (this->getLogScale () ) {
            vt.sprintf ("%.0e", value);
         } else {
            vt.sprintf ("%.1f", value);
         }
         p1 = RPOINT (0.88);
         this->drawText (painter, p1, vt, 7);
      }
   }

   angle = lowerAngle + fraction *(upperAngle - lowerAngle);
   s = sin (angle * RADIANS_PER_DEGREE);
   c = cos (angle * RADIANS_PER_DEGREE);

   pen.setColor (this->getForegroundPaintColour ());
   p1 = RPOINT (0.0);

   p2 = RPOINT (1.0);
   pen.setWidth (2);
   painter.setPen (pen);
   painter.drawLine (p1, p2);

   p2 = RPOINT (0.75);
   pen.setWidth (3);
   painter.setPen (pen);
   painter.drawLine (p1, p2);

   p2 = RPOINT (0.5);
   pen.setWidth (4);
   painter.setPen (pen);
   painter.drawLine (p1, p2);

   p2 = RPOINT (0.25);
   pen.setWidth (5);
   painter.setPen (pen);
   painter.drawLine (p1, p2);

#undef RPOINT
}

//------------------------------------------------------------------------------
//
void QEAnalogIndicator::drawText (QPainter & painter, QPoint & textCentre, QString & text, const int pointSize)
{
   QFont pf (this->font ());

   if (pointSize > 0) {
      pf.setPointSize (pointSize);
   }
   painter.setFont (pf);

   QFontMetrics fm = painter.fontMetrics ();
   QPen pen;
   int x;
   int y;

   // Centre text. For height, pointSize seems better than fm.height ()
   // painter.drawText takes bottom left coordinates.
   //
   x = textCentre.x () - fm.width (text)/2;
   y = textCentre.y () + (pf.pointSize () + 1) / 2;

   pen.setColor (this->getFontPaintColour ());
   painter.setPen (pen);

   // If text too wide, then ensure we show most significant part.
   //
   painter.drawText (MAX (1, x), y, text);
}

//------------------------------------------------------------------------------
//
void QEAnalogIndicator::drawAxisText (QPainter & painter, QPoint & textCentre, QString & text, const int pointSize)
{
   QFont pf (this->font ());

   if (pointSize > 0) {
      pf.setPointSize (pointSize);
   }
   painter.setFont (pf);

   QFontMetrics fm = painter.fontMetrics ();
   QPen pen;
   int x;
   int y;

   // Centre text. For height, pointSize seems better than fm.height ()
   // painter.drawText needs bottom left coordinates.
   //
   if (this->isLeftRight ()) {
      x = textCentre.x () - fm.width (text)/2;
      y = textCentre.y () +  pf.pointSize ();
   } else {
      x = textCentre.x ();
      y = textCentre.y () + (pf.pointSize () + 1) / 2;
   }

   pen.setColor (this->getFontPaintColour ());
   painter.setPen (pen);

   // If text too wide, then ensure we show most significant part.
   //
   painter.drawText (MAX (1, x), y, text);
}

//------------------------------------------------------------------------------
//
void QEAnalogIndicator::paintEvent (QPaintEvent * /* event - make warning go away */)
{
   QPainter painter (this);

   int right;
   int bottom;
   QRect outlineRect;
   QRect areaRect;
   QRect axisRect;
   double fraction;
   QPoint textCentre;

   // Draw everything with antialiasing off.
   //
   painter.setRenderHint (QPainter::Antialiasing, false);

   // Want effective drawing right-most, bottom-most pixels.
   //
   // Note: Actual size appears to be 1 less than widget width/height.
   // Pixels are  in range (0 .. size - 2) which is size - 1 pixels.
   //
   right  = this->width () - 2;
   bottom = this->height () - 2;

   if ( (this->getMode() == Meter ) || (this->getShowScale () == false) ) {
      outlineRect.setTop (0);
      outlineRect.setLeft (0);
      outlineRect.setBottom (bottom);
      outlineRect.setRight (right);

      // No axis required.
      //
      axisRect.setTop (0);
      axisRect.setLeft (0);
      axisRect.setBottom (0);
      axisRect.setRight (0);

   } else {

      // We do draw a separate axis.
      //
      if (this->isLeftRight ()) {
         const int axisSize = 22;   // vertical
         const int edge = 20;       // horizontal


         outlineRect.setTop (0);
         outlineRect.setBottom (bottom - (axisSize + 1));
         outlineRect.setLeft (edge);
         outlineRect.setRight (right - edge);

         axisRect.setTop (bottom - axisSize);
         axisRect.setBottom (bottom);
         axisRect.setLeft (edge + 1);
         axisRect.setRight (right - (edge + 1));

      } else {
         const int axisSize = 44;   // horizontal
         const int edge = 5;        // vertical

         outlineRect.setTop (edge);
         outlineRect.setBottom (bottom - edge);
         outlineRect.setLeft (0);
         outlineRect.setRight (right - (axisSize + 1));

         axisRect.setTop (edge + 1);
         axisRect.setBottom (bottom - (edge + 1));
         axisRect.setLeft (right - axisSize);
         axisRect.setRight (right);
      }
   }

   // Set up main graphic paint area rectangle - 1 pixel boarder.
   //
   areaRect.setTop (outlineRect.top () + 1);
   areaRect.setLeft (outlineRect.left () + 1);
   areaRect.setBottom (outlineRect.bottom () - 1);
   areaRect.setRight (outlineRect.right () - 1);


   // Calculate the fractional scale and constrain to be in range.
   //
   fraction = this->calcFraction (this->mValue);


   // Now lets get drawing.
   //
   this->drawOutline (painter, outlineRect);

   // Set default centre text positions.
   //
   textCentre.setX ((areaRect.left () + areaRect.right ()) / 2);
   textCentre.setY ((areaRect.top ()  + areaRect.bottom ()) / 2);

   switch  (this->mMode) {

      case Bar:
         this->drawBar (painter, areaRect, fraction);
         if (this->getShowScale () == true) {
            this->drawAxis(painter, axisRect);
         }
         break;

      case Scale:
         this->drawMarker (painter, areaRect, fraction);
         if (this->getShowScale () == true) {
            this->drawAxis(painter, axisRect);
         }
         break;

      case Meter:
         this->drawMeter (painter, areaRect, fraction);
         textCentre.setY ((areaRect.top ()  + 3*areaRect.bottom ()) / 4);
         break;

      default:
         // report an error??
         //
         break;
   }

   if (this->getShowText ()) {
      QString barText;

      // This is a dispatching call.
      //
      barText = this->getTextImage ();

      this->drawText (painter, textCentre, barText);
   }
}

//------------------------------------------------------------------------------
//
QString QEAnalogIndicator::getTextImage () const
{
   QString result;

   result.sprintf ("%+0.7g", this->mValue);
   return result;
}

//------------------------------------------------------------------------------
//
QEAnalogIndicator::BandList QEAnalogIndicator::getBandList () const
{
   BandList result;

   result.clear ();
   return result;
}

//------------------------------------------------------------------------------
//
void QEAnalogIndicator::setMinorInterval (const double value)
{
   double temp;
   double hold;

   temp = MAX (0.001, value);
   if (this->mMinorInterval != temp) {
      hold = this->getMajorInterval ();
      this->mMinorInterval = temp;
      this->setMajorInterval (hold);
      this->update ();
   }
}

double QEAnalogIndicator::getMinorInterval () const
{
   return this->mMinorInterval;
}

//------------------------------------------------------------------------------
//
void QEAnalogIndicator::setMajorInterval (const double value)
{
   int temp;

   temp = int (value / this->mMinorInterval + 0.4);
   temp = MAX (2, temp);
   if (this->mMajorMinorRatio != temp) {
      this->mMajorMinorRatio = temp;
      this->update ();
   }
}

double QEAnalogIndicator::getMajorInterval () const
{
   return this->mMajorMinorRatio * this->mMinorInterval;
}

//------------------------------------------------------------------------------
//
void QEAnalogIndicator::setRange (const double MinimumIn,
                                  const double MaximumIn)
{
   this->setMinimum (MinimumIn);
   this->setMaximum (MaximumIn);
}

//------------------------------------------------------------------------------
//
void QEAnalogIndicator::setValue (const int valueIn)
{
   this->setValue (double (valueIn));
}

//------------------------------------------------------------------------------
// Ensure  max - min >= minimum span for all updated
//
void QEAnalogIndicator::setMinimum (const double minimum)
{
   this->mMinimum = minimum;

   // Ensure consistant.
   //
   this->mMaximum = MAX (this->mMaximum, this->mMinimum + MINIMUM_SPAN);
   this->update();
}

//------------------------------------------------------------------------------
//
double QEAnalogIndicator::getMinimum () const
{
   return this->mMinimum;
}

//------------------------------------------------------------------------------
//
void QEAnalogIndicator::setMaximum (const double maximum)
{
   this->mMaximum = maximum;

   // Ensure consistant.
   //
   this->mMinimum = MIN (this->mMinimum, this->mMaximum - MINIMUM_SPAN);
   this->update ();
}

//------------------------------------------------------------------------------
//
double QEAnalogIndicator::getMaximum () const
{
   return this->mMaximum;
}

//------------------------------------------------------------------------------
// Standard propery access macro.
//
#define PROPERTY_ACCESS(type, name, convert)                 \
                                                             \
void QEAnalogIndicator::set##name (const type value)  {      \
   type temp;                                                \
   temp = convert;                                           \
   if (this->m##name != temp) {                              \
      this->m##name = temp;                                  \
      this->update ();                                       \
   }                                                         \
}                                                            \
                                                             \
type QEAnalogIndicator::get##name () const {                 \
   return this->m##name;                                     \
}


// NOTE: we have to use qualified type for Orientation and Mode.
//
PROPERTY_ACCESS (QEAnalogIndicator::Orientations, Orientation, value)

PROPERTY_ACCESS (QEAnalogIndicator::Modes, Mode, value)

PROPERTY_ACCESS (int, CentreAngle, LIMIT (value, -180, +180) )

PROPERTY_ACCESS (int, SpanAngle, LIMIT (value, 15, 350) )

PROPERTY_ACCESS (int, LogScaleInterval, LIMIT (value, 1, 10) )

PROPERTY_ACCESS (QColor, BorderColour, value)

PROPERTY_ACCESS (QColor, ForegroundColour, value)

PROPERTY_ACCESS (QColor, BackgroundColour, value)

PROPERTY_ACCESS (QColor, FontColour, value)

PROPERTY_ACCESS (bool, ShowText, value)

PROPERTY_ACCESS (bool, ShowScale, value)

PROPERTY_ACCESS (bool, LogScale, value)

PROPERTY_ACCESS (double, Value, value)

PROPERTY_ACCESS (bool, IsActive, value)

#undef PROPERTY_ACCESS

// end
