/*  UserMessage.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/* Description:
 *
 * A class to manage user messages.
 *
 * This class passes messages between widgets and application code
 *
 * This class is used as a base class.
 *
 *  Refer to the class description in UserMessage.h for further details
 */

#include "UserMessage.h"
#include <QDebug>
#include <QELog.h>

#define DEBUG qDebug () << "QEDragDrop" << __LINE__ << __FUNCTION__ << "  "

// Static variables used to manage message signals and slots.
UserMessageSignal UserMessage::userMessageSignal;
unsigned int UserMessage::nextMessageFormId = 1;


//------------------------------------------------------------------------------
// Construction
message_types::message_types()
{
    kind_set = MESSAGE_KIND_NONE;
    severity = MESSAGE_TYPE_INFO;
}

//------------------------------------------------------------------------------
message_types::message_types( message_severities severityIn,
                              message_kind_sets kind_setIn )
{
    severity = severityIn;
    kind_set = kind_setIn;
}

//------------------------------------------------------------------------------
// Convenience function to provide string names for each message type
QString message_types::getSeverityName() {
    switch( severity )
    {
        case MESSAGE_TYPE_INFO:    return "Information";
        case MESSAGE_TYPE_WARNING: return "Warning";
        case MESSAGE_TYPE_ERROR:   return "Error";
    }
    return "";
}


//------------------------------------------------------------------------------
// Construction
UserMessage::UserMessage()
{
    // Initialise
    formId = 0;
    sourceId = 0;
    formFilter = QE::None;
    sourceFilter = QE::Any;  // Note, default of QE::Any means default implementation of
                             // newMessage() will be called which cancels future unwanted messages.
                             // An alternate default of MESSAGE_FILTER_NONE would mean newMEssage()
                             // is never called (good), but the opportunity to cancel future signals
                             // for uninterested widgets would be lost (bad)

    childFormId = 0;

    // Allow the object receiving messages to pass them back to us
    userMessageSlot.setOwner( this );

    // Need to register the type prior to any connections as this used in inter-thread slignals.
    qRegisterMetaType<message_types> ("message_types");

    // Establish the connection between the common message signaler, and this instance's message slot
    QObject::connect( &userMessageSignal, SIGNAL( message( QString, message_types, unsigned int, unsigned int, UserMessage* ) ),
                      &userMessageSlot, SLOT( message( QString, message_types, unsigned int, unsigned int, UserMessage* ) ) );

    // Create the QELog master message receiver.
    //
    QELog::createUserMessageReceiver();  // static
}

//------------------------------------------------------------------------------
// Destruction
UserMessage::~UserMessage()
{
}

//------------------------------------------------------------------------------
// Set the source ID
// (the ID set up by the GUI designer, usually matched to the source ID of logging widgets)
void UserMessage::setSourceId( unsigned int sourceIdIn )
{
    sourceId = sourceIdIn;
}

//------------------------------------------------------------------------------
// Set the form ID (the the same ID for all sibling widgets within an QEForm widget)
void UserMessage::setFormId( unsigned int formIdIn )
{
    formId = formIdIn;
}

//------------------------------------------------------------------------------
// Set the message filtering applied to the form ID
void UserMessage::setFormFilter( QE::MessageFilterOptions formFilterIn )
{
    formFilter = formFilterIn;
}

//------------------------------------------------------------------------------
// Set the message filtering applied to the source ID
void UserMessage::setSourceFilter( QE::MessageFilterOptions sourceFilterIn )
{
    sourceFilter = sourceFilterIn;
}

//------------------------------------------------------------------------------
// Get the source ID (the ID set up by the GUI designer, usually matched to the source ID of logging widgets
unsigned int UserMessage::getSourceId() const
{
    return sourceId;
}

//------------------------------------------------------------------------------
// Get the form ID (the the same ID for all sibling widgets within an QEForm widget)
unsigned int UserMessage::getFormId() const
{
    return formId;
}

//------------------------------------------------------------------------------
// Get the message filtering applied to the form ID
QE::MessageFilterOptions UserMessage::getFormFilter() const
{
    return formFilter;
}

//------------------------------------------------------------------------------
// Get the message filtering applied to the source ID
QE::MessageFilterOptions UserMessage::getSourceFilter() const
{
    return sourceFilter;
}

//------------------------------------------------------------------------------
// Set the ID for of all widgets that are children of this widget
void UserMessage::setChildFormId( unsigned int childFormIdIn )
{
    childFormId = childFormIdIn;
}

//------------------------------------------------------------------------------
// Get the ID for of all widgets that are children of this widget
unsigned int UserMessage::getChildFormId() const
{
    return childFormId;
}

//------------------------------------------------------------------------------
// Generate a new form ID for all widgets in a new form
unsigned int UserMessage::getNextMessageFormId() const
{
    return nextMessageFormId++;
}

//------------------------------------------------------------------------------
// Convenience function to provide string names for each message type
QString UserMessage::getMessageTypeName( message_types type )
{
    return type.getSeverityName();
}


//------------------------------------------------------------------------------
// Send a message to the user.
// A string containing the message and a string containing information as
// to the source of the message is required
void UserMessage::sendMessage( QString message,
                               QString source,
                               message_types type )
{
    // Combine message and source and call base send message function
    sendMessage( QString("%1 (Source %2)").arg(message).arg(source), type );
}

//------------------------------------------------------------------------------
// Send a message to the user.
// A string containing the message is required
void UserMessage::sendMessage( QString msg,
                               message_types type )
{
    // Send the message
    userMessageSignal.sendMessage( msg, type, formId, sourceId, this );
}

//------------------------------------------------------------------------------
// Emit a signal to all other user message classes
// Note, there is only a single UserMessageSignal class instance
void UserMessageSignal::sendMessage( QString msg,
                                     message_types type,
                                     unsigned int formId,
                                     unsigned int sourceId,
                                     UserMessage* originator )
{
    emit message( msg, type, formId, sourceId, originator );
}

//------------------------------------------------------------------------------
// Receive a signal from another message class
// Note, there is a UserMessageSlot for every UserMessage class instance
void UserMessageSlot::message( QString msg,
                               message_types type,
                               unsigned int messageFormId,
                               unsigned int messageSourceId,
                               UserMessage* originator )
{
    // Ignore our own messages
    if( originator == owner )
    {
        return;
    }

    // If filter matches, use it.
    if(( owner->formFilter == QE::Any ) ||                                              // Always match on any form ID
       ( owner->formFilter ==  QE::Match && owner->childFormId == messageFormId ) ||    // Match only on specific form ID
       ( owner->sourceFilter == QE::Any ) ||                                            // Always match on any source ID
       ( owner->sourceFilter == QE::Match && owner->getSourceId() == messageSourceId )) // Match only on specific source ID
    {
        owner->newMessage( msg, type );
    }
}

//------------------------------------------------------------------------------
// Default implementation of virtual function to pass messages to derived classes (typicaly logging widgets or application windows)
// If this default function is called it means the widget is not going to receive any messages and so there is no need to be receiving signals.
// Since only a few widgets will be interested in messages, disconnecting uninterested widgets from message signals will reduce the
// message signal count significantly.
// In summarey if this function is called, there is no re implementation by a derived class, so signals can be disconnected.
void UserMessage::newMessage( QString, message_types )
{
    // Disconnect. If this default implementation of useMessage is called, no derived class is interested in messages
    QObject::disconnect( &userMessageSignal, SIGNAL( message( QString, message_types, unsigned int, unsigned int, UserMessage* ) ),
                         &userMessageSlot,   SLOT(   message( QString, message_types, unsigned int, unsigned int, UserMessage* ) ) );
}

// end
