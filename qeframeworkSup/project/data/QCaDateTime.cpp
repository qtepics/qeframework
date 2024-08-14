/*  QCaDateTime.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2024 Australian Synchrotron
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

#include "QCaDateTime.h"
#include <QString>
#include <QTextStream>
#include <QDebug>

static const QDateTime qtEpoch    (QDate( 1970, 1, 1 ), QTime( 0, 0, 0, 0 ), Qt::UTC );
static const QDateTime epicsEpoch (QDate( 1990, 1, 1 ), QTime( 0, 0, 0, 0 ), Qt::UTC );
static unsigned long EPICSQtEpocOffset = qtEpoch.secsTo ( epicsEpoch );

/*
  Construct an empty QCa date time
 */
QCaDateTime::QCaDateTime()
{
    this->nSec = 0;
    this->userTag = 0;
}

/*
  Construct a QCa date time set to the same date/time as another QCa date time .
 */
QCaDateTime::QCaDateTime( const QCaDateTime& other ) : QDateTime( other )
{
    this->nSec = other.nSec;
    this->userTag = other.userTag;
}

/*
  Construct a QCa date time set to the same date/time as a conventional QDateTime
 */
QCaDateTime::QCaDateTime( const QDateTime& dt ) : QDateTime( dt )
{
    this->nSec = 0;
    this->userTag = 0;
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
    // the remaining nanoseconds are saved in this class.
    //
    mSec = nanoseconds / 1000000;
    this->nSec = nanoseconds % 1000000;

    // Calc number of mSecs since the epoc.
    // Note, although the EPICS time stamp is in seconds since a base, the method which
    // takes seconds since a base time uses a different base, so an offset is added.
    //
    qint64 mSecsSinceEpoch;
    mSecsSinceEpoch = ((qint64) (seconds + EPICSQtEpocOffset)) * 1000 + mSec;
    setMSecsSinceEpoch (mSecsSinceEpoch);
    
    this->userTag = userTagIn;
}

// Place holder
//
QCaDateTime::~QCaDateTime() {}


/*
  Copy a QCaDateTime from another and return value to allow t1 = t2 = t3 = etc.
 */
QCaDateTime& QCaDateTime::operator=( const QCaDateTime& other )
{
    // Do parent class stuff assignment.
    *(QDateTime*) this = (QDateTime) other;

    // and then copy class specific stuff.
    this->nSec = other.nSec;
    this->userTag = other.userTag;

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
    out = this->toString( QString( "yyyy-MM-dd hh:mm:ss.zzz" ));

    // Add down to nanosecond resolution
    QTextStream s( &out );
    s.setFieldAlignment( QTextStream::AlignRight );
    s.setPadChar( '0' );
    s.setFieldWidth( 6 );
    s << this->nSec;

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
   qint64 msec = this->msecsTo (target);
   return double (msec) / double (1000.0);
}

/*
  Returns original number of seconds from EPICS Epoch
 */
unsigned long QCaDateTime::getSeconds() const
{
   qint64 msec = epicsEpoch.msecsTo (*this);

   if( msec < 0 ) msec = 0;
   return (unsigned long) (msec / 1000);
}

/*
  Returns original number of nano-seconds.
 */
unsigned long QCaDateTime::getNanoSeconds() const
{
   qint64 msec = epicsEpoch.msecsTo (*this);

   if( msec < 0 ) msec = 0;

   msec = msec % 1000;
   return  (unsigned long) (msec * 1000000) + this->nSec;
}

/*
  Returns original userTag - zero for CA.
 */
int QCaDateTime::getUserTag() const
{
   return this->userTag;
}

// end
