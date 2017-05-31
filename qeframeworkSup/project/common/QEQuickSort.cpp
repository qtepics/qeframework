/*  QEQuickSort.cpp
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

#include "QEQuickSort.h"

//------------------------------------------------------------------------------
//
QEQuickSort::QEQuickSort() { }   // place holder

//------------------------------------------------------------------------------
//
QEQuickSort::~QEQuickSort() { }   // place holder

//------------------------------------------------------------------------------
// quick sort
// Alas qSort API does not accept a class instance LessThan function.
//
void QEQuickSort::sort (const int first, const int last, QObject* context)
{
   const int n = last - first + 1;
   if (n <= 1) return; // nothing to do.

   if (n == 2) {  // degenerate case
      if (this->itemLessThan (last, first, context)) {
         this->swapItems (first, last, context);
      }
      return; // nothing more to do.
   }

   // Three or more items.
   // pivot the set of items about about last.
   //
   int p = last;
   int f = first;
   int l = last - 1;

   while (true) {
      while ((f < p) && !this->itemLessThan (p, f, context)) {  // while f <= p
         f++;
      }
      if (f == p) break;

      // f > p
      this->swapItems (f, p, context);
      p = f; f++;

      while ((l > p) && !this->itemLessThan (l, p, context)) {  // while l >= p
         l--;
      }
      if (l == p) break;

      // l < p
      this->swapItems (l, p, context);
      p = l; l--;
   }

   // Now qsort each sub-list.
   //
   this->sort (first, p - 1, context);
   this->sort (p + 1, last, context);
}

// end
