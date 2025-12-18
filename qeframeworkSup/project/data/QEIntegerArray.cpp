/*  QEIntegerArray.cpp
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

#include <QECommon.h>
#include <QEIntegerArray.h>

//=================================================================================
// QEIntegerArray
//=================================================================================
//
QEIntegerArray::QEIntegerArray () : QVector<long> () { }

QEIntegerArray::QEIntegerArray (int size) : QVector<long> (size) { }

QEIntegerArray::QEIntegerArray (int size, const long& t) : QVector<long> (size, t) { }

QEIntegerArray::QEIntegerArray (const QVector<long>& other) : QVector<long> (other) { }


//---------------------------------------------------------------------------------
// Copy a vertor from another and return value
//
QEIntegerArray& QEIntegerArray::operator=( const  QVector<long>& other )
{
    // Do parent class stuff assignment.
    *(QVector<long>*) this = QVector<long> (other);

    // return value as well.
    return *this;
}

//---------------------------------------------------------------------------------
//
long QEIntegerArray::minimumValue (const long& defaultValue)
{
   int n = this->count ();
   long r;

   if (n == 0) return defaultValue;
   r = this->value (0);
   for (int j = 1; j < n; j++) {
      r = MIN (r, this->value (j));
   }
   return r;
}

//---------------------------------------------------------------------------------
//
long QEIntegerArray::maximumValue (const long& defaultValue)
{
   int n = this->count ();
   long r;

   if (n == 0) return defaultValue;
   r = this->value (0);
   for (int j = 1; j < n; j++) {
      r = MAX (r, this->value (j));
   }
   return r;
}

// end
