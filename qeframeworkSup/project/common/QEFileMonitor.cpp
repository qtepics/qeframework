/*  QEFileMonitor.cpp
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
 *  Copyright (c) 2014 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QDebug>
#include <QFileSystemWatcher>
#include <QHash>
#include "QEFileMonitor.h"

typedef QHash<QString, int> FileNameCounts;

static QFileSystemWatcher* fileWatcher = NULL;
static FileNameCounts* fileNameCounts = NULL;

//------------------------------------------------------------------------------
//
QEFileMonitor::QEFileMonitor (QObject* parent) : QObject (parent)
{
   this->constructor ();
}

//------------------------------------------------------------------------------
//
QEFileMonitor::QEFileMonitor (const QString& pathIn, QObject* parent) : QObject (parent)
{
   this->constructor ();
   this->setPath (pathIn);
}

//------------------------------------------------------------------------------
//
void QEFileMonitor::constructor ()
{
   if (!fileWatcher) {
      fileWatcher = new QFileSystemWatcher (NULL);
   }
   if (!fileNameCounts) {
      fileNameCounts = new FileNameCounts ();
   }

   this->pathOfInterest = "";

   QObject::connect (fileWatcher, SIGNAL (fileChanged (const QString &)),
                     this,    SLOT (filterFileChanged (const QString &)));

   QObject::connect (fileWatcher, SIGNAL (directoryChanged (const QString &)),
                     this,    SLOT (filterDirectoryChanged (const QString &)));
}


//------------------------------------------------------------------------------
//
QEFileMonitor::~QEFileMonitor ()
{
   this->clearPath ();
}

//------------------------------------------------------------------------------
//
void QEFileMonitor::setPath (const QString& pathIn)
{
   // mutux??

   // Remove old name iff significant
   //
   if (!this->pathOfInterest.isEmpty ()) {
      if (fileNameCounts->contains (this->pathOfInterest)) {
         int n = fileNameCounts->value (this->pathOfInterest);
         if (n > 1) {
            // At least one will be left in list.
            //
            (*fileNameCounts) [this->pathOfInterest] = n - 1;
         } else {
            // None left.
            //
            fileNameCounts->remove (this->pathOfInterest);
            fileWatcher->removePath (this->pathOfInterest);
         }
      }
   }

   this->pathOfInterest = pathIn;   // do actual update

   // Add new name iff significant
   //
   if (!this->pathOfInterest.isEmpty()) {
//    qDebug () << "monitoring " << this->pathOfInterest;

      if (fileNameCounts->contains (this->pathOfInterest)) {
         int n = fileNameCounts->value (this->pathOfInterest);
         // At least one already in list.
         //
         (*fileNameCounts) [this->pathOfInterest] = n + 1;
      } else {
         // None in list.
         //
         fileNameCounts->insert (this->pathOfInterest, 1);
         fileWatcher->addPath (this->pathOfInterest);
      }
   }
}

//------------------------------------------------------------------------------
//
QString QEFileMonitor::getPath () const
{
    return this->pathOfInterest;
}

//------------------------------------------------------------------------------
//
void QEFileMonitor::clearPath ()
{
   this->setPath ("");
}

//------------------------------------------------------------------------------
//
void QEFileMonitor::filterFileChanged (const QString& path)
{
   if (path == this->pathOfInterest) {
      emit this->fileChanged (path);
   }
}

//------------------------------------------------------------------------------
//
void QEFileMonitor::filterDirectoryChanged (const QString& path)
{
   if (path == this->pathOfInterest) {
      emit this->directoryChanged (path);
   }
}

// end
