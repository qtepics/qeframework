/*
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
 *  Copyright (c) 2009, 2010 Australian Synchrotron
 *
 *  Author:
 *    Anthony Owen
 *  Contact details:
 *    anthony.owen@gmail.com
 */

#include <db_access.h>
#include <epicsMutex.h>
#include <CaConnection.h>

#include <stdio.h>

using namespace caconnection;

/*
    Initialise the unique ID.
*/
int CaConnection::CA_UNIQUE_CONNECTION_ID = 0;

/*
    Initialise EPICS library and setup working parent. The pointer to the
    working parent is given to the EPICS thread so that any callbacks that
    occur contain the context of the parent that created this CaConnection.
*/
CaConnection::CaConnection( void* newParent )
{
    // Construct or reuse a durable object that can be passed to CA and used as a callback argument
    myRef = CaRef::getCaRef( this, false );

    parent = newParent;
    initialise();
    reset();
}

/*
    Shutdown EPICS library and reset internal data to defaults.
*/
CaConnection::~CaConnection() {

    myRef->discard();
    shutdown();

    // Reset this connection whilset ensuring we are not in CA callback code with
    // a risk of accessing this object (Callback code will check the discard flag
    // only while holding the lock)
    CaRef::accessLock();
    reset();
    CaRef::accessUnlock();
}

/*
    Creates only one EPICS context and registers an exception callback.
    Use hasContext() for feedback.
    Returns: REQUEST_SUCCESSFUL or REQUEST_FAILED
*/
ca_responses CaConnection::establishContext( void (*exceptionHandler)(struct exception_handler_args), void* args ) {
    if( context.activated == false ) {
        if( CA_UNIQUE_CONNECTION_ID <= 1) {
            context.creation = ca_context_create( ca_enable_preemptive_callback );
            context.exception = ca_add_exception_event ( exceptionHandler, args );
        }
        context.activated = true;
        switch( context.creation )  {
            case ECA_NORMAL :
                return REQUEST_SUCCESSFUL;
            break;
            default :
                return REQUEST_FAILED;
            break;
        }
    } else {
        return REQUEST_FAILED;
    }
}

/*
    Establishes client side channel and registers a connection state change
    callback.
    Use activeChannel() for feedback.
    Returns: REQUEST_SUCCESSFUL or REQUEST_FAILED
*/
ca_responses CaConnection::establishChannel( void (*connectionHandler)(struct connection_handler_args), std::string channelName, priorities priority ) {
    if( context.activated == true && channel.activated == false )
    {
        myRef->setPV( channelName );
        channel.creation = ca_create_channel( channelName.c_str(), connectionHandler, myRef, priority, &channel.id );
//        channel.creation = ca_create_channel( channelName.c_str(), connectionHandler, myRef, prio, &channel.id );
        myRef->setChannelId( channel.id );
        // Sanity check
        if( channel.id == 0 )
        {
            printf( "CaConnection::establishChannel() ca_create_channel returned a channel ID of zero\n" ); fflush(stdout);
            return REQUEST_FAILED;
        }

        ca_pend_io( link.searchTimeout );
        channel.activated = true;
//        printf( "CaConnection::establishChannel channel activated %ld  chid: %ld  name: %s prio: %ld\n", (long)(&channel), (long)(channel.id), channelName.c_str(), (long)prio ); fflush(stdout);
        switch( channel.creation ) {
            case ECA_NORMAL :
                return REQUEST_SUCCESSFUL;
            break;
            default :
                return REQUEST_FAILED;
            break;
        }
    } else {
        return REQUEST_FAILED;
    }
}

// Set the channel element count to be requested.
// This must be done before the connection is requested.
void CaConnection::setChannelRequestedElementCount( unsigned long requestedElementCountIn )
{
    channel.requestedElementCount = requestedElementCountIn;
    channel.requestedElementCountSet = true;
}


// Set the channel element count.
// This is the number of elementes returned, not requested.
// This can be done after the connection callback has been called and the connection is up
void CaConnection::setChannelElementCount()
{
    // Get the channel element count. This can be done after the connection callback has been called and the connection is up
    channel.elementCount = ca_element_count( channel.id );

    // If fail, default element count to a single element

    if( channel.elementCount < 1  )
    {
        channel.elementCount = 1;
    }
}

// Get the number of elements to subscribe to,
// the will be the number requested if any, otherwise it will be the array size reported on connection.
unsigned long CaConnection::getSubscribeElementCount()
{
    unsigned long result = channel.elementCount;   //default
    if( channel.requestedElementCountSet )
    {
        // Use requested element count but ensure no more than supported by the channel server.
        if( channel.requestedElementCount < channel.elementCount )
        {
            result = channel.requestedElementCount;
        }
    }
    return result;
}

/*
    Subscribes to the established channel and registers for data callbacks
    Use isSubscribed() for feedback.

    Initially, only a single shot read is performed. An internal callback handler catches the read completion,
    calls the real callback handler, and initiates the real subscription.
    This is a work around to solve the problem that the 'first' subscription callback with static info
    such as units and precision does not always come first
*/
ca_responses CaConnection::establishSubscription( void (*subscriptionHandler)(struct event_handler_args),
                                                  void* args, short initialDbrStructTypeIn,
                                                  short updateDbrStructTypeIn) {

//    printf( "CaConnection::establishSubscription chid: %ld\n", (long)getChannelId() ); fflush(stdout);
    // Save the callers callback information
    // This will be used when a subscription is really established
    subscriptionSubscriptionHandler = subscriptionHandler;
    subscriptionArgs = args;
    initialDbrStructType = initialDbrStructTypeIn;
    updateDbrStructType = updateDbrStructTypeIn;

    if( channel.activated == true && subscription.activated == false ) {
        subscription.creation = ca_array_get_callback( initialDbrStructType, getSubscribeElementCount(), channel.id, subscriptionInitialHandler, myRef );
        ca_flush_io();
        subscription.activated = true;
        switch( subscription.creation ) {
            case ECA_NORMAL :
                return REQUEST_SUCCESSFUL;
            break;
            default :
                printf( "Subscription failure in CaConnection::establishSubscription() %s\n", ca_message( subscription.creation ) ); fflush(stdout);
                return REQUEST_FAILED;
            break;
        }
    } else {
        return REQUEST_FAILED;
    }
}
/*
  Internal handler (to this class) used by CaConnection::establishSubscription() to catch the first
  subscription callback (actually a ca_get callback).
  Deliver the callback to the real subscription callback, then establish a real subscription.

  This is a work around to solve the problem that the 'first' subscription callback with static info
  such as units and precision does not always come first
  */
void CaConnection::subscriptionInitialHandler( struct event_handler_args args )
{
    CaRef::accessLock();
//    printf( "CaConnection::subscriptionInitialHandler\n" ); fflush(stdout);
    // As this is a static function, recover the CaConnection class instance
    CaConnection* me = (CaConnection*)(((CaRef*)(args.usr))->getRef( args.chid ));
    CaRef::accessUnlock();

    if( !me )
    {
        return;
    }

    // Modify the callback argument to hold the data that the caller to CaConnection::establishSubscription() wanted
    args.usr = me->subscriptionArgs;

    // Call the 'real' subscription callback
    me->subscriptionSubscriptionHandler( args );

    // Establish a real subscription now that the initial read is complete.
    // The initial request type requested value(s) together with all the meta data.
    // Now switch to the "time" update type that provides value(s), status and time.
    //
    me->subscription.creation = ca_create_subscription( me->updateDbrStructType,
                                                        me->getSubscribeElementCount(),
                                                        me->channel.id,
                                                        DBE_VALUE|DBE_ALARM,
                                                        me->subscriptionSubscriptionHandler,
                                                        me->subscriptionArgs, &me->eventId );
//    printf( "CaConnection::subscriptionInitialHandler setting real subscription: chid: %ld (%ld) eventId: %ld\n", (long)(me->channel.id), (long)(args.chid), (long)me->eventId ); fflush(stdout);
    ca_flush_io();

}

/*
    Removes channel and associated subscription
    Use activeChannel() for feedback.
*/
void CaConnection::removeChannel() {
//    printf(  "CaConnection::removeChannel() %ld   chid %ld\n", (long)(&channel), (long)(channel.id) ); fflush(stdout);
    // Ensure we are not in a CA callback
    // ***  CaRef::accessLock();

    if( channel.activated == true ) {
        if( eventId )
        {
            ca_clear_subscription( eventId );
            eventId = 0;
        }
        ca_clear_channel( channel.id );
        channel.activated = false;

        channel.creation = -1;
        ca_flush_io();
    }

    // *** CaRef::accessUnlock();
}

/*
    Cancels channel subscription.
*/
void CaConnection::removeSubscription() {
//    printf(  "CaConnection::removeSubscription() (does nothing) %ld   chid %ld\n", (long)(&channel), (long)(channel.id) );  fflush(stdout);
    //NOT IMPLEMENTED
    //ca_clear_subscription( channelId );
}

/*
    Read channel once and register an event handler.
*/
ca_responses CaConnection::readChannel( void (*readHandler)(struct event_handler_args), void* args, short dbrStructType ) {
//    printf(  "CaConnection::readChannel() %ld   chid %ld\n", (long)(&channel), (long)(channel.id) ); fflush(stdout);
    if( channel.activated == true ) {
        channel.readResponse = ca_array_get_callback( dbrStructType, channel.elementCount, channel.id, readHandler, args);
        ca_pend_io( link.readTimeout );
        switch( channel.readResponse ) {
            case ECA_NORMAL :
                return REQUEST_SUCCESSFUL;
            break;
            case ECA_DISCONN :
                return CHANNEL_DISCONNECTED;
            break;
            default :
                return REQUEST_FAILED;
            break;
        }
    } else {
        return CHANNEL_DISCONNECTED;
    }
}

/*
    Write to channel once and register a write handler.
*/
ca_responses CaConnection::writeChannel( void (*writeHandler)(struct event_handler_args), void* args, short dbrStructType, int count, const void* newDbrValue ) {
    if( channel.activated == true ) {
        if( channel.writeWithCallback )
        {
            if( count )
            {
                channel.writeResponse = ca_array_put_callback( dbrStructType, count, channel.id, newDbrValue, writeHandler, args);
            }
            else
            {
                channel.writeResponse = ca_put_callback( dbrStructType, channel.id, newDbrValue, writeHandler, args);
            }
        }
        else
        {
            if( count )
            {
                channel.writeResponse = ca_array_put( dbrStructType, count, channel.id, newDbrValue);
            }
            else
            {
                channel.writeResponse = ca_put( dbrStructType, channel.id, newDbrValue);
            }
        }

        ca_pend_io( link.readTimeout );
        switch( channel.writeResponse ) {
            case ECA_NORMAL :
                return REQUEST_SUCCESSFUL;
            break;
            case ECA_DISCONN :
                return CHANNEL_DISCONNECTED;
            break;
            default :
                return REQUEST_FAILED;
            break;
        }
    } else {
        return CHANNEL_DISCONNECTED;
    }
}

/*
    Set the write callback mode.
    Write with no callback using ca_put() (default)
    or write with callback using ca_put_callback()
    When using write with callback, then record will finish processing before accepting next write.
    Writing with callback may be required when writing code that is tightly integrated with record
    processing and code nneds to know processing has completed.
    Writing with no callback is more desirable when a detachement from record processing is required, for
    example in a GUI after issuing a motor record move a motor stop command will take effect immedietly
    if writing without callback, but will only take affect after the move has finished if writing with callback.
*/
void CaConnection::setWriteWithCallback( bool writeWithCallbackIn )
{
    channel.writeWithCallback = writeWithCallbackIn;
}

/*
    Get the write callback mode.
*/
bool CaConnection::getWriteWithCallback()
{
    return channel.writeWithCallback;
}

/*
    Record the connection link status.
*/
void CaConnection::setLinkState( link_states newLinkState ) {
    link.state = newLinkState;
}

/*
    Retrieve the connection status.
*/
link_states CaConnection::getLinkState() {
    return link.state;
}

/*
    Retrieve the channel connection state. There is no set connection state
    because this is handled by the EPICS library.
*/
channel_states CaConnection::getChannelState() {
    channel.state = ca_state( channel.id );
    switch( channel.state ) {
        case cs_never_conn :
            return NEVER_CONNECTED;
        break;
        case cs_prev_conn :
            return PREVIOUSLY_CONNECTED;
        break;
        case cs_conn :
            return CONNECTED;
        break;
        case cs_closed :
            return CLOSED; 
        break;
        default:
            return CHANNEL_UNKNOWN;
        break;
    }
}

/*
    Retrieve the channel's database type.
*/
short CaConnection::getChannelType() {
    // If the channel is no longer activated, it is possible it has been cleared prior to reuse with a new variable.
    if( !channel.activated || !channel.id )
    {
        printf( "Attempting to get channel type while channel is not active or channel id is zero in CaConnection::getChannelType() %d %ld\n", channel.activated, (long)(channel.id) ); fflush(stdout);
        return 0;
    }

    // Extract and return the channel type
    channel.type = ca_field_type( channel.id );
    return channel.type;
}

// Retrieve the channel id
chid CaConnection::getChannelId()
{
    return channel.id;
}

/*
  Get the the host name from the current data record
 */
std::string CaConnection::getHostName ()
{
    chid ChannelId = getChannelId ();

    if (!ChannelId) return "";    // belts and braces check
    return ca_host_name (ChannelId);
}

/*
  Get the read access flag from the current data record
 */
bool CaConnection::getReadAccess ()
{
    chid ChannelId = getChannelId ();

    if (!ChannelId) return false;    // belts and braces check
    if( ca_read_access (ChannelId) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*
  Get the write access flag from the current data record
 */
bool CaConnection::getWriteAccess ()
{
    chid ChannelId = getChannelId ();

    if (!ChannelId) return false;    // belts and braces check
    if( ca_write_access (ChannelId) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

/*
  Get the the field type from the current data record
 */
std::string CaConnection::getFieldType ()
{
    chid ChannelId = getChannelId ();
    int ft;

    if (!ChannelId) return "";    // belts and braces check

    ft = ca_field_type (ChannelId);
    if (INVALID_DB_FIELD (ft)) ft = DBF_NO_ACCESS;
    return dbf_text [ft + 1];  // YES - that's + 1
}

/*
  Get the number of elements available from CA server.
 */
unsigned long CaConnection::getElementCount()
{
    chid ChannelId = getChannelId ();

    if (!ChannelId) return 0;    // belts and braces check
    return ca_element_count (ChannelId);
}


/*
    Initialise with unique ID and state information
*/
void CaConnection::initialise() {
    CA_UNIQUE_CONNECTION_ID++;
    channel.requestedElementCount = 0;
    channel.requestedElementCountSet = false;
    eventId = 0;
}

/*
    Shutdown and free context if last.
*/
void CaConnection::shutdown() {
    CA_UNIQUE_CONNECTION_ID--;

    if( channel.activated == true ) {
        ca_clear_channel( channel.id );
    }
    if( context.activated == true ) {
        if( CA_UNIQUE_CONNECTION_ID <= 0 ) {
            ca_context_destroy();
        }
    }
}

/*
    Set internal data to startup conditions
*/
void CaConnection::reset() {
    link.searchTimeout = 3.0;
    link.readTimeout = 2.0;
    link.writeTimeout = 2.0;
    link.state = LINK_DOWN;

//    printf( "CaConnection::reset() channel deactivated %ld  chid: %ld\n", (long)(&channel), (long)(channel.id) ); fflush(stdout);

    context.activated = false;
    context.creation = -1;
    context.exception = -1;

    channel.activated = false;
    channel.creation = -1;
    channel.readResponse = -1;
    channel.writeResponse = -1;
    channel.state = cs_never_conn;
    channel.type = -1;
    channel.id = NULL;
    channel.writeWithCallback = false;

    subscription.activated = false;
    subscription.creation = false;

    eventId = 0;
}
