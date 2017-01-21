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
 *  Copyright (c) 2012 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
  This class adds common property support to all QE widgets where the support involves interaction with the QWidget.
  Any common property support that requires a greater scope than the QWidget is managed in QEWidget.
*/

#include <standardProperties.h>
#include <QEWidget.h>
//#include <QCoreApplication>
//#include <QFileInfo>


// Construction.
standardProperties::standardProperties( QWidget* ownerIn )
{
    // Sanity check.
    if( ownerIn == NULL )
    {
        qWarning( "standardProperties constructor called with a null 'owner'" );
        exit( EXIT_FAILURE );
    }

    // Keep a handle on the underlying QWidget of the QE widgets
    owner = ownerIn;

    ContainerProfile profile;

    visibilityLevel = userLevelTypes::USERLEVEL_USER;
    enabledLevel = userLevelTypes::USERLEVEL_USER;
    currentLevel = profile.getUserLevel();

    userLevelDisabled = false;
    applicationVisibility = true;

    displayAlarmState = DISPLAY_ALARM_STATE_ALWAYS;
}

// !!
userLevelTypes::userLevels standardProperties::getUserLevelVisibility()
{
    return visibilityLevel;
}

void standardProperties::setUserLevelVisibility( userLevelTypes::userLevels levelIn )
{
    visibilityLevel = levelIn;
    setSuperVisibility();
}

userLevelTypes::userLevels standardProperties::getUserLevelEnabled()
{
    return enabledLevel;
}

void standardProperties::setUserLevelEnabled( userLevelTypes::userLevels levelIn )
{
    enabledLevel = levelIn;
    setSuperEnabled();
}

// Set the enabled/disabled state of the widget according to user level
void standardProperties::setSuperEnabled()
{
    // Do nothing in designer
    if( QEWidget::inDesigner() )
    {
        return;
    }

    // If the current user level allows the widget to be enabled
    // and it was disabled due to an inapropriate user level, enable it.
    if( currentLevel >= enabledLevel && userLevelDisabled )
    {
        owner->setEnabled( true );
        userLevelDisabled = false;
    }

    // If the current user level prevents the widget from being enabled,
    // and it is enabled, disable it.
    if( currentLevel < enabledLevel && owner->isEnabled() )
    {
        owner->setEnabled( false );
        userLevelDisabled = true;
    }
}

// Set the visibility of the widget.
// Generally it is visible or not according to 'applicationVisibility', however this can be overridden
// if it is not the appropriate user level, or running within designer
void standardProperties::setSuperVisibility()
{
    // Do nothing in designer
    if( QEWidget::inDesigner() )
    {
        return;
    }

    // Hide the widget if the user level is not adequate, or if the desired visibility is 'hide'
    // (It is only visible if both the user level is adequate, and the desired visibility is 'show'
    bool vis = applicationVisibility;
    if( vis )
    {
        ContainerProfile profile;   // Note, scoped so it is only created if we need to check user level
        vis = ( profile.getUserLevel() >= visibilityLevel );
    }

    // Apply the result
    owner->setVisible( vis );
}

// Given a user level, note the new level and determine if the widget should be visible or enabled
void standardProperties::checkVisibilityEnabledLevel( userLevelTypes::userLevels level )
{
    // Note the new user level
    currentLevel = level;

    // Set the 'enabled' state according to the new level
    setSuperEnabled();

    // Set the visibility according to the new level
    setSuperVisibility();
}

// visible (widget is visible outside 'Designer')
void standardProperties::setRunVisible( bool visibleIn )
{
    applicationVisibility = visibleIn;
    setSuperVisibility();
}

bool standardProperties::getRunVisible()
{
    return applicationVisibility;
}

// DEPRECATED. USE setDisplayAlarmStateOption(displayAlarmStateOptions) INSTEAD
// displayAlarmState. If set (default) widget will indicate the alarm state of any variable data is displaying.
void standardProperties::setDisplayAlarmState( bool displayAlarmStateIn )
{
    displayAlarmState = displayAlarmStateIn?DISPLAY_ALARM_STATE_ALWAYS:DISPLAY_ALARM_STATE_NEVER;
}

// DEPRECATED. USE displayAlarmStateOptions getDisplayAlarmStateOption() INSTEAD
bool standardProperties::getDisplayAlarmState()
{
    return displayAlarmState != DISPLAY_ALARM_STATE_NEVER;
}

// displayAlarmState. If set (default) widget will indicate the alarm state of any variable data is displaying.
void standardProperties::setDisplayAlarmStateOption( displayAlarmStateOptions displayAlarmStateIn )
{
    displayAlarmState = displayAlarmStateIn;
}

standardProperties::displayAlarmStateOptions standardProperties::getDisplayAlarmStateOption()
{
    return displayAlarmState;
}
