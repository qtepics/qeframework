/*  standardProperties.h
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
