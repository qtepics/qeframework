/*  QEPlatform.cpp
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

#include "QEPlatform.h"

#ifdef WIN32
#include <float.h>
#else
#include <math.h>
#endif
#include <limits>

#include <QDebug>
#include <QString>
#include <QTimeZone>

#define DEBUG qDebug() << "QEPlatform" << __LINE__ << __FUNCTION__ << "  "


QEPlatform::QEPlatform () { }
QEPlatform::~QEPlatform () { }

//------------------------------------------------------------------------------
//
QMetaType::Type QEPlatform::metaType (const QVariant& v)
{
   QMetaType::Type result;
#if QT_VERSION < 0x060000
   // type() returns QVariant::Type
   result = static_cast<QMetaType::Type>(v.type());
#else
   // typeId() [ same as metaType().id() ] returns an int
   result = static_cast<QMetaType::Type>(v.typeId());
#endif
   return result;
}

//------------------------------------------------------------------------------
//
QPoint QEPlatform::positionOf (QDropEvent* event)
{
   QPoint result;
   if (event) {
#if QT_VERSION < 0x060000
      result = event->pos();
#else
      result = event->position().toPoint();
#endif
   }
   return result;
}

//------------------------------------------------------------------------------
//
QPoint QEPlatform::positionOf (QMouseEvent* event)
{
   QPoint result;
   if (event) {
#if QT_VERSION < 0x060000
      result = event->pos();
#else
      result = event->position().toPoint();
#endif
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEPlatform::setTimeZone (QDateTime& dateTime, const Qt::TimeSpec timeSpec)
{
   if ((timeSpec != Qt::UTC) && (timeSpec != Qt::LocalTime)) {
      DEBUG << "Unexprected time spec" << timeSpec << "ignored.";
      return;
   }

#if QT_VERSION < 0x060500
   dateTime.setTimeSpec (timeSpec);
#else
   // Currently Qt::TimeSpec and QTimeZone::Initialization values are numerically
   // the same; howver that cound change/
   //
   const QTimeZone::Initialization init = (timeSpec == Qt::UTC) ? QTimeZone::UTC : QTimeZone::LocalTime;
   QTimeZone zone (init);
   dateTime.setTimeZone (zone);
#endif
}

//------------------------------------------------------------------------------
//
bool QEPlatform::isNaN (const double x)
{
#ifdef WIN32
   return _isnan (x);
#else
   return isnan (x);
#endif
}

//------------------------------------------------------------------------------
//
bool QEPlatform::isInf (const double x)
{
   if (x > std::numeric_limits<double>::max()){
      return true;
   }
   else if (x < -std::numeric_limits<double>::max()){
      return true;
   } else {
      return false;
   }
}

// end
