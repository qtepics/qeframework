/*  QCaDateTime.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2009-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

// QE Date Time manager

#ifndef QE_DATE_TIME_H
#define QE_DATE_TIME_H

#include <QDateTime>
#include <QEFrameworkLibraryGlobal.h>

/// Extends the Qt datatime object in irder to provide nSec precision
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QCaDateTime : public QDateTime
{
public:
    explicit QCaDateTime();
    QCaDateTime( const QCaDateTime& other );

    QCaDateTime( const QDateTime& dt );

    QCaDateTime( const unsigned long seconds,
                 const unsigned long nanoseconds,
                 const int userTag = 0 );
    ~QCaDateTime();

    QCaDateTime& operator=(const QCaDateTime& other);

    QString text() const;
    QString ISOText() const;

    /// Equivilent of addSecs and secsTo in base class, save that we specify the
    /// seconds as a floating point number, i.e. take into account fractions of a
    /// second (i.e. mSec but not nano sec).
    ///
    QCaDateTime addSeconds( const double seconds ) const;
    double secondsTo( const QDateTime& target ) const;

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
