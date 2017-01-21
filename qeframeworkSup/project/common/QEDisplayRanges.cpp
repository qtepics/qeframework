/*  QEDisplayRanges.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2014 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <math.h>
#include <QECommon.h>
#include "QEDisplayRanges.h"

//==============================================================================
//
QEDisplayRanges::QEDisplayRanges ()
{
   this->clear ();
}

//------------------------------------------------------------------------------
//
QEDisplayRanges::QEDisplayRanges (const double minIn, const double maxIn)
{
   this->setRange (minIn, maxIn);
}


//------------------------------------------------------------------------------
//
bool QEDisplayRanges::operator == (const QEDisplayRanges& other) const
{
   bool result;

   if (this->isDefined == other.isDefined) {
      // Both defined or both not defined.
      //
      if (this->isDefined) {
         // Both defined.
         //
         result = (this->minimum == other.minimum) && (this->maximum == other.maximum);
      } else {
         // Both undefined.
         //
         result = true;
      }
   } else {
      // One defined, one undefined.
      //
      result = false;
   }

   return result;
}

//------------------------------------------------------------------------------
// Define != as not == , this ensures consistancy as is the only sensible definition.
//
bool QEDisplayRanges::operator != (const QEDisplayRanges& other) const
{
   return !(*this == other);
}

//------------------------------------------------------------------------------
//
bool QEDisplayRanges::isSimilar (const QEDisplayRanges& other, const double tolerance) const
{
   bool result;

   if (this->isDefined == other.isDefined) {
      // Both defined or both not defined.
      //
      if (this->isDefined) {
         // Both defined.
         //
         double s1 = this->maximum -this->minimum;
         double s2 = other.maximum - other.minimum;

         double d1 = ABS(this->minimum - other.minimum);
         double d2 = ABS(this->maximum - other.maximum);

         double q = MAX (d1, d2) / MIN (s1, s2);

         result = (q <= tolerance);
      } else {
         // Both undefined, i.e. equal.
         //
         result = true;
      }
   } else {
      // One defined, one undefined.
      //
      result = false;
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEDisplayRanges::clear ()
{
   this->isDefined = false;
   this->minimum = 0.0;
   this->maximum = 0.0;
}

//------------------------------------------------------------------------------
//
void QEDisplayRanges::setRange (const double minIn, const double maxIn)
{
   this->minimum = minIn;
   this->maximum = maxIn;
   this->isDefined = true;
}

//------------------------------------------------------------------------------
//
void QEDisplayRanges::merge (const double d)
{
   if (this->isDefined) {
      // already have at least one value
      //
      this->minimum = MIN (this->minimum, d);
      this->maximum = MAX (this->maximum, d);
   } else {
      // use single value to "start things off".
      this->minimum = d;
      this->maximum = d;
      this->isDefined = true;
   }
}

//------------------------------------------------------------------------------
//
void QEDisplayRanges::merge (const QEDisplayRanges &other)
{
   if ((this->isDefined) && (other.isDefined)) {
      // both are defined
      this->minimum = MIN (this->minimum, other.minimum);
      this->maximum = MAX (this->maximum, other.maximum);
   } else {
      // only this or other or neither are defined, but not both.
      if (other.isDefined) {
         this->isDefined = true;
         this->minimum = other.minimum;
         this->maximum = other.maximum;
      }
   }
}

//------------------------------------------------------------------------------
//
bool QEDisplayRanges::getIsDefined () const
{
   return this->isDefined;
}

//------------------------------------------------------------------------------
//
double QEDisplayRanges::getMinimum () const
{
   return this->minimum;
}

//------------------------------------------------------------------------------
//
double QEDisplayRanges::getMaximum () const
{
   return this->maximum;
}

//------------------------------------------------------------------------------
//
bool QEDisplayRanges::getMinMax (double &min, double &max) const
{
   min = this->minimum;
   max = this->maximum;
   return this->isDefined;
}

//------------------------------------------------------------------------------
//
void QEDisplayRanges::adjustLogMinMax (double& minOut, double& maxOut, double& majorOut) const
{
   minOut = MAX (1.0E-20, this->minimum);  // avoid log of -ve and zero.
   maxOut = this->maximum;

   // calc majorOut - the actual intervals are 10^(majorOut*n)
   //
   double r = maxOut / minOut;
   if (r >= 1.0E24) {
      majorOut = 3.0;
   } else if (r >= 1.0E16) {
      majorOut = 2.0;
   } else {
      majorOut = 1.0;
   }
}

//------------------------------------------------------------------------------
//
void QEDisplayRanges::adjustMinMax (const int number, const bool roundToMajor,
                                    double& minOut, double& maxOut, double& majorOut) const
{
   // Approx min and max values that can be held in a qint64 variable (with safety margin).
   //
   const double qint64min = -9.223372e+18;
   const double qint64max = +9.223372e+18;

   // The compiler does a better job of evaluating these constants and
   // minimising rounding errors than if self generated by the application.
   // It also allows us just to declare a ready-made static array.
   //
   // The range is somewhat arbitary, and may be extended.
   //
   static const double majorValues [] = {
      1.0e-20,  2.0e-20,  5.0e-20,    1.0e-19,  2.0e-19,  5.0e-19,
      1.0e-18,  2.0e-18,  5.0e-18,    1.0e-17,  2.0e-17,  5.0e-17,
      1.0e-16,  2.0e-16,  5.0e-16,    1.0e-15,  2.0e-15,  5.0e-15,
      1.0e-14,  2.0e-14,  5.0e-14,    1.0e-13,  2.0e-13,  5.0e-13,
      1.0e-12,  2.0e-12,  5.0e-12,    1.0e-11,  2.0e-11,  5.0e-11,
      1.0e-10,  2.0e-10,  5.0e-10,    1.0e-09,  2.0e-09,  5.0e-09,
      1.0e-08,  2.0e-08,  5.0e-08,    1.0e-07,  2.0e-07,  5.0e-07,
      1.0e-06,  2.0e-06,  5.0e-06,    1.0e-05,  2.0e-05,  5.0e-05,
      1.0e-04,  2.0e-04,  5.0e-04,    1.0e-03,  2.0e-03,  5.0e-03,
      1.0e-02,  2.0e-02,  5.0e-02,    1.0e-01,  2.0e-01,  5.0e-01,
      1.0e+00,  2.0e+00,  5.0e+00,    1.0e+01,  2.0e+01,  5.0e+01,
      1.0e+02,  2.0e+02,  5.0e+02,    1.0e+03,  2.0e+03,  5.0e+03,
      1.0e+04,  2.0e+04,  5.0e+04,    1.0e+05,  2.0e+05,  5.0e+05,
      1.0e+06,  2.0e+06,  5.0e+06,    1.0e+07,  2.0e+07,  5.0e+07,
      1.0e+08,  2.0e+08,  5.0e+08,    1.0e+09,  2.0e+09,  5.0e+09,
      1.0e+10,  2.0e+10,  5.0e+10,    1.0e+11,  2.0e+11,  5.0e+11,
      1.0e+12,  2.0e+12,  5.0e+12,    1.0e+13,  2.0e+13,  5.0e+13,
      1.0e+14,  2.0e+14,  5.0e+14,    1.0e+15,  2.0e+15,  5.0e+15,
      1.0e+16,  2.0e+16,  5.0e+16,    1.0e+17,  2.0e+17,  5.0e+17,
      1.0e+18,  2.0e+18,  5.0e+18,    1.0e+19,  2.0e+19,  5.0e+19,
      1.0e+20,  2.0e+20,  5.0e+20,    1.0e+21,  2.0e+21,  5.0e+21,
      1.0e+22,  2.0e+22,  5.0e+22,    1.0e+23,  2.0e+23,  5.0e+23,
      1.0e+24,  2.0e+24,  5.0e+24,    1.0e+25,  2.0e+25,  5.0e+25,
      1.0e+26,  2.0e+26,  5.0e+26,    1.0e+27,  2.0e+27,  5.0e+27,
      1.0e+28,  2.0e+28,  5.0e+28,    1.0e+29,  2.0e+29,  5.0e+29,
      1.0e+30,  2.0e+30,  5.0e+30,    1.0e+31,  2.0e+31,  5.0e+31,
      1.0e+32,  2.0e+32,  5.0e+32,    1.0e+33,  2.0e+33,  5.0e+33,
      1.0e+34,  2.0e+34,  5.0e+34,    1.0e+35,  2.0e+35,  5.0e+35,
      1.0e+36,  2.0e+36,  5.0e+36,    1.0e+37,  2.0e+37,  5.0e+37,
      1.0e+38,  2.0e+38,  5.0e+38,    1.0e+39,  2.0e+39,  5.0e+39,
      1.0e+40,  2.0e+40,  5.0e+40,    1.0e+41,  2.0e+41,  5.0e+41,
      1.0e+42,  2.0e+42,  5.0e+42,    1.0e+43,  2.0e+43,  5.0e+43,
      1.0e+44,  2.0e+44,  5.0e+44,    1.0e+45,  2.0e+45,  5.0e+45,
      1.0e+46,  2.0e+46,  5.0e+46,    1.0e+47,  2.0e+47,  5.0e+47,
      1.0e+48,  2.0e+48,  5.0e+48,    1.0e+49,  2.0e+49,  5.0e+49,
      1.0e+50,  2.0e+50,  5.0e+50,    1.0e+51,  2.0e+51,  5.0e+51,
      1.0e+52,  2.0e+52,  5.0e+52,    1.0e+53,  2.0e+53,  5.0e+53,
      1.0e+54,  2.0e+54,  5.0e+54,    1.0e+55,  2.0e+55,  5.0e+55,
      1.0e+56,  2.0e+56,  5.0e+56,    1.0e+57,  2.0e+57,  5.0e+57,
      1.0e+58,  2.0e+58,  5.0e+58,    1.0e+59,  2.0e+59,  5.0e+59,
      1.0e+60,  2.0e+60,  5.0e+60,    1.0e+61,  2.0e+61,  5.0e+61,
      1.0e+62,  2.0e+62,  5.0e+62,    1.0e+63,  2.0e+63,  5.0e+63,
      1.0e+64,  2.0e+64,  5.0e+64,    1.0e+65,  2.0e+65,  5.0e+65,
      1.0e+66,  2.0e+66,  5.0e+66,    1.0e+67,  2.0e+67,  5.0e+67,
      1.0e+68,  2.0e+68,  5.0e+68,    1.0e+69,  2.0e+69,  5.0e+69,
      1.0e+70,  2.0e+70,  5.0e+70,    1.0e+71,  2.0e+71,  5.0e+71,
      1.0e+72,  2.0e+72,  5.0e+72,    1.0e+73,  2.0e+73,  5.0e+73,
      1.0e+74,  2.0e+74,  5.0e+74,    1.0e+75,  2.0e+75,  5.0e+75,
      1.0e+76,  2.0e+76,  5.0e+76,    1.0e+77,  2.0e+77,  5.0e+77,
      1.0e+78,  2.0e+78,  5.0e+78,    1.0e+79,  2.0e+79,  5.0e+79,
      1.0e+80,  2.0e+80,  5.0e+80,    1.0e+81,  2.0e+81,  5.0e+81,
      1.0e+82,  2.0e+82,  5.0e+82,    1.0e+83,  2.0e+83,  5.0e+83,
      1.0e+84,  2.0e+84,  5.0e+84,    1.0e+85,  2.0e+85,  5.0e+85,
      1.0e+86,  2.0e+86,  5.0e+86,    1.0e+87,  2.0e+87,  5.0e+87,
      1.0e+88,  2.0e+88,  5.0e+88,    1.0e+89,  2.0e+89,  5.0e+89,
      1.0e+90,  2.0e+90,  5.0e+90,    1.0e+91,  2.0e+91,  5.0e+91,
      1.0e+92,  2.0e+92,  5.0e+92,    1.0e+93,  2.0e+93,  5.0e+93,
      1.0e+94,  2.0e+94,  5.0e+94,    1.0e+95,  2.0e+95,  5.0e+95,
      1.0e+96,  2.0e+96,  5.0e+96,    1.0e+97,  2.0e+97,  5.0e+97,
      1.0e+98,  2.0e+98,  5.0e+98,    1.0e+99,  2.0e+99,  5.0e+99
  };

   double major;
   double minor;
   double multiplier;
   double tryp, tryq;
   qint64 p, q;
   int s;

   // Find estimated major value - use size (width or height) to help here.
   //
   major = (this->maximum - this->minimum) / MAX (number, 2);

   // Round up major to next standard value.
   //
   s = (major <= 1.0) ? 0 : 60;  // short cut - slot 60 corresponds to 1.0
   while ((major > majorValues [s]) &&
          ((s + 1) < ARRAY_LENGTH (majorValues))) s++;

   majorOut = major = majorValues [s];

   if ((s%3) == 1) {
      // Is a 2.0eN number.
      minor = major / 4.0;
   } else {
      // Is a 1.0eN or 5.0eN number.
      minor = major / 5.0;
   }

   // Select value for which min/max are exact multiples of.
   //
   multiplier = roundToMajor ? major : minor;

   // Determine minOut and maxOut such that they are both exact multiples of
   // multiplier and that:
   //
   //  minOut <= minIn <= maxIn << maxOut
   //
   // First check for overlow.
   //
   while (true) {
      tryp = this->minimum / multiplier;
      tryq = this->maximum / multiplier;

      // Are both p and q (well) within the qint64 range?
      //
      if (qint64min <= tryp && tryp <= qint64max &&
          qint64min <= tryq && tryq <= qint64max) {
         // Yes - conversion will be successfull.
         //
         break;
      }
      multiplier *= 10.0;  // rescale
   }

   p = qint64 (tryp);
   if ((p * multiplier) > this->minimum) p--;

   q = qint64 (tryq);
   if ((q * multiplier) < this->maximum) q++;

   q = MAX (q, p+1);   // Ensure p < q

   // Extend lower/upper limit to include 0 if min < 5% max
   //
   if ((p > 0) && (q > 20*p)) {
      p = 0;
   } else if ((q < 0) && (p < 20*q)) {
      q = 0;
   }

   minOut = p * multiplier;
   maxOut = q * multiplier;
}

// end
