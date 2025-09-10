/*  QEPvNameUri.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (C) 2018-2025 Australian Synchrotron
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
#include <QEAdaptationParameters.h>

#define DEBUG qDebug () << "QEPvNameUri" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
// This must be consistant with the enum Protocol definition out of the header.
//
static const QString prefix[QEPvNameUri::NUMBER_OF_PROTOCOLS] = {
   "__undefined__",    // undefined
   "ca",               // ca
   "pva"               // pva
};

static const QString cds = "://";    // colon double slash

//------------------------------------------------------------------------------
// static
QString QEPvNameUri::protocolImage (const Protocol protocol)
{
   return prefix [protocol];
}


//==============================================================================
// QEPvNameUri
//==============================================================================
// static
QEPvNameUri::Protocol QEPvNameUri::getDefaultProtocol()
{
   // The default protocol is initialised to the default default.
   //
   static QEPvNameUri::Protocol theDefaultProtocol = QEPvNameUri::ca;
   static bool theDefaultIsDefined = false;

   // Has the default protocol been defined?
   //
   if (!theDefaultIsDefined) {
      // No - attempt to figure out what the default is.
      //
      QEAdaptationParameters ap ("QE_");
      const QString defProtoSpec = ap.getString ("default_provider", "ca").toLower();

      for (int j = 0; j < NUMBER_OF_PROTOCOLS; j++) {
         const Protocol protocol = Protocol (j);

         // Don't allow the undefined protocol.
         //
         if (protocol == undefined)
            continue;

         // Don't allow the pv access protocol if not included.
         //
#ifndef QE_INCLUDE_PV_ACCESS
         if (protocol == pva)
            continue;
#endif

         if (defProtoSpec == prefix[j]) {
            // Found it.
            theDefaultProtocol = protocol;
            theDefaultIsDefined = true;
            break;
         }
      }

      // Either no protocol specified or an invalid protocol specified.
      //
      if (!theDefaultIsDefined) {
         DEBUG << "Undefined/invalid default protocol" << defProtoSpec
               << ", going with Channel Access";
         theDefaultProtocol = QEPvNameUri::ca;
         theDefaultIsDefined = true;
      }      
   }

   return theDefaultProtocol;
}

//------------------------------------------------------------------------------
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
QEPvNameUri::QEPvNameUri (const QString& uri, const bool strict)
{
   if (!this->decodeUri (uri, strict)) {
      this->pvName = "";
      this->protocol = QEPvNameUri::undefined;
   }
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
         result = QString ("ca%1%2").arg (cds).arg(this->pvName);
         break;

      case pva:
#ifdef QE_INCLUDE_PV_ACCESS
         result = QString ("pva%1%2").arg (cds).arg (this->pvName);
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

   Protocol specifiedProtocol = QEPvNameUri::undefined;
   QString pvName;

   specifiedProtocol = QEPvNameUri::undefined;
   for (int j = 0; j < NUMBER_OF_PROTOCOLS; j++) {
      const Protocol protocol = Protocol (j);

      // Don't allow the undefined protocol.
      //
      if (protocol == undefined)
         continue;

      // Don't allow the pv access protocol if not included.
      //
#ifndef QE_INCLUDE_PV_ACCESS
      if (protocol == pva)
         continue;
#endif

      QString startCheck = prefix[j] + cds;

      // Note the use of toLower.
      //
      if (work.toLower().startsWith (startCheck)) {
         // We found a valid protocol - remove it from work to from the PV name.
         //
         specifiedProtocol = protocol;
         pvName = work.mid (startCheck.length ());
         break;
      }
   }

   if (specifiedProtocol == QEPvNameUri::undefined) {
      // Either no protocol specified or an invalid protocol specified.
      //
      if (strict)
         return false;

      // Check if any type of protocol defined at all.
      //
      int kp = work.indexOf (cds);  // cds = "://";

      // We allow no protocol when strict is false, but never an unknown protocol.
      //
      if (kp >= 0)
         return false;

      specifiedProtocol = this->getDefaultProtocol();   // use the default.
      pvName = work;
   }

   pvName = pvName.trimmed ();
   if (pvName.isEmpty ()) {
      return false;
   }

   this->protocol = specifiedProtocol;
   this->pvName = pvName;

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
