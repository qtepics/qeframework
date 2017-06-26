/*  windowCustomisation.cpp
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
 *  Copyright (c) 2013,2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/* Description:
 *
 * Helps applications such as QEGui and QE widgets add menu items from the main window menu bar
 * and push buttons to the main window tool bar.
 * Named customisations are saved in .xml files.
 * Any number of customisation .xml files can be read, building up a directory of named customisations.
 * Customisations can be requested by name.
 * Customisations may include other customisations by name.
 * A customisation can be aliased by defining a customisation that only includes another named customisation.
 *
 * REFER TO windowCustomisation.h for more details on how to use this module.
 */

#include "windowCustomisation.h"
#include <QDebug>
#include <QFile>
#include <QMenuBar>
#include <QToolBar>
#include <QEWidget.h>
#include <QDockWidget>
#include <QEScaling.h>
#include <macroSubstitution.h>

//==============================================================================================
// windowCustomisationItem
//==============================================================================================

// Construct instance of class defining an individual item when none exists (for example, a menu placeholder)
windowCustomisationItem::windowCustomisationItem() : iAction( NULL )
{
    commonInit();

//    creationOption = QEActionRequests::OptionNewWindow;
}

// Construct instance of class defining a built in application action
windowCustomisationItem::windowCustomisationItem( const QString builtInActionIn )
                                                  : iAction( NULL )
{
    commonInit();

    builtInAction = builtInActionIn;
}

windowCustomisationItem::windowCustomisationItem( const QString builtInActionIn,
                                                  const QString widgetNameIn )                           // widget name if built in function is for a widget, not the application
                                                  : iAction( NULL )
{
    commonInit();

    builtInAction = builtInActionIn;
    widgetName = widgetNameIn;
}

// Construct instance of class defining an individual item (base class for button or menu item)
windowCustomisationItem::windowCustomisationItem(
    const QObject* /*launchRequestReceiver*/,            // Object (typically QEGui application) which will accept requests to launch a new GUI
    const QList<windowCreationListItem>& windowsIn,      // Windows to display (centrals and docks)
    const QString programIn,                             // Program to run
    const QStringList argumentsIn )                      // Arguments for 'program'
         : iAction( NULL )
{
    commonInit();

    // Save the item details
    for( int i = 0; i < windowsIn.count(); i++ )
    {
        windows.append( windowsIn.at(i));
    }

    programLauncher.setProgramStartupOption( applicationLauncher::PSO_LOGOUTPUT );
    programLauncher.setProgram( programIn );
    programLauncher.setArguments( argumentsIn );
}

// Construct instance of class defining an individual item (base class for button) or menu item
windowCustomisationItem::windowCustomisationItem(windowCustomisationItem* item): iAction( NULL )
{
    commonInit();

    // Save the item details
    for( int i = 0; i < item->windows.count(); i++ )
    {
        windows.append( item->windows.at(i));
    }

    programLauncher.setProgramStartupOption( applicationLauncher::PSO_LOGOUTPUT );
    programLauncher.setProgram( item->getProgram() );
    programLauncher.setArguments( item->getArguments() );

    builtInAction = item->getBuiltInAction();

    widgetName = item->widgetName;

    dockTitle = item->dockTitle;

    userLevelVisible = item->userLevelVisible;
    userLevelEnabled = item->userLevelEnabled;

//    setUserLevelState( profile.getUserLevel() );
}

// Construct instance of class defining a link to an existing dock
windowCustomisationItem::windowCustomisationItem( const QString dockTitleIn, bool /*unused*/ ): iAction( NULL )
{
    commonInit();

    dockTitle = dockTitleIn;
}

// Initialisation common to all constructors
void windowCustomisationItem::commonInit()
{
    profile.takeLocalCopy();

    userLevelVisible = userLevelTypes::USERLEVEL_USER;
    userLevelEnabled = userLevelTypes::USERLEVEL_USER;
}

// A menu item or button has been created, let the application or widget know about it
void windowCustomisationItem::initialise()
{
    if( !builtInAction.isEmpty() && !widgetName.isEmpty() )
    {
        emit newGui( QEActionRequests( builtInAction, widgetName, QStringList(), true, iAction ) );
    }
}

// Return true if at least one dock is created by this item
bool windowCustomisationItem::createsDocks()
 {
     for( int i = 0; i < windows.count(); i++ )
     {
         if( QEActionRequests::isDockCreationOption( windows.at(i).creationOption ) )
         {
             return true;
         }
     }
     return false;
 }

// A user has triggered the menu item or button
void windowCustomisationItem::itemAction()
{
    // If the item action contains any window items, then open those windows
    if( windows.count() )
    {
        profile.publishOwnProfile();
        emit newGui( QEActionRequests( windows ));
        profile.releaseProfile();
    }

    // If the item action references a built-in action, then request it
    else if ( !builtInAction.isEmpty() )
    {
        // If no widget name, then assume the action is for the application
        if( widgetName.isEmpty() )
        {
            emit newGui( QEActionRequests( builtInAction, "" )  );
        }
        // A widget name is present, assume the action is for a QE widget created by the application
        else
        {
            emit newGui( QEActionRequests( builtInAction, widgetName, QStringList(), false, iAction ) );
        }
    }

    // If the action is associated with a program, launch it
    programLauncher.launch( NULL, NULL );
}

// Note user level access restriction if any
void windowCustomisationItem::addUserLevelAccess( QDomElement element, customisationLog& log )
{
    log.startLevel();

    QString userLevelName;

    // Note the user level at which the menu is enabled
    userLevelName = element.attribute( "UserLevelEnabled" );
    if( !userLevelName.isEmpty() )
    {
        userLevelEnabled = ContainerProfile::getUserLevelValue( userLevelName );
        log.add( "Item will be enabled at user level ", ContainerProfile::getUserLevelName( userLevelEnabled ) );
    }

    // Note the user level at which the menu is visible
    userLevelName = element.attribute( "UserLevelVisible" );
    if( !userLevelName.isEmpty() )
    {
        userLevelVisible = ContainerProfile::getUserLevelValue( userLevelName );
        log.add( "Item will be visible at user level ", ContainerProfile::getUserLevelName( userLevelVisible ) );
    }

    log.endLevel();
}

// Set the visibility and enabled state of the item according to the user level
void windowCustomisationItem::setUserLevelState( userLevelTypes::userLevels currentUserLevel )
{
    if( !iAction ) return; // sainty check

    // Set the menu visibility according to user level
    iAction->setVisible( userLevelVisible <= currentUserLevel );

    // Set the menu enabled state according to user level
    iAction->setEnabled( userLevelEnabled <= currentUserLevel );
}

//==============================================================================================
// windowCustomisationMenuItem
//==============================================================================================

// Construct instance of class defining an individual menu item (not a placeholder for items the application might add)
windowCustomisationMenuItem::windowCustomisationMenuItem(
                          customisationLog& log,                               // Log of customisation files loaded for diagnosis.

                          const QStringList menuHierarchyIn,                   // Location in menus to place this item. for example: 'Imaging'->'Region of interest'
                          const QString titleIn,                               // Name of this item. for example: 'Region 1'
                          const menuObjectTypes typeIn,                        // type of menu object - must be MENU_ITEM
                          const bool separatorIn,                              // Separator required before this
                          const itemCheckInfo& checkInfoIn,                    // Information about the item's checkable state

                          const QObject* launchRequestReceiver,                // Object (typically QEGui application) which will accept requests to launch a new GUI
                          const QList<windowCreationListItem>& windowsIn,      // Windows to display (centrals and docks)
                          const QString programIn,                             // Program to run
                          const QStringList argumentsIn )                      // Arguments for 'program'
                          : windowCustomisationItem( launchRequestReceiver, windowsIn, programIn, argumentsIn )
{
    type = typeIn;
    menuHierarchy = menuHierarchyIn;
    title = titleIn;
    separator = separatorIn;
    checkInfo = checkInfoIn;

    log.add( "Adding menu item: ",  title );
    logItem( log );
}

// Log the details of a menu customisation item
void windowCustomisationItem::logItem( customisationLog& log )
{
    log.startLevel();
    if( windows.count() )
    {
        for( int i = 0; i < windows.count(); i++ )
        {
            log.add( "Create new window:" );
            log.startLevel();
            log.add( QString( "Title: " ).append( windows.at(i).title ) );
            log.add( QString( "Customisations: " ).append( windows.at(i).customisationName ) );
            log.add( QString( ".ui file: " ).append( windows.at(i).uiFile ) );
            log.add( QString( "Macro substitutions: " ).append( windows.at(i).macroSubstitutions ) );
            log.endLevel();
        }
    }

    if( !dockTitle.isEmpty() )
    {
        log.add( QString( "Dock title: " ).append( dockTitle ) );
    }

    if( !builtInAction.isEmpty() )
    {
        log.add( QString( "Built-in action: " ).append( builtInAction ) );
    }

    if( !widgetName.isEmpty() )
    {
        log.add( QString( "Associated widget name: " ).append( widgetName ) );
    }

    if( !guiTitle.isEmpty() )
    {
        log.add( QString( "GUI title: " ).append( guiTitle ) );
    }
    log.endLevel();
}

// Construct instance of class defining an item that will request the application (or a QE widget) take a named action
windowCustomisationMenuItem::windowCustomisationMenuItem(
                          customisationLog& log,                               // Log of customisation files loaded for diagnosis.

                          const QStringList menuHierarchyIn,                   // Location in menus for application to place future items. for example: 'File' -> 'Recent'
                          const QString titleIn,                               // Title for this item. for example: 'Region 1' Usually same as name of built in function. (for example, function='Copy' and title='Copy', but may be different (function='LaunchApplication1' and title='paint.exe')
                          const menuObjectTypes typeIn,                        // type of menu object - must be MENU_PLACEHOLDER or MENU_BUILT_IN
                          const bool separatorIn,                              // Separator required before this
                          const itemCheckInfo& checkInfoIn,                     // Information about the item's checkable state

                          const QString builtIn,                               // Name of built in function (built into the application or a QE widget). For example: 'Region 1'
                          const QString widgetNameIn )                         // widget name if built in function is for a widget, not the application

                          : windowCustomisationItem( builtIn, widgetNameIn )
{
    type = typeIn;
    menuHierarchy = menuHierarchyIn;
    title = titleIn;
    separator = separatorIn;
    checkInfo = checkInfoIn;

    log.add( "Adding menu item requesting an action: ",  title );
}

// Construct instance of class defining an item that will be a placeholder. The application can locate placeholder menu items and use them directly
windowCustomisationMenuItem::windowCustomisationMenuItem(
                          customisationLog& log,                               // Log of customisation files loaded for diagnosis.

                          const QStringList menuHierarchyIn,                   // Location in menus for application to place future items. for example: 'File' -> 'Recent'
                          const QString titleIn,                               // Identifier of placeholder. for example: 'Recent'
                          const menuObjectTypes typeIn,                        // type of menu object - must be MENU_PLACEHOLDER or MENU_BUILT_IN
                          const bool separatorIn,                              // Separator required before this
                          const itemCheckInfo& checkInfoIn )                    // Information about the item's checkable state

                          : windowCustomisationItem( titleIn )
{
    type = typeIn;
    menuHierarchy = menuHierarchyIn;
    title = titleIn;
    separator = separatorIn;
    checkInfo = checkInfoIn;

    log.add( "Adding placeholder menu item: ",  title );
}

// Construct instance of class defining an item that will be associated with an existing dock (association is by dock title)
windowCustomisationMenuItem::windowCustomisationMenuItem(
                      customisationLog& log,                               // Log of customisation files loaded for diagnosis.

                      const QStringList menuHierarchyIn,                   // Location in menus for application to place future items. for example: 'File' -> 'Recent'
                      const QString titleIn,                               // Title for this item. for example: 'Brightness/Contrast' Must match the title of the dock widget it is to be associated with.
                      const menuObjectTypes typeIn,                        // type of menu object - must be MENU_ITEM
                      const bool separatorIn,                              // Separator required before this
                      const itemCheckInfo& checkInfoIn,                     // Information about the item's checkable state

                      const QString dockTitleIn )                          // Title of existing dock widget to assocaite the menu item with
                      : windowCustomisationItem( dockTitleIn, true )
{
    type = typeIn;
    menuHierarchy = menuHierarchyIn;
    title = titleIn;
    separator = separatorIn;
    checkInfo = checkInfoIn;

    log.add( "Adding menu item linked to a dock: ",  title );
}


// Copy constructor
windowCustomisationMenuItem::windowCustomisationMenuItem(windowCustomisationMenuItem* menuItem)                  // New window customisation name (menu, buttons, etc)
                          : windowCustomisationItem( menuItem )
{
    type = menuItem->type;
    menuHierarchy = menuItem->getMenuHierarchy();
    title = menuItem->getTitle();
    
    separator = menuItem->separator;
    checkInfo = menuItem->checkInfo;
    iAction = new QAction( title, this );

    // Set up an action to respond to the user
    connect( iAction, SIGNAL( triggered()), this, SLOT(itemAction()));
}

// Add an initial menu hierarchy.
// Used when including a customisation set at a particuar point in another customisation set
void windowCustomisationMenuItem::prependMenuHierarchy( QStringList preMenuHierarchy )
{
    int count = preMenuHierarchy.count();
    for( int i = count - 1; i >= 0; i-- )
    {
        menuHierarchy.prepend( preMenuHierarchy.at(i) );
    }
}

//==============================================================================================
// windowCustomisationButtonItem
//==============================================================================================

// Construct instance of class defining an individual button item
windowCustomisationButtonItem::windowCustomisationButtonItem(
                        const QString buttonGroupIn,                         // Name of toolbar button group in which to place a button
                        const QString buttonToolbarIn,                       // Name of toolbar in which to place a button
                        const Qt::ToolBarArea buttonLocationIn,              // Location of toolbar in which to place a button
                        const QString buttonTextIn,                          // Text to place in button
                        const QString buttonIconIn,                          // Icon for button

                        const QObject* launchRequestReceiver,                // Object (typically QEGui application) which will accept requests to launch a new GUI
                        const QList<windowCreationListItem>& windowsIn,      // Windows to display (centrals and docks)
                        const QString programIn,                             // Program to run
                        const QStringList argumentsIn )                      // Arguments for 'program'
                            : windowCustomisationItem( launchRequestReceiver, windowsIn, programIn, argumentsIn )
{
    buttonGroup    = buttonGroupIn;
    buttonToolbar  = buttonToolbarIn;
    buttonLocation = buttonLocationIn;
    buttonText     = buttonTextIn;
    buttonIcon     = buttonIconIn;
}

// Construct instance of class defining an individual button item
windowCustomisationButtonItem::windowCustomisationButtonItem(
                        const QString buttonGroupIn,                         // Name of toolbar button group in which to place a button
                        const QString buttonToolbarIn,                       // Name of toolbar in which to place a button
                        const Qt::ToolBarArea buttonLocationIn,              // Location of toolbar in which to place a button
                        const QString buttonTextIn,                          // Text to place in button
                        const QString buttonIconIn,                          // Icon for button

                        const QString builtIn,                               // Name of built in function (built into the application or a QE widget). For example: 'Region 1'
                        const QString widgetNameIn )                        // widget name if built in function is for a widget, not the application
                            : windowCustomisationItem( builtIn, widgetNameIn )
{
    buttonGroup    = buttonGroupIn;
    buttonToolbar  = buttonToolbarIn;
    buttonLocation = buttonLocationIn;
    buttonText     = buttonTextIn;
    buttonIcon     = buttonIconIn;
}

// Construct instance of class defining an individual button item
windowCustomisationButtonItem::windowCustomisationButtonItem(
                        const QString buttonGroupIn,                         // Name of toolbar button group in which to place a button
                        const QString buttonToolbarIn,                       // Name of toolbar in which to place a button
                        const Qt::ToolBarArea buttonLocationIn,              // Location of toolbar in which to place a button
                        const QString buttonTextIn,                          // Text to place in button
                        const QString buttonIconIn )                         // Icon for button
                            : windowCustomisationItem()
{
    buttonGroup    = buttonGroupIn;
    buttonToolbar  = buttonToolbarIn;
    buttonLocation = buttonLocationIn;
    buttonText     = buttonTextIn;
    buttonIcon     = buttonIconIn;
}

// Copy construct
windowCustomisationButtonItem::windowCustomisationButtonItem(windowCustomisationButtonItem* buttonItem)                  // New window customisation name (menu, buttons, etc)
                            : windowCustomisationItem( buttonItem )
{
    buttonGroup    = buttonItem->getButtonGroup();
    buttonToolbar  = buttonItem->getButtonToolbar();
    buttonLocation = buttonItem->getButtonLocation();
    buttonText     = buttonItem->getButtonText();
    buttonIcon     = buttonItem->getButtonIcon();
    iAction = new QAction( buttonText, this );

    // Set up an action to respond to the user
    connect( iAction, SIGNAL( triggered()), this, SLOT(itemAction()));
}

//==============================================================================================
// windowCustomisation
//==============================================================================================

// Class defining the customisation for a window.
// Construction - create a named, empty, customisation
windowCustomisation::windowCustomisation( QString nameIn )//: recentMenuPoint(NULL), windowMenuPoint(NULL), recentMenuSet(false), windowMenuSet(false)
{
    name = nameIn;
}

// Destruction - release customisation items
windowCustomisation::~windowCustomisation()
{
    // Release customisation items
    while( !menuItems.isEmpty() )
    {
        delete menuItems.takeFirst();
    }
    while( !buttons.isEmpty() )
    {
        delete buttons.takeFirst();
    }
}

// Add a menu item to the customisation
// NOTE! windowCustomisation TAKES OWNERSHIP of menuItem
void windowCustomisation::addItem( windowCustomisationMenuItem* menuItem, QStringList preMenuHierarchy )
{
    menuItems.append( menuItem );
    menuItems.last()->prependMenuHierarchy( preMenuHierarchy );
}

// Add a button to the customisation
// NOTE! windowCustomisation TAKES OWNERSHIP of button
void windowCustomisation::addItem( windowCustomisationButtonItem* button )
{
    buttons.append( button );
}

// Translate creation option text from .xml file to enumeration in QEActionRequests
QEActionRequests::Options windowCustomisation::translateCreationOption( QString creationOption )
{
         if( creationOption.compare( "Open"              ) == 0 ) { return QEActionRequests::OptionOpen;                   }
    else if( creationOption.compare( "NewTab"            ) == 0 ) { return QEActionRequests::OptionNewTab;                 }
    else if( creationOption.compare( "NewWindow"         ) == 0 ) { return QEActionRequests::OptionNewWindow;              }

    else if( creationOption.compare( "FloatingDock"      ) == 0 ) { return QEActionRequests::OptionFloatingDockWindow;     }

    else if( creationOption.compare( "LeftDock"          ) == 0 ) { return QEActionRequests::OptionLeftDockWindow;         }
    else if( creationOption.compare( "RightDock"         ) == 0 ) { return QEActionRequests::OptionRightDockWindow;        }
    else if( creationOption.compare( "TopDock"           ) == 0 ) { return QEActionRequests::OptionTopDockWindow;          }
    else if( creationOption.compare( "BottomDock"        ) == 0 ) { return QEActionRequests::OptionBottomDockWindow;       }

    else if( creationOption.compare( "LeftDockTabbed"    ) == 0 ) { return QEActionRequests::OptionLeftDockWindowTabbed;   }
    else if( creationOption.compare( "RightDockTabbed"   ) == 0 ) { return QEActionRequests::OptionRightDockWindowTabbed;  }
    else if( creationOption.compare( "TopDockTabbed"     ) == 0 ) { return QEActionRequests::OptionTopDockWindowTabbed;    }
    else if( creationOption.compare( "BottomDockTabbed"  ) == 0 ) { return QEActionRequests::OptionBottomDockWindowTabbed; }

    return QEActionRequests::OptionNewWindow;  // Default
}

//==============================================================================================
// windowCustomisationList
//==============================================================================================

windowCustomisationList::windowCustomisationList()
{
    // Initialise
    toBeActivatedMW = NULL;
    toBeActivatedMenu = NULL;

    // Load QE widget customisations.
    loadCustomisation( ":/qe/configuration/QEImageCustomisationDefault.xml" );
    // Add other QE widget's customisation files here as required
}

// Load a set of customisations
bool windowCustomisationList::loadCustomisation( QString xmlFile )
{
    // If no file specified, silently return (no error)
    if( xmlFile.isEmpty() )
    {
        return false;
    }

    // Log progress
    log.add( "Loading: ",  xmlFile );
    log.startLevel();

    QDomDocument doc;

    // Read and parse xmlFile
    QFile* file = QEWidget::findQEFile( xmlFile );
    if( !file )
    {
        log.add( "Could not find file" );
        log.flagError();
        log.endLevel();
        return false;
    }

    if( !file->open(QIODevice::ReadOnly) )
    {
        QString error = file->errorString();
        log.add( "Error opening file:", error );
        log.flagError();
        log.endLevel();
        return false;
    }

    // if named customisation exists, replace it
    if ( !doc.setContent( file ) )
    {
        file->close();
        delete file;

        log.add( "Could not parse the XML in the customisations file" );
        log.flagError();

        log.endLevel();
        return false;
    }
    file->close();
    delete file;
    file = NULL; // Ensure no further reference
    QDomElement docElem = doc.documentElement();

    // Load customisation include file
    QDomElement customisationIncludeFileElement = docElem.firstChildElement( "CustomisationIncludeFile" );
    while( !customisationIncludeFileElement.isNull() )
    {
        QString includeFileName = customisationIncludeFileElement.text();
        if( !includeFileName.isEmpty() )
        {
            // load customisation file
            log.add( "Including customisations file: ", includeFileName );
            log.startLevel();
            loadCustomisation(includeFileName);
            log.endLevel();
        }
        customisationIncludeFileElement = customisationIncludeFileElement.nextSiblingElement( "CustomisationIncludeFile" );
    }

    // Parse XML using Qt's Document Object Model.
    QDomElement customisationElement = docElem.firstChildElement( "Customisation" );
    while( !customisationElement.isNull() )
    {
        QString customisationName = customisationElement.attribute( "Name" );
        if( !customisationName.isEmpty() )
        {
            log.add( "Load customisation name: ", customisationName );
            // create a window customisation
            windowCustomisation* customisation = new windowCustomisation(customisationName);
            // add the window customisation to the list
            customisationList.prepend( customisation );
            // get a first node
            QDomNode node = customisationElement.firstChild();
            // check if the item is a menu or a button item
            log.startLevel();
            while (!node.isNull())
            {
                QDomElement element = node.toElement();

                // Add a menu
                if( element.tagName() == "Menu" )
                {
                    QString menuName = element.attribute( "Name" );
                    QStringList menuHierarchy;
                    menuHierarchy.append(menuName);
                    log.add( "Adding menu: ",  menuName );

                    // parse menu customisation
                    log.startLevel();
                    parseMenuElement( element, customisation, menuHierarchy );
                    log.endLevel();
                }

                // Create a menu item if required
                else if( element.tagName() == "Item" )
                {
                    QStringList menuHierarchy;
                    customisation->addItem( createMenuItem( element, menuHierarchy ));
                }

                // Create a placeholder item if required
                else if( element.tagName() == "PlaceHolder" )
                {
                    QStringList menuHierarchy;
                    customisation->addItem( createMenuPlaceholder( element, menuHierarchy ));
                }


                // Add a tool bar button
                else if (element.tagName() == "Button")
                {
                    log.startLevel();

                    // create and add a button item
                    windowCustomisationButtonItem* button = createButtonItem( element );
                    log.add( "Adding toolbar button: ",  button->getButtonText() );
                    button->addUserLevelAccess( element, log );
                    customisation->addItem( button );

                    log.endLevel();
                }

                // Add an include file
                else if (element.tagName() == "IncludeCustomisation")
                {
                    // add all customisation items to the current customisation set
                    addIncludeCustomisation( element, customisation );
                }
                node = node.nextSibling();
            }
            log.endLevel();
        }
        customisationElement = customisationElement.nextSiblingElement( "Customisation" );
    }

    log.endLevel();
    return true;
}

// Parse menu customisation data
void windowCustomisationList::parseMenuElement( QDomElement element, windowCustomisation* customisation, QStringList menuHierarchy)
{
    // Parse the menu's children elements
    QDomElement childElement = element.firstChildElement();
    while (!childElement.isNull())
    {
         // check if the item is a submenu or an item
        if (childElement.tagName() == "Menu")
        {
            // get the menu name
            QString menuName = childElement.attribute( "Name" );
            if( !menuName.isEmpty() )
            {
                // copy it over
                QStringList hierarchy = menuHierarchy;
                // update menu hierarchy
                hierarchy.append(menuName);
                // parse menu customisation
                parseMenuElement( childElement, customisation, hierarchy );
            }
        }

        else
        {
            // Add an include file
            if (childElement.tagName() == "IncludeCustomisation")
            {
                // add all customisation items to the current customisation set
                addIncludeCustomisation( childElement, customisation, menuHierarchy );
            }

            // Item to add if found
            windowCustomisationMenuItem* item = NULL;

            // Create a menu item if required
            if( childElement.tagName() == "Item" )
            {
                item = createMenuItem( childElement, menuHierarchy );
            }

            // Create a placeholder item if required
            else if( childElement.tagName() == "PlaceHolder" )
            {
                item = createMenuPlaceholder( childElement, menuHierarchy );
            }

            // If an item was created, add it
            if( item )
            {
                item->addUserLevelAccess( childElement, log );
                customisation->addItem( item );
            }
        }

        childElement = childElement.nextSiblingElement();
    }
}

// Determine if an item contains a 'Separator' tag
bool windowCustomisationList::requiresSeparator( QDomElement itemElement )
{
    // Determine if separator is required
    QDomElement separatorElement = itemElement.firstChildElement( "Separator" );
    return !separatorElement.isNull();
}

//// Add details for a menu item to customisation set
//windowCustomisationMenuItem* windowCustomisationList::createMenuBuiltIn( QDomElement itemElement, QStringList menuHierarchy)
//{
//    QString name = itemElement.attribute( "Name" );
//    if( name.isEmpty() )
//        return NULL;

//    // Add details for a built in menu item to customisation set
//    windowCustomisationMenuItem* item = new windowCustomisationMenuItem( menuHierarchy, name, windowCustomisationMenuItem::MENU_BUILT_IN, requiresSeparator( itemElement ) );
//    return item;
//}

// Add details for a menu item to customisation set
windowCustomisationMenuItem* windowCustomisationList::createMenuPlaceholder( QDomElement itemElement, QStringList menuHierarchy)
{
    QString name = itemElement.attribute( "Name" );
    if( name.isEmpty() )
        return NULL;

    // Add details for a placeholder (where the applicaiton can add menu items) to customisation set
    windowCustomisationMenuItem* item = new windowCustomisationMenuItem( log, menuHierarchy, name,
                                                                         windowCustomisationMenuItem::MENU_PLACEHOLDER,
                                                                         requiresSeparator( itemElement ),
                                                                         itemCheckInfo( itemElement ) );
    return item;
}

// Parse the contents of a menu item or tool bar button
bool windowCustomisationList::parseMenuAndButtonItem( QDomElement itemElement,
                                                      QString& title,
                                                      QList<windowCreationListItem>& windows,
                                                      QString& builtIn,
                                                      QString& program,
                                                      QString& widgetName,
                                                      QStringList& arguments,
                                                      QString& dockTitle )
{
    // Get the name
    //!!! should this be optional for a button if an icon is supplied???
    title = itemElement.attribute( "Name" );
    if( title.isEmpty() )
    {
        return false;
    }

    // Read Program
    QDomElement programElement = itemElement.firstChildElement( "Program" );
    if( !programElement.isNull() )
    {
        // read Program name and args
        program = programElement.attribute( "Name" );
        QDomElement argumentsElement = programElement.firstChildElement( "Arguments" );
        if( !argumentsElement.isNull() )
        {
            arguments = argumentsElement.text().split(" ");
        }
    }

    // Read Built In function
    QDomElement builtInElement = itemElement.firstChildElement( "BuiltIn" );
    if( !builtInElement.isNull() )
    {
        // Read Built In function name
        builtIn = builtInElement.attribute( "Name" );

        QDomElement childElement = builtInElement.firstChildElement();
        while (!childElement.isNull())
        {
            QString tagName = childElement.tagName();

             // Note the widget target, if any
            if( tagName == "WidgetName" )
            {
                // Get the name of the target widget
                widgetName = childElement.text();
            }

           childElement = childElement.nextSiblingElement();
        }
    }

    // Read windows to create
    QDomElement windowElement = itemElement.firstChildElement( "Window" );
    while( !windowElement.isNull() )
    {
        windowCreationListItem windowItem;

        // Read UiFile name
        QDomElement uiFileElement = windowElement.firstChildElement( "UiFile" );
        if( !uiFileElement.isNull() )
        {
            windowItem.uiFile = uiFileElement.text();
        }

        // Read optional macro substitutions
        QDomElement macroSubstitutionsElement = windowElement.firstChildElement( "MacroSubstitutions" );
        if( !macroSubstitutionsElement.isNull() )
        {
            windowItem.macroSubstitutions = macroSubstitutionsElement.text();
        }

        // Read optional customisation name
        QDomElement customisationNameElement = windowElement.firstChildElement( "CustomisationName" );
        if( !customisationNameElement.isNull() )
        {
            windowItem.customisationName = customisationNameElement.text();
        }

        // Read optional creation option
        QDomElement creationOptionElement = windowElement.firstChildElement( "CreationOption" );
        windowItem.creationOption = QEActionRequests::OptionNewWindow;
        if( !creationOptionElement.isNull() )
        {
            windowItem.creationOption = windowCustomisation::translateCreationOption( creationOptionElement.text() );
        }

        // Read optional title (This title will override any title in the title property of the top widget in the .ui file)
        QDomElement titleElement = windowElement.firstChildElement( "Title" );
        if( !titleElement.isNull() )
        {
            windowItem.title = titleElement.text();
        }

        // Add a window to the list of windows to create
        windows.append( windowItem );

        // Read any docks to be added to this window
        // No title is specified - dock will get its title from the .ui file
        QString noTitle;
        parseDockItem( windowElement, windows, dockTitle, noTitle );

        windowElement = windowElement.nextSiblingElement( "Window" );
    }

    // Add a dock if this item contains a dock.
    // If loading a GUI ( not associating the item with an existing dock) the title of the GUI (and of it's associated action) will be set to this item's title
    parseDockItem( itemElement, windows, dockTitle, title );
    return true;
}

// Parse a Dock element.
// If the dock has a title, return the title: This item is to be linked to a pre-existing dock with the given title.
// If the dock has a UI file, return a single 'window creation list item' containing the UI file to be opened as a dock.
void windowCustomisationList::parseDockItem( QDomElement itemElement,
                                              QList<windowCreationListItem>& windows,   // List of docks to create
                                              QString& dockTitle,                       // Title of pre-existing dock a menu item or button is to be associated with
                                              QString& guiTitle )                       // Title of new GUI used when creating a new dock
{
    // Get the dock element
    QDomElement dockElement = itemElement.firstChildElement( "Dock" );
    if( dockElement.isNull() )
    {
        return;
    }

    // If first Dock has a title, we are not creating one or more windows, we are just linking up with a dock with the given title.
    QDomElement titleElement = dockElement.firstChildElement( "Title" );
    if( !titleElement.isNull() )
    {
        dockTitle = titleElement.text();
        return;
    }

    // If first Dock has a UI file, note the UI file name and other details required for starting a new GUI as a dock.
    QDomElement uiFileElement = dockElement.firstChildElement( "UiFile" );
    if( !uiFileElement.isNull() )
    {
        QString uiFile = uiFileElement.text();
        if( !uiFile.isEmpty() )
        {
            windowCreationListItem windowItem;
            windowItem.uiFile = uiFile;
            windowItem.title = guiTitle;

            QDomElement macroSubstitutionsElement = dockElement.firstChildElement( "MacroSubstitutions" );
            if( !macroSubstitutionsElement.isNull() )
            {
                windowItem.macroSubstitutions = macroSubstitutionsElement.text();
            }

            QDomElement creationOptionElement = dockElement.firstChildElement( "CreationOption" );
            windowItem.creationOption = QEActionRequests::OptionFloatingDockWindow;

            if( !creationOptionElement.isNull() )
            {
                windowItem.creationOption = windowCustomisation::translateCreationOption( creationOptionElement.text() );
            }

            QDomElement hiddenElement = dockElement.firstChildElement( "Hidden" );
            if( !hiddenElement.isNull() )
            {
                windowItem.hidden = true;
            }

            windows.append( windowItem );
        }
    }
}


// Add details for a menu item to customisation set
windowCustomisationMenuItem* windowCustomisationList::createMenuItem( QDomElement itemElement, QStringList menuHierarchy)
{
    QString title;                          // Menu item title
    QString program;                        // Program to run when the user selects this menu item
    QStringList arguments;                  // Arguments to supply to 'program'
    QList<windowCreationListItem> windows;  // Windows to create (displaying .UI files) when the user selects this menu item
    QString builtIn;                        // Function (built in to the application, or a QE widget) to call when the user selects this menu item
    QString widgetName;                     // QE widget name to pass built in function request to. If not provided, the built in function is assumed to be handled by the application
    QString dockTitle;                      // Title of existing dock to associate this menu item with

    if( parseMenuAndButtonItem( itemElement, title, windows, builtIn, program, widgetName, arguments, dockTitle ) )
    {
        if( !dockTitle.isEmpty() )
        {
            // Add details for a existing dock menu item to customisation set
            windowCustomisationMenuItem* item = new windowCustomisationMenuItem( log,
                                                                                 menuHierarchy,
                                                                                 title,
                                                                                 windowCustomisationMenuItem::MENU_ITEM,
                                                                                 requiresSeparator( itemElement ),
                                                                                 itemCheckInfo( itemElement ),

                                                                                 dockTitle );
            return item;

        }
        else if( !builtIn.isEmpty() )
        {
            // Add details for a built in menu item to customisation set
            windowCustomisationMenuItem* item = new windowCustomisationMenuItem( log,
                                                                                 menuHierarchy,
                                                                                 title,
                                                                                 windowCustomisationMenuItem::MENU_BUILT_IN,
                                                                                 requiresSeparator( itemElement ),
                                                                                 itemCheckInfo( itemElement ),

                                                                                 builtIn,
                                                                                 widgetName );
            return item;

        }
        else
        {
            // Add details for a menu item to customisation set
            windowCustomisationMenuItem* item = new windowCustomisationMenuItem( log,
                                                                                 menuHierarchy,
                                                                                 title,
                                                                                 windowCustomisationMenuItem::MENU_ITEM,
                                                                                 requiresSeparator( itemElement ),
                                                                                 itemCheckInfo( itemElement ),

                                                                                 NULL/*!!! needs launch receiver object*/,
                                                                                 windows,
                                                                                 program,
                                                                                 arguments );
            return item;
        }
    }
    else
    {
        return NULL;
    }
}

// Add details for a tool bar button item to customisation set
 windowCustomisationButtonItem*  windowCustomisationList::createButtonItem( QDomElement itemElement )
 {
    QString buttonGroup;
    QString buttonToolbar;
    Qt::ToolBarArea buttonLocation;
    QString buttonIcon;

    // Read GroupName
    buttonGroup = itemElement.attribute( "Group" );

    // Read Toolbar name
    buttonToolbar = itemElement.attribute( "Toolbar" );
    if( buttonToolbar.isEmpty() )
    {
        buttonToolbar = "Toolbar";
    }

    // Read Toolbar location
    buttonLocation = Qt::TopToolBarArea;    // Assume top
    QString location = itemElement.attribute( "Location" );

    if( !location.compare( "Left") )
    {
        buttonLocation = Qt::LeftToolBarArea;
    }
    else if( !location.compare( "Right") )
    {
        buttonLocation = Qt::RightToolBarArea;
    }
    else if( !location.compare( "Bottom") )
    {
        buttonLocation = Qt::BottomToolBarArea;
    }

    // Read Icon
    buttonIcon = itemElement.attribute( "Icon" );

    QString title;                          // Menu item title
    QString program;                        // Program to run when the user selects this menu item
    QStringList arguments;                  // Arguments to supply to 'program'
    QList<windowCreationListItem> windows;  // Windows to create (displaying .UI files) when the user selects this menu item
    QString builtIn;                        // Function (built in to the application, or a QE widget) to call when the user selects this menu item
    QString widgetName;                     // QE widget name to pass built in function request to. If not provided, the built in function is assumed to be handled by the application
    QString dockTitle;                      // Title of existing dock to associate this menu item with


     if( parseMenuAndButtonItem( itemElement, title, windows, builtIn, program, widgetName, arguments, dockTitle ) )
     {
         // If any windows or a program, build an item that holds these
         // (Not sure why these are grouped. Either all options (windows, program, or built-in) should be mutually exclusive, or any mix allowed
         if( windows.count() || !program.isEmpty() )
         {
             // Add details for a button item to customisation set
             windowCustomisationButtonItem* item = new windowCustomisationButtonItem( buttonGroup,
                                                                                      buttonToolbar,
                                                                                      buttonLocation,
                                                                                      title,
                                                                                      buttonIcon,
                                                                                      NULL/*!!! needs launch receiver object*/,
                                                                                      windows,
                                                                                      program,
                                                                                      arguments );

             return item;
         }
         // No windows or program, so assume a built in function
         else
         {
             // Add details for a button item to customisation set
             windowCustomisationButtonItem* item = new windowCustomisationButtonItem( buttonGroup,
                                                                                      buttonToolbar,
                                                                                      buttonLocation,
                                                                                      title,
                                                                                      buttonIcon,
                                                                                      builtIn,
                                                                                      widgetName );

             return item;
         }
     }
     else
     {
         return NULL;
     }
}

 // Return a named set of customisations
 windowCustomisation* windowCustomisationList::getCustomisation(QString name)
 {
     // search for and return customisation
     for( int i = 0; i < customisationList.length(); i++ )
     {
         if( customisationList.at(i)->getName() == name )
         {
            return customisationList.at(i);
         }
     }

     // Customisations not found
     return NULL;
 }

 // Parse a named customisation set
 void windowCustomisationList::addIncludeCustomisation( QDomElement includeCustomisationElement, windowCustomisation* customisation, QStringList menuHierarchy )
 {
     QString includeCustomisationName = includeCustomisationElement.attribute( "Name" );

     log.add( "Including named customisation set: ", includeCustomisationName );

     // Attempt to get the named customisation set
     windowCustomisation* includeCustomisation = getCustomisation( includeCustomisationName );

     // If the named customisation set was found, then add it
     if (includeCustomisation)
     {
         // add menu items
         QList<windowCustomisationMenuItem*> menuItems = includeCustomisation->getMenuItems();
         for (int i = 0; i < menuItems.length(); i++)
         {
             customisation->addItem(  menuItems.at(i), menuHierarchy );
         }

         // add button items
         QList<windowCustomisationButtonItem*> buttons = includeCustomisation->getButtons();
         for (int i = 0; i < buttons.length(); i++)
         {
             customisation->addItem(buttons.at(i));
         }
     }
     else
     {
         log.add( "Could not include customisation set. Perhaps it has not be read in at this stage." );
         log.flagError();
     }
}

// Ensure a menu path exists in the menu bar.
// For example, if the menu path required is 'File' -> 'Edit', and a
// 'File' menu exists but does not contain an 'Edit' menu, then add an 'Edit' menu to the 'File' menu.
// Return the end menu (newly created, or found)
QMenu* windowCustomisationList::buildMenuPath( windowCustomisationInfo* customisationInfo, QMenuBar* menuBar, const QStringList menuHierarchy )
{
    // Work through the anticipated menu hierarchy creating missing menus as required
    QMenu* menuPoint = NULL;
    QString hierarchyString;
    int count = menuHierarchy.count();
    bool skipSearch = false;
    for( int i = 0; i < count; i++ )
    {
        // Add the next menu level
        hierarchyString.append( menuHierarchy.at(i) );

        // Look for the next menu level
        QMenu* menu = NULL;
        if( !skipSearch )
        {
            menu = customisationInfo->menus.value( hierarchyString, NULL );
        }

        // If the next menu level is present, note it
        if( menu )
        {
            menuPoint = menu;
        }

        // The next menu level is not present, create it
        else
        {
            // No point looking for deeper menus from now on
            skipSearch = true;

            // Create the menu
            QMenu* newMenu = new QMenu( menuHierarchy.at(i) );
            QEScaling::applyToWidget( newMenu );

            // Add it to the next level up (if any)
            if( menuPoint )
            {
                // add the menu
                menuPoint->addMenu( newMenu );
            }

            // Otherwise add it to the menu bar
            else
            {
                menuBar->addMenu( newMenu );
                menuBar->setVisible( true );
            }

            // Save the menu just added as the menu to add the next menu to in this loop
            menuPoint = newMenu;

            // Save the menu for some future menu search
            customisationInfo->menus.insert( hierarchyString, newMenu );
        }

        // Add the menu separator.
        // Note, this is done so things are easier in the debugger (File>Edit is
        // clearly two levels of menu compared to FileEdit). Also, it may help
        // avoid conflicts; for example if there is a 'File' menu containing an
        // 'Edit' menu and there is also a top level menu is called 'FileEdit'.
        if( i < count - 1 )
        {
            hierarchyString.append( ">" );
        }
    }

    return menuPoint;
}

// Initialise all the customisation items present in a window's menu bar and tool bar.
// The QEGui application uses this method after loading a GUI so that all the QE widgets in the
// GUI will be notified of any customisation items they may be interested in.
// Other application should call this method after creating any QE widgets if they want the QE widgets
// to interact with the customisation system.
void windowCustomisationList::initialise( windowCustomisationInfo* customisationInfo )
{
    int count = customisationInfo->items.count();
    for( int i = 0; i < count; i++ )
    {
        customisationInfo->items.at(i)->initialise();
    }
}

// Repond to a user level change (this is an implementation for the base ContainerProfile class
void windowCustomisationList::userLevelChangedGeneral( userLevelTypes::userLevels currentUserLevel )
{
    // Update the visibility and enabled state of all items in all customisation sets
    for( int i = 0; i < customisationList.count(); i++ )
    {
        windowCustomisation* customisation = customisationList.at( i );

        QList<windowCustomisationMenuItem*> menuItems = customisation->getMenuItems();
        for (int j = 0; j < menuItems.count(); j++)
        {
            menuItems.at(j)->setUserLevelState( currentUserLevel );
        }

        QList<windowCustomisationButtonItem*> buttons = customisation->getButtons();
        for (int j = 0; j < buttons.count(); j++)
        {
            buttons.at(j)->setUserLevelState( currentUserLevel );
        }
    }
}

// Add the named customisation to a main window.
// Return true if named customisation found and loaded.
void windowCustomisationList::applyCustomisation( QMainWindow* mw,                              // Main window to apply customisations to
                                                  QString customisationName,                    // Customisation name used to identify set of customisations
                                                  windowCustomisationInfo* customisationInfo,   // Customisations loaded from customisation file
                                                  dockMap dockedComponents )                    // Map of existing docks
{

    // If this customisation has been applied, do nothing
    // This is a bit more than for efficiency - if docks are present, and have been manipulated (scrolled, etc), we don't want to re-create them
    if( lastAppliedCustomisation == customisationName )
    {
        return;
    }
    lastAppliedCustomisation == customisationName;


    // Clear the existing customisation (but only if we have a customisation name to replace it with)
    if( !customisationName.isEmpty() )
    {
        // Clear references to all menu bar items and tool bar items
        customisationInfo->items.clear();

        // Remove all current menus
        mw->menuBar()->clear();
        mw->menuBar()->setVisible( false );
        customisationInfo->menus.clear();

        // Remove all current toolbars
        foreach (QToolBar* toolBar, customisationInfo->toolbars )
        {
            mw->removeToolBar( toolBar );
            delete toolBar;
        }
        customisationInfo->toolbars.clear();

// The following would be a good idea, but at the moment will delete docks created by any open GUIs
// as well as those created byt the customisation set. For example the 'Image display properties' dock
// of a QEImage widget would get deleted.
// We should be able to identify docks created as part of the customisation. The QEGui MainWindow class
// holds a list of docked components. This MAY be what is required. This class does not know about that
// application though. Perhaps it should signal that is would like customisation docks to be deleted pleas,
// or perhaps a list of customisation docks should be held (like dockRef in QEGui MainWindow)
//        // Remove current docks
//        QMapIterator<QString, QDockWidget*> d(dockedComponents);
//        while( d.hasNext() )
//        {
//            d.next();
//            qDebug() << "removing dock" <<d.key();
//            QDockWidget* dock = d.value();
//            mw->removeDockWidget( dock );
//            delete dock;
//        }
//        dockedComponents.clear();
    }

    // Get the customisations required
    // Do nothing if not found
    windowCustomisation* customisation = getCustomisation( customisationName );
    if (!customisation)
    {
        return;
    }

    // Get the current macro substitutions
    // Note, this uses a new ContainerProfile which will pick up the current macro substitutions.
    // The ContainerProfile base class (which is used to generate signals when the user level changes)
    // will have the old macro substitutions (probably those current on application startup).
    macroSubstitutionList macroSubstitutionParts;
    {
        ContainerProfile profile;
        macroSubstitutionParts = macroSubstitutionList( profile.getMacroSubstitutions() );
    }

    // Add the required toolbar buttons
    QList<windowCustomisationButtonItem*> bList = customisation->getButtons();
    for ( int i = 0; i < bList.length(); i++ )
    {
        QToolBar* tb;

        windowCustomisationButtonItem* item = new windowCustomisationButtonItem( bList.at(i) );

        // If there is no toolbar yet, create it
        // Note, the toolbar location is set when the toolbar is first created.
        // If buttons request the same toolbar name, but different locations, the firs one in wins.
        QMap<QString, QToolBar*>::const_iterator itb = customisationInfo->toolbars.find( item->getButtonToolbar() );
        if( itb != customisationInfo->toolbars.end() )
        {
            tb = itb.value();
        }
        else
        {
            tb = new QToolBar( item->getButtonToolbar(), mw );
            mw->addToolBar( item->getButtonLocation(), tb );

            customisationInfo->toolbars.insert( item->getButtonToolbar(), tb );
        }

        // Add button action
        tb->addAction( item->getAction() );

        // Set the icon if possible
        if( !item->getButtonIcon().isEmpty() )
        {
            QFile* file = QEWidget::findQEFile( item->getButtonIcon() );
            if( file )
            {
                QIcon icon = QIcon( file->fileName() );
                if( !icon.isNull() )
                {
                    item->getAction()->setIcon( icon );
                }
                delete file;
            }
        }

        // Set up an action to respond to the user
        QObject::connect( item, SIGNAL( newGui( const QEActionRequests& ) ),
                          mw, SLOT( requestAction( const QEActionRequests& ) ) );

        // Add the toolbar item to the list of all menu items
        customisationInfo->items.append( item );
    }

    // Get the menu item customisations required
    QList<windowCustomisationMenuItem*> menuItems = customisation->getMenuItems();

    // Apply all the menu customisations
    for (int i = 0; i < menuItems.length(); i++)
    {
        // Get the next customisation required
        windowCustomisationMenuItem* menuItem = new windowCustomisationMenuItem( menuItems.at(i) );

        // Ensure the menu hierarchy is present.
        // For example if the hierarchy required is 'File' -> 'Recent' is required and a 'File' menu is
        // present but it does not contain a 'Recent' menu, then create a 'Recent' menu in the 'File' menu
        QMenu* menu = buildMenuPath( customisationInfo, mw->menuBar(), menuItem->getMenuHierarchy() );

        // Act on the type of customisation required
        switch( menuItem->getType() )
        {
            case windowCustomisationMenuItem::MENU_UNKNOWN:
                break;

            case windowCustomisationMenuItem::MENU_ITEM:
                {
                    // Make the item checkable if required (and if nessesary checked)
                    itemCheckInfo checkInfo = menuItem->getCheckInfo();
                    if( checkInfo.getCheckable() )
                    {
                        menuItem->getAction()->setCheckable( true );
                        menuItem->getAction()->setChecked( macroSubstitutionParts.getValue( checkInfo.getKey() ) == checkInfo.getValue() );
                    }

                    // Set up an action to respond to the user
                    QObject::connect( menuItem, SIGNAL( newGui( const QEActionRequests& ) ),
                                      mw, SLOT( requestAction( const QEActionRequests& ) ) );

                    // Assume there is no action to add yet.
                    // This will remain the case if a dock is to created and the 'toggle view' action from the dock is required.
                    QAction* action = NULL;

                    // If the menu item holds the title of an existing dock to associate with, find the dock,
                    // get the toggle view action from the dock, and add that action as the menu action.
                    if( !menuItem->getDockTitle().isEmpty() )
                    {
                        // Find the existing dock by matching the title
                        QDockWidget* component =  dockedComponents.value( menuItem->getDockTitle(), NULL );

                        // If the existing dock is found, use the dock's toggle action
                        if( component )
                        {
                            // Get the action the user will use to show and hide the dock
                            action =  component->toggleViewAction();  // <-- crashed here re-opening .ui with image and customisation that included all image options

                            // Set the visibility state of the dock to reflect the state of the action (checked or unchecked).
                            // Originally, the dock was hidden when created if appropriate, but on Centos6 it was never shown when
                            // the user asked for the dock by checking the dock action.
                            // Search for 'Centos6 visibility problem' to find other fragments of code relating to this problem
                            component->setVisible( action->isChecked() );
                        }

                        // If the existing dock is not found, log an error
                        else
                        {
                            // Required dock not found. Note the title that could not be found and list those that are available
                            QMapIterator<QString, QDockWidget*> i(dockedComponents);
                            qDebug() << "When applying window customisations, could not find a dock titled:" << menuItem->getDockTitle() << ". Dock titles found were:";
                            if( i.hasNext() )
                            {
                                while (i.hasNext())
                                {
                                    i.next();
                                    qDebug() << "   " << i.key();
                                }
                            }
                            else
                            {
                                qDebug() << "   <none>";
                            }
                        }
                    }

                    // If the menu item creates at least one dock, prepare to activate the item (which will create the docked GUI).
                    // Once activated the dock's 'toggle view' action will be added to the menu noted here.
                    else if( menuItem->createsDocks() )
                    {
                        // Save this menu item, and the associated menu, in a transient list of dock related
                        // items to be activated as soon as this window's set of customisations has been applied
                        toBeActivatedList.append(menuItemToBeActivated( menuItem, menu ));
                    }

                    // Not dock related, just add the menu item as the action
                    else
                    {
                        action = menuItem->getAction();
                    }

                    // If the required action is available, add the item action to the correct menu.
                    // (it won't be available if the action is the 'toggle view' action of a dock that is yet to be created)
                    if( action )
                    {
                        if( menu )
                        {
                            if( menuItem->hasSeparator() )
                            {
                                menu->addSeparator();
                            }
                            menu->addAction( action );
                        }

                        // Or add the item action to the menu bar, if not in a menu
                        // (Unusual, but OK)
                        else
                        {
                            mw->menuBar()->addAction( action );
                        }
                    }
                }
                break;

            case windowCustomisationMenuItem::MENU_PLACEHOLDER:
                {
                    QMenu* placeholderMenu;
                    QString menuTitle = menuItem->getTitle();

                    // Add the placeholder to the menu if there is one
                    if( menu )
                    {
                        if( menuItem->hasSeparator() )
                        {
                            menu->addSeparator();
                        }
                        placeholderMenu = menu->addMenu( menuTitle );
                    }

                    // If no menu, add the placeholder to the menu bar
                    else
                    {
                        placeholderMenu = mw->menuBar()->addMenu( menuTitle );
                        mw->menuBar()->setVisible( true );
                    }
                    customisationInfo->placeholderMenus.insert( menuTitle, placeholderMenu );

                    // Save the menu for some future menu search
                    customisationInfo->menus.insert( menuTitle, placeholderMenu );
                }
                break;

            case windowCustomisationMenuItem::MENU_BUILT_IN:
                // Add the item to the correct menu
                if( menu )
                {
                    if( menuItem->hasSeparator() )
                    {
                        menu->addSeparator();
                    }
                    menu->addAction( menuItem->getAction() );
                }

                // Or add the item to the menu bar, if not in a menu
                // (This is normal if the built-in function adds a menu to the action - such as the QEImage zoom menu,
                // but is a bit unusual otherwise)
                else
                {
                    mw->menuBar()->addAction( menuItem->getAction() );
                }

                // Set the item to request an action from the main window it has been added to.
                QObject::connect( menuItem, SIGNAL( newGui( const QEActionRequests& ) ),
                                  mw, SLOT( requestAction( const QEActionRequests& ) ) );

                break;
        }

        // Add the menu item to the list of all menu items
        customisationInfo->items.append( menuItem );
    }

    // Activate any dock related items.
    // The dock related items have been added to the transient list 'toBeActivatedList'.
    // NOTE, the action is performed as a timer event not to delay it, but to ensure
    // is called after this customisation has been applied. This is required since
    // there is a currenly published profile (published for the creation of the new
    // window this customisation is being applied to). When the item acction is performed,
    // it will (if creating a gui) want to publish it's own profile, and can't do so while
    // one is already published.
    if( toBeActivatedList.count() )
    {
        toBeActivatedMW = mw;
        QTimer::singleShot( 0, this, SLOT(activateDocks()));
    }
}

// Slot to create any docks required to support dock menu items.
// Docked GUIs are created once customisation has been applied.
// They are created in this slot as a timer event, not because a delay is required,
// but to ensure is occurs after the customisation has been applied. This is required since
// there is a currenly published profile (published for the creation of the new
// window this customisation is being applied to). When the item acction is performed,
// it will (if creating a gui) want to publish it's own profile, and can't do so while
// one is already published.
void windowCustomisationList::activateDocks()
{
    // Sanity check. Do nothing if a main window was not noted
    if( !toBeActivatedMW )
    {
        return;
    }

    // Prepare to catch signals when a dock is created
    QObject::connect( toBeActivatedMW, SIGNAL( dockCreated( QDockWidget* ) ),
                      this, SLOT( useDock( QDockWidget* ) ) );

    // Activate each of the dock related menu items.
    for( int i = 0; i < toBeActivatedList.count(); i++ )
    {
        // Extract the next dock related item from the list
        menuItemToBeActivated mitba = toBeActivatedList.at(i);

        // Note the menu that an action should be added to
        toBeActivatedMenu = mitba.menu;

        // Perform the menu action (create the dock)
        mitba.item->itemAction();
    }
    QObject::disconnect(toBeActivatedMW, SIGNAL( dockCreated( QDockWidget* ) ), this, SLOT( useDock( QDockWidget* ) ));

    // To prevent accidental missuse, clear the transitory variables that
    // are only used while the docks are created
    toBeActivatedList.clear();
    toBeActivatedMenu = NULL;
    toBeActivatedMW = NULL;
}

// A dock has just been created in response to applying a customisation menu item.
// Add the dock's 'toggle view' action to the appropriate menu
// NOTE: This will result in all dock 'toggle view' actions appearing at the bottom of the menu.
// NOTE, Also, if the menu item indicates requests a seperator before this action, this is not honoured
void windowCustomisationList::useDock( QDockWidget* dock )
{
    // Add the dock's 'toggle view' action to the appropriate menu.
    if( toBeActivatedMenu )
    {
        toBeActivatedMenu->addAction( dock->toggleViewAction() );
    }
}

//==============================================================================================
// itemCheckInfo
//==============================================================================================

// Constructor
itemCheckInfo::itemCheckInfo()
{
    checkable = false;
}

// itemCheckInfo constructor.
// Parses xml to determine if an item is checkable (check box or radio button) and if it is exclusive (a radio button)
itemCheckInfo::itemCheckInfo( QDomElement itemElement )
{
    // Assume not checkable
    checkable = false;

    // Determine if a item is to be presented checkable
    QDomElement element = itemElement.firstChildElement( "Checkable" );
    if( !element.isNull() )
    {
        // Note it is checkable
        checkable = true;

        // Get the macro substitution key and value that will be used to determine if it is checked
        QString subs = element.text();
        macroSubstitutionList macros( subs );
        if( macros.getCount() == 1 )
        {
            key = macros.getKey( 0 );
            value = macros.getValue( 0 );
        }
    }
}

// Copy constructor
itemCheckInfo::itemCheckInfo( const itemCheckInfo &other )
{
    key = other.key;
    value = other.value;
    checkable = other.checkable;
}

//==============================================================================================
// windowCustomisationInfo
//==============================================================================================

// Respond to a user level change (this is an implementation for the base ContainerProfile class)
// Update all the items used to customise a window.
//
// Note, it is simpler to just update all items regardless, but note that setting the user level of
// some items will not have any effect. For example, most customisation items in this list are also
// the actual QAction used in the menus and buttons, so enabling them according to the user level will
// show them enabled or disabled in the menu bar and tool bar. For docks, the QAction is sourced from
// the dock, so enabling or disabling this customisation item will have no effect. This is OK as docks
// cannot be enabled or dissabled according to user level. Applying user level to docks is not done as
// it would interfere with the standard dock paradigm.
void windowCustomisationInfo::userLevelChangedGeneral( userLevelTypes::userLevels userLevel )
{
    for( int i = 0; i < items.count(); i++ )
    {
        items.at( i )->setUserLevelState( userLevel );
    }
}

// end
