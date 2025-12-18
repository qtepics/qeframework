/*  QEPlotterState.h
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

#ifndef QEPLOTTERSTATE_H
#define QEPLOTTERSTATE_H

#include <QDateTime>
#include <QList>

#include <QEFrameworkLibraryGlobal.h>
#include <persistanceManager.h>
#include <QEPlotterNames.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPlotterState {
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
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPlotterStateList {
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
