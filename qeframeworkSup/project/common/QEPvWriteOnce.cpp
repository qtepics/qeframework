/*  QEPvWriteOnce.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2022-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEPvWriteOnce.h"
#include <QDebug>
#include <QTimer>
#include <QEWidget.h>
#include <macroSubstitution.h>

#define DEBUG  qDebug () << "QEPvWriteOnce" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QEPvWriteOnce::QEPvWriteOnce(const QString& pvNameIn,
                             const QString& valueIn,
                             const QE::Formats formatIn,
                             QObject* parent) : QObject (parent)
{
   this->pvName = pvNameIn;
   this->value = valueIn;
   this->format = formatIn;
   this->qca = nullptr;
}

//------------------------------------------------------------------------------
// place holder
QEPvWriteOnce::~QEPvWriteOnce() { }

//------------------------------------------------------------------------------
//
bool QEPvWriteOnce::writeNow (const QString& macroSubs)
{
   const macroSubstitutionList substitutions (macroSubs);

   // Perform any required macro substitutions.
   //
   const QString usePvName = substitutions.substitute (this->pvName);
   this->substitutedValue = substitutions.substitute (this->value);

   if (usePvName.isEmpty()) return false;

   this->qca = new qcaobject::QCaObject (usePvName, this, 0);

   // We are writing, only need wait for a successful connection.
   // No subscribe/single shot read needed.
   //
   QObject::connect (qca,  SIGNAL (connectionChanged (QCaConnectionInfo&, const unsigned int&)),
                     this, SLOT   (connectionChanged (QCaConnectionInfo&, const unsigned int&)));

   // Set timeout (to recover object) - 2 secs should be plenty of time.
   //
   QTimer::singleShot (2000, this, SLOT (connectionTimeout()));

   // Need to explicity subscribe, singleShotRead or connectChannel.
   // As we are write only, connectChannel will do.
   //
   return qca->connectChannel ();
}

//------------------------------------------------------------------------------
//
void QEPvWriteOnce::writeToVariable (qcaobject::QCaObject* qca)
{
   if (!qca) return;  // sanity check

   bool okay = false;
   QVariant pvData;

   switch (this->format) {
      case QE::Floating:
         pvData = QVariant (double (this->substitutedValue.toDouble (&okay)));
         break;

      case QE::Integer:
         pvData = QVariant (int (this->substitutedValue.toInt (&okay)));
         break;

      case QE::UnsignedInteger:
         pvData = QVariant (qlonglong (this->substitutedValue.toLongLong (&okay)));
         break;

      case QE::Default:
         /// TODO - fix this option, but go with string for now.
      case QE::String:
         pvData = QVariant (this->substitutedValue);
         okay = true;
         break;

      case QE::Time:
      case QE::LocalEnumeration:
         DEBUG << "unhandled format value" << this->format;
         okay = false;
         break;

//    case QE::NT_Table:
//    case QE::NT_Image:
//    case QE::Opaque:
//         DEBUG << "unexpected format value" << this->format;
//         okay = false;
//         break;

      default:
         DEBUG << "erroneous format value" << this->format;
         okay = false;
         break;
   }

   // Was the conversion successful.
   //
   if (okay) {
      qca->writeData (pvData);
   } else {
      DEBUG << "conversion of " << substitutedValue <<  "to"
            << this->format <<  " failed.";
   }

   // The object will be deleted when control returns to the event loop.
   // Or can we just delete it now??
   //
   this->qca->deleteLater();
   this->qca = nullptr;

   this->deleteLater();
}

//------------------------------------------------------------------------------
// slot
void QEPvWriteOnce::connectionChanged (QCaConnectionInfo& connectionInfo,
                                       const unsigned int&)
{
   if (!this->qca) return;  // sanity check

   if (connectionInfo.isChannelConnected()) {
      this->writeToVariable (qca);
   }
}

//------------------------------------------------------------------------------
// slot
void QEPvWriteOnce::connectionTimeout()
{
   if (this->qca) {
      delete this->qca;
      this->qca = nullptr;
   }
   this->deleteLater();
}

// end
