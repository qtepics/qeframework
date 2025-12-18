/*  QEFileMonitor.h
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
