/*  QCaObject.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
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
 *  Copyright (c) 2009,2010,2016,2017  Australian Synchrotron
 *
 *  Author:
 *    Anthony Owen
 *  Contact details:
 *    anthony.owen@gmail.com
 */

#ifndef QCA_OBJECT_H
#define QCA_OBJECT_H

#include <QObject>
#include <QMutex>
#include <QList>
#include <QTimer>
#include <QString>
#include <QStringList>
#include <QVector>

#include <CaObject.h>
#include <QCaStateMachine.h>
#include <QCaEventFilter.h>
#include <QCaEventUpdate.h>
#include <UserMessage.h>
#include <QCaAlarmInfo.h>
#include <QCaDateTime.h>
#include <QCaConnectionInfo.h>
#include <QEPluginLibrary_global.h>

namespace qcaobject {

// bit significant
//
#define SIG_VARIANT   1
#define SIG_BYTEARRAY 2

  class QEPLUGINLIBRARYSHARED_EXPORT QCaObject : public QObject, caobject::CaObject {
      Q_OBJECT

    public:
      enum priorities{ QE_PRIORITY_LOW, QE_PRIORITY_NORMAL, QE_PRIORITY_HIGH };

      QCaObject( const QString& recordName, QObject *eventObject, const unsigned int variableIndex,
                 unsigned char signalsToSendIn=SIG_VARIANT, priorities priorityIn=QE_PRIORITY_NORMAL );

      QCaObject( const QString& recordName, QObject *eventObject, const unsigned int variableIndex,
                 UserMessage* userMessageIn, unsigned char signalsToSendIn=SIG_VARIANT, priorities priorityIn=QE_PRIORITY_NORMAL );

      virtual ~QCaObject();

      bool subscribe();
      bool singleShotRead();


      static void deletingEventStatic( QCaEventUpdate* dataUpdateEvent );
      static void processEventStatic( QCaEventUpdate* dataUpdateEvent );

      bool dataTypeKnown();

      // Setup parameter access function
      unsigned int getVariableIndex () const;

      // State machine access functions
      bool createChannel();
      void deleteChannel();
      bool createSubscription();
      bool getChannel();
      bool putChannel();
      bool isChannelConnected();
      void startConnectionTimer();
      void stopConnectionTimer();

      void setUserMessage( UserMessage* userMessageIn );

      void enableWriteCallbacks( bool enable );
      bool isWriteCallbacksEnabled();

      void setRequestedElementCount( unsigned int elementCount );

      // Get database information relating to the variable
      QString getRecordName();
      QString getEgu();
      QStringList getEnumerations();
      unsigned int getPrecision();
      QCaAlarmInfo getAlarmInfo();
      QCaDateTime getDateTime ();
      double getDisplayLimitUpper();
      double getDisplayLimitLower();
      double getAlarmLimitUpper();
      double getAlarmLimitLower();
      double getWarningLimitUpper();
      double getWarningLimitLower();
      double getControlLimitUpper();
      double getControlLimitLower();
      generic::generic_types getDataType();
      QString getHostName();
      QString getFieldType();
      unsigned long getElementCount();  // num elements available on server as oppsoed to num elements actually subscribed for.
      bool getReadAccess();
      bool getWriteAccess();

      // Set/get the array index use to extract scaler value form an array.
      // Default to 0, i.e. first element of the array.
      void setArrayIndex( const int index );
      int getArrayIndex() const;

      // Essentially provides same data as the dataChanged signal. The parameter isDefined indicates whether
      // the data is valid, i.e. has been received since the channel last connected.
      void getLastData( bool& isDefined, QVariant& value, QCaAlarmInfo& alarmInfo, QCaDateTime& timeStamp );

      // Get last connection info.
      //
      bool getChannelIsConnected () const;
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
      void connectionChanged( QCaConnectionInfo& connectionInfo );

    public slots:
      bool writeData( const QVariant& value );

      // This function can be used when data is an array variable.
      // It uses arrayIndex in order to update a particular element of the array.
      // Also works for scaler variables.
      bool writeDataElement( const QVariant& elementValue );

      void resendLastData();


    private:
      void initialise( const QString& newRecordName,
                       QObject *newEventHandler,
                       const unsigned int variableIndex,
                       UserMessage* userMessageIn,
                       unsigned char signalsToSendIn,
                       priorities priorityIn );

      unsigned int variableIndex; // The variable index within a widget. If not used within a widget, can hold arbitary number.
      long lastEventChannelState; // Channel state from most recent update event. This is actually of type caconnection::channel_states
      long lastEventLinkState;    // Link state from most recent update event. This is actually of type aconnection::link_states

      QString recordName;
      QVariant writingData;

      QObject* eventHandler;                  // Event handler
      static QMutex pendingEventsLock;        // Used to protect access to pendingEvents list
      static QCaEventFilter eventFilter;      // Event filter to filter in own events
      QList<QCaEventItem> pendingEvents;      // List of pending data events
      QCaEventItem* lastDataEvent;            // Outstanding data event
      QTimer setChannelTimer;

      void removeEventFromPendingList( QCaEventUpdate* dataUpdateEvent );     // Ensure there is no reference to an update event in the pending list
      bool removeNextEventFromPendingList( QCaEventUpdate* dataUpdateEvent ); // Remove the event from the pending list if it was the next expected event

      qcastatemachine::ConnectionQCaStateMachine *connectionMachine;
      qcastatemachine::SubscriptionQCaStateMachine *subscriptionMachine;
      qcastatemachine::ReadQCaStateMachine *readMachine;
      qcastatemachine::WriteQCaStateMachine *writeMachine;

      void signalCallback( caobject::callback_reasons reason );  // CA callback function processed within an EPICS thread
      void processEvent( QCaEventUpdate* dataUpdateEvent );      // Continue processing CA callback but within the contect of a Qt event
      void processData( void* newData );                         // Process new CA data. newData is actually of type carecord::CaRecord*

      UserMessage* userMessage;

      // Current data
      QByteArray   byteArrayValue;

      // Last connection info emited
      bool         lastIsChannelConnected;
      bool         lastIsLinkUp;

      // Last data emited
      QCaDateTime  lastTimeStamp;
      QCaAlarmInfo lastAlarmInfo;
      bool         lastValueIsDefined;
      QVariant     lastVariantValue;
      QByteArray   lastByteArrayValue;
      void*        lastNewData; // Record containing data directly refernced by lastByteArrayValue (actually of type carecord::CaRecord*)
      unsigned long lastDataSize;

      // Index to be used to extact scalar value fron an array.
      // (Only used by QEInteger/QEFloating so far)
      int arrayIndex;

      // Database information relating to the variable
      QString egu;
      int precision;

      double displayLimitUpper;
      double displayLimitLower;

      double alarmLimitUpper;
      double alarmLimitLower;

      double warningLimitUpper;
      double warningLimitLower;

      double controlLimitUpper;
      double controlLimitLower;

      QStringList enumerations;
      bool isStatField;

      unsigned char signalsToSend;
      priorities priority;

      bool channelExpiredMessage;

    private slots:
      void setChannelExpired();
  };

}      // end qcaobject namespace

#endif // QCA_OBJECT_H
