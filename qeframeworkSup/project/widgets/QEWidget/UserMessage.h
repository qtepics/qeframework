/*  UserMessage.h
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
 *  Copyright (c) 2012,2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef QE_USER_MESSAGE_H
#define QE_USER_MESSAGE_H

#include <QMetaType>
#include <QObject>
#include <QEFrameworkLibraryGlobal.h>
#include <QtDebug>

// Note: message_types is now a class (see below) - what was message_types is now message_severities.
//
enum message_severities {
   MESSAGE_TYPE_INFO,
   MESSAGE_TYPE_WARNING,
   MESSAGE_TYPE_ERROR
};

// These values should be or-ed together.
// Other kind values, i.e. 4, 8, 16 etc. can be added as necessary.
//
enum message_kinds {
   MESSAGE_KIND_NONE = 0,         ///< Degeneate case
   MESSAGE_KIND_EVENT = 1,        ///< Message is significant event (captured by QELog)
   MESSAGE_KIND_STATUS = 2        ///< Message is transient statue (captured QEForm/QEGui status bar)
};

typedef unsigned int  message_kind_sets;

// Standard/default kind.
const message_kind_sets MESSAGE_KIND_STANDARD = ( MESSAGE_KIND_EVENT | MESSAGE_KIND_STATUS );

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT message_types {
public:
   explicit message_types ();
   explicit message_types (message_severities severityIn,
                           message_kind_sets kind_setIn = MESSAGE_KIND_STANDARD);

   /// Function to provide string name for each message type severity
   QString getSeverityName ();

   message_severities severity;
   message_kind_sets kind_set;
};

Q_DECLARE_METATYPE (message_types)


class UserMessage;

/// Class used to send message signals.
/// Used only within UserMessage.cpp
/// A single instance of this class is shared by all instances of
/// the UserMessage class. This allows every UserMessage class instance to
/// connect to a single source of messages
class UserMessageSignal : public QObject
{
    Q_OBJECT

public:
    void sendMessage( QString msg,
                      message_types type,
                      unsigned int formId,
                      unsigned int sourceId,
                      UserMessage* originator );    ///< Send a message to all widgets based on the UserMessage class

signals:
    void message( QString msg,
                  message_types type,
                  unsigned int formId,
                  unsigned int sourceId,
                  UserMessage* originator );        ///< Emit a message signal. Any widget based on the UserMessage class can recieve these messages, filtered on formId and sourceId
};

/// Class used to receive message signals.
/// Used only within UserMessage.cpp
/// An instance of this class is created by all instances of
/// the UserMessage class. The UserMessage class uses an instance of this class
/// to receive messages so it does not have to be based on QObject itself. This is
/// required as derived classes generally need to be also based on another object
/// derived from QObject (and QObject can only be the base of a single base class)
class UserMessageSlot : public QObject
{
    Q_OBJECT

public:
//    UserMessageSlot(){}
//    ~UserMessageSlot(){}

    void setOwner( UserMessage* ownerIn ){ owner = ownerIn; }   ///< Set the UserMessage class this is a part of

public slots:
    void message( QString msg,
                  message_types type,
                  unsigned int formId,
                  unsigned int sourceId,
                  UserMessage* originator );                    ///< A message has been received

private:
    UserMessage* owner;                                         // UserMessage class that this instance is a part of
};

/**
 * A class to manage user messages.
 *
 * This class passes messages between widgets and application code
 *
 * This class is used as a base class.
 *
 * Messages are sent by calling sendMessage()
 * Messages are received by implementing newMessage() in the derived class.
 *
 * Messages can be filtered based on a source ID or a form ID
 *
 * The derived widget is free to set the source ID to any value
 *
 * Derived form widgets (QEForm) get a unique form ID using getNextMessageFormId()
 * (as well as being able to set a source ID like any other QE widget) and pass
 * this unique form ID to all widgets within the form using the ContainerProfile class.
 *
 * Messages sent by a QE widget are received by all QE widgets and can filter the messages
 * required by form ID and source ID.
 * The form ID is under the management of the QEForm widget, the source ID is under
 * the control of the GUI designer.
 *
 * The QEForm widget does not display messages, but re-send them using its own
 * form ID. Read on to see how this can be used.
 *
 * Widgets that generate messages, and widgets (or application code) that use messages
 * can be set up as follows:
 *
 * - Application wide logging:
 *   An application with a single log window can can base a class on the UserMessage class
 *   and set up filtering to receive all messages.
 *   An application with log messages for seperate windows containing QEForm
 *   widgets (such as QEGui) can base each window class on the UserMessage class, then
 *   set up filtering for the appropriate form ID.
 *
 * - Logging within a QEForm.
 *   A logging widget can be set to filter matching on the current form and so will pick up
 *   messages from any sibling widget. This includes messages from a sibling widget which is
 *   a nested QEForm. Whatever messages that nested form is set to receive, it will resend
 *   to its siblings. For example, if it is set to receive messages from the widgets it
 *   contains, these are resent up one level to the main form.
 *   If messages are dealt with within the nested QEForm (for example, it may have its own
 *   logging QE widget) then the nested QEForm could be set up not to filter and resend any messages.
 */

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT UserMessage
{
public:
    friend class UserMessageSlot;
    friend class UserMessageSignal;

    enum message_filter_options {MESSAGE_FILTER_ANY, MESSAGE_FILTER_MATCH, MESSAGE_FILTER_NONE };
    UserMessage();
    virtual ~UserMessage();

    void setSourceId( unsigned int sourceId );                    ///< Set the source ID (the ID set up by the GUI designer, usually matched to the source ID of logging widgets)
    void setFormId( unsigned int formId );                        ///< Set the form ID (the the same ID for all sibling widgets within an QEForm widget)
    void setFormFilter( message_filter_options formFilterIn );    ///< Set the message filtering applied to the form ID
    void setSourceFilter( message_filter_options sourceFilterIn );///< Set the message filtering applied to the source ID

    unsigned int getSourceId() const;                           ///< Get the source ID (the ID set up by the GUI designer, usually matched to the source ID of logging widgets
    unsigned int getFormId() const;                             ///< Get the form ID (the the same ID for all sibling widgets within an QEForm widget)
    message_filter_options getFormFilter() const;               ///< Get the message filtering applied to the form ID
    message_filter_options getSourceFilter() const;             ///< Get the message filtering applied to the source ID

    void setChildFormId( unsigned int );                        ///< Set the for ID of all widgets that are children of this widget
    unsigned int getChildFormId() const;                        ///< Get the for ID of all widgets that are children of this widget

    unsigned int getNextMessageFormId() const;                  ///< Generate a new form ID for all widgets in a new form

    void sendMessage( QString message,
                      message_types type = message_types (MESSAGE_TYPE_INFO) ); ///< Send a message to the user
    void sendMessage( QString message,
                      QString source,
                      message_types type = message_types (MESSAGE_TYPE_INFO) ); ///< Send a message to the user with a source reference

    QString getMessageTypeName( message_types type );           ///< Convenience function to provide string names for each message type

    virtual void newMessage( QString, message_types );          ///< Virtual function to pass messages to derived classes (typicaly logging widgets or application windows)


private:
    static UserMessageSignal userMessageSignal;                 // Single object to send all message signals
    static unsigned int nextMessageFormId;                      // The next message form ID
    unsigned int formId;                                        // The form ID passed with each message. Shared by all widgets within an QEForm widget
    unsigned int sourceId;                                      // The source ID passed with each message. Set to any value the GUI designer requires.
    UserMessageSlot userMessageSlot;                            // QObject based object to receive all messages. It calls newMessage() with each message.

    unsigned int childFormId;                                   // Only relevent for form (QEForm) widgets. Form ID of all child widgets
    message_filter_options formFilter;                          // Message filtering to apply to form ID
    message_filter_options sourceFilter;                        // Message filtering to apply to source ID
};

#endif // QE_USER_MESSAGE_H
