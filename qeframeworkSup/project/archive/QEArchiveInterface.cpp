/*  QEArchiveInterface.cpp
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

#include "QEArchiveInterface.h"

#include <QDebug>
#include <QEPlatform.h>

#define DEBUG qDebug () << "QEArchiveInterface" << __LINE__ << __FUNCTION__  << "  "

//------------------------------------------------------------------------------
// Similar to EPICS times - Archive times are specified as a number of seconds
// and nano seconds from an Epoch data time (1/1/1970). This happens to be the
// epoch used by Qt but we don't make use of that assumption.
//
static const QDateTime archiveEpoch = QEPlatform::constructEpoch (1970);
static const QDateTime epicsEpoch   = QEPlatform::constructEpoch (1990);

// Static count of the number of seconds between the archiver and epics epochs.
//
static unsigned long e2aOffset = archiveEpoch.secsTo (epicsEpoch);


//------------------------------------------------------------------------------
// static
QCaDateTime QEArchiveInterface::convertArchiveToEpics (const int seconds, const int nanoSecs)
{
   const unsigned long epicsSeconds = (unsigned long) seconds - e2aOffset;
   const unsigned long epicsNanoSec = (unsigned long) nanoSecs;

   return QCaDateTime (epicsSeconds, epicsNanoSec);
}

//------------------------------------------------------------------------------
// static
void QEArchiveInterface::convertEpicsToArchive (const QCaDateTime& datetime, int& seconds, int& nanoSecs)
{
   const unsigned long epicsSeconds = datetime.getSeconds ();
   const unsigned long epicsNanoSec = datetime.getNanoSeconds ();

   seconds = (int) (epicsSeconds + e2aOffset);
   nanoSecs = (int) (epicsNanoSec);
}

//------------------------------------------------------------------------------
//
QEArchiveInterface::QEArchiveInterface (QObject *parent) : QObject (parent) { }

//------------------------------------------------------------------------------
//
QEArchiveInterface::~QEArchiveInterface () { }

//------------------------------------------------------------------------------
//
bool QEArchiveInterface::registerMetaTypes ()
{
   qRegisterMetaType<QEArchiveInterface::Archive> ("QEArchiveInterface::Archive");
   qRegisterMetaType<QEArchiveInterface::ArchiveList> ("QEArchiveInterface::ArchiveList");
   qRegisterMetaType<QEArchiveInterface::PVNameList> ("QEArchiveInterface::PVNameList");
   qRegisterMetaType<QEArchiveInterface::ResponseValueList> ("QEArchiveInterface::ResponseValueList");
   qRegisterMetaType<QEArchiveInterface::Context> ("QEArchiveInterface::Context");
   qRegisterMetaType<QEArchiveInterface::States> ("QEArchiveInterface::States");
   return true;
}

// Elaborate on start up.
//
static const bool _elaborate = QEArchiveInterface::registerMetaTypes ();

//------------------------------------------------------------------------------
//
QString QEArchiveInterface::alarmSeverityName (enum archiveAlarmSeverity severity)
{
   QString result;

   switch (severity) {
   case archSevNone:       result = "No Alarm";   break;
   case archSevMinor:      result = "Minor";      break;
   case archSevMajor:      result = "Major";      break;
   case archSevInvalid:    result = "Invalid";    break;
   case archSevEstRepeat:  result = "Est_Repeat"; break;
   case archSevDisconnect: result = "Disconnect"; break;
   case archSevStopped:    result = "Stopped";    break;
   case archSevRepeat:     result = "Repeat";     break;
   case archSevDisabled:   result = "Disabled";   break;
   default:
      result = QString ("Archive Invalid Sevrity (%1)").arg ((int) severity);
      break;
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEArchiveInterface::setUrl (QUrl url)
{
   this->mUrl = url;
}

//------------------------------------------------------------------------------
//
QUrl QEArchiveInterface::getUrl () const
{
   return this->mUrl;
}

//------------------------------------------------------------------------------
//
QString QEArchiveInterface::getName () const
{
   return this->getUrl ().toString ();
}

// end
