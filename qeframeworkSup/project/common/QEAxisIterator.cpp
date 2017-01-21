/*  QEAxisIterator.cpp
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
 */

#include <math.h>
#include <QECommon.h>
#include "QEAxisIterator.h"


#define LAST_ITERATION_VALUE   0x7FFFFFFF

//------------------------------------------------------------------------------
//
QEAxisIterator::QEAxisIterator (const double minimumIn,
                                const double maximumIn,
                                const double minorIntervalIn,
                                const int majorMinorRatioIn,
                                const bool isLogarithmicIn)
{
   this->reInitialise (minimumIn, maximumIn,
                       minorIntervalIn, majorMinorRatioIn,
                       isLogarithmicIn);
}

//------------------------------------------------------------------------------
// place holder
//
QEAxisIterator::~QEAxisIterator () { }


//------------------------------------------------------------------------------
//
void QEAxisIterator::reInitialise (const double minimumIn,
                                   const double maximumIn,
                                   const double minorIntervalIn,
                                   const int majorMinorRatioIn,
                                   const bool isLogarithmicIn)
{
   // Assign values.
   //
   this->minimum = minimumIn;
   this->maximum = maximumIn;

   // Constrain values to be at least semi-sensible.
   //
   this->minorInterval = MAX (1.0e-20, minorIntervalIn);
   this->majorMinorRatio = MAX (1, majorMinorRatioIn);
   this->isLogarithmic = isLogarithmicIn;

   // Avoid rounding errors at boundaries, esp when logarithmic.
   //
   this->minTolerance = 1.0e-9 * ABS (this->minimum);
   this->maxTolerance = 1.0e-9 * ABS (this->maximum);

   // Calculate the origin.
   //
   if (this->isLogarithmic) {
      this->origin = 0.0;  // not used per se
   } else {
      // Choose an origin which is an exact multiple of the major interval.
      // The particular value coosen not important, but want a value that does
      // not cause iteratorControl to overflow.
      //
      const double majorInterval = double (this->majorMinorRatio) * this->minorInterval;
      const double mean = (this->minimum + this->maximum) / 2.0;
      const double temp = floor (mean  / majorInterval);
      this->origin = temp * majorInterval;
   }

   // Force any on going iteration to stop on next call to next ().
   //
   this->iteratorControl = LAST_ITERATION_VALUE;
}

//------------------------------------------------------------------------------
//
bool QEAxisIterator::firstValue (double& value, bool& isMajor, const int maxIterationsIn)
{
   double realITC;
   bool result;

   this->maxIterations = maxIterationsIn;
   this->iterationCount = 0;

   if (this->isLogarithmic) {
      realITC = 9.0 * LOG10 (this->minimum); //
   } else {
      realITC = (this->minimum - this->origin) / this->minorInterval;
   }

   // Use floor to round down and - 0.5 to mitigate any rounding effects.
   // Subtract an addition -1 to ensure first call to nextValue returns a
   // value no greater than the first required value.
   //
   this->iteratorControl = int (floor (realITC) - 0.5) - 1;

   result = this->nextValue (value, isMajor);
   while (result && (value < (this->minimum - this->minTolerance))) {
      result = this->nextValue (value, isMajor);
   }

   this->iterationCount = 0;  // first calls to next value update this, so reset

   return result;
}

//------------------------------------------------------------------------------
//
bool QEAxisIterator::nextValue  (double& value, bool& isMajor)
{
   // sainity check.
   //
   this->iterationCount++;
   if (this->iterationCount >= this->maxIterations) return false;

   if (this->iteratorControl == LAST_ITERATION_VALUE) return false;

   this->iteratorControl++;
   if (this->isLogarithmic) {
      static const int fs = 9;

      // Ensure round down towards -infinity (as opposed to 0)
      // Oh how I wish C/C++ has a proper "mod" operator.
      //
      int d = iteratorControl / fs;
      if ((fs * d) > iteratorControl) d--;
      int f = iteratorControl - (fs * d);

      value = (1.0 + f) * EXP10 (d);
      if (f == 0) {
         // Is an exact power of 10 - test for being major.
         //
         isMajor = ((d % this->majorMinorRatio) == 0);
      } else {
         // Is not an exact power of 10 - cannot be major.
         //
         isMajor = false;
      }
   } else {
      value = (this->iteratorControl * this->minorInterval) + this->origin;
      isMajor = ((this->iteratorControl % this->majorMinorRatio) == 0);
   }

   bool result = (value <= this->maximum + this->maxTolerance);
   return result;
}

//------------------------------------------------------------------------------
//
double QEAxisIterator::getMinimum ()         const { return this->minimum; }
double QEAxisIterator::getMaximum ()         const { return this->maximum; }
double QEAxisIterator::getMinorInterval ()   const { return this->minorInterval; }
int    QEAxisIterator::getMajorMinorRatio () const { return this->majorMinorRatio; }
bool   QEAxisIterator::getIsLogarithmic ()   const { return this->isLogarithmic; }

// end
