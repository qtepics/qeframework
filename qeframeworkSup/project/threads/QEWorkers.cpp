/*  QEWorkers.cpp
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

#include <QDebug>
#include <QThread>

#include <QEWorkers.h>


#define DEBUG  qDebug () << "QE Worker::" << __FUNCTION__  << ":" << __LINE__
#define CTID  ( 1000 + QThread::currentThreadId () % 1000 )

namespace QE {

//==============================================================================
//
Worker::Worker () : QObject (NULL)
{
   this->instance = 0;
   this->number = 0;
}

//------------------------------------------------------------------------------
//
void Worker::initialise (const Counts, const Counts)
{
   // place holder
}

//------------------------------------------------------------------------------
//
void Worker::process (QObject* workPackage, const Counts i, const Counts n)
{
   // place holder
   qDebug ()  << CTID
              <<  " process (" << workPackage << "," << i << "," << n << ")"
              <<" - this should be overriden.";
}


//------------------------------------------------------------------------------
// slot
void Worker::started ()
{
   this->initialise (this->instance, this->number);
}

//------------------------------------------------------------------------------
// slot
void Worker::startProcessing (const QE::SequenceNumbers sequenceNumberIn,
                              QObject* workPackage)
{
   this->sequenceNumber = sequenceNumberIn;
   this->process (workPackage, this->instance, this->number);
   emit this->processingComplete (this->sequenceNumber, this->instance);
}


//==============================================================================
//
class WorkerThread : public QThread {
public:
   WorkerThread (Worker *worker,
                 Counts intance,
                 Counts number,
                 QObject* parent = 0);

protected:
   void run ();

private:
   Worker *worker;

   friend class WorkerManager;
};

//------------------------------------------------------------------------------
//
WorkerThread::WorkerThread (Worker *workerIn,
                            Counts intanceIn,
                            Counts numberIn,
                            QObject* parent) : QThread (parent)
{
   this->worker = workerIn;


   this->worker->instance = intanceIn;
   this->worker->number = numberIn;

   // Ensure the worker is an orphan prior to rehoming it's thread.
   //
   this->worker->setParent (NULL);
   this->worker->moveToThread (this);
}


//------------------------------------------------------------------------------
//
void WorkerThread::run ()
{
   this->exec();
}


//==============================================================================
//
class WorkerManager::ReallyPrivate {
public:
   WorkerThread* threadList [MAXIMUM_THREADS];
   bool workerComplete [MAXIMUM_THREADS];
};


//==============================================================================
//
WorkerManager::WorkerManager (const WorkerList& workForce, QObject *parent) : QObject (parent)
{
   Counts j;

   // Register types.
   //
   qRegisterMetaType<QE::Counts> ("QE::Counts");
   qRegisterMetaType<QE::SequenceNumbers> ("QE::SequenceNumbers");


   this->pd = new ReallyPrivate ();

   this->number =  workForce.count ();
   if (this->number > MAXIMUM_THREADS) {
      this->number = MAXIMUM_THREADS;
   }

   for (j = 0; j < this->number; j++) {
      Worker* worker = workForce.value (j);
      WorkerThread* thread = new WorkerThread (worker, j, this->number, parent);

      this->pd->threadList [j] = thread;

      QObject::connect (thread, SIGNAL (started ()), worker, SLOT (started ()));

      QObject::connect (this,   SIGNAL (startProcessing (const QE::SequenceNumbers, QObject*)),
                        worker, SLOT   (startProcessing (const QE::SequenceNumbers, QObject*)));

      QObject::connect (worker, SIGNAL (processingComplete (const QE::SequenceNumbers, QE::Counts)),
                        this,   SLOT   (processingComplete (const QE::SequenceNumbers, QE::Counts)));
   }

   for (j = 0; j < this->number; j++) {
      WorkerThread* thread = this->pd->threadList [j];
      thread->start ();
   }
}

//------------------------------------------------------------------------------
//
WorkerManager::~WorkerManager ()
{
   // place holder
}

//------------------------------------------------------------------------------
//
void WorkerManager::process (QObject* workPackageIn)
{
   Counts j;

   this->workPackage = workPackageIn;

   this->sequenceNumber++;
   for (j = 0; j < this->number; j++) {
      this->pd->workerComplete [j] = false;
   }

   emit this->startProcessing (this->sequenceNumber, this->workPackage);
}

//------------------------------------------------------------------------------
// slot
void WorkerManager::processingComplete (const QE::SequenceNumbers workerSequenceNumber,
                                        const QE::Counts instance)
{
   if (workerSequenceNumber == this->sequenceNumber) {
      this->pd->workerComplete[instance] = true;
      // All done??
      //
      if (this->isComplete ()) {
         emit this->complete (this->workPackage);
      }
   } else {
      DEBUG << "sequenceNumber mismatch, "
            << "instance"   << instance
            << "returned"   << workerSequenceNumber
            << ", expected" <<this->sequenceNumber;
      // otherwise ignore for now.
   }

}

//------------------------------------------------------------------------------
//
bool WorkerManager::isComplete ()
{
   bool result;
   Counts j;

   result = true;               //hypothesize okay.
   for (j = 0; j < this->number; j++) {
      if (!this->pd->workerComplete [j]) {
         result = false;
         break;
      }
   }

   return result;
}

}  // end QE

// end
