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
 *  Copyright (c) 2009, 2010, 2013 Australian Synchrotron
 *
 *  Author:
 *    Glenn Jackson
 *  Contact details:
 *    glenn.jackson@internode.net
 */

#ifndef CACONNECTION_H_
#define CACONNECTION_H_

#include <string>
#include <cadef.h>
#include <CaRef.h>

namespace caconnection {

  enum link_states { LINK_UP, LINK_DOWN, LINK_UNKNOWN };
  enum channel_states { NEVER_CONNECTED, PREVIOUSLY_CONNECTED, CONNECTED, CLOSED, CHANNEL_UNKNOWN };
  enum ca_responses { REQUEST_SUCCESSFUL, CHANNEL_DISCONNECTED, REQUEST_FAILED };
  enum priorities { PRIORITY_LOW = CA_PRIORITY_MIN, PRIORITY_DEFAULT = CA_PRIORITY_MIN+1, PRIORITY_HIGH = CA_PRIORITY_MIN+2 };

  // Connection link state and properties.
  struct caLink {
      double searchTimeout;
      double readTimeout;
      double writeTimeout;
      link_states state;
  };

  // EPICS context state information.
  struct caContext {
      bool activated;
      int creation;
      int exception;
  };

  // Channel state and properties.
  struct caChannel {
      bool activated;
      int creation;
      int readResponse;
      int writeResponse;
      int state;
      short type;
      bool requestedElementCountSet;
      unsigned long requestedElementCount;
      unsigned long elementCount;
      chid id;
      bool writeWithCallback;
  };

  // Subscription state and properties.
  struct caSubscription {
      bool activated;
      int creation;
  };

  class CaConnection {
    public:      
      CaConnection( void* newParent );
      ~CaConnection();

      ca_responses establishContext( void (*exceptionHandler)(struct exception_handler_args), void* args );
      ca_responses establishChannel( void (*connectionHandler)(struct connection_handler_args), std::string channelName, priorities priority );

      // initialDbrStructType is for the initial read - provides values and all all meta data, e.g. DBR_CTRL_LONG
      // updateDbrStructType is for on going updates -provides value(s), status and timestamp, e.g. DBR_TIME_LONG
      ca_responses establishSubscription( void (*subscriptionHandler)(struct event_handler_args), void* args,
                                          short initialDbrStructType, short updateDbrStructType );

      void removeChannel();
      void removeSubscription(); //< NOT IMPLEMENTED

      ca_responses readChannel( void (*readHandler)(struct event_handler_args), void* args, short dbrStructType );
      ca_responses writeChannel( void (*writeHandler)(struct event_handler_args), void* args, short dbrStructType, int count, const void* dbrValue );

      void setLinkState( link_states newLinkState );
      link_states getLinkState();

      channel_states getChannelState();
      short getChannelType();    // field type as int
      chid getChannelId();
      std::string   getHostName();
      std::string   getFieldType();      // field type as string
      unsigned long getElementCount();   // number on server, as opposed to number subscribed for.
      bool getReadAccess();
      bool getWriteAccess();

      void* getParent() { return parent; }

      bool hasContext() { return context.activated; }
      bool activatedChannel() { return channel.activated; }
      bool isSubscribed() { return subscription.activated; }
      void setChannelElementCount();
      void setChannelRequestedElementCount( unsigned long requestedElementCountIn );

      void setWriteWithCallback( bool writeWithCallbackIn );
      bool getWriteWithCallback();

      bool getChannelActivated(){ return channel.activated; } // Get activated state when checking CA callbacks are OK

      unsigned long getSubscribeElementCount();

    private:
      CaRef* myRef;
      caLink link;
      caContext context;
      caChannel channel;
      caSubscription subscription;

      // Unique ID assigned to each instance.
      static int CA_UNIQUE_CONNECTION_ID;
      void* parent;

      void initialise();
      void shutdown();
      void reset();

      evid eventId;
      void (*subscriptionSubscriptionHandler)(struct event_handler_args);       // Subscription callback handler
      void* subscriptionArgs;                                                   // Data to be passed to subscription callback
      short initialDbrStructType;                                               // Data type to be used for read
      short updateDbrStructType;                                                // Data type to be used for update subscription
      static void subscriptionInitialHandler( struct event_handler_args args ); // Internal callback handler for initial subscription callback (actually a ca_get callback)
  };

}

#endif  // CACONNECTION_H_
