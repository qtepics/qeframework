/*  QEEmitter.h
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

#ifndef QE_EMMITTER_H
#define QE_EMMITTER_H

#include <QWidget>
#include <QString>
#include <QEFrameworkLibraryGlobal.h>

class QEWidget;

// Support class for QEWidget
//
// This class provides the means to allow any class that inherited directly or
// indirectly from both QWidget and QEWidget to programatically emit the following
// signals:
//
//   dbConnectionChanged (const bool&)
//   dbValueChanged (const QString&)
//   dbValueChanged (const int&)
//   dbValueChanged (const long&)
//   dbValueChanged (const qlonglong&)
//   dbValueChanged (const double&)
//   dbValueChanged (const bool&)
//
// In order for these signals to be emitted, the standard signal declaration must
// be placed in the widget header file - see QELabel.h as example.
// Also, the underlying channel's QVariant value must also be able to be converted
// to the appropriate numerical value. For the boolean dbValueChanged signal, the
// numerical value must available as well. The signal value is true for non-zero
// values and false for zero values.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEEmitter {

public:
   // Need both views as we cannot do dynamic castimng during construction
   explicit QEEmitter (QEWidget* qew, QWidget* owner);
   ~QEEmitter ();

protected:
   // Emit dbConnectionChanged signal if applicable.
   //
   void emitDbConnectionChanged (const unsigned int variableIndex);

   // Emit dbValueChanged signals if applicable.
   //
   void emitDbValueChanged (const unsigned int variableIndex);

   // Overloaded form to use given formatted text, esp useful for QELabel.
   //
   void emitDbValueChanged (const QString& formatedText,
                            const unsigned int variableIndex);

private:
   void setupFilter ();       // idempotent
   void emitDbValueChangedPrivate (const bool useFormmattedText,
                                   const QString& formatedText,
                                   const unsigned int variableIndex);

   QWidget* owner;            // Widget which will emit signals.
   QEWidget* qew;             // Same object - different view
   bool filter [8];
   bool setupFilterComplete;
};

#endif  // QE_EMMITTER_H
