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

#ifndef QCASTATEMACHINE_H_
#define QCASTATEMACHINE_H_

#include <QtDebug>
#include <QMutex>

class StateMachineTemplate {
  public:
    virtual ~StateMachineTemplate(){}
    int currentState;
    int requestState;
    virtual bool process( int requestedState ) = 0;
};

namespace qcastatemachine {

  enum connection_states { DISCONNECTED, CONNECTED, CONNECT_FAIL, CONNECTION_EXPIRED };
  enum subscription_states { UNSUBSCRIBED, SUBSCRIBED, SUBSCRIBED_READ, SUBSCRIBE_SUCCESS, SUBSCRIBE_FAIL };
  enum read_states { READ_IDLE, READING, READING_FAIL };
  enum writing_states { WRITE_IDLE, WRITING, WRITING_FAIL };

  class QCaStateMachine : public StateMachineTemplate {
    public:
      QCaStateMachine( void *parent );
      virtual ~QCaStateMachine(){}
      QMutex lock;
      bool pending;
      bool active;
      bool expired;
      void *myWorker;
      virtual bool process( int requestedState ) = 0;
  };

  class ConnectionQCaStateMachine : public QCaStateMachine {
    public:
      ConnectionQCaStateMachine( void *parent );
      ~ConnectionQCaStateMachine();
      bool process( int requestedState );

      static int disconnectedCount;
      static int connectedCount;
  };

  class SubscriptionQCaStateMachine : public QCaStateMachine {
    public:
      SubscriptionQCaStateMachine( void *parent );
      ~SubscriptionQCaStateMachine(){}
      bool process( int requestedState );
  };

  class ReadQCaStateMachine : public QCaStateMachine {
    public:
      ReadQCaStateMachine( void *parent );
      ~ReadQCaStateMachine(){}
      bool process( int requestedState );
  };

  class WriteQCaStateMachine : public QCaStateMachine {
    public:
      WriteQCaStateMachine( void *parent );
      ~WriteQCaStateMachine(){}
      bool process( int requestedState );
  };

}

#endif // QCASTATEMACHINE_H_
