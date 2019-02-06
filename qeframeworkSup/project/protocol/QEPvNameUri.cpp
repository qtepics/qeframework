/*  QEPvNameUri.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (C) 2018 Australian Synchrotron
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

#include "QEPvNameUri.h"

#include <QDebug>
#include <QECommon.h>
#include <QEPvaCheck.h>

#define DEBUG qDebug () << "QEPvNameUri" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
static const QString prefix[QEPvNameUri::NUMBER_OF_PROTOCOLS] = {
   "__undefined__", "ca", "pva"
};

static const QString cds = "://";    // colon double slash

//==============================================================================
// QEPvNameUri
//==============================================================================
//
QEPvNameUri::QEPvNameUri ()
{
   this->pvName = "";
   this->protocol = QEPvNameUri::undefined;
}

//------------------------------------------------------------------------------
//
QEPvNameUri::QEPvNameUri (const QString& pvNameIn,
                          const Protocol protocolIn)
{
   this->pvName = pvNameIn;
   this->protocol = protocolIn;
}

//------------------------------------------------------------------------------
//
QEPvNameUri::~QEPvNameUri ()
{
   // place holder
}

//------------------------------------------------------------------------------
//
QString QEPvNameUri::encodeUri () const
{
   QString result = "";

   switch (this->protocol) {
      case ca:
         result = QString ("ca%1").arg (cds).arg(this->pvName);
         break;

      case pva:
#ifdef QE_INCLUDE_PV_ACCESS
         result = QString ("pva%1").arg (cds).arg (this->pvName);
#endif
         break;

      default:
         result = "";
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
//
bool QEPvNameUri::decodeUri (const QString& uri, const bool strict)
{
   QString work = uri.trimmed ();

   Protocol pr = QEPvNameUri::undefined;
   QString pv;

   pr = QEPvNameUri::undefined;
   for (int j = 0; j < NUMBER_OF_PROTOCOLS; j++) {

      // Don't allow the undefined protocol.
      //
      if (Protocol (j) == undefined)
         continue;

#ifndef QE_INCLUDE_PV_ACCESS
      // Don't allow the pv access protocol if not included.
      //
      if (Protocol (j) == pva)
         continue;
#endif

      QString startCheck = prefix[j] + cds;

      // Note use of toLower
      //
      if (work.toLower ().startsWith (startCheck)) {
         // We found a valid protocol - remove it from work to from the PV name
         //
         pr = Protocol (j);
         pv = work.mid (startCheck.length ());
         break;
      }
   }

   if (pr == QEPvNameUri::undefined) {
      // Either no protocol specified or an invalid protocol specified.
      //
      if (strict)
         return false;

      // Check if any type of protocol defined at all.
      //
      int kp = work.indexOf (cds);

      // We allow no protocol when strict is false, but never an unknown protocol.
      //
      if (kp >= 0)
         return false;

      pr = ca;                  // set default.
      pv = work;
   }

   pv = pv.trimmed ();
   if (pv.isEmpty ()) {
      return false;
   }

   this->protocol = pr;
   this->pvName = pv;

   return true;
}

//------------------------------------------------------------------------------
//
void QEPvNameUri::setProtocol (const Protocol protocolIn)
{
   this->protocol = protocolIn;
}

//------------------------------------------------------------------------------
//
QEPvNameUri::Protocol QEPvNameUri::getProtocol () const
{
   return this->protocol;
}

//------------------------------------------------------------------------------
//
void QEPvNameUri::setPvName (const QString & pvNameIn)
{
   this->pvName = pvNameIn;
}

//------------------------------------------------------------------------------
//
QString QEPvNameUri::getPvName () const
{
   return this->pvName;
}

//==============================================================================
// QEPvNameUriList
//==============================================================================
//
QEPvNameUriList::QEPvNameUriList ()
{
}

//------------------------------------------------------------------------------
//
QEPvNameUriList::~QEPvNameUriList ()
{
   this->clear ();
}

//------------------------------------------------------------------------------
//
void QEPvNameUriList::clear ()
{
   this->data.clear ();
}

//------------------------------------------------------------------------------
//
void QEPvNameUriList::append (const QEPvNameUri & other)
{
   this->data.append (other);
}

//------------------------------------------------------------------------------
//
int QEPvNameUriList::count () const
{
   return this->data.count ();
}

//------------------------------------------------------------------------------
//
QEPvNameUri QEPvNameUriList::value (const int j) const
{
   return this->data.value (j);
}

// end
