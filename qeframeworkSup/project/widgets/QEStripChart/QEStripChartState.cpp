/*  QEStripChartState.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
