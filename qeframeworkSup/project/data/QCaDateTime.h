/*  QCaDateTime.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2020 Australian Synchrotron
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

// CA Date Time manager

#ifndef QE_DATE_TIME_H
#define QE_DATE_TIME_H

#include <QDateTime>
#include <QEFrameworkLibraryGlobal.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QCaDateTime : public QDateTime
{
public:
    QCaDateTime();
    QCaDateTime( QDateTime dt );
    // void operator=( const QCaDateTime& other );
    QCaDateTime& operator=(const QCaDateTime& other);

    QCaDateTime( const unsigned long seconds,
                 const unsigned long nanoseconds,
                 const int userTag = 0 );

    QString text() const;
    QString ISOText() const;

    /// Equivilent of addSecs and secsTo in base class, save that we specify the
    /// seconds as a floating point number, i.e. take into account fractions of a
    /// second (i.e. mSec but not nano sec).
    ///
    QCaDateTime addSeconds( const double seconds ) const;
    double secondsTo( const QDateTime & target ) const;

    /// Duration in seconds from base time to this time.
    /// Note: this is the opposite sense to the parent QDateTime daysTo, secsTo
    /// and msecsTo functions. Phase out
    ///
    Q_DECL_DEPRECATED
    double floating( const QDateTime & base ) const;

    /// Recover original EPICS time constructor parameters.
    //
    unsigned long getSeconds() const;
    unsigned long getNanoSeconds() const;
    int getUserTag() const;

private:
    unsigned long nSec;
    int userTag;
};

#endif // QE_DATE_TIME_H
