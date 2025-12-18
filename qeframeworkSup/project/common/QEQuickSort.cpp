/*  QEQuickSort.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2016-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEQuickSort.h"
#include <QDebug>

#define DEBUG qDebug () << "QEQuickSort" << __LINE__ << __FUNCTION__ << " "

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
   const int number = last - first + 1;

   if (number <= 1) return; // nothing to do.

   if (number == 2) {  // degenerate case
      if (this->itemLessThan (last, first, context)) {
         this->swapItems (first, last, context);
      }
      return;   // nothing more to do.
   }

   if (number == 3) {   // simple case - saves about 18%
      const int middle = first + 1;
      if (this->itemLessThan (middle, first, context)) {
         this->swapItems (middle, first, context);
      }
      if (this->itemLessThan (last, middle, context)) {
         this->swapItems (last, middle, context);
      }
      if (this->itemLessThan (middle, first, context)) {
         this->swapItems (middle, first, context);
      }
      return;   // nothing more to do.
   }

   // Special for four items - prob not worth the effort.

   // Four or more items.
   // Partition about a pivot value (use last/right most element)
   //
   int pivot = last;
   int left = first;
   int right = last - 1;

   while (true) {
      // while left <= pivot
      while ((left < pivot) && !this->itemLessThan (pivot, left, context)) {
         left++;
      }
      if (left == pivot) break;

      // left > pivot
      this->swapItems (left, pivot, context);
      pivot = left; left++;

      // while right >= pivot
      while ((right > pivot) && !this->itemLessThan (right, pivot, context)) {
         right--;
      }
      if (right == pivot) break;

      // right < pivot
      this->swapItems (right, pivot, context);
      pivot = right; right--;
   }

   // Now qsort each sub-list.
   //
   this->sort (first, pivot - 1, context);
   this->sort (pivot + 1, last, context);
}

// end
