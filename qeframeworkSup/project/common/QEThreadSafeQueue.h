/*  QEThreadSafeQueue.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2023 Australian Synchrotron.
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
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#ifndef QE_THREAD_SAFE_QUEUE_H
#define QE_THREAD_SAFE_QUEUE_H

#include <QMutex>
#include <QQueue>
#include <QEFrameworkLibraryGlobal.h>

/// QEThreadSafeQueue is a wrapper around a QQueue with a mutex to make thread safe.
/// If a queue of references, these may become un-referenced orphans when the queue
/// cleared or deleted.
/// Note: the dequeue API is differnt from the embedded queue's dequeue method.
//
template <typename Type>
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEThreadSafeQueue {
public:
   explicit QEThreadSafeQueue () { }

   ~QEThreadSafeQueue ()
   {
      this->clear ();
   }

   // not a deep clear - that is up to the user to mange.
   //
   inline void clear () {
      QMutexLocker locker (&this->mutex);
      this->queue.clear ();
   }

   // Thread safe enqueue.
   //
   inline void enqueue (const Type &t)
   {
      QMutexLocker locker (&this->mutex);
      this->queue.enqueue (t);
   }

   // Thread safe dequeue. Returns true if an item has been dequeued,
   // othwewise false. Note: different API.
   //
   inline bool dequeue (Type &t)
   {
      QMutexLocker locker (&this->mutex);
      if (!this->queue.isEmpty()) {
         t = this->queue.dequeue();
         return true;
      }
      return false;
   }

   inline int size () {
      QMutexLocker locker (&this->mutex);
      return this->queue.size();
   }

   inline bool isEmpty () {
      QMutexLocker locker (&this->mutex);
      return this->queue.isEmpty();
   }

private:
   QMutex mutex;
   QQueue<Type> queue;
};

#endif  // QE_THREAD_SAFE_QUEUE_H
