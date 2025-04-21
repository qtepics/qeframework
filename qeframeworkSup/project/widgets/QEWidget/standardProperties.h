/*  standardProperties.h
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

#ifndef QE_STANDARD_PROPERTIES_H
#define QE_STANDARD_PROPERTIES_H

#include <QWidget>
#include <QEEnums.h>
#include <QCaAlarmInfo.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT standardProperties
{
public:
   explicit standardProperties (QWidget* ownerIn);
   virtual ~standardProperties ();

public:
   QE::UserLevels getUserLevelVisibility () const;
   void setUserLevelVisibility (QE::UserLevels level);

   QE::UserLevels getUserLevelEnabled () const;
   void setUserLevelEnabled (QE::UserLevels level);

   bool getApplicationEnabled () const;
   void setApplicationEnabled (bool state);

   // visible (widget is visible outside 'Designer')
   void setRunVisible (bool visibleIn);
   bool getRunVisible () const;

   void setDisplayAlarmStateOption (QE::DisplayAlarmStateOptions displayAlarmStateIn);
   QE::DisplayAlarmStateOptions getDisplayAlarmStateOption () const;

   // Controls if widget is OOS aware, i.e. responds to being declared out of service.
   //
   void setOosAware (const bool oosAware);
   bool getOosAware () const;

   // Utility function to determine if the alarm state colour is to be used.
   // This include the OOS pseudo-alarm colour.
   //
   bool getUseAlarmState (const QCaAlarmInfo& alarmInfo) const;

protected:
   void checkVisibilityEnabledLevel (QE::UserLevels level);

private:
   QWidget* owner;

   QE::UserLevels currentLevel;

   void invokeStringFormattingChange ();
   void setSuperEnabled ();
   void setSuperVisibility ();

   QE::UserLevels visibilityLevel;  // User level below which the widget will be made invisible
   QE::UserLevels enabledLevel;     // User level below which the widget will be made disabled

   bool userLevelDisabled;      // Flag indicating the widget has been disabled due to inapropriate user level
   bool applicationVisibility;  // The 'visibility' state of the widget unless held invisible due to inapropriate user level
   QE::DisplayAlarmStateOptions displayAlarmState;  // Flag when the widget should display alarm state
   bool isOosAware;             // Flag is widget should display OOS status.
};

#endif   // QE_STANDARD_PROPERTIES_H
