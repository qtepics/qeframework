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

#ifndef QE_FILE_MONITOR_H
#define QE_FILE_MONITOR_H

#include <QObject>
#include <QString>
#include <QEFrameworkLibraryGlobal.h>

/// The QEFileMonitor class provides a wrapper around a single QFileSystemWatcher
/// object which can be used to monitor all file/directories that require watching.
/// This class acts as an agent between the user and the singleton QFileSystemWatcher.
/// As each instance of a QFileSystemWatcher object creates a new thread, it is far
/// more thread efficient to use a single watcher to monitor many files/directories.
///
/// Note: each agent only sets up monitoring for a single file/directory.
/// Multiple agents may/can monitor the same file/directory.
///
/// Refer to QFileSystemWatcher documentation on limits and monitoring features.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEFileMonitor : public QObject
{
   Q_OBJECT
public:
   explicit QEFileMonitor (QObject* parent = NULL);
   explicit QEFileMonitor (const QString& path, QObject* parent = NULL);
   ~QEFileMonitor ();

   // Set the path to be monitored.
   //
   void setPath (const QString& path);
   QString getPath () const;

   void clearPath ();   // same as setPath ("");

signals:
   // This signal is emitted when the file at the specified path is modified,
   // renamed or removed from disk.
   //
   void fileChanged (const QString& path);

   // This signal is emitted when the directory at a specified path, is modified
   // (e.g., when a file is added, modified or deleted) or removed from disk.
   //
   void directoryChanged (const QString& path);

private:
   void constructor ();
   QString pathOfInterest;

private slots:
   // These slot receive and filter signals from the embedded QFileSystemWatcher.
   //
   void filterFileChanged (const QString& path);
   void filterDirectoryChanged (const QString& path);
};

#endif // QE_FILE_MONITOR_H
