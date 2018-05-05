/*  QCaObject.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2018 Australian Synchrotron
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

#include <QtCore>
#include <QtDebug>
#include <QByteArray>

#include <QECommon.h>
#include <CaObjectPrivate.h>
#include <CaObject.h>
#include <QCaObject.h>
#include <QCaEventUpdate.h>
#include <CaRecord.h>
#include <CaConnection.h>

#define DEBUG  qDebug () << "QCaObject" << __LINE__ << __FUNCTION__ << "  "

using namespace qcaobject;
using namespace generic;
using namespace caconnection;

// Used to protect access to outstandingEvents list
QMutex QCaObject::pendingEventsLock;

// An event filter for processing data updates in a Qt thread.
QCaEventFilter QCaObject::eventFilter;

/*
   The event object can be any Qt object with an event queue.
   A filter will be inserted (and removed) by this class to catch
   events from this class and pass them back to this class for processing.
   In other words, the event object does not need to be set up in any way.
   It just need to have a suitable event loop running.
*/
QCaObject::QCaObject( const QString& newRecordName,
                      QObject *newEventHandler,
                      const unsigned int variableIndexIn,
                      const SignalsToSend signalsToSendIn,
                      priorities priorityIn )
{
   initialise( newRecordName, newEventHandler, variableIndexIn, NULL,
               signalsToSendIn, priorityIn );
}

QCaObject::QCaObject( const QString& newRecordName,
                      QObject *newEventHandler,
                      const unsigned int variableIndexIn,
                      UserMessage* userMessageIn,
                      const SignalsToSend signalsToSendIn,
                      priorities priorityIn )
{
   initialise( newRecordName, newEventHandler, variableIndexIn, userMessageIn,
               signalsToSendIn, priorityIn );
}

void QCaObject::initialise( const QString& newRecordName,
                            QObject *newEventHandler,
                            const unsigned int variableIndexIn,
                            UserMessage* userMessageIn,
                            const SignalsToSend signalsToSendIn,
                            priorities priorityIn )
{

   // Initialise variables
   arrayIndex = 0;
   precision = 0;

   displayLimitUpper = 0;
   displayLimitLower = 0;

   alarmLimitUpper = 0;
   alarmLimitLower = 0;

   warningLimitUpper = 0;
   warningLimitLower = 0;

   controlLimitUpper = 0;
   controlLimitLower = 0;

   isStatField = false;

   lastIsChannelConnected = false;
   lastIsLinkUp = false;

   lastTimeStamp = QCaDateTime( QDateTime::currentDateTime() );
   lastVariantValue = (double)0.0;
   lastValueIsDefined = false;
   lastDataSize = 0;

   lastNewData = NULL;

   signalsToSend = signalsToSendIn;
   priority = priorityIn;

   // Setup any the mechanism to handle messages to the user, if supplied
   setUserMessage( userMessageIn );

   // Note the record required name and associated index.
   recordName = newRecordName;
   variableIndex = variableIndexIn;

   // Note if the field the value represents is a STAT field.
   // This is important when formatting strings as only a maximum of 16 enumerated strings
   // are available from the database yet a STAT field has 22 defined states.
   QString statName( ".STAT" );
   if( recordName.right( statName.length() ) == statName )
      isStatField = true;

   // Set initial states of the connection and link as reported by the event system.
   lastEventChannelState = caconnection::NEVER_CONNECTED;
   lastEventLinkState = caconnection::LINK_DOWN;

   // This object will post events to itself to transfer processing to a Qt aware thread via this filter.
   eventHandler = newEventHandler;
   eventFilter.addFilter( eventHandler );

   connectionMachine = new qcastatemachine::ConnectionQCaStateMachine( this );
   subscriptionMachine = new qcastatemachine::SubscriptionQCaStateMachine( this );
   readMachine = new qcastatemachine::ReadQCaStateMachine( this );
   writeMachine = new qcastatemachine::WriteQCaStateMachine( this );

   // Set a timer to retry if no connection
   channelExpiredMessage = false;
   QObject::connect( &setChannelTimer, SIGNAL( timeout() ), this, SLOT( setChannelExpired() ) );
   setChannelTimer.stop();

   // Start/request connecting state
   connectionMachine->process( qcastatemachine::CONNECTED );

   // Add the record name to the drag text
   QStringList dragText = eventHandler->property( "dragText" ).toStringList();
   dragText.append( recordName );
   eventHandler->setProperty( "dragText", dragText );
}

/*
    Destructor. Remove the channel and ensure that any unprocessed events posted by this obect will be ignored when
    they pop out of the event queue. Also, remove the event filter if this is the last QCaObject.
*/
QCaObject::~QCaObject() {

   // Send disconnected signal to monitoring widgets.
   //
   QCaConnectionInfo connectionInfo ( caconnection::CLOSED, caconnection::LINK_DOWN, getRecordName() );

   // Save last connetion info.
   lastIsChannelConnected = connectionInfo.isChannelConnected ();
   lastIsLinkUp = connectionInfo.isLinkUp ();

   emit connectionChanged( connectionInfo, variableIndex );
   emit connectionChanged( connectionInfo );

   // Remove our PV from the drag text.
   QStringList dragText = eventHandler->property( "dragText" ).toStringList();
   for( int i = 0; i < dragText.size(); i++ )
   {
      if( !dragText[i].compare( recordName ) )
      {
         dragText.removeAt( i );
         eventHandler->setProperty( "dragText", dragText );
         break;
      }
   }

   // Prevent channel access callbacks.
   // There should be no more callbacks from CaObject after this call returns. (This does not appear to always be the case)
   // Without this, a callback could occur while the outstanding events list contents
   // is being marked as 'to be ignored' (below). While access to the list is thread safe, this would
   // result in an active event in the event queue which would be cause this QCaObject to be accessed
   // after deletion.
   caPrivate->removeChannel();

   // Prevent callbacks to this class from the base CaObject class through the CaObject::signalCallback() virtual function.
   // Callbacks should not occur after removeChannel is called (above) but CA callbacks sometimes do.
   // Although there is a mechanism to catch these late CA callbacks to the CaObject class this mechanism is only initiated
   // in the CAObject destructor. This leaves a window after this QCaObject destrucor has run but before the the base class
   // CaObject destructor has run.
   // Ensuring the CaObject will not deliver any more callbacks to this class (through the signalCallback() virtual function) will
   // protect this window.
   inhibitCallbacks();

   // Protect access to pending events list
   QMutexLocker locker( &pendingEventsLock );

   // Ensure processing of outstanding events will not access this QCaObject after deletion.
   // If an event has been posted by a QCaObject, and the QCaObject is deleted before
   // the event is processed, the event will still be processed if the event filter is
   // still in place - and the filter will still be there if any other QCaObjects are using
   // the same QObject to process events. In this case the event will reference a QCaObject
   // which no longer exists.
   // To manage this problem outstanding events are marked 'to be ignored'.
   int pendingEventsSize = pendingEvents.size();
   for( int i = 0; i < pendingEventsSize; i++ ) {
      pendingEvents[i].event->acceptThisEvent = false;
      pendingEvents[i].event->emitterObject = NULL;   // Ensure a 'nice' crash if referenced in error
   }
   pendingEvents.clear();

   // Remove the event filter for this QCaObject.
   // Note, this only removes the filter if this is the last QCaObject to use the event loop for 'eventObject'.
   // If this is not the last QCaObject to use the event loop for 'eventObject' any remaining events will still
   // be processed as the event filter remains to handle events for other QCaOjects. The outstanding events are,
   // however, now safe.
   eventFilter.deleteFilter( eventHandler );

   // Release any 'last data'
   if( lastNewData )
      delete (carecord::CaRecord*)lastNewData;

   // Release state machines
   delete connectionMachine;
   delete subscriptionMachine;
   delete readMachine;
   delete writeMachine;
}

/*
    Clear channel connection state - and signal "initial" change of state
    Note: was original in setChannelExpired
*/
void QCaObject::clearConnectionState()
{
   // setChannelExpired
   // Signal a connection change.
   // (This is done with some licence. There isn't really a connection change.
   //  The connection has gone from 'no connection' to 'not connectet yet')
   QCaConnectionInfo connectionInfo( caconnection::NEVER_CONNECTED, caconnection::LINK_DOWN, getRecordName() );

   // Save last connetion info.
   lastIsChannelConnected = connectionInfo.isChannelConnected ();
   lastIsLinkUp = connectionInfo.isLinkUp();

   emit connectionChanged( connectionInfo, variableIndex );
   emit connectionChanged( connectionInfo );
}

/*
    Subcribe
*/
bool QCaObject::subscribe()
{
   bool result;
   clearConnectionState();
   result = subscriptionMachine->process( qcastatemachine::SUBSCRIBED );
   return result;
}

/*
    Initiate a single shot read
*/
bool QCaObject::singleShotRead()
{
   bool result;
   clearConnectionState();
   result = readMachine->process( qcastatemachine::READING );
   return result;
}

/*
   An update event is being deleted. It will be deleted by the event queueing system and may happen after
   it has been processed, or before it is delivered if the object that queued it is being deleted.
   Note, this is a static method since it is possible for a QCaObject to clear its list of pending events - effectively
   abandoning them in the event queue - and then delete itself. If it has done this 'acceptThisEvent' will be false and
   the emitterObject reference will be cleared. So if 'acceptThisEvent' is false, don't expect emitterObject (the QCaObject)
   that posted this event to still exist.
*/
void QCaObject::deletingEventStatic( QCaEventUpdate* dataUpdateEvent )
{
   // Protect access to the pending events list
   QMutexLocker locker( &pendingEventsLock );

   // If the originating object still exists, remove the event from its list of pending events
   if( dataUpdateEvent->acceptThisEvent == true )
   {
      dataUpdateEvent->emitterObject->removeEventFromPendingList( dataUpdateEvent );
   }
}


/*
   Process self generated events and only accept them if the
   originating QCaObject still exists.
   Note, this is a static method
*/
void QCaObject::processEventStatic( QCaEventUpdate* dataUpdateEvent )
{
   bool eventValid;
   eventValid = false;

   { // Limit scope of pending event list lock

      // Protect access to the pending events list
      QMutexLocker locker( &pendingEventsLock );

      // If the originating object still exists, remove the event from its list of pending events
      if( dataUpdateEvent->acceptThisEvent == true )
      {
         // Remove the event from the list of pending events.
         eventValid = dataUpdateEvent->emitterObject->removeNextEventFromPendingList( dataUpdateEvent );
      }

   } // Pending event list unlocked here

   // If the originating object still exists, process the event
   if( eventValid && dataUpdateEvent->acceptThisEvent == true )
   {
      dataUpdateEvent->emitterObject->processEvent( dataUpdateEvent );
   }
}

/*
   Remove an event from the pending event list.
   This QCaObject can't afford to hold a reference to a data update event after it has been deleted.
   The list must be locked (using pendingEventsLock) prior to calling this method
*/
void QCaObject::removeEventFromPendingList( QCaEventUpdate* dataUpdateEvent )
{
   int pendingEventsSize = pendingEvents.size();
   for( int i = 0; i < pendingEventsSize; i++ )
   {
      if( pendingEvents[i].event == dataUpdateEvent)
      {
         // Ensure the event will no longer be used.
         // This is belt and braces since this function is used when the event is being deleted, so it should never be processed.
         dataUpdateEvent->acceptThisEvent = false;
         dataUpdateEvent->emitterObject = NULL;   // Ensure a 'nice' crash if referenced in error
         pendingEvents.removeAt(i);
         break;
      }
   }

}

/*
   Remove the next expected event from the pending event list.
   If the event can't be found log an error and return false indicating 'the event is suspect - don't use it'.
   The list must be locked (using pendingEventsLock) prior to calling this method
*/
bool QCaObject::removeNextEventFromPendingList( QCaEventUpdate* dataUpdateEvent )
{
   // If list is empty, something is wrong - report it
   if( pendingEvents.isEmpty() )
   {
      QString msg( recordName );
      if( userMessage )
      {
         QString msg( recordName );
         userMessage->sendMessage( msg.append( " Outstanding events list is empty. It should contain at least one event"),
                                   "QCaObject::removeNextEventFromPendingList()",
                                   message_types ( MESSAGE_TYPE_ERROR ) );
      }
      return false;
   }

   // If the first item in the list is not the current event, something is wrong - report it
   if( pendingEvents[0].event != dataUpdateEvent )
   {
      QString msg( recordName );
      if( userMessage )
      {
         QString msg( recordName );
         userMessage->sendMessage( msg.append( " Outstanding events list is corrupt. The first event is not the event being processed" ),
                                   "QCaObject::removeNextEventFromPendingList()",
                                   message_types ( MESSAGE_TYPE_ERROR ) );
      }
      return false;
   }

   // Remove this event from the list
   pendingEvents.removeFirst();
   return true;
}

/*
    Returns true if the type of data being read or to be written is known.
    The data type will be unknown until a connection is established
*/
bool QCaObject::dataTypeKnown() {
   return( ( getType() != generic::GENERIC_UNKNOWN ) ? true:false );
}

/*
    Returns the assigned variable index.
 */
unsigned int QCaObject::getVariableIndex () const
{
   return variableIndex;
}

/*
    Create a channel
*/
bool QCaObject::createChannel() {

   // Select the CA priority appropriate
   caconnection::priorities caPriority;
   switch( priority )
   {
      case QE_PRIORITY_LOW:    caPriority = caconnection::PRIORITY_LOW;     break;
      case QE_PRIORITY_NORMAL: caPriority = caconnection::PRIORITY_DEFAULT; break;
      case QE_PRIORITY_HIGH:   caPriority = caconnection::PRIORITY_HIGH;    break;
      default:                 caPriority = caconnection::PRIORITY_DEFAULT; break;
   };

   // Try to create the channel
   caconnection::ca_responses response = caPrivate->setChannel( recordName.toStdString(), caPriority );
   if( response == caconnection::REQUEST_SUCCESSFUL )
   {
      return true;
   }
   else if( userMessage )
   {
      QString msg( recordName );
      msg.append( " Create channel failed." );
      switch( response )
      {
         case CHANNEL_DISCONNECTED: msg.append( " Channel disconnected."); break;
         case REQUEST_FAILED:       msg.append( " Request failed.");       break;
         default:                   msg.append( " Unknown error");         break;
      }
      userMessage->sendMessage( msg, "QCaObject::createChannel()", message_types ( MESSAGE_TYPE_ERROR ) );
      return false;
   }
   return false;
}

/*
    Delete a channel
*/
void QCaObject::deleteChannel() {

   // Delete the channel
   caPrivate->removeChannel();
}

/*
    Create a subscription
*/
bool QCaObject::createSubscription() {

   // Try to start a subscription
   caconnection::ca_responses response = caPrivate->startSubscription();
   if( response == caconnection::REQUEST_SUCCESSFUL )
   {
      return true;
   }
   else if( userMessage )
   {
      QString msg( recordName );
      msg.append( " Create subscription failed." );
      switch( response )
      {
         case CHANNEL_DISCONNECTED: msg.append( " Channel disconnected."); break;
         case REQUEST_FAILED:       msg.append( " Request failed.");       break;
         default:                   msg.append( " Unknown error");         break;
      }
      userMessage->sendMessage( msg, "QCaObject::createSubscription()", message_types ( MESSAGE_TYPE_ERROR ) );
      return false;
   }
   return false;
}

/*
    Read from a PV
*/
bool QCaObject::getChannel() {

   // ???
   caconnection::ca_responses response = caPrivate->readChannel();
   if( response == caconnection::REQUEST_SUCCESSFUL )
   {
      return true;
   }
   else if( userMessage )
   {
      QString msg( recordName );
      msg.append( " Get channel failed." );
      switch( response )
      {
         case CHANNEL_DISCONNECTED: msg.append( " Channel disconnected."); break;
         case REQUEST_FAILED:       msg.append( " Request failed.");       break;
         default:                   msg.append( " Unknown error");         break;
      }
      userMessage->sendMessage( msg, "QCaObject::getChannel()", message_types ( MESSAGE_TYPE_ERROR ) );
      return false;
   }
   return false;
}

/*
    Write to a PV
*/
bool QCaObject::putChannel() {

   // Generate the output data based on the type
   Generic outputData;

   // If an array represented by a string
   if(( writingData.type() == QVariant::String )
      &&
      ( getElementCount() > 1 ))
   {
      // Copy the string and a zero terminator into the output data
      // (a zero terminator is required when writing an empty string,
      //  and is also required in some cases such as AreaDetector file path
      //  If the string is the full length of the array, adding this zero terminator
      //  will cause the write to fail. Perhaps this should not add the zero if
      //  already at the full array length???)
      QString s = writingData.toString();
      outputData.setUnsignedChar( NULL, s.length()+1 );
      int i;
      for( i = 0; i < s.length(); i++ )
      {
         outputData.updateUnsignedChar( s.at(i).toLatin1(), i );
      }
      outputData.updateUnsignedChar( 0, i );
   }

   // If a scalar
   else if( writingData.type() != QVariant::List )
   {
      // Generate the output data based on the single value types
      switch( writingData.type() )
      {
         case QVariant::Double :
            outputData.setDouble( writingData.toDouble() );
            break;
         case QVariant::Int :
         case QVariant::LongLong :
            outputData.setLong( writingData.toInt() );
            break;
         case QVariant::UInt :
         case QVariant::ULongLong :
            outputData.setUnsignedLong( writingData.toUInt() );
            break;
         case QVariant::String :
            outputData.setString( writingData.toString().toStdString() );
            break;
         case QVariant::ByteArray :
            outputData.setUnsignedChar( ((unsigned char*)writingData.toByteArray().data()), writingData.toByteArray().count() );
            break;
         default:
            return false;
            break;
      }
   }

   // If an array
   else
   {
      // Generate the output data based on the array value types
      QVariantList list = writingData.toList();
      if( !list.count() )
      {
         return false;
      }
      switch( list[0].type() )
      {
         case QVariant::Double :
            outputData.setDouble( NULL, list.count() );
            for( int i = 0; i < list.count(); i++ )
            {
               outputData.updateDouble( list[i].toDouble(), i );
            }
            break;
         case QVariant::Int :
         case QVariant::LongLong :
            outputData.setUnsignedLong( NULL, list.count() );
            for( int i = 0; i < list.count(); i++ )
            {
               outputData.updateUnsignedLong( list[i].toInt(), i );
            }
            break;
         case QVariant::UInt :
         case QVariant::ULongLong :
            outputData.setUnsignedLong( NULL, list.count() );
            for( int i = 0; i < list.count(); i++ )
            {
               outputData.updateUnsignedLong( list[i].toUInt(), i );
            }
            break;
         case QVariant::String :
            //??? Don't do arrays of strings
            outputData.setString( writingData.toString().toStdString() );
            break;
         case QVariant::ByteArray :
            // Copy the byte array as an array of unsigned chars.
            // There is no need to convert each byte to the data size of the CA data.
            // A byte array simply holds the data byte after byte regardless of the data type.
            outputData.setUnsignedChar( (unsigned char*)(writingData.toByteArray().data()), writingData.toByteArray().length() );

            // Now set the data type.
            // This is ONLY ok since a byte array is treated as raw data. Each byte does not represent one element.
            outputData.setType( getDataType() );
            break;
         default:
            return false;
            break;
      }

   }

   // Write the data
   caconnection::ca_responses response = caPrivate->writeChannel( &outputData );
   if( response == caconnection::REQUEST_SUCCESSFUL )
   {
      return true;
   }
   else if( userMessage )
   {
      QString msg( recordName );
      msg.append( " Put channel failed." );
      switch( response )
      {
         case CHANNEL_DISCONNECTED: msg.append( " Channel disconnected."); break;
         case REQUEST_FAILED:       msg.append( " Request failed.");       break;
         default:                   msg.append( " Unknown error");         break;
      }
      userMessage->sendMessage( msg, "QCaObject::putChannel()", message_types ( MESSAGE_TYPE_ERROR ) );
      return false;
   }
   return false;
}

/*
    Determine if the channel is currently connected
*/
bool QCaObject::isChannelConnected() {
   return ( connectionMachine->currentState == qcastatemachine::CONNECTED );
}

/*
    Wait one minute for a connection, then re-attempt the connection
 */
void QCaObject::startConnectionTimer() {
   setChannelTimer.start( 60000 );
}

/*
    Connection has been achieved within the expected time, stop the timer used to wait for a connection
 */
void QCaObject::stopConnectionTimer() {
   setChannelTimer.stop();
}

/*
    Returns the type of data being read or to be written.
    This will be Type::GENERIC_UNKNOWN if a connection has never been established.
*/
generic::generic_types QCaObject::getDataType() {
   return getType();
}

/*
    Write a data out to channel
*/
bool QCaObject::writeData( const QVariant& newData ) {
   writingData = newData;
   return writeMachine->process( qcastatemachine::WRITING );
}

/*
    Update current data [arrayIndex] with new element value and write to channel.
    Returns false if array index is out of range.
*/
bool QCaObject::writeDataElement( const QVariant& elementValue )
{
   bool result;
   if( lastVariantValue.type() ==  QVariant::List )
   {
      QVariantList valueList = lastVariantValue.toList();
      if( ( arrayIndex >= 0 ) && ( arrayIndex < valueList.size() ) )
      {
         valueList.replace( arrayIndex, elementValue );  // replace with new value
         result = writeData( valueList );
      }
      else {
         result = false;
      }
   }
   else
   {
      if( arrayIndex == 0 ) {
         result = writeData( elementValue );       // not an array - write as scalar
      }
      else {
         result = false;
      }
   }
   return result;
}

/*
    Implemetation of virtual CA callback function.
    This code is executed by an EPICS library thread. It packages data and
    posts via an event.
*/
void QCaObject::signalCallback( caobject::callback_reasons newReason ) {
   // Initialise data package.
   // It is really of type carecord::CaRecord*
   void* dataPackage = NULL;

   // Only case where data is processed. Package the data
   if( newReason == caobject::SUBSCRIPTION_SUCCESS || newReason == caobject::READ_SUCCESS )
   {
      dataPackage = getRecordCopyPtr();
   }

   // If the callback is a data update callback, and there is an earlier, unprocessed, data event
   // of the same type in the queue (but not an initial update that carries extra info such as precision and units),
   // and the event is still usable (not for a QCaObject that has been deleted),
   // then replace the data package with this one.
   // This is better than adding events faster than they can be processed.
   bool replaced = false;  // True if data replaced in earlier event
   if( dataPackage )
   { // Limit scope of pending event list lock
      QMutexLocker locker( &pendingEventsLock );
      for( int i = 0; i < pendingEvents.count(); i++ )
      {
         QCaEventUpdate* event = pendingEvents[i].event;
         if( event->reason == newReason )
         {
            carecord::CaRecord* record = (carecord::CaRecord*)(event->dataPtr);
            if( record->isFirstUpdate() == false )
            {
               delete (carecord::CaRecord*)(event->dataPtr);
               event->dataPtr = dataPackage;
               replaced = true;
            }
            break;
         }
      }
   }

   // This is not a data update event, or there is no earlier, unprocessed, data event in the queue,
   // create a new event and post it to the event queue
   if( !replaced )
   {
      // Package the data to be processed within the context of a Qt thread.
      QCaEventUpdate* newDataEvent = new QCaEventUpdate( this, newReason, dataPackage );

      // Add the event to the list of pending events.
      // A list is maintained to allow pending events to be updated 'in transit'.
      // This must be done before posting the event as the event is likely to be processed before
      // the postEvent call returns.
      QCaEventItem item( newDataEvent );
      { // Limit scope of pending event list lock
         QMutexLocker locker( &pendingEventsLock );
         pendingEvents.append( item );
      }

      // Post the data to be processed within the context of a Qt thread.
      QCoreApplication::postEvent( eventHandler, newDataEvent );

   }

   // Processing will continue within the context of a Qt thread in QCaObject::processEvent() below.
}

/*
    Process events posted from the EPICS library thread. The event is expected
    to provide snapshot of data.
    This method completes the processing of a CA callback, started in QCaObject::signalCallback() above,
    within a Qt thread
    Note,
*/
void QCaObject::processEvent( QCaEventUpdate* dataUpdateEvent ) {

   // Process the event, based on the event type
   switch( dataUpdateEvent->reason ) {
      case caobject::CONNECTION_UP :
         {
            channelExpiredMessage = false;
            connectionMachine->active = true;
            connectionMachine->process( qcastatemachine::CONNECTED );
            subscriptionMachine->process( subscriptionMachine->requestState );
            readMachine->process( readMachine->requestState );
            writeMachine->process( qcastatemachine::WRITE_IDLE );
            break;
         }
      case caobject::CONNECTION_DOWN :
         {
            if( userMessage )
            {
               QString msg( recordName );
               userMessage->sendMessage( msg.append( " Connection down" ), "QCaObject::processEvent()",
                                         message_types ( MESSAGE_TYPE_WARNING ) );
            }
            connectionMachine->active = false;
            connectionMachine->process( qcastatemachine::DISCONNECTED );
            subscriptionMachine->process( subscriptionMachine->requestState );
            readMachine->process( readMachine->requestState );
            writeMachine->process( qcastatemachine::WRITE_IDLE );
            break;
         }
      case caobject::SUBSCRIPTION_SUCCESS :
         {
            if( subscriptionMachine->currentState == qcastatemachine::UNSUBSCRIBED ) {
               subscriptionMachine->active = true;
               subscriptionMachine->process( qcastatemachine::SUBSCRIBED );
            } else {
               subscriptionMachine->process( qcastatemachine::SUBSCRIBED_READ );
            }
            processData( (void*)(dataUpdateEvent->dataPtr) );
            break;
         }
      case caobject::SUBSCRIPTION_FAIL :
         {
            if( userMessage )
            {
               QString msg( recordName );
               userMessage->sendMessage( msg.append( " Subscription failed" ), "QCaObject::processEvent()",
                                         message_types ( MESSAGE_TYPE_WARNING ) );
            }

            subscriptionMachine->active = false;
            subscriptionMachine->process( qcastatemachine::SUBSCRIBE_FAIL );
            break;
         }
      case caobject::READ_SUCCESS :
         {
            readMachine->active = false;
            readMachine->process( qcastatemachine::READ_IDLE );

            processData( (void*)(dataUpdateEvent->dataPtr) );
            break;
         }
      case caobject::READ_FAIL :
         {
            if( userMessage )
            {
               QString msg( recordName );
               userMessage->sendMessage( msg.append( " Read failed" ), "QCaObject::processEvent()",
                                         message_types ( MESSAGE_TYPE_WARNING ) );
            }
            readMachine->active = false;
            readMachine->process( qcastatemachine::READING_FAIL );
            break;
         }
      case caobject::WRITE_SUCCESS :
         {
            writeMachine->active = false;
            writeMachine->process( qcastatemachine::WRITE_IDLE );
            break;
         }
      case caobject::WRITE_FAIL :
         {
            if( userMessage )
            {
               QString msg( recordName );
               userMessage->sendMessage( msg.append( " Write failed" ), "QCaObject::processEvent()",
                                         message_types ( MESSAGE_TYPE_WARNING ) );
            }

            writeMachine->active = false;
            writeMachine->process( qcastatemachine::WRITING_FAIL );
            break;
         }
      case caobject::EXCEPTION :
         {
            if( userMessage )
            {
               QString msg( recordName );
               userMessage->sendMessage( msg.append( " Exception" ), "QCaObject::processEvent()",
                                         message_types ( MESSAGE_TYPE_ERROR ) );
            }
            break;
         }
      case caobject::CONNECTION_UNKNOWN :
         {
            if( userMessage )
            {
               QString msg( recordName );
               userMessage->sendMessage( msg.append( " Unknown connection" ), "QCaObject::processEvent()",
                                         message_types ( MESSAGE_TYPE_WARNING ) );
            }
            break;
         }
      default :
         {
            if( userMessage )
            {
               QString msg( recordName );
               userMessage->sendMessage( msg.append( " Unknown CA callback" ), "QCaObject::processEvent()",
                                         message_types ( MESSAGE_TYPE_ERROR ) );
            }
         }
         break;
   }

   // Assume there will be no change in the channel or link
   bool connectionChange = false;

   // If the channel state has changed, signal if the channel is connected or not
   if( caPrivate->getChannelState() != lastEventChannelState ) {
      lastEventChannelState = caPrivate->getChannelState();
      connectionChange = true;
   }

   // If the link state has changed, signal if the link is up or not
   if( caPrivate->getLinkState() != lastEventLinkState ) {
      lastEventLinkState = caPrivate->getLinkState();
      connectionChange = true;
   }

   // If there is a change in the connection (link of channel), signal it
   if( connectionChange )
   {
      QCaConnectionInfo connectionInfo( lastEventChannelState, lastEventLinkState, getRecordName() );
      if (!connectionInfo.isChannelConnected()) {
         lastValueIsDefined = false;
      }

      // Save last connetion info.
      lastIsChannelConnected = connectionInfo.isChannelConnected ();
      lastIsLinkUp = connectionInfo.isLinkUp ();

      emit connectionChanged( connectionInfo, variableIndex );
      emit connectionChanged( connectionInfo );
   }
}

/*
    Process new data arrivals.
    This is called when appropriate while processing an event containing CA data
*/
void QCaObject::processData( void* newDataPtr ) {

   // Recover the data record
   carecord::CaRecord* newData = (carecord::CaRecord*)newDataPtr;

   // Sanity check. Check newData is set up. newData is only set up for some
   // callback reasons and this method is also only called for some callback reasons.
   // This test is in case the two sets of reasons are not the same.
   if( !newData )
      return;

   // On the first update, gather static information for the variable
   if( newData->isFirstUpdate() ) {
      // Note the engineering units
      egu = QString( getUnits().c_str() );

      // Note the enumeration strings
      int numEnums = getEnumStateCount();
      for( int i = 0; i < numEnums; i++ )
      {
         std::string s = getEnumState(i);
         if( s.length() )
            enumerations.append( QString( s.c_str() ) );
         else
            enumerations.append( QString::number( i ) );
      }

      // If this PV is a record's STAT field, then for value between 16 and 21 use hard coded
      // enumerated strings.  This is due to a limit in EPICS. The STAT field which has 22
      // enumerated values, but only 16 enumerated strings can be included in the CA protocol.
      if (isStatField) {
         enumerations.append ("BAD_SUB");        // 16
         enumerations.append ("UDF");            // 17
         enumerations.append ("DISABLE");        // 18
         enumerations.append ("SIMM");           // 19
         enumerations.append ("READ_ACCESS");    // 20
         enumerations.append ("WRITE_ACCESS");   // 21
      }

      // Note the precision
      precision = CaObject::getPrecision();

      // Note the display limits
      displayLimitUpper = CaObject::getDisplayUpper();
      displayLimitLower = CaObject::getDisplayLower();

      if (isStatField) {
         // And do special for a record's STAT field.
         displayLimitUpper = enumerations.count () - 1;
      }

      // Note the alarm limits
      alarmLimitUpper = CaObject::getAlarmUpper();
      alarmLimitLower = CaObject::getAlarmLower();

      // Note the warning limits
      warningLimitUpper = CaObject::getWarningUpper();
      warningLimitLower = CaObject::getWarningLower();

      // Note the control limits
      controlLimitUpper = CaObject::getControlUpper();
      controlLimitLower = CaObject::getControlLower();

   }

   // Build the alarm infomation (alarm state and severity)
   QCaAlarmInfo alarmInfo( getAlarmStatus(), getAlarmSeverity() );

   // Build the time stamp (Note, a QCaDateTime is a QDateTime with nanoseconds past the millisecond)
   // Use current time if no EPICS timestamp is available
   QCaDateTime timeStamp;
   unsigned long timeStampSeconds = 0;
   if( ( timeStampSeconds = getTimeStampSeconds() ) != 0 )
   {
      QCaDateTime caTimeStamp( timeStampSeconds, getTimeStampNanoseconds() );
      timeStamp = caTimeStamp;
   }
   else
   {
      QCaDateTime localTimeStamp( QDateTime::currentDateTime() );
      timeStamp = localTimeStamp;
   }

   // Determine size of data array.
   unsigned long arrayCount = newData->getArrayCount();

   // Save date about be be emitted - for resend/access purposes.
   lastAlarmInfo = alarmInfo;
   lastTimeStamp = timeStamp;

   // Build and emit a Qt variant containing the data
   if( signalsToSend & SIG_VARIANT )
   {
      // Package up the CA data as a Qt variant
      QVariant value;
      switch( newData->getType() ) {
         case generic::GENERIC_STRING :
            value = QVariant( QString::fromStdString( newData->getString() ) );
            break;
         case generic::GENERIC_SHORT :
            if( arrayCount <= 1 )
            {
               value = QVariant( (qlonglong)newData->getShort() );
            }
            else
            {
               QVariantList values;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
               values.reserve( arrayCount );
#endif
               short* data;
               newData->getShort( &data );
               for( unsigned long i = 0; i < arrayCount; i++ )
               {
                  values.append( (qlonglong)(data[i]) );
               }
               value = QVariant( values );
            }
            break;
         case generic::GENERIC_UNSIGNED_SHORT :
            if( arrayCount <= 1 )
            {
               value = QVariant( (qulonglong)newData->getUnsignedShort() );
            }
            else
            {
               QVariantList values;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
               values.reserve( arrayCount );
#endif
               unsigned short* data;
               newData->getUnsignedShort( &data );

               for( unsigned long i = 0; i < arrayCount; i++ )
               {
                  values.append( (qulonglong)(data[i]) );
               }
               value = QVariant( values );
            }
            break;
         case generic::GENERIC_UNSIGNED_CHAR :
            if( arrayCount <= 1 )
            {
               value = QVariant( (qulonglong)newData->getUnsignedChar() );
            }
            else
            {
               QVariantList values;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
               values.reserve( arrayCount );
#endif
               unsigned char* data;
               newData->getUnsignedChar( &data );

               for( unsigned long i = 0; i < arrayCount; i++ )
               {
                  values.append( (qulonglong)(data[i]) );
               }
               value = QVariant( values );
            }
            break;
         case generic::GENERIC_LONG :
            if( arrayCount <= 1 )
            {
               value = QVariant( (qlonglong)newData->getLong() );
            }
            else
            {
               QVariantList values;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
               values.reserve( arrayCount );
#endif
               qint32* data;
               newData->getLong( &data );

               for( unsigned long i = 0; i < arrayCount; i++ )
               {
                  values.append( (qlonglong)(data[i]) );
               }
               value = QVariant( values );
            }
            break;
         case generic::GENERIC_UNSIGNED_LONG :
            if( arrayCount <= 1 )
            {
               value = QVariant( (qulonglong)newData->getUnsignedLong() );
            }
            else
            {
               QVariantList values;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
               values.reserve( arrayCount );
#endif
               quint32* data;
               newData->getUnsignedLong( &data );

               for( unsigned long i = 0; i < arrayCount; i++ )
               {
                  values.append( (qulonglong)(data[i]) );
               }
               value = QVariant( values );
            }
            break;
         case generic::GENERIC_FLOAT :
            if( arrayCount <= 1 )
            {
               value = QVariant( (double)newData->getFloat() );
            }
            else
            {
               QVariantList values;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
               values.reserve( arrayCount );
#endif
               float* data;
               newData->getFloat( &data );

               for( unsigned long i = 0; i < arrayCount; i++ )
               {
                  values.append( (double)(data[i]) );
               }
               value = QVariant( values );
            }
            break;
         case generic::GENERIC_DOUBLE :
            if( arrayCount <= 1 )
            {
               value = QVariant( newData->getDouble() );
            }
            else
            {
               QVariantList values;
#if (QT_VERSION >= QT_VERSION_CHECK(4, 7, 0))
               values.reserve( arrayCount );
#endif
               double* data;
               newData->getDouble( &data );

               for( unsigned long i = 0; i < arrayCount; i++ )
               {
                  values.append( data[i] );
               }
               value = QVariant( values );
            }
            break;
         case generic::GENERIC_UNKNOWN :
            value = QVariant();
            break;
      }

      // Save the data just about to be emited
      lastVariantValue = value;
      lastValueIsDefined = true;

      // Send off the new data
      emit dataChanged( value, alarmInfo, timeStamp, variableIndex );
   }

   // Build and emit a byte array containing the data.
   // Note, the byte array (and copies of it such as the lastByteArrayValue will
   // have a pointer directly into the data, so don't delete the data until all
   // byte arrays referencing it have been deleted.
   if( signalsToSend & SIG_BYTEARRAY )
   {
      char* data;
      unsigned long dataSize = 0;

      // Get a pointer to the data
      switch( newData->getType() ) {
         case generic::GENERIC_STRING         : newData->getString       ( (char**)          (&data) ); dataSize = 1; break;
         case generic::GENERIC_SHORT          : newData->getShort        ( (short**)         (&data) ); dataSize = 2; break;
         case generic::GENERIC_UNSIGNED_SHORT : newData->getUnsignedShort( (unsigned short**)(&data) ); dataSize = 2; break;
         case generic::GENERIC_UNSIGNED_CHAR  : newData->getUnsignedChar ( (unsigned char**) (&data) ); dataSize = 1; break;
         case generic::GENERIC_LONG           : newData->getLong         ( (qint32**)        (&data) ); dataSize = 4; break;
         case generic::GENERIC_UNSIGNED_LONG  : newData->getUnsignedLong ( (quint32**)       (&data) ); dataSize = 4; break;
         case generic::GENERIC_FLOAT          : newData->getFloat        ( (float**)         (&data) ); dataSize = 4; break;
         case generic::GENERIC_DOUBLE         : newData->getDouble       ( (double**)        (&data) ); dataSize = 8; break;
         case generic::GENERIC_UNKNOWN        : data = NULL;                                            dataSize = 0; break;
      }

      // Build a byte array from the data
      unsigned long arraySize = arrayCount * dataSize;
      byteArrayValue.resize( arraySize );
      char* baData = byteArrayValue.data();
      memcpy( baData, data, arraySize );

      // Save the data just about emited so it can be re-sent if required
      lastByteArrayValue = byteArrayValue;
      lastDataSize = dataSize;

      // Send off the new data
      // NOTE, the signal/slot connections to this signal must be Qt::DirectConnection
      // as the byte array refernces the data directly which may be deleted before a queued connection is completed
      emit dataChanged( byteArrayValue, dataSize, alarmInfo, timeStamp, variableIndex );

      // Delete any old data now it is no longer referenced by byte arrays
      if( lastNewData )
      {
         delete (carecord::CaRecord*)lastNewData;
      }

      // Save the current event data for later deletion
      lastNewData = (void*)newData;
   }

   // If not emiting an array, the data can be deleted (emitted byte arrays have pointers directly into the data)
   else
   {
      // Discard the event data
      delete newData;
   }
}

/*
    Connecting timeout.
    Generally, we could just wait forever for a connection to complete, but due to
    rumours of gateways not honouring an old request when an IOC serving the requested
    PV finally appears, we re-attempt the connection here after an extended wait.
*/
void QCaObject::setChannelExpired() {
   // Generate a user message
   if( userMessage && !channelExpiredMessage )
   {
      QString msg( recordName );
      userMessage->sendMessage( msg.append( " Channel expired, will keep retrying" ),
                                "QCaObject::setChannelExpired()",
                                message_types ( MESSAGE_TYPE_WARNING ) );
      channelExpiredMessage = true;
   }

   // Update the current state
   connectionMachine->expired = true;
   connectionMachine->process( qcastatemachine::CONNECTION_EXPIRED );

   // Attempt a re-connection
   connectionMachine->process( qcastatemachine::CONNECTED );
   subscribe();
}

/*
  Setup the user message mechanism.
  After calling this method the QCaObject can report errors to the user.
  */
void QCaObject::setUserMessage( UserMessage* userMessageIn )
{
   userMessage = userMessageIn;
}

/*
 Setup the number of elements required.
 This can be called before a subscription, or during a subscription, in which
 case the connection will be renewed with the new element count, if changed.
 */
void QCaObject::setRequestedElementCount( unsigned int elementCount )
{
   caPrivate->caConnection->setChannelRequestedElementCount( elementCount );
}

/*
  Re-emit the last data emited, if any.
  This can be used after a property of a widget using this QCaObject has changed to
  force an update of the data and a re-presentation of the data in the widget to reflect the new property
  */
void QCaObject::resendLastData()
{
   if( !getDataIsAvailable() ) return;

   if( signalsToSend & SIG_VARIANT )
   {
      emit dataChanged( lastVariantValue, lastAlarmInfo, lastTimeStamp, variableIndex );
   }
   if( signalsToSend & SIG_BYTEARRAY )
   {
      // NOTE, the signal/slot connections to this signal must be Qt::DirectConnection as the byte array
      // refernces the data held in lastNewData directly which may be deleted before a queued connection is completed
      emit dataChanged( lastByteArrayValue, lastDataSize, lastAlarmInfo, lastTimeStamp, variableIndex );
   }
}

/*
  Extract last emmited data
  */
void QCaObject::getLastData( bool& isDefinedOut, QVariant& valueOut, QCaAlarmInfo& alarmInfoOut, QCaDateTime& timeStampOut )
{
   isDefinedOut = lastValueIsDefined;
   valueOut = lastVariantValue;
   alarmInfoOut = lastAlarmInfo;
   timeStampOut = lastTimeStamp;
}


/*
  Extract last emmited connection info: indicates if channel is connected.
  */
bool QCaObject::getChannelIsConnected () const
{
   return lastIsChannelConnected;
}

/*
  Extract last emmited connection info: indicates if channel link is up.
  */
bool QCaObject::getIsLinkUp () const
{
   return lastIsLinkUp;
}


/*
  Indicates if last data is defined/meaningful.
  */
bool QCaObject::getDataIsAvailable() const
{
   return lastValueIsDefined;
}

/*
  Return the current value as string
  */
QString QCaObject::getStringValue() const
{
   return lastVariantValue.toString();
}

/*
  Return the current value as integer
  */
long QCaObject::getIntegerValue() const
{
   bool okay;
   long result;
   result = lastVariantValue.toInt( &okay );
   if( !okay ) result = 0;
   return result;
}

/*
  Return the current value as floating
  */
double QCaObject::getFloatingValue() const
{
   bool okay;
   double result;
   result = lastVariantValue.toDouble( &okay );
   if( !okay ) result = 0.0;
   return result;
}

/*
  Return the current value as integer array
  */
QVector<long> QCaObject::getIntegerArray () const
{
   QVector<long> result;

   if( lastVariantValue.type () == QVariant::List ) {
      QVariantList list = lastVariantValue.toList();
      result.reserve( list.count() );
      for( int j = 0; j < list.count(); j++ ){
         bool okay;
         long item;
         item = list.value( j ).toInt ( &okay );
         if( !okay )break;
         result.append ( item );
      }
   } else {
      // array of 1 element
      bool okay;
      long item;
      item = lastVariantValue.toInt( &okay );
      if( okay )result.append ( item );
   }

   return result;
}

/*
  Return the current value as floating array
  */
QVector<double> QCaObject::getFloatingArray () const
{
   QVector<double> result;

   if( lastVariantValue.type () == QVariant::List ) {
      QVariantList list = lastVariantValue.toList();
      result.reserve( list.count() );
      for( int j = 0; j < list.count(); j++ ){
         bool okay;
         double item;
         item = list.value( j ).toDouble( &okay );
         if( !okay )break;
         result.append ( item );
      }
   } else {
      // array of 1 element
      bool okay;
      double item;
      item = lastVariantValue.toDouble( &okay );
      if( okay )result.append ( item );
   }

   return result;
}


/*
  Return the record name (technically the process variable name).
  */
QString QCaObject::getRecordName()
{
   return recordName;
}

/*
 Return the engineering units, if any
*/
QString QCaObject::getEgu() {
   return egu;
}

/*
 Return the PV hostname
*/
QString QCaObject::getHostName()
{
   return  QString (caobject::CaObject::getHostName ().c_str());
}

/*
  Get the read access of the current connection.
 */
bool QCaObject::getReadAccess()
{
   return caobject::CaObject::getReadAccess();
}

/*
  Get the write access of the current connection.
 */
bool QCaObject::getWriteAccess()
{
   return caobject::CaObject::getWriteAccess();
}

/*
 Return the PV native field type
*/
QString QCaObject::getFieldType()
{
   return  QString (caobject::CaObject::getFieldType ().c_str());
}

/*
 Return the PV native element count
*/
unsigned long QCaObject::getElementCount()
{
   return caobject::CaObject::getElementCount ();
}

/*
  Set nominated array index used to extract a scalar from an array data set.
  Defaults to zero, i.e. first element.
 */
void QCaObject::setArrayIndex( const int indexIn )
{
   arrayIndex = MAX( 0, indexIn );
}

/*
  Return nominated array index used to extract a scalar from an array data set.
 */
int QCaObject::getArrayIndex() const
{
   return arrayIndex;
}

/*
 Return the enumerations strings, if any
*/
QStringList QCaObject::getEnumerations() {
   return enumerations;
}

/*
 Return the precision, if any
*/
unsigned int QCaObject::getPrecision()
{
   return precision;
}

/*
 Return the display upper limit, if any
*/
double QCaObject::getDisplayLimitUpper()
{
   return displayLimitUpper;
}

/*
 Return the display lower limit, if any
*/
double QCaObject::getDisplayLimitLower()
{
   return displayLimitLower;
}

/*
 Return the alarm upper limit, if any
*/
double QCaObject::getAlarmLimitUpper()
{
   return alarmLimitUpper;
}

/*
 Return the alarm lower limit, if any
*/
double QCaObject::getAlarmLimitLower()
{
   return alarmLimitLower;
}

/*
 Return the warning upper limit, if any
*/
double QCaObject::getWarningLimitUpper()
{
   return warningLimitUpper;
}

/*
 Return the warning lower limit, if any
*/
double QCaObject::getWarningLimitLower()
{
   return warningLimitLower;
}

/*
 Return the control upper limit, if any
*/
double QCaObject::getControlLimitUpper()
{
   return controlLimitUpper;
}

/*
 Return the control lower limit, if any
*/
double QCaObject::getControlLimitLower()
{
   return controlLimitLower;
}

/*
  Set if callbacks are required on write completion. (default is write with no callback)
  Note, this is not just for better write status, if affects the behaviour of the write as follows:
  When using write with callback, then record will finish processing before accepting next write.
  Writing with callback may be required when writing code that is tightly integrated with record
  processing and code nneds to know processing has completed.
  Writing with no callback is more desirable when a detachement from record processing is required, for
  example in a GUI after issuing a motor record move a motor stop command will take effect immedietly
  if writing without callback, but will only take affect after the move has finished if writing with callback.
  */
void QCaObject::enableWriteCallbacks( bool enable )
{
   setWriteWithCallback( enable );
}

/*
  Determine if callbacks are required on write completion.
*/
bool QCaObject::isWriteCallbacksEnabled()
{
   return getWriteWithCallback();
}

/*
    Return alarm state associated with last update
 */
QCaAlarmInfo QCaObject::getAlarmInfo()
{
   return lastAlarmInfo;
}

/*
    Return date time associated with last update
 */
QCaDateTime QCaObject::getDateTime ()
{
   return lastTimeStamp;
}

// end
