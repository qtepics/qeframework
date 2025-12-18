/*  qepicspv.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2011-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Anton Maksimenko
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/*
 * Description:
 *
 * A class to manage simple non-event driven access to CA data.
 *
 * A wrapper around the QCaObject designed to hide EPICS-related features
 * unimportant and unclear for the end-user. Also implements some frequently
 * used operations in a single methods.
 *
 */

#ifndef QEPICSPV_H
#define QEPICSPV_H

#include <QtCore/qglobal.h>
#include <QObject>
#include <QVariant>
#include <QStringList>
#include <QEFrameworkLibraryGlobal.h>


class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEpicsPV : public QObject {
  Q_OBJECT

private:

  // Basic object representing the field. Is always of type QCaObject*,
  // here is void* to avoid installing all headers in the src directory.
  void* qCaField;

  // The PV name of the field
  QString pvName;

  // Last value of the field picked by the ::updateValue() slot.
  // Is invalid if the pv is not connected or before first update.
  QVariant lastData;

  // Turns true whenever the value is updated in the ::updateValue() slot.
  // Used in the ::needUpdated() / ::getUpdated() pair.
  mutable bool updated;

  // The enumeration of the PV field (if any, empty list otherwise).
  QStringList theEnum;

  // Invalid QVariant object to be returned by ::get(), ::getUpdated(),
  // ::set(), etc.
  static const QVariant badData;

  // Default delay in milliseconds used in the ::getUpdated(), ::getReady() and get() methods.
  static const int defaultDelay = 1000;

  // \brief Initializes the environment. Automatically called on program start.
  //
  // Registers the QCaConnectionInfo, QCaAlarmInfo, QCaDateTime. Needed to use
  // the types in queued signal and slot connections in ::setPV().
  //
  // @return always true.
  //
  static bool init();

  // Used to automatically run ::init() before any instance of the object is created.
  static const bool inited;

  // debug level. 0 - no debug;
  static unsigned debugLevel;


public:

  // Constructor.
  // @param _pvName new PV name.
  // @param parent Object's parent.
  QEpicsPV(const QString & _pvName, QObject *parent = 0);

  // Constructor.
  // @param parent Object's parent.
  QEpicsPV(QObject *parent = 0);

  // Destructor
  ~QEpicsPV();

  static void setDebugLevel(unsigned level = 0);

  // Returns current value of the field.
  //
  // @return Latest value of the PV field or invalid object
  // if the field is not connected or before first update.
  //
  const QVariant & get() const;

  // \brief Starts monitoring for the updates.
  //
  // Makes sence only in combination with further call to ::getUpdated().
  // Ensures that the next call to ::getUpdated() method will return only after
  // a new value of the PV is received.
  //
  void needUpdated() const;

  // \brief Returns an updated value of the field.
  //
  // Same as ::get(), but ensures that the PV's value was updated since the last call to
  // ::needUpdated() method or establishing the connection.
  //
  // @param delay Maximum waiting time in milliseconds. If zero, then unlimited waiting.
  //
  // @return Updated value of the field. If the value was not updated during
  //  the specified delay then returns an invalid object.
  //
  // WARNING: BUG
  // There is a bug which may cause the main application to crash
  // if this static member is called (directly or inderectly)
  // from whithin a constructor.
  // It has something to do with the threading.
  const QVariant & getUpdated(int delay=defaultDelay) const;

  // Connection status.
  // @return true if connected, false otherwise.
  bool isConnected() const;

  // PV's enumeration.
  // @return The enumeration of the PV field, or the empty list if the PV is not an enumeration.
  const QStringList & getEnum() const;

  // PV name.
  // @return PV name.
  const QString & pv() const ;


  // \brief Ensures that the object is ready to operate.
  //
  // Connection to the EPICS PV and first update takes certain time after the ::setPV() method
  // or the constructor with name are called. This method waits for the PV to get connected and
  // the value to be initialized before returning.
  //
  // @param delay Maximum waiting time in milliseconds. If zero, then unlimited waiting.
  // Note that the actual waiting time can be twice as large as the delay: one delay to wait for the
  // connection and one more to wait for the first update.
  //
  // @return Field's updated value. If the connection could not be established or the value was
  // not updated then returns an invalid data.
  //
  // WARNING: BUG
  // There is a bug which may cause the main application to crash
  // if this static member is called (directly or inderectly)
  // from whithin a constructor.
  // It has something to do with the threading.
  const QVariant & getReady(int delay=defaultDelay) const;

  // \brief Static version of the ::getUpdated() method.
  //
  // Constructs new object of the QEpicsPV type, gets it ready and returns the value.
  // Useful for the single shot readouts.
  //
  // @param _pvName PV name of the field.
  // @param delay Maximum waiting time in milliseconds. If zero, then unlimited waiting.
  //
  // @return Value of the field. If the value was not updated during
  // the specified delay then returns an invalid object.
  //
  // WARNING: BUG
  // There is a bug which may cause the main application to crash
  // if this static member is called (directly or inderectly)
  // from whithin a constructor.
  // It has something to do with the threading.
  static QVariant get(const QString & _pvName, int delay=defaultDelay);

  // \brief Static version of the ::set() method.
  //
  // Constructs new object of the QEpicsPV type, gets it ready, sets the new value
  // and returns the result of the ::set() method. Useful for the single shot write.
  //
  // @param _pvName PV name of the field.
  // @param value Value to be set.
  // @param delay Negative delay tells that the confirmation of the update is
  // nor required (just puts the new value and returns).
  // If non-negative, then sets the maximum time in milliseconds to wait for the update event before return.
  // Zero delay sets unlimited waiting time. Note that the method does not confirm the updated value
  // to be equal to the one set.
  //
  // @return Updated value or invalid object if could not confirm the update within
  // the specified delay time.
  //
  //
  // WARNING: BUG
  // There is a bug which may cause the main application to crash
  // if this static member is called (directly or inderectly)
  // from whithin a constructor.
  // It has something to do with the threading.
  static QVariant set(QString & _pvName, const QVariant & value, int delay = -1);

public slots:

  // \brief Sets new value for the field.
  //
  // @param value Value to be set.
  // @param delay Negative delay tells that the confirmation of the update is
  // nor required (just puts the new value and returns).
  // If non-negative, then sets the maximum time in milliseconds to wait for the update event before return.
  // Zero delay sets unlimited waiting time. Note that the method does not confirm the updated value
  // to be equal to the one set.
  //
  // @return Last updated value or invalid object if could not confirm the update within
  // the specified delay time.
  //
  const QVariant & set(QVariant value, int delay = -1);

  // \brief Sets new PV.
  //
  // First disconnects any previous PV (if any was connected) and
  // connects to the new one (if non-empty).
  //
  // @param _pvName New PV name.
  //
  void setPV(const QString & _pvName="");

private slots:

  // Used to catch the QCaObject::dataChanged() signal from the ::qCaField.
  // @param data new data.
  void updateValue(const QVariant & data);

  // Used to catch QCaObject::connectionChanged() signal from the ::qCaField.
  void updateConnection();

signals:

  // Emitted whenever the connection status has changed.
  // @param connected new connection status.
  void connectionChanged(bool connected);

  // Emitted on successful connection.
  void connected();

  // Emitted on disconnection, destruction and in ::setPV() method.
  void disconnected();

  // Emitted whenever the field is updated with a value different from the old one.
  // @param value new value.
  void valueChanged(const QVariant & value);

  // Emitted whenever the field is updated (even if the new value is equal to the old one).
  // @param value new value.
  void valueUpdated(const QVariant & value);

  // Emitted on first value update after the connection was established.
  void valueInited(const QVariant & value);

};

#endif // QEPICSPV_H
