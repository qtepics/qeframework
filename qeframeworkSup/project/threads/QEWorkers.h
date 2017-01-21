/*  QEWorkers.h
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

#ifndef QEWORKERS_H
#define QEWORKERS_H

#include <QList>
#include <QMetaType>
#include <QObject>
#include <QtGlobal>
#include <QThread>

/*!
 * Proof of concept class - NOT currently in use.
 *
 * When manager's process function called, each worker class object process
 * function is called (in a separate thread).
 */

#define MAXIMUM_THREADS   16

namespace QE {

typedef quint8  Counts;
typedef quint32 SequenceNumbers;

//------------------------------------------------------------------------------
//
class Worker : public QObject {
   Q_OBJECT
public:
   explicit Worker ();   // no parent is specified.

private:
   // Derived classes may override this function.
   // This is executed once in the context of the associated thread
   // when the thread is started.
   //
   virtual void initialise (const Counts i, const Counts n);

   // Derived classes should override this function.  The function in the derived
   // class must do the i-th part of the n-part total work where i is 0 to (n-1).
   // The nominal workPackage is represented as a QObject and is the same object
   // as passed to the manager process function.
   //
   // Each call to process is in the context of a separate thread.
   //
   // It is the responsibilty of the derived class's process function to ensure:
   // a/ that it performs the rquired part and only the required part of
   //    the overall total work package.
   // b/ that all the instances of execution do not step on each others toes
   // (that's a technical expression ;-), and that any interaction between each
   // instance and the rest of the system is thread safe.
   //
   virtual void process (QObject* workPackage, const Counts i, const Counts n);

   Counts getInstance () { return instance; }
   Counts getNumber   () { return number; }

private slots:
   void started ();

   void startProcessing (const QE::SequenceNumbers sequenceNumber,
                         QObject* workPackage);

signals:
   void processingComplete (const QE::SequenceNumbers sequenceNumber,
                            const QE::Counts instance);
private:
   Counts instance;
   Counts number;
   SequenceNumbers sequenceNumber;

   friend class WorkerThread;
};


//------------------------------------------------------------------------------
// Although not enforced by this thread framework, it would seem eminently
// sensible that all workers objects are of the same derived class type.
//
typedef QList <Worker*> WorkerList;

//------------------------------------------------------------------------------
// Creates and manages a set of threads - one for each of the given work force.
//
class WorkerManager : public QObject {
   Q_OBJECT
public:
   // The workForce should be between 1 and MAXIMUM_THREADS (16) workers.
   // Additional worker objects are ignored.
   // Note: the worker objects are created independently of the manager.
   // Also they must be disposed of independently of the manager.
   //
   explicit WorkerManager (const WorkerList& workForce,
                           QObject* parent = 0);
   virtual ~WorkerManager ();

   void process (QObject* workPackage);

   Counts getNumber () { return number; }

signals:
   // The work load is returned on completion to provide context.
   //
   void complete (const QObject* workPackage);

private:
   Counts number;                      // size of work force.
   SequenceNumbers sequenceNumber;     // task identifier
   QObject* workPackage;

   bool isComplete ();

   // Internally we declare a number of WorkerThread pointer items.
   // Declareing these in the header of a QObject seems to make the Qt SDK
   // get very upset and confused, so need to hide this a little.
   //
   class ReallyPrivate;
   ReallyPrivate* pd;

signals:
   // Sent to the work force objects.
   //
   void startProcessing (const QE::SequenceNumbers sequenceNumber,
                         QObject* workPackage);

private slots:
   // From the work force.
   //
   void processingComplete (const QE::SequenceNumbers sequenceNumber,
                            const QE::Counts instance);

   friend class WorkerThread;
};

}

Q_DECLARE_METATYPE (QE::SequenceNumbers)
Q_DECLARE_METATYPE (QE::Counts)


#endif  // QEWORKERS_H
