/*  QEOpaqueData.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (C) 2019-2022 Australian Synchrotron
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
 */

#include "QEOpaqueData.h"
#include <iostream>
#include <QEPvaData.h>

#define DEBUG qDebug() << "QEOpaqueData" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QEOpaqueData::QEOpaqueData ()
{
   // needed for types to be registrered as meta type.
}

//------------------------------------------------------------------------------
//
QEOpaqueData::QEOpaqueData (const QEOpaqueData & other)
{
   // needed for types to be registrered as meta type.
   *this = other;
}

//------------------------------------------------------------------------------
//
QEOpaqueData::~QEOpaqueData ()
{
   // needed for types to be registrered as meta type.
}

//------------------------------------------------------------------------------
//
#ifdef QE_INCLUDE_PV_ACCESS

bool QEOpaqueData::assignFrom (const epics::pvData::StructureConstPtr)
{
   return true;
}

#endif  // QE_INCLUDE_PV_ACCESS

//------------------------------------------------------------------------------
//
void QEOpaqueData::clear () { }   // place holder.


//------------------------------------------------------------------------------
//
QVariant QEOpaqueData::toVariant () const
{
   QVariant result;
   result.setValue (*this);
   return result;
}

//------------------------------------------------------------------------------
//
bool QEOpaqueData::assignFromVariant (const QVariant & item)
{
   bool result = QEOpaqueData::isAssignableVariant (item);
   if (result) {
      this->clear ();
      *this = item.value < QEOpaqueData > ();
   }
   return result;
}

//------------------------------------------------------------------------------
// static
bool QEOpaqueData::isAssignableVariant (const QVariant & item)
{
   return item.canConvert < QEOpaqueData > ();
}

//------------------------------------------------------------------------------
// static
bool QEOpaqueData::registerMetaType ()
{
   qRegisterMetaType < QEOpaqueData > ();
   return true;
}

// Elaborate on start up.
//
static const bool _elaborate = QEOpaqueData::registerMetaType ();

//------------------------------------------------------------------------------
//
QDebug operator<< (QDebug dbg, const QEOpaqueData&)
{
   dbg << "QEOpaqueData" << "\n";
   return dbg.maybeSpace ();
}

// end
