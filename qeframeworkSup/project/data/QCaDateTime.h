/*  QCaDateTime.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
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
 *  Copyright (c) 2009, 2010 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

// CA Date Time manager

#ifndef QCADATETIME_H
#define QCADATETIME_H

#include <QDateTime>
#include <QEPluginLibrary_global.h>

class QEPLUGINLIBRARYSHARED_EXPORT QCaDateTime : public QDateTime
{
public:
    QCaDateTime();
    QCaDateTime( QDateTime dt );
    // void operator=( const QCaDateTime& other );
    QCaDateTime& operator=(const QCaDateTime& other);

    QCaDateTime( unsigned long seconds, unsigned long nanoseconds );

    QString text();

    /// Equivilent of addSecs and secsTo in base class, save that we specify the
    /// seconds as a floating point number.
    ///
    QCaDateTime addSeconds( const double seconds ) const;
    double secondsTo( const QDateTime & target ) const;

    /// Duration in seconds from base time to this time.
    /// Note: this is the opposite sense to the parent QDateTime daysTo, secsTo and msecsTo functions.
    /// Phase out
    ///
    double floating( const QDateTime & base ) const;

    /// Recover original EPICS time constructor parameters.
    //
    unsigned long getSeconds() const;
    unsigned long getNanoSeconds() const;

private:
    unsigned long nSec;
};

#endif // QCADATETIME_H
