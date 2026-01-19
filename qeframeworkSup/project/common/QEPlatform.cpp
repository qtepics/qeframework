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

// TODO check for QT 5.X and use inbuilt functions.

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
void QEPlatform::setUTCTimeZone (QDateTime& dateTime)
{
#if QT_VERSION < 0x060000
   dateTime.setTimeSpec (Qt::UTC);
#else
   QTimeZone zone (QTimeZone::UTC);
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
