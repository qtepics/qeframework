/*  QEToolTip.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2021 Australian Synchrotron
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

/// QE widget tool tips may be set to display the names of the process variables
/// supplying data to the widget and the alarm state and connectino status of those variables.
/// The QE widget may also set some custom text to be displayed along with this information.
/// The QEToolTip class manages building and setting the QE widget tool tips when this functino is required.

#ifndef QE_TOOL_TIP_H
#define QE_TOOL_TIP_H

#include <QEvent>
#include <QList>
#include <QObject>
#include <QWidget>
#include <QString>
#include <QTimer>
#include <QEString.h>
#include <QEFrameworkLibraryGlobal.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEToolTip
{
public:
   QEToolTip (QWidget* ownerIn);
   virtual ~ QEToolTip ();

   void setNumberToolTipVariables (const unsigned int number);
   void updateToolTipVariable (const QString& variable, const unsigned int variableIndex);    // Update a variable name

   void updateToolTipAlarm (const QCaAlarmInfo& alarmInfo, const unsigned int variableIndex); // Update an alarm state

   void updateToolTipConnection (bool connection, const unsigned int variableIndex = 0);      // Update a connection status
   void updateToolTipCustom (const QString& custom);                                          // Update the customisable part of the tooltip

   void setVariableAsToolTip (const bool variableAsToolTip);                                  // Set flag indicating variable names should be in the tool tip
   bool getVariableAsToolTip () const;                                                        // Get the flag indicating variable names should be in the tool tip

private:
   class Variable {
   public:
      explicit Variable ();
      ~Variable ();
      QString tip () const;  // partial tool tip for this variable.

      QString pvName;        // variable name to be included in the tooltip
      QString description;   // variable description to be included in the tooltip
      QString alarm;         // alarm state to be included in the tool tip
      bool isConnected;      // connection status to be included in the tool tip
   };

   typedef QList<Variable> VariableLists;

   void updateToolTipDescription (const QString & desc,
                                  const unsigned int variableIndex);   // Update description
   bool variableAsToolTip;          // Flag the tool tip should be set to the variable name
   void displayToolTip ();          // Built a tool tip from all the required information and set it
   int number;                      // Count of variables that will be included in the tooltip
   VariableLists variableList;      // List of variable information.
   QString toolTipCustom;           // Custion tool tip extra for specific widget types
   QWidget *owner;                  // Widget whos tool tip will be updated

   friend class QEToolTipSingleton;
};


//------------------------------------------------------------------------------
// This is essentially a private singleton class, but must be declared in the
// header file in order to use the meta object compiler (moc) to allow setup of
// the the QTime timeout slot.
//
class QEWidget;  // differed

class QEToolTipSingleton:public QObject
{
   Q_OBJECT
private:
   static void constructSingleton ();

   explicit QEToolTipSingleton (QObject* parent = 0);
   ~QEToolTipSingleton ();

   void registerWidget (QWidget* widget);
   void deregisterWidget (QWidget* widget);

   void updateWidget ();
   void enterWidget (QEWidget* qewidget);
   void leaveWidget (QEWidget* qewidget);
   bool eventFilter (QObject* watched, QEvent* event);

   QTimer* refreshTimer;
   QEWidget* currentWidget;

private slots:
   void refreshTimerHandler ();

   friend class QEToolTip;
};

#endif // QE_TOOL_TIP_H
