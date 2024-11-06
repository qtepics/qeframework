/*  QEPlatform.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2015-2024 Australian Synchrotron.
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
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
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

// TODO check for QT 5.X and use inbuilt functions.

QEPlatform::QEPlatform () { }
QEPlatform::~QEPlatform () { }

//------------------------------------------------------------------------------
//
QMetaType::Type QEPlatform::metaType (const QVariant& v)
{
#if QT_VERSION < 0x060000
   // type() returns QVariant::Type
   return static_cast<QMetaType::Type>(v.type());
#else
   // typeId() [ same as metaType().id() ] returns an int
   return static_cast<QMetaType::Type>(v.typeId());
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
