/*  standardProperties.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2019 Australian Synchrotron
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
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef QE_STANDARD_PROPERTIES_H
#define QE_STANDARD_PROPERTIES_H

#include <QWidget>
#include <ContainerProfile.h>

class  QE_FRAMEWORK_LIBRARY_SHARED_EXPORT standardProperties
{

public:
    explicit standardProperties( QWidget* ownerIn );
    virtual ~standardProperties(){}

public:
    // Options regarding when the widget should display alarm state
    enum displayAlarmStateOptions {
       DISPLAY_ALARM_STATE_NEVER,
       DISPLAY_ALARM_STATE_ALWAYS,
       DISPLAY_ALARM_STATE_WHEN_IN_ALARM
    };

    userLevelTypes::userLevels getUserLevelVisibility() const;
    void setUserLevelVisibility( userLevelTypes::userLevels level );

    userLevelTypes::userLevels getUserLevelEnabled() const;
    void setUserLevelEnabled( userLevelTypes::userLevels level );

    bool getApplicationEnabled() const;
    void setApplicationEnabled( bool state );

    // visible (widget is visible outside 'Designer')
    void setRunVisible( bool visibleIn );
    bool getRunVisible() const;

    // DEPRECATED. USE setDisplayAlarmStateOption(displayAlarmStateOptions) INSTEAD
    Q_DECL_DEPRECATED
    void setDisplayAlarmState( bool displayAlarmStateIn );

    // DEPRECATED. USE displayAlarmStateOptions getDisplayAlarmStateOption() INSTEAD
    Q_DECL_DEPRECATED
    bool getDisplayAlarmState() const;

    void setDisplayAlarmStateOption( displayAlarmStateOptions displayAlarmStateIn );
    displayAlarmStateOptions getDisplayAlarmStateOption() const;

protected:
    void checkVisibilityEnabledLevel( userLevelTypes::userLevels level );

private:
    QWidget* owner;

    userLevelTypes::userLevels currentLevel;

    void setSuperEnabled();
    void setSuperVisibility();

    userLevelTypes::userLevels visibilityLevel;  // User level below which the widget will be made invisible
    userLevelTypes::userLevels enabledLevel;     // User level below which the widget will be made disabled

    bool userLevelDisabled;         // Flag indicating the widget has been disabled due to inapropriate user level
    bool applicationVisibility;     // The 'visibility' state of the widget unless held invisible due to inapropriate user level
    displayAlarmStateOptions displayAlarmState;  // Flag when the widget should display alarm state
};

#endif // QE_STANDARD_PROPERTIES_H
