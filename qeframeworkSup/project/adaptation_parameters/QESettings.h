/*  QESettings.h $
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

#ifndef QE_SETTINGS_H
#define QE_SETTINGS_H

#include <QString>
#include <QStringList>
#include <QSettings>
#include <QVariant>
#include <QEPluginLibrary_global.h>
#include <UserMessage.h>

/// This class provides access to user defined settings. The setting file
/// is expected to conform to the QSettings::IniFormat, i.e. like a windows
/// style .ini file.
///
/// The types of data that my be extracted from an option are bool, QString,
/// int and double; These are accessed by the associated getXxxx () functions
/// where Xxxx is one of Bool, String, Int and Float.
///
/// Each of these function takes a default value of the appropriate type which
/// is returned to the caller if the value is not defined or, as in the case of
/// numerical values, is ill-defined.
///
/// Numeric validity is defined by QString::toInt () and QString::toDouble ()
/// Do note that QString::toInt () does not accept 0x... hexadecimal or
/// 0... octal numbers.
///
/// The class also provides a number of addironal conveniance functions that
/// are described below.
///
/// This class is one of several provided to access configuration data such
/// as from environment variables, command line options and settings. They all
/// provide a similar API.
///
// This class contains a QSettings objects (as opposed to extending it).
//
class QEPLUGINLIBRARYSHARED_EXPORT QESettings : private UserMessage {
public:
   // Contructor that locates and uses the applicatuion's adaptation_parmeters.ini
   // This should be located in the application's current directory at start up.
   // The class keeps a copy of this such that it may be accessed even if/when
   // the application changes its current directory.
   //
   explicit QESettings ();

   // Contructor that uses an arbitary setting file.
   //
   explicit QESettings (const QString& fileName);

   // Destructor
   //
   virtual ~QESettings ();

   bool isDefined () const;

   // Provides access to embedded settings functions.
   //
   QString getSettingFileName () const;
   QStringList groupKeys (const QString &group);


   QVariant getValue  (const QString& key, const QVariant& defaultValue);
   bool     getBool   (const QString& key, const bool      defaultValue);
   QString  getString (const QString& key, const QString&  defaultValue);
   int      getInt    (const QString& key, const int       defaultValue);
   double   getFloat  (const QString& key, const double    defaultValue);


   // Essentially the same as getString except that the returned file name
   // is converted from a relative path name to a full path name relative
   // to the directory containing the this adaptation parameter file.
   //
   QString getFilename (const QString& key, const QString& defaultValue);

   // Assumes key points to a file name.
   // Returns nil if file does not exist.
   // If not nill, then user must delete the returned object.
   // This is a essentially conveniance function for:
   //     s = getFilename (key);
   //     return new QESettings (s).
   //
   QESettings* getSettings (const QString &key);


   // This is a essentially conveniance function for:
   //     key =  section/'Configuration_File';
   //     QESettings standard;
   //     s = standard.getFilename (key);
   //     return new QESettings (s).
   //
   // Returns nil if file does not exist.
   // If not nill, then user must delete the returned object.
   //
   static QESettings* getConfigurationParameters (const QString& section);

private:
   QSettings* settings;
   QString directoryName;

   QString getDirectoryName ();

   void commonSetup (const QString& fileName);
};

#endif // QE_SETTINGS_H
