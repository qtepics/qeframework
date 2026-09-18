/*  styleManager.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "styleManager.h"
#include <QDebug>
#include <QEWidget.h>

#define DEBUG qDebug () << "QEStyleManager" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
// Construction.
//
styleManager::styleManager (QWidget* ownerIn)
{
   this->owner = ownerIn;

   // Sanity check.
   if (ownerIn == NULL) {
      qWarning ("styleManager constructor called with a null 'owner'");
      DEBUG  << "styleManager constructor called with a null 'owner'";
      return;
   }

   // Keep a handle on the underlying QWidget of the QE widgets.
   //
   this->defaultStyleSheet = "";
   this->level = QE::User;

   // Note the current style sheet.
   // This will be kept up to date as this manager manages changes to the component
   // parts of the style, even if the style is not currently being applied to the widget
   // because it is disabled.
   // This means that when the widget is re-enabled, the currentStyle can just be applied.
   //
   this->currentStyle = owner->styleSheet();

   // Add an event filter to catch enables and disabled (all styles are removed when disabled).
   //
   this->eventFilter = new changeEventFilter (this);
   this->owner->installEventFilter (eventFilter);
}

//------------------------------------------------------------------------------
// Destruction
//
styleManager::~styleManager()
{
   // Remove the event filter to catch enables and disabled.
   //
   this->owner->removeEventFilter (this->eventFilter);
   delete eventFilter;
}

//------------------------------------------------------------------------------
// Allow the default style sheet to be programatically set.
//
void styleManager::setStyleDefault (const QString& style)
{
   if (this->defaultStyleSheet != style) {
      this->defaultStyleSheet = style;
      this->updateStyleSheet();
   }
}

//------------------------------------------------------------------------------
// Get the current default Style string
// If not set we return the style out of the widget sttyle sheet.
//
QString styleManager::getStyleDefault() const
{
   return this->defaultStyleSheet;
}

//------------------------------------------------------------------------------
// Set the Style Sheet string to be applied when the widget is displayed in 'User' mode.
// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
//
void styleManager::setStyleUser (const QString& style)
{
   this->userUserStyle = style;
}

//------------------------------------------------------------------------------
// Get the Style Sheet string to be applied when the widget is displayed in 'User' mode.
//
QString styleManager::getStyleUser() const
{
   return this->userUserStyle;
}

//------------------------------------------------------------------------------
// Set the Style Sheet string to be applied when the widget is displayed in 'Scientist' mode.
// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
//
void styleManager::setStyleScientist (const QString& style)
{
   this->userScientistStyle = style;
}

//------------------------------------------------------------------------------
// Get the Style Sheet string to be applied when the widget is displayed in 'Scientist' mode.
//
QString styleManager::getStyleScientist() const
{
   return this->userScientistStyle;
}

//------------------------------------------------------------------------------
// Set the Style Sheet string to be applied when the widget is displayed in 'Engineer' mode.
// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
//
void styleManager::setStyleEngineer (const QString& style)
{
   this->userEngineerStyle = style;
}

//------------------------------------------------------------------------------
// Get the Style Sheet string to be applied when the widget is displayed in 'Engineer' mode.
//
QString styleManager::getStyleEngineer() const
{
   return this->userEngineerStyle;
}


//------------------------------------------------------------------------------
// Set the Style Sheet string to be applied to reflect an aspect of the current data.
// For example, a value over a high limit may be displayed in red.
//
void styleManager::updateDataStyle (const QString& style)
{
   if (this->dataStyleSheet != style) {
      this->dataStyleSheet = style;
      this->updateStyleSheet();
   }
}

//------------------------------------------------------------------------------
// Set the Style Sheet string to be applied to reflect an aspect of the current status.
// For example, invalid data may be displayed with a white background.
//
void styleManager::updateStatusStyle (const QString& style)
{
   if (this->statusStyleSheet != style) {
      this->statusStyleSheet = style;
      this->updateStyleSheet();
   }
}

//------------------------------------------------------------------------------
// Set the Style Sheet string to be applied to reflect the current connection state
// (connected or disconnected) of the current data.
// For example, a disconnected value may be greyed out.
//
void styleManager::updateConnectionStyle (const bool connected)
{
   const QString style = connected ? "" : "QWidget { color: grey }";

   if (this->connectionStyleSheet != style) {
      this->connectionStyleSheet = style;
      this->updateStyleSheet();
   }
}

//------------------------------------------------------------------------------
// Set the Style Sheet string to be applied to implement a widget property.
// For example, a style string is used to set QE button text alignment.
//
void styleManager::updatePropertyStyle (const QString& style)
{
   if (this->propertyStyleSheet != style) {
      this->propertyStyleSheet = style;
      this->updateStyleSheet();
   }
}

//------------------------------------------------------------------------------
// Update the style sheet with the various style sheet components used to modify the
// style (user level, connection state, alarm info, enumeration color)
//
void styleManager::updateStyleSheet()
{
   // Note,for QE widgets the styleSheet is now a non-designable property,
   // so inhibiting style updates is no longer applicable.
   //
   // Select the appropriate user level style
   //
   QString userLevelStyle;
   switch (level)
   {
      case QE::User:
         userLevelStyle = this->userUserStyle;
         break;

      case QE::Scientist:
         userLevelStyle = this->userScientistStyle;
         break;

      case QE::Engineer:
         userLevelStyle = this->userEngineerStyle;
         break;
   }

   // Compile and apply the entire style string if there is any difference
   // with what is currently there.
   //
   QString newStyleSheet = this->defaultStyleSheet;

   // Macro function to append style with "\n" separator iff required.
   //
#define APPEND_STYLE(nextStyleSheet) {                                         \
   if (!newStyleSheet.isEmpty() && !nextStyleSheet.isEmpty()) {                \
      newStyleSheet.append ("\n");                                             \
      newStyleSheet.append (nextStyleSheet);                                   \
   }                                                                           \
}


   // Note: with styles: Last in - best dressed.
   //
   APPEND_STYLE (this->propertyStyleSheet);
   APPEND_STYLE (this->statusStyleSheet);
   APPEND_STYLE (this->connectionStyleSheet);
   APPEND_STYLE (this->dataStyleSheet);
   APPEND_STYLE (userLevelStyle);

#undef APPEND_STYLE

   // Apply the new style sheet if the widget is enabled
   // (and it is different to the current one)
   // (and we are not in Designer)
   //
   if (this->owner->isEnabled() &&
       newStyleSheet.compare (owner->styleSheet()) &&
       !QEWidget::inDesigner())
   {
      this->owner->setStyleSheet (newStyleSheet);
   }

   // Keep an up-to-date copy of the style sheet. It will be applied to the
   // widget if the widget changes from being disabled to enabled.
   //
   this->currentStyle = newStyleSheet;
}

//------------------------------------------------------------------------------
// Set the current user level.
//
void styleManager::styleUserLevelChanged (const QE::UserLevels levelIn)
{
   // Note the new style and update the style string if changed.
   //
   if (this->level != levelIn) {
      this->level = levelIn;
      this->updateStyleSheet();
   }
}

//------------------------------------------------------------------------------
// Called to notify the manager that the enabled state of the widget has changed.
// Styles are removed while disabled so the 'disabled' look is not hidden by
// the applied style)
// Styles are re-applied when enabled. The re-applied style is current and may
// have been calculated by the manager while the widget was disabled.
//
void styleManager::enabledChange()
{
   // Do nothing if running within designer
   if (QEWidget::inDesigner())
      return;

   // Enable use or don't use the current style sheet according to the disabled state.
   //
   if (this->owner->isEnabled())
   {
      this->owner->setStyleSheet (currentStyle);
   }
   else
   {
      this->owner->setStyleSheet ("");
   }
}


//==============================================================================
// changeEventFilter
//==============================================================================
//
styleManager::changeEventFilter::changeEventFilter (styleManager* managerIn)
{
   this->manager = managerIn;
}

//------------------------------------------------------------------------------
//
styleManager::changeEventFilter::~changeEventFilter ()
{
}

//------------------------------------------------------------------------------
// Change Event Filter used to note when the widget becomes enabled or disabled.
// (styles are removed while disabled so the 'disabled' look is not hidden by the applied style)
//
bool styleManager::changeEventFilter::eventFilter (QObject* watched, QEvent* event)
{
   // If the enabled state has changed, report this to the style manager.
   //
   if (event->type() == QEvent::EnabledChange) {
      this->manager->enabledChange();
   }

   // Do standard event processing.
   //
   return QObject::eventFilter (watched, event);
}

// end
