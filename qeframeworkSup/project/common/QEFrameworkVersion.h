/*  QEFrameworkVersion.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2018 Australian Synchrotron
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
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#ifndef QE_FRAMEWORK_VERSION_H
#define QE_FRAMEWORK_VERSION_H

#include <QString>
#include <QEFrameworkLibraryGlobal.h>

#define STRINGIFY2(s)         #s
#define STRINGIFY(s)          STRINGIFY2(s)

// These version numbers are updated for each public release.
//
// Before tagging, determine if previously incremented 'RELEASE' number is appropriate (fixes only) or
// if the 'MAJOR' or 'MINOR' numbers should be incremented instead.
// (Also, before tagging QE_VERSION_STAGE should be set to QE_VERSION_STAGE_PRODUCTION)
//
// After tagging, increment QE_VERSION_RELEASE so all future development will
// be marked with a higher version than the current release.
// (Also, after tagging QE_VERSION_STAGE should be set to QE_VERSION_STAGE_DEVELOPMENT)
//
#define QE_VERSION_MAJOR      3     // Public releases that include interface or paradigm changes that may require changes to the way the framework is used.
#define QE_VERSION_MINOR      6     // Public releases containing new functionality (may include fixes as well)
#define QE_VERSION_RELEASE    1     // Public releases containing fixes only.

// During tagging the version stage should be set to production
//
#define QE_VERSION_STAGE_DEVELOPMENT "Development"
#define QE_VERSION_STAGE_PRODUCTION  "Production"
#define QE_VERSION_STAGE             QE_VERSION_STAGE_DEVELOPMENT

#define QE_VERSION_STRING     STRINGIFY(QE_VERSION_MAJOR) "." \
                              STRINGIFY(QE_VERSION_MINOR) "." \
                              STRINGIFY(QE_VERSION_RELEASE) \
                              " (" QE_VERSION_STAGE ")"

#define QE_VERSION_DATE_TIME  __DATE__ " " __TIME__

// Provides values used to build shared library.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEFrameworkVersion {
public:
   explicit QEFrameworkVersion();
   virtual ~QEFrameworkVersion();

   static unsigned int getMajor ();         // e.g. 3
   static unsigned int getMinor ();         // e.g. 6
   static unsigned int getRelease ();       // e.g. 2
   static const QString getStage ();        // e.g. "Production"
   static const QString getString ();       // e.g. "3.6.2 (Production)"
   static const QString getDateTime ();     // e.g. "Apr 21 2018 16:12:08"

   // Conditional compile attributes.
   //
   static const QString getAttributes ();   // e.g. "FFMPEG video streaming, Archiver Appliance"

   // Other version info
   //
   static const QString getQtVersionStr ();     // e.g. "QT 5.10.0"
   static const QString getQwtVersionStr ();    // e.g. "QWT 6.1.3"
   static const QString getEpicsVersionStr ();  // e.g. "EPICS 7.0.1.1"
};

#endif // QE_FRAMEWORK_VERSION_H
