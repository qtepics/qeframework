/*  QEStripChartState.cpp
 *
 *  Copyright (c) 2013-2022 Australian Synchrotron.
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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

#include <QEStripChartState.h>

#include <QECommon.h>

#define MAXIMUM_CHART_STATES   40

//==============================================================================
//
//==============================================================================
//
QEStripChartState::QEStripChartState ()
{
   this->isNormalVideo = true;
}


//------------------------------------------------------------------------------
//
void QEStripChartState::saveConfiguration (PMElement& parentElement)
{
   QEStripChartNames meta;

   PMElement stateElement = parentElement.addElement ("ChartState");

   stateElement.addValue ("IsNormalVideo", this->isNormalVideo);
   stateElement.addValue ("ChartTimeMode", QEUtilities::enumToString (meta, "ChartTimeModes",this->chartTimeMode));
   stateElement.addValue ("YScaleMode", QEUtilities::enumToString (meta, "YScaleModes", this->yScaleMode));
   stateElement.addValue ("ChartYScale", QEUtilities::enumToString (meta, "ChartYRanges", this->chartYScale));
   stateElement.addValue ("YMinimum", this->yMinimum);
   stateElement.addValue ("YMaximum", this->yMaximum);
   stateElement.addValue ("Duration", this->duration);
   stateElement.addValue ("TimeZoneSpec", (int) this->timeZoneSpec);

   // We use double here as toTime_t returns a uint (as opposed to an int).
   //
   stateElement.addValue ("EndDateTime", (double) this->endDateTime.toSecsSinceEpoch ());
}

//------------------------------------------------------------------------------
//
void QEStripChartState::restoreConfiguration (PMElement& parentElement)
{
   QEStripChartNames meta;

   PMElement stateElement = parentElement.getElement ("ChartState");
   bool status;
   bool boolVal;
   int intVal;
   double doubleVal;
   QString stringVal;

   if (stateElement.isNull ()) return;

   status = stateElement.getValue ("IsNormalVideo", boolVal);
   if (status) {
      this->isNormalVideo = boolVal;
   }

   status = stateElement.getValue ("ChartTimeMode", stringVal);
   if (status) {
      intVal = QEUtilities::stringToEnum (meta, "ChartTimeModes", stringVal, &status);
      if (status) {
         this->chartTimeMode = (QEStripChartNames::ChartTimeModes) intVal;
      }
   }

   status = stateElement.getValue ("YScaleMode", stringVal);
   if (status) {
      intVal = QEUtilities::stringToEnum (meta, "YScaleModes", stringVal, &status);
      if (status) {
         this->yScaleMode = (QEStripChartNames::YScaleModes) intVal;
      }
   }

   status = stateElement.getValue ("ChartYScale", stringVal);
   if (status) {
      intVal = QEUtilities::stringToEnum (meta, "ChartYRanges", stringVal, &status);
      if (status) {
         this->chartYScale = (QEStripChartNames::ChartYRanges) intVal;
      }
   }

   status = stateElement.getValue ("YMinimum", this->yMinimum);

   status = stateElement.getValue ("YMaximum", this->yMaximum);

   status = stateElement.getValue ("Duration", this->duration);

   status = stateElement.getValue ("TimeZoneSpec", intVal);
   if (status) {
      this->timeZoneSpec = (Qt::TimeSpec) intVal;
   }

   status = stateElement.getValue ("EndDateTime", doubleVal);
   if (status) {
      this->endDateTime.setSecsSinceEpoch (qint64 (doubleVal));
   }
}


//==============================================================================
// QEStripChartStateList
//==============================================================================
//
QEStripChartStateList::QEStripChartStateList ()
{
   this->chartStatePointer = 0;
}

//------------------------------------------------------------------------------
//
void QEStripChartStateList::clear ()
{
   this->stateList.clear ();   // call parent method
   this->chartStatePointer = 0;
}

//------------------------------------------------------------------------------
//
void QEStripChartStateList::push (const QEStripChartState& state)
{
   // New state - all potential next states are lost.
   //
   while (this->stateList.count () > this->chartStatePointer) {
      this->stateList.removeLast ();
   }

   this->stateList.append (state);

   // Don't let this list get tooo big.
   //
   if (this->stateList.count () > MAXIMUM_CHART_STATES) {
       this->stateList.removeFirst ();  // remove oldest
   }

   this->chartStatePointer = this->stateList.count ();
}

//------------------------------------------------------------------------------
//
bool QEStripChartStateList::prev (QEStripChartState& state)
{
   bool result;

   if (this->stateList.count () < 1) return false;    // sanity check.

   // Ensure in range.
   //
   this->chartStatePointer = LIMIT (this->chartStatePointer, 1, this->stateList.count ());

   if (this->chartStatePointer > 1) {
      this->chartStatePointer--;
      state = this->stateList.value (this->chartStatePointer - 1);  // zero indexed
      result = true;
   } else {
      result = false;
   }

   return result;
}

//------------------------------------------------------------------------------
//
bool QEStripChartStateList::next (QEStripChartState& state)
{
   bool result;

   if (this->stateList.count () < 1) return false; // sanity check.

   // Ensure in range.
   //
   this->chartStatePointer = LIMIT (this->chartStatePointer, 1, this->stateList.count ());

   if (this->chartStatePointer < this->stateList.count ()) {
      this->chartStatePointer++;
      state = this->stateList.value (this->chartStatePointer - 1);  // zero indexed
      result = true;
   } else {
      result = false;
   }

   return result;
}

//------------------------------------------------------------------------------
//
bool QEStripChartStateList::prevAvailable ()
{
   return (this->chartStatePointer > 1);
}

//------------------------------------------------------------------------------
//
bool QEStripChartStateList::nextAvailable ()
{
   return (this->chartStatePointer < this->stateList.count ());
}

// end
