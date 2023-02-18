/*  standardProperties.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2023 Australian Synchrotron
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

/*
  This class adds common property support to all QE widgets where the support
  involves interaction with the QWidget.  Any common property support that
  requires a greater scope than the QWidget is managed in QEWidget.
*/

#include "standardProperties.h"
#include <QDebug>
#include <QEWidget.h>
#include <QEStringFormattingMethods.h>

#define DEBUG  qDebug () << "standardProperties" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
// Construction.
standardProperties::standardProperties (QWidget* ownerIn)
{
   // Keep a handle on the underlying QWidget of the QE widgets.
    //
   this->owner = ownerIn;

   ContainerProfile profile;

   this->visibilityLevel = userLevelTypes::USERLEVEL_USER;
   this->enabledLevel = userLevelTypes::USERLEVEL_USER;
   this->currentLevel = profile.getUserLevel ();

   this->userLevelDisabled = false;
   this->applicationVisibility = true;

   // This is the default-default state.
   // Some widgets, esp QEAbstractDynamicWidgets can/di overide this default.
   //
   this->displayAlarmState = DISPLAY_ALARM_STATE_ALWAYS;
   this->isOosAware = true;
}

standardProperties::~standardProperties () {}

//------------------------------------------------------------------------------
//
userLevelTypes::userLevels standardProperties::getUserLevelVisibility () const
{
   return this->visibilityLevel;
}

//------------------------------------------------------------------------------
//
void standardProperties::setUserLevelVisibility (userLevelTypes::userLevels levelIn)
{
   this->visibilityLevel = levelIn;
   this->setSuperVisibility ();
}

//------------------------------------------------------------------------------
//
userLevelTypes::userLevels standardProperties::getUserLevelEnabled () const
{
   return this->enabledLevel;
}

//------------------------------------------------------------------------------
//
void standardProperties::setUserLevelEnabled (userLevelTypes::userLevels levelIn)
{
   this->enabledLevel = levelIn;
   this->setSuperEnabled ();
}

//------------------------------------------------------------------------------
// Set the enabled/disabled state of the widget according to user level.
//
void standardProperties::setSuperEnabled ()
{
   // sanity check
   if (!this->owner) return;

   // Do nothing in designer
   //
   if (QEWidget::inDesigner ()) {
      return;
   }

   // If the current user level allows the widget to be enabled
   // and it was disabled due to an inapropriate user level, enable it.
   //
   if ((currentLevel >= enabledLevel) && userLevelDisabled) {
      this->owner->setEnabled (true);
      this->userLevelDisabled = false;
   }

   // If the current user level prevents the widget from being enabled,
   // and it is enabled, disable it.
   //
   if ((currentLevel < enabledLevel) && owner->isEnabled ()) {
      this->owner->setEnabled (false);
      this->userLevelDisabled = true;
   }
}

//------------------------------------------------------------------------------
// Set the visibility of the widget.
// Generally it is visible or not according to 'applicationVisibility', however this can be overridden
// if it is not the appropriate user level, or running within designer.
//
void standardProperties::setSuperVisibility ()
{
   if (!this->owner) return;   // sanity check

   // Do nothing in designer
   if (QEWidget::inDesigner ()) {
      return;
   }

   // Hide the widget if the user level is not adequate, or if the desired visibility is 'hide'
   // (It is only visible if both the user level is adequate, and the desired visibility is 'show'.
   //
   bool vis = this->applicationVisibility;
   if (vis) {
      ContainerProfile profile; // Note, scoped so it is only created if we need to check user level
      vis = (profile.getUserLevel () >= this->visibilityLevel);
   }

   // Apply the result
   this->owner->setVisible (vis);
}

//------------------------------------------------------------------------------
// Given a user level, note the new level and determine if the widget should be visible or enabled.
//
void standardProperties::checkVisibilityEnabledLevel (userLevelTypes::userLevels level)
{
   // Note the new user level
   this->currentLevel = level;

   // Set the 'enabled' state according to the new level
   this->setSuperEnabled ();

   // Set the visibility according to the new level
   this->setSuperVisibility ();
}

//------------------------------------------------------------------------------
// visible (widget is visible outside 'Designer').
//
void standardProperties::setRunVisible (bool visibleIn)
{
   this->applicationVisibility = visibleIn;
   this->setSuperVisibility ();
}

//------------------------------------------------------------------------------
//
bool standardProperties::getRunVisible () const
{
   return this->applicationVisibility;
}

//------------------------------------------------------------------------------
//
void standardProperties::invokeStringFormattingChange ()
{
   QEStringFormattingMethods* sfm = dynamic_cast<QEStringFormattingMethods*>(this->owner);
   if (sfm) {
      sfm->stringFormattingChange ();
   }
}

//------------------------------------------------------------------------------
// displayAlarmState. If set (default) widget will indicate the alarm state of
// any variable data is displaying.
//
void standardProperties::
setDisplayAlarmStateOption (displayAlarmStateOptions displayAlarmStateIn)
{
   this->displayAlarmState = displayAlarmStateIn;
   this->invokeStringFormattingChange ();
}

//------------------------------------------------------------------------------
//
standardProperties::displayAlarmStateOptions
standardProperties::getDisplayAlarmStateOption () const
{
   return this->displayAlarmState;
}

//------------------------------------------------------------------------------
//
void standardProperties::setOosAware (const bool oosAwareIn)
{
   this->isOosAware = oosAwareIn;
   this->invokeStringFormattingChange ();
}

//------------------------------------------------------------------------------
//
bool standardProperties::getOosAware () const
{
   return this->isOosAware;
}

//------------------------------------------------------------------------------
//
bool standardProperties::getUseAlarmState (const QCaAlarmInfo & alarmInfo) const
{
   bool result = true;

   if (this->getOosAware () && alarmInfo.isOutOfService ()) {
      // Use the pseudo OOS alarm colour.
      //
      result = true;

   } else {
      switch (this->getDisplayAlarmStateOption ()) {

         case DISPLAY_ALARM_STATE_ALWAYS:
            result = true;
            break;

         case DISPLAY_ALARM_STATE_WHEN_IN_ALARM:
            result = alarmInfo.isInAlarm ();
            break;

         case DISPLAY_ALARM_STATE_WHEN_INVALID:
            result = alarmInfo.isInvalid ();
            break;

         case DISPLAY_ALARM_STATE_NEVER:
            result = false;
            break;
      }
   }

   return result;
}

// end
