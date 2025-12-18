/*  QEToolTip.cpp
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

#include "QEToolTip.h"
#include <QApplication>
#include <QDebug>
#include <QEWidget.h>

#define DEBUG qDebug () << "QEToolTip" << __LINE__ << __FUNCTION__ << "  "


//==============================================================================
// QEToolTipSingleton class
//==============================================================================
//
static QEToolTipSingleton* toolTipSingleton = NULL;

//------------------------------------------------------------------------------
// static: call constructor - this function is idempotent.
//
void QEToolTipSingleton::constructSingleton ()
{
   if (!toolTipSingleton) {
      // Use the application itself as parent.
      toolTipSingleton = new QEToolTipSingleton (QApplication::instance ());
   }
}

//------------------------------------------------------------------------------
// Constructor
//
QEToolTipSingleton::QEToolTipSingleton (QObject* parent) : QObject (parent)
{
   this->currentWidget = NULL;
   this->refreshTimer = new QTimer (this);

   // Connect and start regular timed event.
   //
   QObject::connect (this->refreshTimer, SIGNAL (timeout()),
                     this, SLOT (refreshTimerHandler ()));
   this->refreshTimer->start (250);   // mSec - 4Hz
}

//------------------------------------------------------------------------------
// Decontruct the singleton.
//
QEToolTipSingleton::~QEToolTipSingleton ()
{
   this->currentWidget = NULL;
   toolTipSingleton = NULL;
}

//------------------------------------------------------------------------------
//
void QEToolTipSingleton::refreshTimerHandler ()
{
    if (this->currentWidget) {
       // Update the current the widget
       this->updateWidget ();
    }
}

//------------------------------------------------------------------------------
// Installs and event handler for the specified owner widget.
//
void QEToolTipSingleton::registerWidget (QWidget* widget)
{
   if (widget) {
      widget->installEventFilter (this);
   }
}

//------------------------------------------------------------------------------
// Widget is being deleted: If it is the current qe widget, then tidy up.
//
void QEToolTipSingleton::deregisterWidget (QWidget* widget)
{
   QEWidget* qewidget = dynamic_cast <QEWidget*>(widget);
   if (qewidget == this->currentWidget) {
      this->currentWidget = NULL;
   }

   if (widget) {
      widget->removeEventFilter (this);
   }
}

//------------------------------------------------------------------------------
//
void QEToolTipSingleton::updateWidget ()
{
   if (!this->currentWidget) return;   // sanity check

   const unsigned int number = (unsigned int) this->currentWidget->getNumberVariables ();
   for (unsigned int j = 0; j < number; j++) {
      QString desc = "";       // blank until we know better.
      qcaobject::QCaObject* qca = this->currentWidget->getQcaItem (j);
      if (qca) {  // sanity check
         desc = qca->getDescription();
      }
      this->currentWidget->updateToolTipDescription (desc, j);
   }
}

//------------------------------------------------------------------------------
// On entry to the widget request the PV.
// We also do this on a regular basis as getDescription initiates the connection
// and most certainly will return a null string the first time it is called.
//
void QEToolTipSingleton::enterWidget (QEWidget* qewidget)
{
   if (!qewidget) return;   // sanity check

   // Do nothing if not displaying Variable names in the tool tip
   if (!qewidget->variableAsToolTip) {
      return;
   }

   this->currentWidget = qewidget;  // save a reference widget of interest.
   this->updateWidget ();
}

//------------------------------------------------------------------------------
// Process leaving the widget.
//
void QEToolTipSingleton::leaveWidget (QEWidget* /* qewidget */ )
{
   this->currentWidget = NULL;
}

//------------------------------------------------------------------------------
// Common event filter for all widgets that arer also QEWidgets.
//
bool QEToolTipSingleton::eventFilter (QObject* watched, QEvent* event)
{
   const QEvent::Type type = event->type ();
   QEWidget* qewidget;

   switch (type) {
      case QEvent::Enter:
         qewidget = dynamic_cast <QEWidget*>(watched);
         if (qewidget) this->enterWidget (qewidget);
         break;

      case QEvent::Leave:
         qewidget = dynamic_cast <QEWidget*>(watched);
         if (qewidget) this->leaveWidget (qewidget);
         break;

      default:
         break;
   }

   // We don't handle events per se - just intercept them.
   return false;
}


//==============================================================================
// QEToolTip class
//==============================================================================
// Constructor
//
QEToolTip::QEToolTip (QWidget* ownerIn)
{
   // Sanity check.
   if (ownerIn == NULL) {
      qWarning ("QEToolTip constructor called with a null 'owner'");
      exit (EXIT_FAILURE);   /// *** Fix this
   }

   // Keep a handle on the underlying QWidget of the QE widgets
   this->owner = ownerIn;

   // Initially there are no variables associated with the tool tip
   this->number = 0;
   this->variableAsToolTip = true;

   // Create singleton object if needs be.
   QEToolTipSingleton::constructSingleton ();
   toolTipSingleton->registerWidget (owner);
}

//------------------------------------------------------------------------------
//
QEToolTip::~QEToolTip ()
{
   toolTipSingleton->deregisterWidget (owner);
}


//------------------------------------------------------------------------------
// Property set: variable as tool tip
//
void QEToolTip::setVariableAsToolTip (const bool variableAsToolTipIn)
{
   // If and only if turning off variable as tool tip then clear the tool tip
   if (this->variableAsToolTip && !variableAsToolTipIn) {
      this->owner->setToolTip ("");
   }

   // Set the new tool tip type
   this->variableAsToolTip = variableAsToolTipIn;

   // Update the tool tip to match the new state
   this->displayToolTip ();
}

//------------------------------------------------------------------------------
// Property get: variable as tool tip
//
bool QEToolTip::getVariableAsToolTip () const
{
   return this->variableAsToolTip;
}

//------------------------------------------------------------------------------
// Ensures list are large enough.
//
void QEToolTip::setNumberToolTipVariables (const unsigned int numberIn)
{
   this->number = int (numberIn);

   static Variable emptyVariable;
   while (this->variableList.count() < this->number) this->variableList.append (emptyVariable);
   while (this->variableList.count() > this->number) this->variableList.removeLast();
}

//------------------------------------------------------------------------------
// Update the variable used in the tool tip
// (Used when displaying a dynamic tool tip only)
//
void QEToolTip::updateToolTipVariable (const QString& pvName,
                                       const unsigned int variableIndex)
{
   if ((int) variableIndex < this->variableList.count ()) {
      static Variable emptyVariable;
      Variable var = this->variableList.value (variableIndex, emptyVariable);
      var.pvName = pvName;
      this->variableList.replace (variableIndex, var);
      this->displayToolTip ();
   }
}

//------------------------------------------------------------------------------
// Update the description associated with the variable.
// (Used when displaying a dynamic tool tip only)
//
void QEToolTip::updateToolTipDescription (const QString& desc,
                                          const unsigned int variableIndex)
{
   if ((int) variableIndex < this->variableList.count ()) {
      static Variable emptyVariable;
      Variable var = this->variableList.value (variableIndex, emptyVariable);
      var.description = desc;
      this->variableList.replace (variableIndex, var);
      this->displayToolTip ();
   }
}

//------------------------------------------------------------------------------
// Update the variable alarm status used in the tool tip
// (Used when displaying a dynamic tool tip only)
//
void QEToolTip::updateToolTipAlarm (const QCaAlarmInfo& alarmInfo,
                                    const unsigned int variableIndex)
{
   if ((int) variableIndex < this->variableList.count ()) {
      QString alarmText;
      QString alarmMessageText;

      alarmText = alarmInfo.severityName();

      // Add status, however avoid double no alarm.
      //
      if (alarmInfo.getStatus() > 0) {
         alarmText.append (", ");
         alarmText.append (alarmInfo.statusName());
      }

      // Add message if there is any.
      // Note: is always an empty string for Channel Access.
      //
      alarmMessageText = alarmInfo.messageText();
      if (!alarmMessageText.isEmpty()) {
         alarmText.append (", ");
         alarmText.append (alarmMessageText);
      }

      if (alarmInfo.isOutOfService()) {
         alarmText.append (", OOS");
      }

      static Variable emptyVariable;
      Variable var = this->variableList.value (variableIndex, emptyVariable);
      var.alarm = alarmText;
      this->variableList.replace (variableIndex, var);
      this->displayToolTip ();
   }
}


//------------------------------------------------------------------------------
// Update the variable custom information used in the tool tip
// (Used when displaying a dynamic tool tip only)
//
void QEToolTip::updateToolTipCustom (const QString& custom)
{
   this->toolTipCustom = custom;
   this->displayToolTip ();
}

//------------------------------------------------------------------------------
// Update the variable connection status used in the tool tip
// (Used when displaying a dynamic tool tip only)
//
void QEToolTip::updateToolTipConnection (bool isConnectedIn,
                                         const unsigned int variableIndex)
{
   if ((int) variableIndex < this->variableList.count ()) {
      static Variable emptyVariable;
      Variable var = this->variableList.value (variableIndex, emptyVariable);
      var.isConnected = isConnectedIn;
      this->variableList.replace (variableIndex, var);
      this->displayToolTip ();
   }
}

//------------------------------------------------------------------------------
// Build and display the tool tip from the name and state if dynamic
//
void QEToolTip::displayToolTip ()
{   
   // If using the variable name as the tool tip, build the tool tip
   if (this->variableAsToolTip) {
      int count = 0;
      QString toolTip = "";

      for (int j = 0; j < this->number; j++) {
         static Variable emptyVariable;
         Variable var = this->variableList.value (j, emptyVariable);

         QString partTip = var.tip();
         if (partTip.size ()) {
            if (count > 0) {
               toolTip.append ("\n");
            }
            count++;
            toolTip.append (partTip);
         }
      }

      if (count == 0) {
         toolTip = "No variables defined";
      }

      if (toolTipCustom.size ()) {
         toolTip.append (" - ").append (toolTipCustom);
      }

      this->owner->setToolTip (toolTip);
   }
}

//==============================================================================
// QEToolTip::Variable class
//==============================================================================
//
QEToolTip::Variable::Variable ()
{
   this->pvName = "";
   this->description = "";
   this->alarm = "";
   this->isConnected = false;
}

//------------------------------------------------------------------------------
//
QEToolTip::Variable::~Variable () { }

//------------------------------------------------------------------------------
//
QString QEToolTip::Variable::tip () const
{
    QString result = "";

    if (!this->pvName.isEmpty()) {
       result.append (this->pvName);

       if (!this->description.isEmpty ()) {
          result.append (" : ");
          result.append (this->description);
       }

       if (this->isConnected) {
          // Only connected PVs have an alarm state.
          //
          if (!this->alarm.isEmpty()) {
             result.append (" - ").append (this->alarm);
           }
       } else {
          result.append (" - Disconnected");
       }
    }

    return result;
}

// end
