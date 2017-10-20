/*  styleManager.cpp
 *
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
 *  Copyright (c) 2012,2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */


#include <styleManager.h>
#include <QEWidget.h>

// Construction.
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
    level = userLevelTypes::USERLEVEL_USER;

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

// Destruction
styleManager::~styleManager()
{
    // Remove the event filter to catch enables and disabled
    owner->removeEventFilter( eventFilter );
    delete eventFilter;
}


// Allow the default style sheet to be programatically set.
void styleManager::setStyleDefault( QString styleIn )
{
    defaultStyleSheet = styleIn;
    updateStyleSheet();
}

// Get the current default Style string
// If not set we return the style out of the widget sttyle sheet.
QString styleManager::getStyleDefault() const
{
   return defaultStyleSheet;
}


// Set the Style Sheet string to be applied when the widget is displayed in 'User' mode.
// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
void styleManager::setStyleUser( QString style )
{
    userUserStyle = style;
}

// Get the Style Sheet string to be applied when the widget is displayed in 'User' mode.
QString styleManager::getStyleUser() const
{
    return userUserStyle;
}

// Set the Style Sheet string to be applied when the widget is displayed in 'Scientist' mode.
// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
void styleManager::setStyleScientist( QString style )
{
    userScientistStyle = style;
}

// Get the Style Sheet string to be applied when the widget is displayed in 'Scientist' mode.
QString styleManager::getStyleScientist() const
{
    return userScientistStyle;
}

// Set the Style Sheet string to be applied when the widget is displayed in 'Engineer' mode.
// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
void styleManager::setStyleEngineer( QString style )
{
    userEngineerStyle = style;
}

// Get the Style Sheet string to be applied when the widget is displayed in 'Engineer' mode.
QString styleManager::getStyleEngineer() const
{
    return userEngineerStyle;
}


// Set the Style Sheet string to be applied to reflect an aspect of the current data.
// For example, a value over a high limit may be displayed in red.
void styleManager::updateDataStyle( QString style )
{
    dataStyleSheet = style;
    updateStyleSheet();
}

// Set the Style Sheet string to be applied to reflect an aspect of the current status.
// For example, invalid data may be displayed with a white background.
void styleManager::updateStatusStyle( QString style )
{
    statusStyleSheet = style;
    updateStyleSheet();
}

// Set the Style Sheet string to be applied to reflect the current connection state
// (connected or disconnected) of the current data.
// For example, a disconnected value may be greyed out.
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

// Set the Style Sheet string to be applied to implement a widget property.
// For example, a style string is used to set QE button text alignment.
void styleManager::updatePropertyStyle( QString style )
{
    propertyStyleSheet = style;
    updateStyleSheet();
}

// Update the style sheet with the various style sheet components used to modify the
// style (user level, connection state, alarm info, enumeration color)
void styleManager::updateStyleSheet()
{
    // Note,for QE widgets the styleSheet is now a non-designable property,
    // so inhibiting style updates is no longer applicable.
    //
    // Select the appropriate user level style
    QString userLevelStyle;
    switch( level )
    {
        case userLevelTypes::USERLEVEL_USER:
            userLevelStyle = userUserStyle;
            break;

        case userLevelTypes::USERLEVEL_SCIENTIST:
            userLevelStyle = userScientistStyle;
            break;

        case userLevelTypes::USERLEVEL_ENGINEER:
            userLevelStyle = userEngineerStyle;
            break;
    }

    // Compile and apply the entire style string if there is any difference
    // with what is currently there
    QString newStyleSheet = defaultStyleSheet;

    if( !newStyleSheet.isEmpty() && !connectionStyleSheet.isEmpty() )
        newStyleSheet.append( "\n" );

    newStyleSheet.append( connectionStyleSheet );

    if( !newStyleSheet.isEmpty() && !propertyStyleSheet.isEmpty() )
        newStyleSheet.append( "\n" );

    newStyleSheet.append( propertyStyleSheet );

    if( !newStyleSheet.isEmpty() && !statusStyleSheet.isEmpty() )
        newStyleSheet.append( "\n" );

    newStyleSheet.append( statusStyleSheet );

    if( !newStyleSheet.isEmpty() && !dataStyleSheet.isEmpty() )
        newStyleSheet.append( "\n" );

    newStyleSheet.append( dataStyleSheet );

    if( !newStyleSheet.isEmpty() && !userLevelStyle.isEmpty() )
        newStyleSheet.append( "\n" );

    newStyleSheet.append( userLevelStyle );

    // Apply the new style sheet if the widget is enabled
    // (and it is different to the current one)
    // (and we are not in Designer)
    if( owner->isEnabled() && newStyleSheet.compare( owner->styleSheet() ) && !QEWidget::inDesigner() )
    {
        owner->setStyleSheet( newStyleSheet );
    }

    // Keep an up-to-date copy of the style sheet. It will be applied to the widget if
    // the widget changes from being disabeld to enabled .
    currentStyle = newStyleSheet;
}

// Set the current user level.
void styleManager::styleUserLevelChanged( userLevelTypes::userLevels levelIn )
{
    // Note the new style and update the style string if changed
    bool newLevel = level != levelIn;
    level = levelIn;
    if( newLevel )
    {
        updateStyleSheet();
    }
}

// Called to notify the manager that the enabled state of the widget has changed.
// Styles are removed while disabled so the 'disabled' look is not hidden by
// the applied style)
// Styles are re-applied when enabled. The re-applied style is current and may
// have been calculated by the manager while the widget was disabled.
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

// Change Event Filter used to note when the widget becomes enabled or disabled.
// (styles are removed while disabled so the 'disabled' look is not hidden by the applied style)
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
