/*  QCaVariableNamePropertyManager.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2009-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/* Description:
 *
 * When QEWidgets are used within 'designer' they need to request CA data whenever a
 * variable name or macro substitution changes.
 * This may be when a user is typing into the variable name or macro substitution fields.
 * In these cases the appropriate 'set property' function is called with every key stoke
 * resulting in many unwanted requests being initiated then canceled, and possibly also invalid variable name error messages.
 * To avoid this, this class notes changes to these properties but only triggers a request when changes
 * have stopped occuring for a reasonable time.
 * The result is a user can type a variable name, and once they stop typing the data for the completed
 * variable name is requested.
 *
 * NOTE, if the QE widgets plugin has been loaded by an application other than 'designer', such as a gui application,
 * then the properties will all be set once by the UI file loader. In this case data should be requested
 * once all properties have been set. As there is no way to know within a single 'properties set' function
 * if there are more 'property set' functions still to be called by the UI loader, a request for data should be
 * initiated by the code calling the UI loader after the UI loader has returned.
 * Therefore this class needs to know if the properties are being entered by a user, in which case the data request
 * should be made a short time after user changes stop, or if the properties are being set by loading a UI file,
 * in which case this code should not initiate data updates at all.
 * This knowledge is infered by checking if there is an envirionmental profile defined. All code loading a
 * UI file should create and setup an instance of the ContainerProfile class. If this has been done then it is
 * safe to assume the properties are not being set by a user tying into 'designer'.
 *
 * This class calls the virtual
 * function establishConnection() whenever a translated variable name is updated
 */

#include <QCaVariableNamePropertyManager.h>
#include <QDebug>
#include <QEWidget.h>

#define WAIT_FOR_TYPING_TO_FINISH 1000  // One Second

/*
    Construction
*/
QCaVariableNamePropertyManager::QCaVariableNamePropertyManager() {

    // If a container profile has been defined, then this widget isn't being created within designer,
    // so flag the variable name and substitutions are not being modified interactively.
    // If a user is not modifying the variable name or macro substitutions there is no need for
    // the variable name property name manager to wait for a user to finish typing before using a variable name.
    interactive = QEWidget::inDesigner();

    // Setup a timer so rapid changes to the variable name property are ignored.
    // Only after the user has stopped typing for a while will the entry be used.
    // The timer will be set on the first keystroke and reset with each subsequent keystroke
    // untill the keystrokes stop for longer than the timeout period.
    // Note, timers are not required if there is no user entering variable names of macro substitutions.
    if( interactive )
    {
        setSingleShot( true );
        QObject::connect( this, SIGNAL( timeout() ), this, SLOT( subscribeDelayExpired() ) );
    }

    // Default to first (and only) variable
    variableIndex = 0;
}

/*
    Set the variable index.
    Used when multiple variables can affect an object.
*/
void QCaVariableNamePropertyManager::setVariableIndex( unsigned int variableIndexIn ) {
    variableIndex = variableIndexIn;
}

/*
    Get the variable index.
*/
unsigned int QCaVariableNamePropertyManager::getVariableIndex() const
{
    return variableIndex;
}

/*
    Set the Variable Name property
    This changes with every keystroke.
    Store the new value but don't subscribe yet.
    Instead, set a timer that will expire only when changes stop occuring for
    a while.
*/
void QCaVariableNamePropertyManager::setVariableNameProperty( QString variableNamePropertyIn ) {

    // If the variable name has changed as a result of a user typing, save it and
    // set (or reset) a timer to complete when changes stop occuring.
    // The timer will signal subscribeDelayExpired()
    // If the change was not interactive, use the change immedietly.
    if( variableNameProperty != variableNamePropertyIn ) {
        variableNameProperty = variableNamePropertyIn;
        if( interactive )
            start( WAIT_FOR_TYPING_TO_FINISH );
        else
            subscribeDelayExpired();
    }
}

/*
    Set the Variable Name Substitutions property.
    This changes with every keystroke.
    Store the new value but don't subscribe yet.
    Instead, set a timer that will expire only when changes stop occuring for
    a while.
*/
void QCaVariableNamePropertyManager::setSubstitutionsProperty( QString substitutionsPropertyIn ) {
    // If the substitutions have changed as a result of a user typing, save them and
    // set (or reset) a timer to complete when changes stop occuring.
    // The timer will signal subscribeDelayExpired()
    // If the change was not interactive, use the change immedietly.
    if( substitutionsProperty != substitutionsPropertyIn ) {
        substitutionsProperty = substitutionsPropertyIn;
        if( interactive )
            start( WAIT_FOR_TYPING_TO_FINISH );
        else
            subscribeDelayExpired();
    }
}

/*
    Return the Variable Name property.
*/
QString QCaVariableNamePropertyManager::getVariableNameProperty() const {
    return variableNameProperty;
}

/*
    Return the Variable Name Substitutions property.
*/
QString QCaVariableNamePropertyManager::getSubstitutionsProperty() const {
    return substitutionsProperty;
}

/*
    Subscribe to an an updated variable name.
    The variable name property is changed by the user with every keystroke.
    A timer expires (and this method is called) if keystrokes have not
    occured for a while implying the user has completed entering the
    variable name.
*/
void QCaVariableNamePropertyManager::subscribeDelayExpired() {
    emit newVariableNameProperty( variableNameProperty, substitutionsProperty, variableIndex );
}

// end
