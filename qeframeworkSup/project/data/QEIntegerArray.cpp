/*  QEIntegerArray.cpp
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
 *  Copyright (c) 2013,2018 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
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
