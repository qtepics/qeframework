/*  QELitClient.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QELitClient.h"
#include <QDebug>
#include <QTimer>
#include <QEPlatform.h>
#include <QEStringFormatting.h>
#include <QEVectorVariants.h>

#define DEBUG qDebug () << "QELitClient" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QELitClient::QELitClient (const QString& pvName,
                          QObject* parent) :
   QEBaseClient (QEBaseClient::LitType, pvName, parent)
{
}

//------------------------------------------------------------------------------
//
QELitClient::~QELitClient ()
{
   this->allowSignalEmission = false;
}

//------------------------------------------------------------------------------
//
bool QELitClient::readVariable ()
{
   const QString name = this->getPvName();
   bool okay = true;   // hypothesize all ok

   if (name.endsWith(".RTYP") || name.endsWith(".RTYP$")) {
      this->pvData = QVariant ("qelit");
      this->precision = 0;
      this->dateTime = QCaDateTime::currentDateTime();

   } else if (name.endsWith(".DESC") || name.endsWith(".DESC$")) {
      this->pvData = QVariant (this->getDescription());
      this->precision = 0;
      this->dateTime = QCaDateTime::currentDateTime();

   } else {
      this->pvData = QEStringFormatting::fromString (name, this->precision, okay);
      this->dateTime = QCaDateTime::currentDateTime();
   }

   return okay;
}

//------------------------------------------------------------------------------
//
bool QELitClient::openChannel (const ChannelModesFlags)
{
   // We need to complete opening the channel before we emit
   // the connection and data update signals.
   //
   const bool status = this->readVariable ();
   if (status) {
      // We need to complete opening the channel before we emit
      // the connection and data update signals.
      //
      QTimer::singleShot (1, this, SLOT (doConnectionUpdated()));
      QTimer::singleShot (2, this, SLOT (doDataUpdated()));
   }

   return status;
}

//------------------------------------------------------------------------------
// slot
void QELitClient::doConnectionUpdated()
{
   if (this->allowSignalEmission) {
      emit this->connectionUpdated (true);
   }
}

//------------------------------------------------------------------------------
// slot
void QELitClient::doDataUpdated()
{
   if (this->allowSignalEmission) {
      emit this->dataUpdated (true);
   }
}

//------------------------------------------------------------------------------
//
void QELitClient::closeChannel ()  { }
QVariant QELitClient::getPvData () const { return this->pvData; }
bool QELitClient::putPvData (const QVariant&)  { return false; }
bool QELitClient::getIsConnected () const { return true; }
bool QELitClient::dataIsAvailable () const { return true; }

//------------------------------------------------------------------------------
//
QString QELitClient::getId () const
{
   QString dataType = this->pvData.typeName();
   if (dataType == "QString") dataType = "string";
   return QString ("Literal: %1").arg (dataType);
}

//------------------------------------------------------------------------------
//
QString QELitClient::getRemoteAddress() const { return "localhost"; }
QString QELitClient::getEgu () const { return ""; }
int QELitClient::getPrecision() const { return this->precision; }

//------------------------------------------------------------------------------
//
unsigned int QELitClient::hostElementCount () const
{
   int result = 1;

   QMetaType::Type mtype = QEPlatform::metaType (this->pvData);
   if (mtype == QMetaType::QStringList) {
      result = this->pvData.toStringList().count();
   } else if (QEVectorVariants::isVectorVariant (this->pvData)) {
      result = QEVectorVariants::vectorCount (this->pvData);
   }

   return result;
}

//------------------------------------------------------------------------------
//
unsigned int QELitClient::dataElementCount () const
{
   return this->hostElementCount();
}

//------------------------------------------------------------------------------
//
double QELitClient::getDisplayLimitHigh () const { return 0.0; }
double QELitClient::getDisplayLimitLow () const { return 0.0; }
double QELitClient::getHighAlarmLimit () const { return 0.0; }
double QELitClient::getLowAlarmLimit () const { return 0.0; }
double QELitClient::getHighWarningLimit () const { return 0.0; }
double QELitClient::getLowWarningLimit () const { return 0.0; }
double QELitClient::getControlLimitHigh () const { return 0.0; }
double QELitClient::getControlLimitLow () const { return 0.0; }
double QELitClient::getMinStep () const { return 0.0; }
QStringList QELitClient::getEnumerations() const { QStringList d; return d; }
QCaAlarmInfo QELitClient::getAlarmInfo () const { QCaAlarmInfo d; return d; }
QCaDateTime QELitClient::getTimeStamp () const { return this->dateTime; }
QString QELitClient::getDescription () const { return "Literal value"; }
bool QELitClient::getReadAccess() const { return true; }
bool QELitClient::getWriteAccess() const { return false; }

// end
