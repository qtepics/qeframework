/*  QEEmitter.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2015-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
//   dbValueChanged ()
//   dbValueChanged (const QString&)
//   dbValueChanged (const int&)
//   dbValueChanged (const long&)
//   dbValueChanged (const qlonglong&)
//   dbValueChanged (const double&)
//   dbValueChanged (const bool&)
//
// In order for any of these signals to be emitted, the standard signal declarations
// must be placed in the widget header file - see QELabel.h as example.
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

   bool isDbValueChangedEmitInProgress() const;

private:
   void setupFilter ();       // idempotent
   void emitDbValueChangedPrivate (const bool useFormmattedText,
                                   const QString& formatedText,
                                   const unsigned int variableIndex);

   QWidget* owner;            // Widget which will emit signals.
   QEWidget* qew;             // Same object - different view
   bool filter [9];           // Indicates if particular signal is defined
   bool setupFilterComplete;
   bool dbValueChangedEmitInProgress;   // Circuit breaker to avoid infinte loops
};

#endif  // QE_EMMITTER_H
