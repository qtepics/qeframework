/*  QArchiveInterface.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2021 Australian Synchrotron
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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
 *    andrews@ansto.gov.au
 */

#include "QEArchiveInterface.h"

#include <QDebug>

#define DEBUG qDebug () << "QEArchiveInterface" << __LINE__ << __FUNCTION__  << "  "

//------------------------------------------------------------------------------
// Similar to EPICS times - Archive times are specified as a number of seconds
// and nano seconds from an Epoch data time (1/1/1970). This happens to be the
// epoch used by Qt but we don't make use of that assumption.
//
static const QDateTime archiveEpoch (QDate (1970, 1, 1), QTime (0, 0, 0, 0), Qt::UTC);
static const QDateTime epicsEpoch   (QDate (1990, 1, 1), QTime (0, 0, 0, 0), Qt::UTC);

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
