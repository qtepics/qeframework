/*  QEFrameworkVersion.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
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
#include <QEPvaCheck.h>

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
   attributes.append ("PV Access");
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

// end
