/*  QEOptions.h $
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

#ifndef QE_OPTIONS_H
#define QE_OPTIONS_H

#include <QString>
#include <QStringList>
#include <QEPluginLibrary_global.h>

/// This class provides a basic means to parse command line options or any
/// other arbitary list of strings. Options are named and the name format
/// may be either the short format (single letter) and/or the long format.
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
/// Examples:
/// program   -v                     -- boolean or flag (short format)
/// program   --verbose              -- boolean or flag (long format)
/// program   -n=31                  -- integer (short format)
/// program   -n 31                  -- integer (alternate short format)
/// program   --number=31            -- integer (long format)
/// program   --number 31            -- integer (alternate long format)
/// program   --output="fred.txt"    -- string (long format)
/// program   --output fred.txt      -- string (alternate long format)
/// program   --output=              -- specifies a null string
///
/// Note: Currently it provides no means to "compress" multiple flags, e.g. to
/// interpret:   program -cap   as   program -c -a -p
///
/// This class is one of several provided to access configuration data such
/// as from environment variables, command line options and settings. They all
/// provide a similar API.
///
class QEPLUGINLIBRARYSHARED_EXPORT QEOptions {
public:
   // Creates an object based on the command line (QCoreApplication::arguments ()).
   // Note: the program name (first item) is removed from the arguments.
   //
   explicit QEOptions ();

   // Creates an object based on the given, arbitary set of strings.
   //
   explicit QEOptions (const QStringList& args);

   // Destructor
   //
   virtual ~QEOptions ();

   // Get the ith parameter, i.e. non option argument. i = 0, 1, 2...
   //
   // Example: Condider arguments  "--tom"  "dick"  "harry"
   //
   // "--tom" is an option, and "dick" is deemed the first parameter (#0)
   // and "harry" the second  parameter (#1).
   //
   // However this is ambiguous as "dick" may be the value associated with option "tom".
   // This can only be resovled if user calls, e.g. getString ("tom"), then we know
   // "dick" is the value associated with "tom" and that "harry" is the first parameter.
   //
   // TODO: - specify a syntax to allow non-ambiguous validation.
   //
   int getParameterCount ();
   QString getParameter (const int i);

   // Overloaded functions to determine if an option has been specified.
   // Not really applicable to flags/booleans.
   //
   bool isSpecified (const QString& option, const QChar letter);
   bool isSpecified (const QString& option);
   bool isSpecified (const QChar letter);

   // Access functions. These functions allow either a short or long format, e.g:
   // -v     or  --version     flags/booleans.
   // -n=31  or  --number=31   values.
   // -n 31  or  --number 31   also allowed for values.
   //
   // Overloaded functions to get the specified flag/boolean option.
   // The default value is implicitly false.
   //
   bool    getBool   (const QString& option, const QChar letter);
   bool    getBool   (const QString& option);
   bool    getBool   (const QChar letter);

   // Overloaded functions to get the specified string option.
   //
   QString getString (const QString& option, const QChar letter, const QString& defaultValue);
   QString getString (const QString& option, const QString & defaultValue);
   QString getString (const QChar letter, const QString & defaultValue);

   // Overloaded functions to get the specified int option.
   //
   int     getInt    (const QString& option, const QChar letter, const int defaultValue);
   int     getInt    (const QString& option, const int defaultValue);
   int     getInt    (const QChar letter, const int defaultValue);

   // Overloaded functions to get the specified double option.
   //
   double  getFloat  (const QString& option, const QChar letter, const double defaultValue);
   double  getFloat  (const QString& option, const double defaultValue);
   double  getFloat  (const QChar letter, const double defaultValue);

private:
   QStringList args;
   int parameterOffset;

   void setUpCommon ();
   void registerOptionArgument (int p);
};

#endif  // QE_OPTIONS_H
