/*  QEPvNameUri.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2018-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
static const QString s_protocolPrefixList [QEPvNameUri::NUMBER_OF_PROTOCOLS] = {
   "__undefined__",    // undefined
   "ca",               // ca
   "pva",              // pva
   "env",              // env
   "lit",              // lit
};

static const QString cds = "://";    // colon double slash

//------------------------------------------------------------------------------
// static
QString QEPvNameUri::protocolImage (const Protocol protocol)
{
   return s_protocolPrefixList [protocol];
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

         if (defProtoSpec == s_protocolPrefixList[j]) {
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

   QString prefix = s_protocolPrefixList [this->protocol] + cds;

   switch (this->protocol) {
      case ca:
      case env:
      case lit:
         result = QString ("%1%2").arg (prefix).arg (this->pvName);
         break;

      case pva:
#ifdef QE_INCLUDE_PV_ACCESS
         result = QString ("%1%2").arg (prefix).arg (this->pvName);
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

      QString startCheck = s_protocolPrefixList[j] + cds;

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
