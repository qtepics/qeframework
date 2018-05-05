/*  QCaDateTime.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
 *
 *  Copyright (c) 2009-2018 Australian Synchrotron
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
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#include <QString>
#include <QTextStream>
#include <QCaDateTime.h>
#include <QDebug>

static const QDateTime qtEpoch    (QDate( 1970, 1, 1 ), QTime( 0, 0, 0, 0 ), Qt::UTC );
static const QDateTime epicsEpoch (QDate( 1990, 1, 1 ), QTime( 0, 0, 0, 0 ), Qt::UTC );
static unsigned long EPICSQtEpocOffset = qtEpoch.secsTo ( epicsEpoch );

/*
  Qt 4.6 does not have the msecsTo function - so we roll our own.

  Return the number of milliseconds from this datetime to the other datetime.
  If the other datetime is earlier than this datetime, the value returned is negative.

  Based on msecsTo out of qt-everywhere-opensource-src-4.8.4/src/corelib/tools/qdatetime.cpp
*/
static qint64 msecsTo_48 (const QDateTime& self, const QDateTime& other)
{
#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
   return self.msecsTo( other );
#else
   // More or less a direct copy of 4.8 code.
   //
   enum { MSECS_PER_DAY = 86400000 };

   QDate selfDate;
   QDate otherDate;
   QTime selfTime;
   QTime otherTime;

   selfDate = self.toUTC().date();
   selfTime = self.toUTC().time();

   otherDate = other.toUTC().date();
   otherTime = other.toUTC().time();

   return (static_cast<qint64>(selfDate.daysTo(otherDate)) * static_cast<qint64>(MSECS_PER_DAY)) +
           static_cast<qint64>(selfTime.msecsTo(otherTime));
#endif
}


/*
  Construct an empty QCa date time
 */
QCaDateTime::QCaDateTime()
{
    nSec = 0;
    userTag = 0;
}

/*
  Construct a QCa date time set to the same date/time as a conventional QDateTime
 */
QCaDateTime::QCaDateTime( QDateTime dt ) : QDateTime( dt )
{
    nSec = 0;
    userTag = 0;
}

/*
  Construct a QCa date time set to the same date/time as an EPICS time stamp
 */
QCaDateTime::QCaDateTime( const unsigned long seconds,
                          const unsigned long nanoseconds,
                          const int userTagIn )
{
    qint64 mSec;

    // First calculate mSecs and remaining nSecs
    // Down to the millisecond goes in the Qt base class structure,
    // the remaining nanoseconds are saved in this class
    //
    mSec = nanoseconds / 1000000;
    nSec = nanoseconds % 1000000;

#if (QT_VERSION >= QT_VERSION_CHECK(4, 8, 0))
    // Calc number of mSecs since the epoc.
    // Note, although the EPICS time stamp is in seconds since a base, the method which
    // takes seconds since a base time uses a different base, so an offset is added.
    //
    qint64 mSecsSinceEpoch;
    mSecsSinceEpoch = ((qint64) (seconds + EPICSQtEpocOffset)) * 1000 + mSec;
    setMSecsSinceEpoch (mSecsSinceEpoch);
#else
    // setMSecsSinceEpoch does not exist in older versions.
    //
    QDateTime temp;
    temp.setTime_t( seconds + EPICSQtEpocOffset );
    *this = temp.addMSecs (mSec);
#endif
    
    userTag = userTagIn;
}

/*
  Copy a QCaDateTime from another and return value to allow t1 = t2 = t3 = etc.
 */
QCaDateTime& QCaDateTime::operator=( const QCaDateTime& other )
{
    // Do parent class stuff assignment.
    *(QDateTime*) this = (QDateTime) other;

    // and then copy class specific stuff.
    nSec = other.nSec;
    userTag = other.userTag;

    // return value as well.
    return *this;
}

/*
  Returns a string which represents the date and time
 */
QString QCaDateTime::text() const
{
    // Format the date and time to millisecond resolution
    QString out;
    out = toString( QString( "yyyy-MM-dd hh:mm:ss.zzz" ));

    // Add down to nanosecond resolution
    QTextStream s( &out );
    s.setFieldAlignment( QTextStream::AlignRight );
    s.setPadChar( '0' );
    s.setFieldWidth( 6 );
    s << nSec;

    return out;
}

/*
  Returns an ISO 8601 string which represents the date and time
 */
QString QCaDateTime::ISOText() const
{
    // Format the date and time to millisecond resolution
    QString out;
    out = toString( QString( "yyyy-MM-dd'T'HH:mm:ss.zzzZ" ));

    return out;
}

/*
  Returns time represented by object plus specified number of seconds.
 */
QCaDateTime QCaDateTime::addSeconds( const double seconds ) const
{
   QCaDateTime result;

   // Note addMSecs is a QDateTime operation which ignore the nano secs.
   result = this->addMSecs (qint64 (1000.0 * seconds));
   result.nSec = this->nSec;
   return result;
}

/*
  Returns a double which represents time in seconds (to mS resolution) to specified target time.
 */
double QCaDateTime::secondsTo( const QDateTime & target ) const
{
   qint64 msec = msecsTo_48 (*this, target);
   return (double) msec / (double) 1000.0;
}


/*
  Returns a double which represents the date and time in seconds (to mS resolution) from the base time
 */
double QCaDateTime::floating( const QDateTime & base ) const
{
    qint64 msec = msecsTo_48 (base, *this);
    return (double) msec / (double) 1000.0;
}

/*
  Returns original number of seconds from EPICS Epoch
 */
unsigned long QCaDateTime::getSeconds() const
{
   qint64 msec = msecsTo_48 (epicsEpoch, *this);

   if( msec < 0 ) msec = 0;
   return (unsigned long) (msec / 1000);
}

/*
  Returns original number of nano-seconds.
 */
unsigned long QCaDateTime::getNanoSeconds() const
{
   qint64 msec = msecsTo_48 (epicsEpoch, *this);

   if( msec < 0 ) msec = 0;

   msec = msec % 1000;
   return  (unsigned long) (msec * 1000000) + nSec;
}

/*
  Returns original userTag - zero for CA.
 */
int QCaDateTime::getUserTag() const
{
   return userTag;
}

// end
