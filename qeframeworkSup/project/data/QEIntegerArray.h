/*  QEIntegerArray.h
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

#ifndef QEINTEGER_ARRAY_H
#define QEINTEGER_ARRAY_H

#include <QVector>
#include <QEFrameworkLibraryGlobal.h>

/// This class provides short hand for QVector<long> together with some
/// basic long vector operations.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT  QEIntegerArray : public QVector<long> {
public:
   // Various constructors that replicate those provided by QVector
   //
   explicit QEIntegerArray ();
   explicit QEIntegerArray (int size);
   explicit QEIntegerArray (int size, const long& t);
   explicit QEIntegerArray (const QVector<long>& other);

   // Allow assignment
   //
   QEIntegerArray& operator=(const QVector<long>& other);

   // Find min/max values of the array. If array has zero elements then
   // the returned value is the defaultValue.
   //
   long minimumValue (const long& defaultValue = 0);
   long maximumValue (const long& defaultValue = 0);
};

#endif   // QEINTEGER_ARRAY_H
