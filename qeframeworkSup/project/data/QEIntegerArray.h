/*  QEIntegerArray.h
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
 *  Copyright (c) 2013 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
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
