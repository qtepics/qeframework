/*  QEOpaqueData.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2019-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
QEOpaqueData& QEOpaqueData::operator=(const QEOpaqueData& )
{
   // nothing to copy per se.
   return *this;
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
