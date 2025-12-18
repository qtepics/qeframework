/*  QEFileMonitor.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2014-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
