/*  QEPvNameUri.h
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

#ifndef QE_PV_NAME_URI_H
#define QE_PV_NAME_URI_H

#include <QString>
#include <QList>
#include <QEFrameworkLibraryGlobal.h>

/// This class create a PV name uniform resource identifier.
/// This includes the protocol, i.e. Channel Access or PV access.
/// The format is, for example:
///
/// "ca://SR11BCM01:CURRENT_MONITOR"
/// "pva://SR11BCM01:CURRENT_MONITOR"
///
/// where "ca://" specifies the Channel Access protocol
/// and   "pva://" specifies the PV Access protocol
/// and   "SR11BCM01:CURRENT_MONITOR" is the PV name.
///
/// The default provider, when not specified as indicated above, may be specified by
/// the environment variable QE_DEFAULT_PROVIDER, and can be defined as either "CA"
/// or "PVA" - case insensitive.  If the environment variable is not defined or is
/// ill-defined the default default provider is Channel Access.
//
// Rationale:
// 1) We have extended the framework to handle PV Access, and
//    designate such PVs as "pva://SR11BCM01:CURRENT_MONITOR"
// 2) To potentially allow other "PV" protocols such as:
//    "env://XXX"  to access environment variable XXX
//    "lit://14"   to access literal values (read only)
//    "loc://name" to access some application local value.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPvNameUri
{
public:
   enum Protocol {
      undefined = 0,  // or invalid
      ca,             // Channel Access           - prefix ca://
      pva,            // Process Variable Access  - prefix pva://
      NUMBER_OF_PROTOCOLS
   };

   static QString protocolImage (const Protocol protocol);

   // Contruct an undefined uri
   //
   explicit QEPvNameUri ();

   // Contruct uri with PV name and protocol.
   //
   explicit QEPvNameUri (const QString& pvName, const Protocol protocol = ca);

   // Contruct uri with name.  Essential constructor with built in decode uri.
   //
   explicit QEPvNameUri (const QString& uri, const bool strict);

   // Destruct object.
   //
   virtual ~QEPvNameUri();

   // Forms uniform resource identifier, as a string e.g.:
   // "ca://SR11BCM01:CURRENT_MONITOR[0]"
   //
   QString encodeUri () const;

   // Decodes the uri. If and only if successful, the object is updated.
   // The strict parameter controls if strict syntax checking applied.
   //
   // Thus when strict is true, this function expects a uri like:
   // "ca://SR11BCM01:CURRENT_MONITOR"
   //
   // When false, default ca protocol is used no protocol is defined.
   // The following are allowed:
   //
   // "ca://SR11BCM01:CURRENT_MONITOR"
   // "SR11BCM01:CURRENT_MONITOR"
   //
   // Note: strict set false does NOT mean interpret an invalid protocol as ca.
   // If present, the protocol specified MUST be valid.
   //
   // The protocol scheme prefix is case insensitive, so the following
   // are all accepted:
   // "ca://SR11BCM01:CURRENT_MONITOR"
   // "cA://SR11BCM01:CURRENT_MONITOR"
   // "Ca://SR11BCM01:CURRENT_MONITOR"
   // "CA://SR11BCM01:CURRENT_MONITOR"
   //
   bool decodeUri (const QString& uri, const bool strict);

   void setProtocol (const Protocol protocol);  // set uri protocol
   Protocol getProtocol () const;               // get uri protocol

   void setPvName (const QString& pvName);      // set pv name part of uri
   QString getPvName () const;                  // get pv name part of uri

private:
   static Protocol getDefaultProtocol();        // uses an adaptation parameter
   Protocol protocol;
   QString pvName;
};


// Provides a list of URI items.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPvNameUriList
{
public:
   explicit QEPvNameUriList ();
   virtual ~QEPvNameUriList ();

   void clear ();
   void append (const QEPvNameUri& other);
   int count () const;
   QEPvNameUri value (const int j) const;

private:
   QList<QEPvNameUri> data;
};


#endif // QE_PV_NAME_URI_H
