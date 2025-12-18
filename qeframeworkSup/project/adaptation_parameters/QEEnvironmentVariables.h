/*  QEEnvironmentVariables.h
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

#ifndef QE_ENVIRONMENT_VARIABLES_H
#define QE_ENVIRONMENT_VARIABLES_H

#include <QString>
#include <QEFrameworkLibraryGlobal.h>

/// This class provides a means to access environment variables.
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
/// The environment variable name is formed by concating the prefix and
/// the suffix and conveting this to upper case if needs be. The prefix
/// can be specified when the object is constucted otherwise the default
/// prefix is used when no prefix provided. Example prefix could be "QE_".
///
/// This class is one of several provided to access configuration data such
/// as from environment variables, command line options and settings. They all
/// provide a similar API.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEEnvironmentVariables  {
public:
   // Uses prefix as specified by setDefaultPrefix.
   // The default default prefix "".
   //
   explicit QEEnvironmentVariables ();

   // Specifiy prefix - can be an empty string.
   //
   explicit QEEnvironmentVariables (const QString& prefix);

   // Destructor
   //
   ~QEEnvironmentVariables ();

   // Set the default prefix.
   //
   static void setDefaultPrefix (const QString& prefix);
   static QString getDefaultPrefix ();

   bool    getBool   (const QString& name, const bool     defaultValue) const;
   QString getString (const QString& name, const QString& defaultValue) const;
   int     getInt    (const QString& name, const int      defaultValue) const;
   double  getFloat  (const QString& name, const double   defaultValue) const;

private:
   QString prefix;
   static QString defaultPrefix;
};

#endif // QE_ENVIRONMENT_VARIABLES_H
