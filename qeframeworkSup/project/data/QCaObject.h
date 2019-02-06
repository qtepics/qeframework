/*  QCaObject.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2019 Australian Synchrotron
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
 *    Anthony Owen
 *  Contact details:
 *    anthony.owen@gmail.com
 */

#ifndef QCA_OBJECT_H
#define QCA_OBJECT_H

#include <QObject>
#include <QString>
#include <QFlags>

#include <UserMessage.h>
#include <Generic.h>
#include <QCaAlarmInfo.h>
#include <QCaDateTime.h>
#include <QCaConnectionInfo.h>
#include <QEFrameworkLibraryGlobal.h>

// differed, so we don't need to include headers
//
class QECaClient;
class QEPvaClient;

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

   QCaObject( const QString& recordName, QObject *eventObject,
              const unsigned int variableIndex,
              SignalsToSendFlags signalsToSend=SIG_VARIANT,
              priorities priorityIn=QE_PRIORITY_NORMAL );

   QCaObject( const QString& recordName, QObject *eventObject,
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
   SignalsToSendFlags getSignalsToSend () const;

   bool subscribe();        // open channel and subscribe
   bool singleShotRead();   // open channel and initiate a single read
   bool connectChannel();   // open channel only.
   void closeChannel();

   bool dataTypeKnown() const;

   // Setup parameter access function
   unsigned int getVariableIndex () const;

   // State machine access functions - use getChannelIsConnected
   Q_DECL_DEPRECATED
   bool isChannelConnected() const;

   void setUserMessage( UserMessage* userMessageIn );

   void enableWriteCallbacks( bool enable );
   bool isWriteCallbacksEnabled() const;

   void setRequestedElementCount( unsigned int elementCount );

   // Get database information relating to the variable
   QString getRecordName() const;
   QString getEgu() const;
   QStringList getEnumerations() const;
   unsigned int getPrecision() const;
   QCaAlarmInfo getAlarmInfo() const;
   QCaDateTime getDateTime () const;
   double getDisplayLimitUpper() const;
   double getDisplayLimitLower() const;
   double getAlarmLimitUpper() const;
   double getAlarmLimitLower() const;
   double getWarningLimitUpper() const;
   double getWarningLimitLower() const;
   double getControlLimitUpper() const;
   double getControlLimitLower() const;
   generic::generic_types getDataType() const;
   QString getHostName() const;
   QString getFieldType() const;
   unsigned long getElementCount() const;  // num elements available on server as oppsoed to num elements actually subscribed for.
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

   Q_DECL_DEPRECATED
   bool getIsLinkUp () const;

   // Conveniance functions for getLastData for scaler values and vector values.
   //
   bool getDataIsAvailable () const;
   QString getStringValue () const;   // note: raw string conversion, no units, precision or other QEString formatting
   long getIntegerValue () const;
   double getFloatingValue () const;
   QVector<long> getIntegerArray () const;
   QVector<double> getFloatingArray () const;

signals:
   void dataChanged( const QVariant& value, QCaAlarmInfo& alarmInfo, QCaDateTime& timeStamp, const unsigned int& variableIndex );
   void dataChanged( const QByteArray& value, unsigned long dataSize, QCaAlarmInfo& alarmInfo, QCaDateTime& timeStamp, const unsigned int& variableIndex );
   void connectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex );
   void connectionChanged( QCaConnectionInfo& connectionInfo );   // deprecated

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

   // Clear the connection state - and signal
   //
   void clearConnectionState();

   QString recordName;
   unsigned int variableIndex; // The variable index within a widget. If not used within a widget, can hold arbitary number.
   UserMessage* userMessage;
   SignalsToSendFlags signalsToSend;
   int arrayIndex;
   bool firstUpdate;

   QECaClient* caClient;     // CA Interface class.
   QEPvaClient* pvaClient;   // PVA Interface class.

   QVariant getVariant () const;
   QByteArray getByteArray () const;

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
