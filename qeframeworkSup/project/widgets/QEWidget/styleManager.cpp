/*  styleManager.cpp
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

#include "styleManager.h"
#include <QDebug>
#include <QEWidget.h>

#define DEBUG qDebug () << "styleManager" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
// Construction.
//
styleManager::styleManager( QWidget* ownerIn )
{
    // Sanity check.
    if( ownerIn == NULL )
    {
        qWarning( "styleManager constructor called with a null 'owner'" );
        exit( EXIT_FAILURE );
    }

    // Keep a handle on the underlying QWidget of the QE widgets
    owner = ownerIn;
    defaultStyleSheet = "";
    level = QE::User;

    // Note the current style sheet.
    // This will be kept up to date as this manager manages changes to the component
    // parts of the style, even if the style is not currently being applied to the widget
    // because it is disabled.
    // This means that when the widget is re-enabled, the currentStyle can just be applied.
    currentStyle = owner->styleSheet();

    // Add an event filter to catch enables and disabled (all styles are removed when disabled)
    eventFilter = new changeEventFilter( this );
    owner->installEventFilter(eventFilter);
}

//------------------------------------------------------------------------------
// Destruction
//
styleManager::~styleManager()
{
    // Remove the event filter to catch enables and disabled
    owner->removeEventFilter( eventFilter );
    delete eventFilter;
}

//------------------------------------------------------------------------------
// Allow the default style sheet to be programatically set.
//
void styleManager::setStyleDefault( QString styleIn )
{
    defaultStyleSheet = styleIn;
    updateStyleSheet();
}

//------------------------------------------------------------------------------
// Get the current default Style string
// If not set we return the style out of the widget sttyle sheet.
//
QString styleManager::getStyleDefault() const
{
   return defaultStyleSheet;
}

//------------------------------------------------------------------------------
// Set the Style Sheet string to be applied when the widget is displayed in 'User' mode.
// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
//
void styleManager::setStyleUser( QString style )
{
    userUserStyle = style;
}

//------------------------------------------------------------------------------
// Get the Style Sheet string to be applied when the widget is displayed in 'User' mode.
//
QString styleManager::getStyleUser() const
{
    return userUserStyle;
}

//------------------------------------------------------------------------------
// Set the Style Sheet string to be applied when the widget is displayed in 'Scientist' mode.
// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
//
void styleManager::setStyleScientist( QString style )
{
    userScientistStyle = style;
}

//------------------------------------------------------------------------------
// Get the Style Sheet string to be applied when the widget is displayed in 'Scientist' mode.
//
QString styleManager::getStyleScientist() const
{
    return userScientistStyle;
}

//------------------------------------------------------------------------------
// Set the Style Sheet string to be applied when the widget is displayed in 'Engineer' mode.
// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
//
void styleManager::setStyleEngineer( QString style )
{
    userEngineerStyle = style;
}

//------------------------------------------------------------------------------
// Get the Style Sheet string to be applied when the widget is displayed in 'Engineer' mode.
//
QString styleManager::getStyleEngineer() const
{
    return userEngineerStyle;
}


//------------------------------------------------------------------------------
// Set the Style Sheet string to be applied to reflect an aspect of the current data.
// For example, a value over a high limit may be displayed in red.
//
void styleManager::updateDataStyle( QString style )
{
    dataStyleSheet = style;
    updateStyleSheet();
}

//------------------------------------------------------------------------------
// Set the Style Sheet string to be applied to reflect an aspect of the current status.
// For example, invalid data may be displayed with a white background.
//
void styleManager::updateStatusStyle( QString style )
{
    statusStyleSheet = style;
    updateStyleSheet();
}

//------------------------------------------------------------------------------
// Set the Style Sheet string to be applied to reflect the current connection state
// (connected or disconnected) of the current data.
// For example, a disconnected value may be greyed out.
//
void styleManager::updateConnectionStyle( bool connected )
{
    if( connected )
    {
        connectionStyleSheet = "";
    }
    else
    {
        connectionStyleSheet = "QWidget { color: grey }";
    }
    updateStyleSheet();
}

//------------------------------------------------------------------------------
// Set the Style Sheet string to be applied to implement a widget property.
// For example, a style string is used to set QE button text alignment.
//
void styleManager::updatePropertyStyle( QString style )
{
    propertyStyleSheet = style;
    updateStyleSheet();
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
    switch( level )
    {
        case QE::User:
            userLevelStyle = userUserStyle;
            break;

        case QE::Scientist:
            userLevelStyle = userScientistStyle;
            break;

        case QE::Engineer:
            userLevelStyle = userEngineerStyle;
            break;
    }

    // Compile and apply the entire style string if there is any difference
    // with what is currently there.
    //
    QString newStyleSheet = defaultStyleSheet;

// Macro function to append style with "\n" separator iff required.
//
#define APPEND_STYLE(nextStyleSheet) {                                         \
   if( !newStyleSheet.isEmpty() && !nextStyleSheet.isEmpty() )                 \
       newStyleSheet.append( "\n" );                                           \
   newStyleSheet.append( nextStyleSheet );                                     \
}


    // Note: with styles: Last in - best dressed.
    //
    APPEND_STYLE (propertyStyleSheet);
    APPEND_STYLE (statusStyleSheet);
    APPEND_STYLE (connectionStyleSheet);
    APPEND_STYLE (dataStyleSheet);
    APPEND_STYLE (userLevelStyle);

#undef APPEND_STYLE

    // Apply the new style sheet if the widget is enabled
    // (and it is different to the current one)
    // (and we are not in Designer)
    //
    if( owner->isEnabled() &&
        newStyleSheet.compare( owner->styleSheet() ) &&
        !QEWidget::inDesigner() )
    {
        owner->setStyleSheet( newStyleSheet );
    }

    // Keep an up-to-date copy of the style sheet. It will be applied to the
    // widget if the widget changes from being disabled to enabled.
    //
    currentStyle = newStyleSheet;
}

//------------------------------------------------------------------------------
// Set the current user level.
//
void styleManager::styleUserLevelChanged( QE::UserLevels levelIn )
{
    // Note the new style and update the style string if changed
    bool newLevel = level != levelIn;
    level = levelIn;
    if( newLevel )
    {
        updateStyleSheet();
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
    if( QEWidget::inDesigner() )
        return;

    // Enable use or don't use the current style sheet according to the disabled state
    if( owner->isEnabled() )
    {
        owner->setStyleSheet( currentStyle );
    }
    else
    {
        owner->setStyleSheet( "" );
    }
}

//------------------------------------------------------------------------------
// Change Event Filter used to note when the widget becomes enabled or disabled.
// (styles are removed while disabled so the 'disabled' look is not hidden by the applied style)
//
bool changeEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    // If the enabled state has changed, report this to the style manager
    if ( event->type() == QEvent::EnabledChange )
    {
        manager->enabledChange();
    }

    // Do standard event processing.
    return QObject::eventFilter(obj, event);
}

// end
