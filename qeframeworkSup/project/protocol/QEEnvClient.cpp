/*  QEEnvClient.cpp
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

#include "QEEnvClient.h"
#include <QDebug>
#include <QTimer>
#include <QEPlatform.h>
#include <QEStringFormatting.h>
#include <QEVectorVariants.h>
#include <stdlib.h>

#define DEBUG qDebug () << "QEEnvClient" << __LINE__ << __FUNCTION__ << "  "

//==============================================================================
// QEEnvClient
//==============================================================================
//
QEEnvClient::QEEnvClient (const QString& pvName,
                          QObject* parent) :
   QEBaseClient (QEBaseClient::EnvType, pvName, parent)
{
   this->envVarExists = false;   // channel closed.

   // Allow this instance to be notified if another QEEnvClient instance
   // updates the environment variable.
   //
   QEEnvValueDispatcher* relay = QEEnvValueDispatcher::initialise (this);
   QObject::connect (relay, SIGNAL (environmentVariableUpdate (const QString&)),
                     this,  SLOT   (updateEnvironmentVariable (const QString&)));
}

//------------------------------------------------------------------------------
//
QEEnvClient::~QEEnvClient ()
{
   this->allowSignalEmission = false;
   this->envVarExists = false;
}

//------------------------------------------------------------------------------
//
bool QEEnvClient::readVariable ()
{
   const QString name = this->getPvName();

   bool okay = true;   // hypothesize all ok

   // Provide some pseudo fields.
   //
   if (name.endsWith(".RTYP") || name.endsWith(".RTYP$")) {
      this->pvData = QVariant ("qeenv");
      this->dateTime = QCaDateTime::currentDateTime();

   } else if (name.endsWith(".DESC") || name.endsWith(".DESC$")) {
      this->pvData = QVariant (this->getDescription());
      this->dateTime = QCaDateTime::currentDateTime();

   } else {
      char* value = secure_getenv (name.toStdString().c_str());
      if (value) {
         this->pvData = QEStringFormatting::fromString (QString (value), this->precision, okay);
         this->envVarExists = true;
         this->dateTime = QCaDateTime::currentDateTime();
      } else {
         okay = false;
      }
   }

   return okay;
}

//------------------------------------------------------------------------------
//
void QEEnvClient::doRefresh()
{
   QVariant prev = this->pvData;
   this->envVarExists = this->readVariable ();
   if (this->envVarExists) {
      if (this->pvData != prev) {
         QTimer::singleShot (1, this, SLOT (doDataUpdated()));
      }
   }
}

//------------------------------------------------------------------------------
// slot
void QEEnvClient::updateEnvironmentVariable (const QString& pvName)
{
   if (this->envVarExists && (pvName == this->getPvName())) {
      this->doRefresh();
   }
}

//------------------------------------------------------------------------------
//
bool QEEnvClient::openChannel (const ChannelModesFlags)
{
   this->envVarExists = this->readVariable ();  // also sets envVarExists
   if (this->envVarExists) {
      // We need to complete opening the channel before we emit
      // the connection and data update signals.
      //
      QTimer::singleShot (1, this, SLOT (doConnectionUpdated()));
      QTimer::singleShot (2, this, SLOT (doDataUpdated()));
   }

   return this->envVarExists;
}

//------------------------------------------------------------------------------
// slot
void QEEnvClient::doConnectionUpdated()
{
   if (this->allowSignalEmission) {
      emit this->connectionUpdated (true);
   }
}

//------------------------------------------------------------------------------
// slot
void QEEnvClient::doDataUpdated()
{
   if (this->allowSignalEmission) {
      emit this->dataUpdated (true);
   }
}

//------------------------------------------------------------------------------
//
void QEEnvClient::closeChannel ()
{
   this->envVarExists = false;
}

//------------------------------------------------------------------------------
//
QVariant QEEnvClient::getPvData () const
{
   return this->pvData;
}

//------------------------------------------------------------------------------
//
bool QEEnvClient::putPvData (const QVariant& newData)
{
   QString name = this->getPvName();
   QString assign = QString("%1=%2").arg(name).arg(newData.toString());
   char* copy = strdup (assign.toStdString().c_str());
   int status = putenv (copy);
   /// we do not free copy;

   emit environmentVariableUpdate (this->getPvName());
   return status == 0;
}

//------------------------------------------------------------------------------
//
bool QEEnvClient::getIsConnected () const { return this->envVarExists; }
bool QEEnvClient::dataIsAvailable () const { return this->envVarExists; }

//------------------------------------------------------------------------------
//
QString QEEnvClient::getId () const
{
   QString dataType = this->pvData.typeName();
   if (dataType == "QString") dataType = "string";
   if (dataType == "QStringList") dataType = "string array";
   if (dataType == "QList<int>") dataType = "int array";
   if (dataType == "QList<double>") dataType = "double array";
   return QString ("Environment: %1").arg (dataType);
}

//------------------------------------------------------------------------------
//
QString QEEnvClient::getRemoteAddress() const { return "localhost"; }
QString QEEnvClient::getEgu () const { return ""; }
int QEEnvClient::getPrecision() const { return this->precision; }

//------------------------------------------------------------------------------
//
unsigned int QEEnvClient::hostElementCount () const
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
unsigned int QEEnvClient::dataElementCount () const
{
   return this->hostElementCount();
}

//------------------------------------------------------------------------------
//
double QEEnvClient::getDisplayLimitHigh () const { return 0.0; }
double QEEnvClient::getDisplayLimitLow () const { return 0.0; }
double QEEnvClient::getHighAlarmLimit () const { return 0.0; }
double QEEnvClient::getLowAlarmLimit () const { return 0.0; }
double QEEnvClient::getHighWarningLimit () const { return 0.0; }
double QEEnvClient::getLowWarningLimit () const { return 0.0; }
double QEEnvClient::getControlLimitHigh () const { return 0.0; }
double QEEnvClient::getControlLimitLow () const { return 0.0; }
double QEEnvClient::getMinStep () const { return 0.0; }
QStringList QEEnvClient::getEnumerations() const { QStringList d; return d; }
QCaAlarmInfo QEEnvClient::getAlarmInfo () const { QCaAlarmInfo d; return d; }
QCaDateTime QEEnvClient::getTimeStamp () const { return this->dateTime; }
QString QEEnvClient::getDescription () const { return "Environment variable value"; }
bool QEEnvClient::getReadAccess() const { return true; }
bool QEEnvClient::getWriteAccess() const { return true; }


//==============================================================================
// QEEnvValueDispatcher
//==============================================================================
//
QEEnvValueDispatcher::QEEnvValueDispatcher () {}
QEEnvValueDispatcher::~QEEnvValueDispatcher () {}

//------------------------------------------------------------------------------
// static
QEEnvValueDispatcher* QEEnvValueDispatcher::initialise (QEEnvClient* client)
{
   // The singleton object created and seup when first needed, and not before.
   // Just declaring as a regular static QObject doesn't work as that would
   // require it to initialise before the construction of the QApplication.
   //
   static QEEnvValueDispatcher singleton;

   QObject::connect (client,   SIGNAL(environmentVariableUpdate (const QString&)),
                     &singleton, SLOT(updateEnvironmentVariable (const QString&)));

   return &singleton;
}

//------------------------------------------------------------------------------
// slots
void QEEnvValueDispatcher::updateEnvironmentVariable (const QString& pvName)
{
   emit this->environmentVariableUpdate (pvName);
}

// end
