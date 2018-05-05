/*  QEFrameworkVersion.cpp
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

#include "QEFrameworkVersion.h"
#include <qglobal.h>
#include <QStringList>
#include <epicsVersion.h>
#include <qwt_global.h>

//------------------------------------------------------------------------------
//
QEFrameworkVersion::QEFrameworkVersion ()
{
}

QEFrameworkVersion::~QEFrameworkVersion ()
{
}

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
const QString QEFrameworkVersion::getAttributes ()
{
   QStringList result;

   // Conditional compile attributes.
   //
   #ifdef QE_USE_MPEG
   result.append ("FFMPEG video streaming");
   #endif

   #ifdef QE_ARCHAPPL_SUPPORT
   result.append ("Archiver Appliance");
   #endif

   #ifdef QE_INCLUDE_PV_ACCESS
   result.append ("PV Access");
   #endif

   return result.join (", ");
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

// end
