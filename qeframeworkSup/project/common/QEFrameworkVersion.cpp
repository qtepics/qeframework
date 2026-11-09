/*  QEFrameworkVersion.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEFrameworkVersion.h"
#include <qglobal.h>
#include <QStringList>
#include <epicsVersion.h>
#include <qwt_global.h>
#include <acai_version.h>

#if defined (QE_INCLUDE_PV_ACCESS) && defined (QE_USING_PVXS)
#include <pvxs/version.h>
#endif

//------------------------------------------------------------------------------
//
QEFrameworkVersion::QEFrameworkVersion () { }

QEFrameworkVersion::~QEFrameworkVersion () { }

//------------------------------------------------------------------------------
//
unsigned int QEFrameworkVersion::getMajor ()
{
   return QE_VERSION_MAJOR;
}

//------------------------------------------------------------------------------
//
unsigned int QEFrameworkVersion::getMinor ()
{
   return QE_VERSION_MINOR;
}

//------------------------------------------------------------------------------
//
unsigned int QEFrameworkVersion::getRelease ()
{
   return QE_VERSION_RELEASE;
}

//------------------------------------------------------------------------------
//
const QString QEFrameworkVersion::getStage ()
{
   return QE_VERSION_STAGE;
}

//------------------------------------------------------------------------------
// static
bool QEFrameworkVersion::getPVAccessInuse ()
{
#if defined(QE_INCLUDE_PV_ACCESS)
   return true;
#else
   return false;
#endif
}

//------------------------------------------------------------------------------
// static
bool QEFrameworkVersion::getPvxsInuse ()
{
#if defined(QE_INCLUDE_PV_ACCESS) && defined (QE_USING_PVXS)
   return true;
#else
   return false;
#endif
}

//------------------------------------------------------------------------------
//
const QString QEFrameworkVersion::getAttributes ()
{
   QString result;
   QStringList attributes;

   // Conditional compile attributes.
   //
   #ifdef QE_USE_MPEG
   attributes.append ("FFMPEG video streaming");
   #endif

   #ifdef QE_ARCHAPPL_SUPPORT
   attributes.append ("Archiver Appliance");
   #endif

   #ifdef QE_INCLUDE_PV_ACCESS
   #ifdef QE_USING_PVXS
   attributes.append ("PV Access (PVXS)");
   #else
   attributes.append ("PV Access");
   #endif
   #endif

   #ifdef QE_AD_SUPPORT
   attributes.append ("Image decompression");
   #endif

   if (attributes.count() > 0) {
      result = attributes.join (", ");
   } else {
      result = "None";
   }

   return result;
}

//------------------------------------------------------------------------------
//
const QString QEFrameworkVersion::getString ()
{
   return QString (QE_VERSION_STRING);
}

//------------------------------------------------------------------------------
//
const QString QEFrameworkVersion::getDateTime ()
{
   return QString (QE_VERSION_DATE_TIME);
}

//------------------------------------------------------------------------------
//
const QString QEFrameworkVersion::getQtVersionStr ()
{
   return QString (QT_VERSION_STR);
}

//------------------------------------------------------------------------------
//
const QString QEFrameworkVersion::getQwtVersionStr ()
{
   return QString ("QWT %1").arg (QWT_VERSION_STR);
}

//------------------------------------------------------------------------------
//
const QString QEFrameworkVersion::getEpicsVersionStr ()
{
   return QString (EPICS_VERSION_STRING);
}

//------------------------------------------------------------------------------
//
const QString QEFrameworkVersion::getAcaiVersionStr ()
{
   return QString (ACAI_VERSION_STRING);
}

//------------------------------------------------------------------------------
//
const QString QEFrameworkVersion::getPvxsVersionStr ()
{
#if defined(QE_INCLUDE_PV_ACCESS) && defined (QE_USING_PVXS)
   // The PVXS version string appends the tag/commit.
   QString v = QString (pvxs::version_str());
   const int n = v.indexOf(" (");
   if (n > 0) {
      v = v.left(n);
   }
   return v;
#else
   return "n/a";
#endif
}

// end
