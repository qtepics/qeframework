/*  QEAdaptationParameters.h
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

#ifndef QE_ADAPTATION_PARAMETERS_H
#define QE_ADAPTATION_PARAMETERS_H

#include <QString>

#include <QEEnvironmentVariables.h>
#include <QESettings.h>
#include <QEOptions.h>
#include <QEFrameworkLibraryGlobal.h>

/// This class provides general access to adaptation parameters that may be specified in
/// a variety of ways. Specifcally it provides a standard and consistent wrapper around
/// the getXxxx () functions out of QEOptions, QEEnvironmentVariables and QESettings.
/// (where Xxxx is one of Bool, String, Int and Float).
///
/// Each get getXxxx () function attempts to extract a value of each source. Where
/// multiple values are available the highest prioity value is returned. The priorities
/// are (from lowest to highest):
///
///    default_value         - as passed into the getXxxx function.
///    environment variable  - the name is converted to upper case and is prefixed with
///                            default or specified prefix if this has been specified e.g. QE_.
///    setting               - read from adaptation section if the adaptation_parameters_file.ini file
///    command line option   - command line parameter: --name=value. Note is case sensitive.
///
/// If a numeric value is ill-defined, then next lower priority value is used.
/// Numeric validity is defined by QString::toInt () and QString::toDouble ()
/// Do note that QString::toInt () does not accept 0x... hexadecimal or 0... 
/// octal numbers. Users may always use getString () and parse such values themselves.
///
/// If only command line options, or setting or environment variables to be considered, then
/// use the getXxxx () function out of the QEOptions, QEQEEnvironmentVariables or QESettings.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEAdaptationParameters {
public:
   QEAdaptationParameters ();  // uses curent default prefix for environment variables.
   QEAdaptationParameters (const QString& envPrefix);  // specifies environment prefix
   ~QEAdaptationParameters ();

   bool    getBool   (const QString& name); // the default is implicitly false
   bool    getBool   (const QString& name, const QChar letter);

   QString getString (const QString& name, const QString& defaultValue);
   QString getString (const QString& name, const QChar letter, const QString& defaultValue);

   int     getInt    (const QString& name, const int      defaultValue);
   int     getInt    (const QString& name, const QChar letter, const int      defaultValue);

   double  getFloat  (const QString& name, const double   defaultValue);
   double  getFloat  (const QString& name, const QChar letter, const double   defaultValue);

   // Essentially the same as getString except that a returned file name of the
   // form "~/abc/def" maps to "<home_path><separator>abc/def".
   // <separator> is platform dependent.
   //
   QString getFilename (const QString& name, const QString& defaultValue);
   QString getFilename (const QString& name, const QChar letter, const QString& defaultValue);

private:
   QEEnvironmentVariables* environment;
   QESettings* settings;
   QEOptions* options;
};

#endif  // QE_ADAPTATION_PARAMETERS_H
