/*  qepicspv.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
 *
 *  Copyright (c) 2011-2018 Australian Synchrotron
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
 *    Anton Maksimenko
 *  Contact details:
 *    anton.maksimenko@synchrotron.org.au
 */

/*
 * Description:
 *
 * A class to manage simple non-event driven access to CA data.
 * Refer to qepicspv.h for details
 *
 */

#include "qepicspv.h"
#include "QCaObject.h"

#include <QTime>
#include <QTimer>
#include <QCoreApplication>
#include <QDebug>

using namespace qcaobject;

const QVariant QEpicsPV::badData = QVariant();

const bool QEpicsPV::inited = QEpicsPV::init();

unsigned QEpicsPV::debugLevel = 0;

bool QEpicsPV::init() {
  qRegisterMetaType<QCaConnectionInfo>("QCaConnectionInfo&");
  qRegisterMetaType<QCaAlarmInfo>("QCaAlarmInfo&");
  qRegisterMetaType<QCaDateTime>("QCaDateTime&");
  qRegisterMetaType<QVariant>("QVariant&");
  qRegisterMetaType<QVariant>("QVariant");
  return true;
}

void QEpicsPV::setDebugLevel(unsigned level){
  debugLevel = level;
}

QEpicsPV::QEpicsPV(const QString & _pvName, QObject *parent) :
    QObject(parent),
    qCaField(0),
    pvName(_pvName),
    lastData(),
    updated(false),
    theEnum()
{
  setPV(pvName);
}

QEpicsPV::QEpicsPV(QObject *parent) :
    QObject(parent),
    qCaField(0),
    pvName(),
    lastData(),
    updated(false),
    theEnum()
{
}


QEpicsPV::~QEpicsPV(){
  setPV();
}


void QEpicsPV::setPV(const QString & _pvName) {
  pvName = _pvName;
  if (qCaField) {
    delete (QCaObject *) qCaField;
    qCaField = 0;
  }
  updateConnection();

  if ( pvName.isEmpty() )
    return;

  QCaObject * _qCaField = new QCaObject(pvName, this, 0);   // Use arbitary variable index
  qCaField = _qCaField;

  // Qt::QueuedConnection here is needed to ensure the QEventLoop in
  // ::getUpdated() and ::getReady() methods are running smoothly.
  //
  // This requirement to use the queued signal and slot connections
  // caused the necessity to register the corresponding types using
  // qRegisterMetaType() functions.
  //
  connect(_qCaField, SIGNAL(connectionChanged(QCaConnectionInfo&, const unsigned int&)),
          SLOT(updateConnection()), Qt::QueuedConnection);
  connect(_qCaField, SIGNAL(dataChanged(QVariant,QCaAlarmInfo&,QCaDateTime&, const unsigned int&)),
          SLOT(updateValue(QVariant)), Qt::QueuedConnection);

  _qCaField->subscribe();

}

const QString & QEpicsPV::pv() const {
  return pvName;
}


bool QEpicsPV::isConnected() const {
  return qCaField && ((QCaObject *) qCaField) -> getChannelIsConnected();
}

const QVariant & QEpicsPV::get() const {
  return lastData;
}

void QEpicsPV::needUpdated() const {
  updated = false;
}

const QVariant & QEpicsPV::getUpdated(int delay) const {

  if ( ! isConnected() )
    return badData;
  if ( updated )
    return lastData;



  if (delay < 0) delay = 0;

  QEventLoop q;
  QTimer tT;
  tT.setSingleShot(true);

  connect(&tT, SIGNAL(timeout()), &q, SLOT(quit()));
  connect(this, SIGNAL(valueUpdated(QVariant)), &q, SLOT(quit()));
  connect(this, SIGNAL(connectionChanged(bool)), &q, SLOT(quit()));

  if (delay) tT.start(delay);
  q.exec();
  if(tT.isActive()) tT.stop();

  return updated ? lastData : badData ;

}



const QVariant & QEpicsPV::getReady(int delay) const {

  if ( ! qCaField )
    return badData;

  if ( ! isConnected() ) {

    QEventLoop q;
    QTimer tT;
    tT.setSingleShot(true);

    connect(&tT, SIGNAL(timeout()), &q, SLOT(quit()));
    connect(this, SIGNAL(connectionChanged(bool)), &q, SLOT(quit()));

    if (delay) tT.start(delay);
    q.exec();
    if (tT.isActive()) tT.stop();

    if ( ! isConnected() )
      return badData;

  }

  return lastData.isValid()  ?  get()  :  getUpdated(delay);

}




QVariant QEpicsPV::get(const QString & _pvName, int delay) {
  if (_pvName.isEmpty())
    return badData;
  QEpicsPV * tpv = new QEpicsPV(_pvName);
  QVariant ret = tpv->getReady(delay);
  delete tpv;
  return  ret;
}

const QVariant & QEpicsPV::set(QVariant value, int delay) {

  if ( debugLevel > 0 )
    qDebug() << "QEpicsPV DEBUG: SET" << this << isConnected() << pv() << get() << value << getEnum();

  if ( ! isConnected() || ! value.isValid() )
    return badData ;

  if (delay >= 0)
    needUpdated();

  if ( getEnum().size() ) {
    if ( ! getEnum().contains(value.toString()) ) {
      bool ok;
      qlonglong val = value.toLongLong(&ok);
      if (!ok) {
        qDebug() << "QEpicsPV. Error. Value" << value << "to set the PV" << pv()
            << "of the enum type could not be found in the list of possible values"
            << getEnum() << "and could not be converted into integer.";
        return badData;
      }
      if ( val >= getEnum().size() || val < 0 ) {
        qDebug() << "QEpicsPV. Error. Value" << value << "to set the PV" << pv()
            << "of the enum type, when converted into integer" << val
            << "is not a valid index in the list of possible values"
            << getEnum() << ".";
        return badData;
      }
      value = val;
    }
  } else if ( get().type() != value.type()  && ! value.convert(get().type()) ) {
    qDebug() << "QEpicsPV. Error. Could not convert type QVariant from" << value.typeName()
        << "to" << get().typeName() << "to set the PV" << pv();
    return badData;
  }

  ((QCaObject *) qCaField) -> writeData(value);

  return delay >= 0  ?  getUpdated(delay)  :  get();

}



QVariant QEpicsPV::set(QString & _pvName, const QVariant & value, int delay) {
  if (_pvName.isEmpty())
    return badData;
  QEpicsPV * tpv = new QEpicsPV(_pvName);
  QVariant ret = tpv->getReady().isValid()  ?  tpv->set(value, delay)  :  badData;
  delete tpv;
  return  ret;
}


void QEpicsPV::updateValue(const QVariant & data){

  if ( debugLevel > 0 )
    qDebug() << "QEpicsPV DEBUG: UPD" << this << isConnected() << pv() << get() << data << getEnum();

  updated = true;
  bool firstRead = ! lastData.isValid();
  bool changed = firstRead || (lastData != data);
  lastData = data;

  if (firstRead) {
    theEnum = ((QCaObject *) qCaField) -> getEnumerations();
    emit valueInited(lastData);
  }
  if (changed)
    emit valueChanged(lastData);
  emit valueUpdated(lastData);

}


void QEpicsPV::updateConnection() {
  if ( debugLevel > 0 )
    qDebug() << "QEpicsPV DEBUG: CON" << this << pv() << isConnected();
  if (isConnected()) {
    emit connected();
  } else {
    updated=false;
    lastData.clear();
    theEnum.clear();
    emit disconnected();
  }
  emit connectionChanged(isConnected());
}

const QStringList & QEpicsPV::getEnum() const {
  return theEnum;
}

// end
