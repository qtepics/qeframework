/*  QEPlotterState.cpp
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
 *  Copyright (c) 2013 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QEPlotterState.h>

#include <QECommon.h>

#define MAXIMUM_CHART_STATES   40

//==============================================================================
//
//==============================================================================
//
QEPlotterState::QEPlotterState ()
{
   this->isXLogarithmic = false;
   this->isYLogarithmic = false;
   this->isReverse = false;
}

//------------------------------------------------------------------------------
//
void QEPlotterState::saveConfiguration (PMElement& parentElement)
{
   QEPlotterNames meta;
   PMElement stateElement = parentElement.addElement ("PlotterState");

   stateElement.addValue ("isXLogarithmic", this->isXLogarithmic);
   stateElement.addValue ("isYLogarithmic", this->isYLogarithmic);
   stateElement.addValue ("isReverse", this->isReverse);

   stateElement.addValue ("xMinimum", this->xMinimum);
   stateElement.addValue ("xMaximum", this->xMaximum);
   stateElement.addValue ("xScaleMode", QEUtilities::enumToString (meta, "ScaleModes", this->xScaleMode));

   stateElement.addValue ("yMinimum", this->yMinimum);
   stateElement.addValue ("yMaximum", this->yMaximum);
   stateElement.addValue ("yScaleMode", QEUtilities::enumToString (meta, "ScaleModes", this->yScaleMode));
}

//------------------------------------------------------------------------------
//
void QEPlotterState::restoreConfiguration (PMElement& parentElement)
{
   QEPlotterNames meta;
   PMElement stateElement = parentElement.getElement ("PlotterState");
   bool status;
   bool boolVal;
   int intVal;
   QString stringVal;

   if (stateElement.isNull ()) return;

   status = stateElement.getValue ("isXLogarithmic", boolVal);
   if (status) {
      this->isXLogarithmic = boolVal;
   }

   status = stateElement.getValue ("isYLogarithmic", boolVal);
   if (status) {
      this->isYLogarithmic = boolVal;
   }

   status = stateElement.getValue ("isReverse", boolVal);
   if (status) {
      this->isReverse = boolVal;
   }

   status = stateElement.getValue ("xMinimum", this->xMinimum);
   status = stateElement.getValue ("xMaximum", this->xMaximum);

   // Read enum - read as string - convert to enum int value then convert to enum.
   //
   status = stateElement.getValue ("xScaleMode", stringVal);
   if (status) {
      intVal = QEUtilities::stringToEnum (meta, "ScaleModes", stringVal, &status);
      if (status) {
         this->xScaleMode = (QEPlotterNames::ScaleModes) intVal;
      }
   }

   status = stateElement.getValue ("yMinimum", this->yMinimum);
   status = stateElement.getValue ("yMaximum", this->yMaximum);
   status = stateElement.getValue ("yScaleMode", stringVal);
   if (status) {
      intVal = QEUtilities::stringToEnum (meta, "ScaleModes", stringVal, &status);
      if (status) {
         this->yScaleMode = (QEPlotterNames::ScaleModes) intVal;
      }
   }
}

//==============================================================================
//
//==============================================================================
//
QEPlotterStateList::QEPlotterStateList ()
{
   this->chartStatePointer = 0;
}

//------------------------------------------------------------------------------
//
void QEPlotterStateList::clear ()
{
   this->stateList.clear ();
   this->chartStatePointer = 0;
}

//------------------------------------------------------------------------------
//
void QEPlotterStateList::push (const QEPlotterState& state)
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
bool QEPlotterStateList::prev (QEPlotterState& state)
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
bool QEPlotterStateList::next (QEPlotterState& state)
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
bool QEPlotterStateList::prevAvailable ()
{
   return (this->chartStatePointer > 1);
}

//------------------------------------------------------------------------------
//
bool QEPlotterStateList::nextAvailable ()
{
   return (this->chartStatePointer < this->stateList.count ());
}

// end
