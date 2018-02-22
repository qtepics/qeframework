/*  VariableManager.cpp
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
 *  Copyright (c) 2015,2016,2018 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
  This class is used as a base for QEWidget and provides services for
  managing CA process variable connections. Refer to VariableManager.h for a full class description
 */

#include <VariableManager.h>
#include <QCaObject.h>

// Constructor
//
VariableManager::VariableManager()
{
    // Initially flag no variables array is defined.
    // This will be corrected when the first variable is declared
    numVariables = 0;
    qcaItem = 0;
}

// Destruction:
// Delete all variable sources for the widgeet
//
VariableManager::~VariableManager()
{
    // Delete all the QCaObject instances
    for( unsigned int i = 0; i < numVariables; i++ ) {
        deleteQcaItem( i, true );
    }

    // Release the list
    delete[] qcaItem;
    qcaItem = NULL;
}

// Set the number of variables that will be used for this widget.
// Create an array of QCaObject based objects to suit.
// This is called by the CA aware widgets based on this class, such as a QELabel.
//
void VariableManager::setNumVariables( unsigned int numVariablesIn )
{

    // Get the number of variables that will be used by this widget
    // Don't accept zero or the qca array will be invalid
    if( numVariablesIn ) {
        numVariables = numVariablesIn;
    } else {
        numVariables = 1;
    }

    // Set up the number of variables managed by the variable name manager
    variableNameManagerInitialise( numVariables );

    // Allocate the array of QCa objects
    qcaItem = new qcaobject::QCaObject* [numVariables];
    for( unsigned int i = 0; i < numVariables; i++ ) {
        qcaItem[i] = NULL;
    }
}

// Initiate updates.
// This is only required when QE widgets are loaded within a form and not directly by 'designer'.
// When loaded directly by 'designer' they are activated (a CA connection is established) as soon as either
// the variable name or variable name substitution properties are set
//
void VariableManager::activate()
{
    // For each variable, ask the CA aware widget based on this class to initiate updates and to set up
    // whatever signal/slot connections are required to make use of data updates.
    // Note, establish connection is a virtual function of the VariableNameManager class and is normally
    // called by that class when a variable name is defined or changed
    for( unsigned int i = 0; i < numVariables; i++ )
        establishConnection( i );

    // Ask the widget to perform any tasks which should only be done once all other widgets have been created.
    // For example, if a widget wants to notify other widgets through signals during construction, other widgets
    // may not be present yet to recieve the signals. This type of notification could be held off untill now.
    activated();
}

// Terminate updates.
// This has been provided for third party (non QEGui) applications using the framework.
//
void VariableManager::deactivate()
{
    // Ask the widget to perform any tasks which should done prior to being deactivated.
    deactivated();

    // Delete all the QCaObject instances
    for( unsigned int i = 0; i < numVariables; i++ ) {
        deleteQcaItem( i, false );
    }
}


// Create a CA connection and initiates updates if required.
// This is called by the establishConnection function of CA aware widgets based on this class, such as a QELabel.
// If successfull it will return the QCaObject based object supplying data update signals
//
qcaobject::QCaObject* VariableManager::createVariable( unsigned int variableIndex,
                                                       const bool do_subscribe )
{

    // If the index is invalid do nothing
    // This same test is also valid if qcaItem has never been set up yet as numVariables will be zero
    if( variableIndex >= numVariables ) {
        return NULL;
    }

    // Remove any existing QCa connection
    deleteQcaItem( variableIndex, false );

    // Connect to new variable.
    // If a new variable name is present, ask the CA aware widget based on this class to create an
    // appropriate object based on a QCaObject (by calling its createQcaItem() function).
    // If that is successfull, supply it with a mechanism for handling errors and subscribe
    // to the new variable if required.
    if( getSubstitutedVariableName( variableIndex ).length() > 0 ) {
        qcaItem[variableIndex] = createQcaItem( variableIndex );
        if( qcaItem[variableIndex] ) {

            qcaItem[variableIndex]->setUserMessage( (UserMessage*)this );

            if( do_subscribe )
                qcaItem[variableIndex]->subscribe();
        }
    }

    // Return the QCaObject, if any
    return qcaItem[variableIndex];
}

// Default implementation of createQcaItem().
// Usually a QE widgets will request a connection be established by this class and this class will
// call back the QE widgets for it to create the specific flavour of QCaObject required using this function.
// Since this class can also be used as a base class for widgets that don't establish any CA connection,
// this default implementation is here to always return NULL when asked to create a QCaObject.
//
qcaobject::QCaObject* VariableManager::createQcaItem( unsigned int )
{
    return NULL;
}

// Default implementation of establishConnection().
// Usually a QE widgets will request a connection be established by this class and this class will
// call back the QE widgets for it to establish a connection on a newly created QCaObject using this function.
// Since this class can also be used as a base class for widgets that don't establish any CA connection,
// this default implementation is here as a default when not implemented
//
void VariableManager::establishConnection( unsigned int )
{
}

// Default implementation of activated().
// Widgets may have tasks which should only be done once all other widgets have been created.
// For example, if a widget wants to notify other widgets through signals during construction, other widgets
// may not be present yet to recieve the signals. This type of notification could be held off untill now.
//
void VariableManager::activated()
{
}

//------------------------------------------------------------------------------
// Default implementation of deactivated().
//
void VariableManager::deactivated ()
{
}

// Return a reference to one of the qCaObjects used to stream CA data updates to the widget
// This is called by CA aware widgets based on this class, such as a QELabel, mainly when they
// want to connect to its signals to recieve data updates.
qcaobject::QCaObject* VariableManager::getQcaItem( unsigned int variableIndex ) const {
    // If the index is invalid return NULL.
    // This same test is also valid if qcaItem has never been set up yet as numVariables will be zero
    if( variableIndex >= numVariables )
        return NULL;

    // Return the QCaObject used for the specified variable name
    return qcaItem[variableIndex];
}

// Remove any previous QCaObject created to supply CA data updates for a variable name
// If the object connected to the QCaObject is being destroyed it is not good to receive signals
// so the disconnect parameter should be true in this case
//
void VariableManager::deleteQcaItem( unsigned int variableIndex, bool disconnect ) {
    // If the index is invalid do nothing.
    // This same test is also valid if qcaItem has never been set up yet as numVariables will be zero
    if( variableIndex >= numVariables )
        return;

    // Remove the reference to the deleted object to prevent accidental use
    qcaobject::QCaObject* qca = qcaItem[variableIndex];
    qcaItem[variableIndex] = NULL;

    // Delete the QCaObject used for the specified variable name
    if( qca )
    {
        // If we need to disconnect first, do so.
        // If the object connected is being destroyed it is not good to receive signals. (this happened)
        // If the object connected is not being destroyed is will want to know a disconnection has occured.
        if( disconnect )
        {
            qca->disconnect();
        }

        // Delete the QCaObject
        delete qca;
    }
}


// Perform a single shot read on all variables.
// Widgets may be write only and do not need to subscribe (subscribe property is false).
// When not subscribing it may still be usefull to do a single shot read to get initial
// values, or perhaps confirm a write.
//
void VariableManager::readNow()
{
    // Perform a single shot read on all variables.
    qcaobject::QCaObject* qca;
    for( unsigned int i = 0; i < numVariables; i++ )
    {
        qca = getQcaItem( i );
        if( qca ) // If variable exists...
        {
            qca->singleShotRead();
        }
    }
}

//------------------------------------------------------------------------------
// Provides default implementation of writeNow.
//
void VariableManager::writeNow()
{
    qDebug() << "default VariableManager::writeNow - this function should be overridden";
}

using namespace qcastatemachine;

// Return references to the current count of disconnections.
// The plugin library (and therefore the static connection and disconnection counts)
// can be mapped twice (on Windows at least). So it is no use just referencing these
// static variables from an application if the widgets of interest have been created
// by the UI Loader. This function can be called on any widget loaded by the UI loader
// and the reference returned can be used to get counts for all widgets loaded by the
// UI loader.
//
int* VariableManager::getDisconnectedCountRef() const
{
    return &ConnectionQCaStateMachine::disconnectedCount;
}

// Return references to the current count of connections.
// The plugin library (and therefore the static connection and disconnection counts)
// can be mapped twice (on Windows at least). So it is no use just referencing these
// static variables from an application if the widgets of interest have been created
// by the UI Loader. This function can be called on any widget loaded by the UI loader
// and the reference returned can be used to get counts for all widgets loaded by the
// UI loader.
//
int* VariableManager::getConnectedCountRef() const
{
    return &ConnectionQCaStateMachine::connectedCount;
}

// end
