/*  QEQuickSort.h
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
 *  Copyright (c) 2016,2017 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#ifndef QE_QUICK_SORT_H
#define QE_QUICK_SORT_H

#include <QObject>
#include <QEFrameworkLibraryGlobal.h>


/// This base class provides the means to add quick sort functionality to a class.
/// Alas the algorithms qSort API does not accept a class instance LessThan
/// function, so we roll out own.
/// The items to be sorted must be indexable via consecutive integers.
///
class  QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEQuickSort
{
public:
   explicit QEQuickSort ();
   virtual ~QEQuickSort ();

   // Sorts items from first to last inclusive.
   // The optional parameter context provides user specific conext for the sort.
   // This is useful when the sub-class has two or more sets that need to be sorted.
   //
   void sort (const int first, const int last, QObject* context = NULL);

protected:
   // The sub class must provide the means to compare two items.
   // The function should compare the value associated with index a against the
   // value associated with index b as opposed to comparing the a and b values.
   //
   // Examples
   // Standard array data:   return data [a] < data [b];
   // For QList list:        return list.value (a) <  list.value (b);
   //
   virtual bool itemLessThan (const int a, const int b, QObject* context) const = 0;

   // The sub class must provide the means to swap two items.
   //
   // Examples
   // Standard array data:   temp = data [a]; data [a] = data [b]; data [b] = temp;
   //
   virtual void swapItems (const int a, const int b, QObject* context) = 0;
};

#endif // QE_QUICK_SORT_H
