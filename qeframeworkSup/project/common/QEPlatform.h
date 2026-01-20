/*  QEPlatform.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2015-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_PLATFORM_H
#define QE_PLATFORM_H

#include <Qt>
#include <QtGlobal>
#include <QDateTime>
#include <QDropEvent>
#include <QMetaType>
#include <QMouseEvent>
#include <QPoint>
#include <QString>
#include <QVariant>
#include <QEFrameworkLibraryGlobal.h>

/// Geneal purpose platform specific functions used to hide compiler, environment
/// and version specific features. This locates all the #indef and the like tests
/// into one centralised and consistent location.
/// We use a class of static methods as opposed to a set of regular functions.
/// This provide a name space, but also allows inheritance if needs be.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPlatform {
public:
   /// Returns the meta type type of a variant, e.g. QMetaType::QStringList,
   /// as opposed to returning an int.
   static QMetaType::Type metaType (const QVariant& v);

   /// Returns the position associated with various types of event.
   ///
   static QPoint positionOf (QDropEvent* event);
   static QPoint positionOf (QMouseEvent* event);

   /// Construct an epoch date time, always Jan 1st 00:00:00
   ///
   static QDateTime constructEpoch (const int year);

   /// Changes the time zone to UTC or LocalTime only.
   ///
   static void setTimeZone (QDateTime& dateTime, const Qt::TimeSpec timeSpec);

   /// This converts the actual time to UTC or LocalTime.
   ///
   static QDateTime toTimeZone (const QDateTime& dateTime, const Qt::TimeSpec timeSpec);

   /// This function test if the specified double floating point number is 'Not a Number'.
   static bool isNaN (const double x);

   /// This function test if the specified double floating point number is +/-Infinity.
   static bool isInf (const double x);


#if QT_VERSION < 0x060000
#define QEKeepEmptyParts QString::KeepEmptyParts
#define QESkipEmptyParts QString::SkipEmptyParts
#else
#define QEKeepEmptyParts Qt::KeepEmptyParts
#define QESkipEmptyParts Qt::SkipEmptyParts
#endif

protected:
   explicit QEPlatform ();
   ~QEPlatform ();
};

# endif // QE_PLATFORM_H
