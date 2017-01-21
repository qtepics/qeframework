/*  QEPlotterState.h
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

#ifndef QEPLOTTERSTATE_H
#define QEPLOTTERSTATE_H

#include <QDateTime>
#include <QList>

#include <QEPluginLibrary_global.h>
#include <persistanceManager.h>
#include <QEPlotterNames.h>

class QEPLUGINLIBRARYSHARED_EXPORT QEPlotterState {
public:
   QEPlotterState ();
   void saveConfiguration (PMElement& parentElement);
   void restoreConfiguration (PMElement& parentElement);

   bool isXLogarithmic;  // vs. Linear
   bool isYLogarithmic;  // vs. Linear
   bool isReverse;       // vs. Normal
   bool isPaused;        // vs. Updating
   double xMinimum;
   double xMaximum;
   double yMinimum;
   double yMaximum;
   QEPlotterNames::ScaleModes xScaleMode;
   QEPlotterNames::ScaleModes yScaleMode;
};

// This class uses a QList in order to implement a stack.
//
class QEPLUGINLIBRARYSHARED_EXPORT QEPlotterStateList {
public:
   QEPlotterStateList ();

   void clear ();
   void push (const QEPlotterState& state);
   bool prev (QEPlotterState& state);
   bool next (QEPlotterState& state);

   bool prevAvailable ();
   bool nextAvailable ();

private:
   int chartStatePointer;
   QList<QEPlotterState> stateList;
};

#endif  // QEPLOTTERSTATE_H
