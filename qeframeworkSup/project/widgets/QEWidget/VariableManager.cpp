/*  VariableManager.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2015-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/*
  This class is used as a base for QEWidget and provides services for managing CA/PVA process
  variable connections. Refer to VariableManager.h for a full class description
 */

#include "VariableManager.h"
#include <QDebug>
#include <QECommon.h>

#define DEBUG qDebug () << "VariableManager" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
// Constructor
//
VariableManager::VariableManager ()
{
   // Initially flag no variables array is defined.
   // This will be corrected when the first variable is declared.
   //
   this->clearQcaItems();
}

//------------------------------------------------------------------------------
// Destruction:
// Delete all variable sources for the widgeet
//
VariableManager::~VariableManager ()
{
   // Delete all the QEChannel instances.
   //
   this->clearQcaItems();
}

//------------------------------------------------------------------------------
// Deallocate and free all QEChannels, i.e. does a deep clear.
// Smart pointers would have been nice.
//
void VariableManager::clearQcaItems()
{
   const int number = this->channelList.size();
   for (int i = 0; i < number; i++) {
      QEChannel* qca = this->channelList.value (i, NULL);
      if (qca) qca->deleteLater();
   }
   this->channelList.clear();
}

//------------------------------------------------------------------------------
// Set the number of variables that will be used for this widget.
// Create an array of QEChannel based objects to suit.
// This is called by the CA/PVA aware widgets based on this class, such as a QELabel.
//
void VariableManager::setNumVariables (const unsigned int numVariablesIn)
{
   // Get the number of variables that will be used by this widget
   // Don't accept zero or the qca array will be invalid.
   //
   const unsigned int numVariables = MAX (1, numVariablesIn);

   // Set up the number of variables managed by the variable name manager.
   //
   this->variableNameManagerInitialise (numVariables);

   // Allocate/extend the array of channel objects.
   //
   while (this->channelList.size() < int(numVariables)) {
      this->channelList.append (NULL);   // Add place holder NULL value
   }
}

//------------------------------------------------------------------------------
//
unsigned int VariableManager::getNumVariables () const
{
   return this->channelList.size();
}

//------------------------------------------------------------------------------
// Initiate updates.
// This is only required when QE widgets are loaded within a form and not directly by 'designer'.
// When loaded directly by 'designer' they are activated (a CA/PVA connection is established) as
// soon as either the variable name or variable name substitution properties are set
//
void VariableManager::activate ()
{
   // For each variable, ask the CA/PVA aware widget based on this class to initiate updates
   // and to set up whatever signal/slot connections are required to make use of data updates.
   // Note, establish connection is a virtual function of the VariableNameManager class
   // and is normally called by that class when a variable name is defined or changed.
   //
   const unsigned int number = this->channelList.size();
   for (unsigned int i = 0; i < number; i++) {
      this->establishConnection (i);
   }

   // Ask the widget to perform any tasks which should only be done once all other widgets
   // have been created.  For example, if a widget wants to notify other widgets through
   // signals during construction, other widgets may not be present yet to recieve the
   // signals. This type of notification could be held off untill now.
   //
   this->activated ();
}

//------------------------------------------------------------------------------
// Terminate updates.
// This has been provided for third party (non QEGui) applications using the framework.
// Specifically, this is used by kubili.
//
void VariableManager::deactivate ()
{
   // Ask the widget to perform any tasks which should done prior to being deactivated.
   //
   this->deactivated ();

   // Delete all the QEChannel instances.
   //
   const unsigned int number = this->channelList.size();
   for (unsigned int i = 0; i < number; i++) {
      this->deleteQcaItem (i, true);
   }
}

//------------------------------------------------------------------------------
// Create a CA/PVA connection and initiates updates if required.
// This is called by the establishConnection function of CA/PVA aware widgets based on
// this class, such as a QELabel. If successfull it will return the QEChannel based
// object supplying data update signals.
//
QEChannel* VariableManager::createVariable (const unsigned int variableIndex,
                                            const bool do_subscribe)
{
   // Return NULL if invalid or has never been set up.
   //
   const unsigned int number = this->channelList.size();
   if (variableIndex >= number) {
      return NULL;
   }

   // Remove any existing CA/PVA connection.
   //
   this->deleteQcaItem (variableIndex, false);

   // Connect to new variable.
   // If a new variable name is present, ask the CA/PVA aware widget based on this class to create an
   // appropriate object based on a QEChannel (by calling its createQcaItem() function).
   // If that is successfull, supply it with a mechanism for handling errors and subscribe
   // to the new variable if required.
   //
   const QString pvName = this->getSubstitutedVariableName (variableIndex);
   if (pvName.length () > 0) {
      QEChannel* qca = this->createQcaItem (variableIndex);
      this->channelList.replace (variableIndex, qca);
      if (qca) {
         qca->setUserMessage ((UserMessage *) this);
         if (do_subscribe) {
            qca->subscribe ();       // connect and subscribe
         } else {
            qca->connectChannel ();  // just connect
         }
      }
   }

   // Return the QEChannel, if any.
   //
   return this->channelList.value (variableIndex, NULL);
}

//------------------------------------------------------------------------------
// Default implementation of createQcaItem().
// Usually a QE widgets will request a connection be established by this class and this class will
// call back the QE widgets for it to create the specific flavour of QEChannel required using this function.
// Since this class can also be used as a base class for widgets that don't establish any CA/PVA connection,
// this default implementation is here to always return NULL when asked to create a QEChannel.
//
QEChannel* VariableManager::createQcaItem (unsigned int)
{
   return NULL;
}

//------------------------------------------------------------------------------
// Default implementation of establishConnection().
// Usually a QE widgets will request a connection be established by this class and this class will
// call back the QE widgets for it to establish a connection on a newly created QEChannel using this function.
// Since this class can also be used as a base class for widgets that don't establish any CA/PVA connection,
// this default implementation is here as a default when not implemented
//
void VariableManager::establishConnection (unsigned int) { }

//------------------------------------------------------------------------------
// Default implementation of activated().
// Widgets may have tasks which should only be done once all other widgets have been created.
// For example, if a widget wants to notify other widgets through signals during construction, other widgets
// may not be present yet to recieve the signals. This type of notification could be held off untill now.
//
void VariableManager::activated () { }

//------------------------------------------------------------------------------
// Default implementation of deactivated().
//
void VariableManager::deactivated () { }

//------------------------------------------------------------------------------
// Return a reference to one of the QEChannels used to stream CA/PVA data updates to the widget
// This is called by CA/PVA aware widgets based on this class, such as a QELabel, mainly when they
// want to connect to its signals to recieve data updates.
//
QEChannel* VariableManager::getQcaItem (unsigned int variableIndex) const
{
   // Return the QEChannel used for the specified variable name, or
   // return NUMM if index is invalid or has never been set up.
   //
   return this->channelList.value (variableIndex, NULL);
}

//------------------------------------------------------------------------------
// Remove any previous QEChannel created to supply CA/PVA data updates for a variable name
// If the object connected to the QEChannel is being destroyed it is not good to receive signals
// so the disconnect parameter should be true in this case.
//
void VariableManager::deleteQcaItem (const unsigned int variableIndex,
                                     const bool disconnect)
{
   // If the index is invalid do nothing.
   //
   const unsigned int number = this->channelList.size();
   if (variableIndex >= number)
      return;

   // Remove the reference to the deleted object to prevent accidental use.
   //
   QEChannel* qca = this->channelList.value (variableIndex, NULL);
   this->channelList.replace (variableIndex, NULL);

   // Delete the QEChannel used for the specified variable name.
   //
   if (qca) {
      // If we need to disconnect first, do so.
      // If the object connected is being destroyed it is not good to receive signals. (this happened)
      // If the object connected is not being destroyed is will want to know a disconnection has occured.
      if (disconnect) {
         qca->disconnect ();
      }

      // Delete the QEChannel
      /// Note: we use deleteLater() here (and above). This is because if a
      /// widget calls (re-)establishConnection from within a slot function
      /// responding to a signal from this QEChannel object (as does QEImage),
      /// this can often cause a segmentation fault.
      // Consider jist reusing the exiting QEChannel object.
      qca->deleteLater();
   }
}

//------------------------------------------------------------------------------
// Perform a single shot read on all variables.
// Widgets may be write only and do not need to subscribe (subscribe property is false).
// When not subscribing it may still be usefull to do a single shot read to get initial
// values, or perhaps confirm a write.
//
void VariableManager::readNow ()
{
   // Perform a single shot read on all variables.
   //
   const unsigned int number = this->channelList.size();
   for (unsigned int i = 0; i < number; i++) {
      QEChannel* qca = this->getQcaItem (i);
      if (qca) {                  // If variable exists...
         qca->singleShotRead ();
      }
   }
}

//------------------------------------------------------------------------------
// Provides default implementation of writeNow.
//
void VariableManager::writeNow ()
{
   DEBUG << "This method should be overridden";
}

//------------------------------------------------------------------------------
// Return references to the current count of disconnections.
// The plugin library (and therefore the static connection and disconnection counts)
// can be mapped twice (on Windows at least). So it is no use just referencing these
// static variables from an application if the widgets of interest have been created
// by the UI Loader. This function can be called on any widget loaded by the UI loader
// and the reference returned can be used to get counts for all widgets loaded by the
// UI loader.
//
int* VariableManager::getDisconnectedCountRef () const
{
   return QEChannel::getDisconnectedCountRef ();
}

//------------------------------------------------------------------------------
// Return references to the current count of connections.
// The plugin library (and therefore the static connection and disconnection counts)
// can be mapped twice (on Windows at least). So it is no use just referencing these
// static variables from an application if the widgets of interest have been created
// by the UI Loader. This function can be called on any widget loaded by the UI loader
// and the reference returned can be used to get counts for all widgets loaded by the
// UI loader.
//
int* VariableManager::getConnectedCountRef () const
{
   return QEChannel::getConnectedCountRef ();
}

// end
