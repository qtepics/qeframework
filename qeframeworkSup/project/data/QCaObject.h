/*  QCaObject.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2009-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Anthony Owen
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QCA_OBJECT_H
#define QCA_OBJECT_H

#include <QMetaType>
#include <QObject>
#include <QString>
#include <QFlags>
#include <QVariant>

#include <UserMessage.h>
#include <QEEnums.h>
#include <QCaAlarmInfo.h>
#include <QCaDateTime.h>
#include <QCaConnectionInfo.h>
#include <QEBaseClient.h>
#include <QEFrameworkLibraryGlobal.h>

// differed, so we don't need to include headers
//
class QECaClient;
class QEPvaClient;

// Structures used in signals to indicate connection and data updates.
//
struct QEConnectionUpdate {
   QCaConnectionInfo connectionInfo;
   unsigned int variableIndex;
};
Q_DECLARE_METATYPE (QEConnectionUpdate)

struct QEVariantUpdate {
   QVariant value;
   QCaAlarmInfo alarmInfo;
   QCaDateTime timeStamp;
   unsigned int variableIndex;
   bool isMetaUpdate;
};
Q_DECLARE_METATYPE (QEVariantUpdate)

struct QEByteArrayUpdate {
   QByteArray array;
   QCaAlarmInfo alarmInfo;
   QCaDateTime timeStamp;
   unsigned int dataElementSize;
   unsigned int variableIndex;
   bool isMetaUpdate;
};
Q_DECLARE_METATYPE (QEByteArrayUpdate)


// TODO: Consider renameing QCaObject to something more vanilla (e.g. QEClient)
// and dropping the name space and that not used anywhere else in the framework.
//
namespace qcaobject {

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QCaObject : public QObject {
   Q_OBJECT

public:
   // bit significant
   //
   enum SignalsToSend {
      SIG_NONE = 0x00,
      SIG_VARIANT = 0x01,
      SIG_BYTEARRAY = 0x02
   };
   Q_DECLARE_FLAGS (SignalsToSendFlags, SignalsToSend)

   enum priorities {
      QE_PRIORITY_LOW = 0,
      QE_PRIORITY_NORMAL = 10,
      QE_PRIORITY_HIGH = 20
   };

   // Referenced by VariableManager's getConnectedCountRef and getDisconnectedCountRef functions.
   //
   static int* getDisconnectedCountRef();
   static int* getConnectedCountRef();

   QCaObject( const QString& pvName, QObject *parent,
              const unsigned int variableIndex,
              SignalsToSendFlags signalsToSend=SIG_VARIANT,
              priorities priorityIn=QE_PRIORITY_NORMAL );

   QCaObject( const QString& pvName, QObject *parent,
              const unsigned int variableIndex,
              UserMessage* userMessageIn,
              SignalsToSendFlags signalsToSend=SIG_VARIANT,
              priorities priorityIn=QE_PRIORITY_NORMAL );

   virtual ~QCaObject();

   // Sometimes the widget needs to know the underlying channel kind.
   bool isCaChannel () const;
   bool isPvaChannel () const;

   // Allow dynamic modification of the signals to send.
   void setSignalsToSend (const SignalsToSendFlags signalsToSend);
   QCaObject::SignalsToSendFlags getSignalsToSend () const;

   bool subscribe();        // open channel and subscribe
   bool singleShotRead();   // open channel and initiate a single read
   bool connectChannel();   // open channel only.
   void closeChannel();

   bool dataTypeKnown() const;

   // Setup parameter access function
   unsigned int getVariableIndex () const;

   // isChannelConnected - use getChannelIsConnected

   void setUserMessage( UserMessage* userMessageIn );

   void enableWriteCallbacks( bool enable );
   bool isWriteCallbacksEnabled() const;

   void setRequestedElementCount( unsigned int elementCount );

   // Get database information relating to the variable   
   QString getPvName() const;

   QT_DEPRECATED_X("use getPvName instead")
   QString getRecordName() const;

   QString getEgu() const;
   QStringList getEnumerations() const;
   unsigned int getPrecision() const;
   QCaAlarmInfo getAlarmInfo() const;
   QCaDateTime getDateTime () const;
   QString getDescription () const;
   double getDisplayLimitUpper() const;
   double getDisplayLimitLower() const;
   double getAlarmLimitUpper() const;
   double getAlarmLimitLower() const;
   double getWarningLimitUpper() const;
   double getWarningLimitLower() const;
   double getControlLimitUpper() const;
   double getControlLimitLower() const;
   QString getHostName() const;
   QString getFieldType() const;
   unsigned long getHostElementCount() const;  // num elements available on server
   unsigned long getDataElementCount() const;  // num elements actually subscribed for.

   Q_DECL_DEPRECATED    // use getHostElementCount
   unsigned long getElementCount() const;

   bool getReadAccess() const;
   bool getWriteAccess() const;

   // Set/get the array index use to extract scaler value form an array.
   // Default to 0, i.e. first element of the array.
   void setArrayIndex( const int index );
   int getArrayIndex() const;

   // Essentially provides same data as the dataChanged signal. The parameter isDefined indicates whether
   // the data is valid, i.e. has been received since the channel last connected.
   void getLastData( bool& isDefined, QVariant& value, QCaAlarmInfo& alarmInfo, QCaDateTime& timeStamp ) const;

   // Get last connection info.
   //
   bool getChannelIsConnected () const;

   // Conveniance functions for getLastData for scaler values and vector values.
   // The scalar functions honour arrayIndex.
   //
   bool getDataIsAvailable () const;

   // Indicates if this is a meta data update; the first
   // post connection update is always a meta data update.
   //
   bool getIsMetaDataUpdate () const;

   // note: apart for array action, this provides a raw string conversion,
   // i.e. no units, precision or other QEString formatting
   //
   QString getStringValue (const QE::ArrayActions arrayAction = QE::Index) const;

   bool getBooleanValue () const;     // 0 values are false, non-zero are true, non-numeric are undefined.
   long getIntegerValue () const;
   double getFloatingValue () const;
   QVector<bool> getBooleanArray () const;
   QVector<long> getIntegerArray () const;
   QVector<double> getFloatingArray () const;

   // And the associate setter functions.
   // Essentially conveniance function for writeDataElement/writeData.
   // The scalar functions honour arrayIndex and use writeDataElement.
   //
   void writeStringValue (const QString& value);
   void writeBooleanValue (const bool value);
   void writeIntegerValue (const long value);
   void writeFloatingValue (const double value);
   void writeBooleanArray (const QVector<bool>& array);
   void writeIntegerArray (const QVector<long>& array);
   void writeFloatingArray (const QVector<double>& array);

   // Identity type. The type should be considered private.
   //
   typedef quint64 ObjectIdentity;

   static ObjectIdentity nullObjectIdentity ();    // provides the null identifier value
   ObjectIdentity getObjectIdentity () const;

signals:
   void connectionUpdated (const QEConnectionUpdate&);
   void valueUpdated (const QEVariantUpdate&);
   void byteArrayUpdated (const QEByteArrayUpdate&);

   // Deprecated - use above.
   //
   void dataChanged( const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& );
   void byteArrayChanged( const QByteArray&, unsigned long, QCaAlarmInfo&, QCaDateTime&, const unsigned int& );
   void connectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex );

public slots:
   bool writeData( const QVariant& value );

   // This function can be used when data is an array variable.
   // It uses arrayIndex in order to update a particular element of the array.
   // Also works for scaler variables.
   bool writeDataElement( const QVariant& elementValue );

   void resendLastData();

private:
   // start of private
   void initialise( const QString& newRecordName,
                    const unsigned int variableIndex,
                    UserMessage* userMessage,
                    SignalsToSendFlags signalsToSend,
                    priorities priority );

   // qobject cast to required types or return null_ptr
   //
   QECaClient* asCaClient () const;
   QEPvaClient* asPvaClient () const;

   // Clear the connection state - and signal
   //
   void clearConnectionState();

   QString processVariableName;
   unsigned int variableIndex; // The variable index within a widget. If not used within a widget, can hold arbitary number.
   UserMessage* userMessage;
   SignalsToSendFlags signalsToSend;
   int arrayIndex;
   bool isFirstMetaUpdate;

   // This can be one of QECaClient, QEPvaClient or QENullClient.
   //
   QEBaseClient* client;

   QVariant getVariant () const;
   QByteArray getByteArray () const;
   unsigned getDataElementSize () const;

   quint64 objectIdentity;   // this object's identity
   static ObjectIdentity nextObjectIdentity;

   // Thus method checks is writes have not been disabled by means of
   // the of read-only adaptation parameter.
   //
   bool writeEnabled() const;
   
   static int disconnectedCount;
   static int connectedCount;
   static int totalChannelCount;

private slots:
   void connectionUpdate (const bool isConnected);
   void dataUpdate (const bool firstUpdate);
   void putCallbackNotifcation (const bool isSuccessful);
};

}    // end qcaobject namespace

Q_DECLARE_OPERATORS_FOR_FLAGS (qcaobject::QCaObject::SignalsToSendFlags)

#endif // QCA_OBJECT_H
