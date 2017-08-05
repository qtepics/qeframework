/*  QEScanTimers.h
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
 *  Copyright (c) 2014,2017 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_SCAN_TIMERS_H
#define QE_SCAN_TIMERS_H

#include <QObject>
#include <QTimer>
#include <QEFrameworkLibraryGlobal.h>

// Used by, for example QSimpleShape, to ensure ALL QSimpleShape widgets
// flash in sync.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEScanTimers : public QObject {
Q_OBJECT
public:
   enum ScanRates {
      VerySlow,     // 0.25 Hz
      Slow,         // 0.5 Hz
      Medium,       // 1 Hz
      Fast,         // 2 Hz
      VeryFast      // 4 Hz
   };

   Q_ENUMS (ScanRates)

   // Connext (attach) ir disconnect (detach) form specific scan timer.
   // Example: attach (this, SLOT (scanSlot (const bool)));
   //
   static bool attach (QObject* target, const  char* member, const ScanRates scanRate);
   static void detach (QObject* target, const  char* member);

private:
   // Actual instances of these objects are set up statically and ONLY created from
   // within this class.
   //
   explicit QEScanTimers (QObject* parent = 0);
   ~QEScanTimers ();

   static void initialise ();

   QTimer* timer;

   unsigned int count;

private slots:
   void timeout ();

signals:
   void flipFlopVerySlow (const bool);
   void flipFlopSlow     (const bool);
   void flipFlopMedium   (const bool);
   void flipFlopFast     (const bool);
   void flipFlopVeryFast (const bool);
};


#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEScanTimers::ScanRates)
#endif

#endif  // QE_SCAN_TIMERS_H
