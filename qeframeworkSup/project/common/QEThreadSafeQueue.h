/*  QEThreadSafeQueue.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2023-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_THREAD_SAFE_QUEUE_H
#define QE_THREAD_SAFE_QUEUE_H

#include <QMutex>
#include <QQueue>
#include <QEFrameworkLibraryGlobal.h>

/// QEThreadSafeQueue is a wrapper around a QQueue with a mutex to make thread safe.
/// If a queue of references, these may become un-referenced orphans when the queue
/// cleared or deleted.
/// Note: the dequeue API is different from the embedded queue's dequeue method.
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
