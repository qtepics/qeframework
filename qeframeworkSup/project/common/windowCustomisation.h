/*
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
 *  Copyright (c) 2013 Australian Synchrotron
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
 * Main class descriptions:
 *  - windowCustomisationList contains a list of windowCustomisation customisations.
 *
 *  - windowCustomisation contains a named customisation consisting of a name and a list of windowCustomisationMenuItem and
 *    a list of windowCustomisationButtonItem classes.
 *
 *  - windowCustomisationMenuItem and windowCustomisationButtonItem classes are both based on windowCustomisationItem which holds
 *    details about what to do (which .ui file to open, etc)
 *
 * To use this module:
 * Create a single instance of windowCustomisationList class.
 * Call windowCustomisationList::loadCustomisation() at any time to load .xml files containing one or more named customisations.
 * When starting GUIs, or creating QE widgets (or any other appropriate time) call windowCustomisationList::loadCustomisation()
 * to apply a named customisation to a QMainWindow
 *
 * The QEGui application loads a built in .xml file (QEGuiCustomisationDefault.xml). This file is a useful example of customisation XML.
 *
 * The following example allows for the following:
 *
 * - IMBL main GUIs such as the general beamline overview GUI request customisation 'IBML_MAIN'
 *   which has a large set of menu options.
 *
 * - IMBL minor GUIs such as transient status GUIs request customisation 'IBML_MINOR' which adds
 *   a single menu option to return to the main beamline GUI.
 *
 * - IMBL Region Of Interest (ROI) GUIs request customisation 'ROI' which adds some options relevent
 *   for region of interest images (including both menu items and tool bar buttons) and also includes
 *   the 'IBML_MINOR' customisation.
 *

    <QEWindowCustomisation>
        <Customisation Name="IMBL_MAIN">
            <Menu Name="File">
                <BuiltIn Name="Open" >
                <BuiltIn Name="Exit" >
                <PlaceHolder Name="Recent" >
            </Menu>

            <Menu Name="Imaging">
                <Menu Name="Regions">
                    <Item Name="ROI 1">
                        <UiFile>ROI.ui</UiFile>
                        <Program Name="firefox">
                            <Arguments>www.google.com</Arguments>
                        </Program>
                        <MacroSubstitutions>REGION=1</MacroSubstitutions>
                        <Customisation>ROI</Customisation>
                    </Item>

                    <Item Name="ROI 2">
                        <UiFile>ROI.ui</UiFile>
                        <MacroSubstitutions>REGION=2</MacroSubstitutions>
                        <Customisation>ROI</Customisation>
                    </Item>


                    <Item Name="Status" UserLevelEnabled="Scientist">
                        <Separator/>
                        <UiFile>status.ui</UiFile>
                    </Item>

                </Menu>
            </Menu>
        </Customisation>


        <Customisation Name="IMBL_MINOR">
            <Menu Name="File">
                <Item Name="Main Window">
                    <UiFile>IMBL.ui</UiFile>
                    <Customisation>IMBL_MAIN</Customisation>
                </Item>
            </Menu>
        </Customisation>


        <Customisation Name="ROI">
            <Menu Name="Imaging">
                <Item Name="Plot">
                    <UiFile>ROI_Plot.ui</UiFile>
                    <Customisation>IMBL_MINOR</Customisation>
                </Item>
            </Menu>
            <Button Name="Plot">
                <Icon>plot.png</Icon>
                <UiFile>ROI_Plot.ui</UiFile>
                <Customisation>IMBL_MINOR</Customisation>
            </Button>
            <IncludeCustomisation Name="IMBL_MINOR"></IncludeCustomisation>
        </Customisation>

        <CustomisationIncludeFile> </CustomisationIncludeFile>

    </QEWindowCustomisation>
 */

#ifndef WINDOWCUSTOMISATION_H
#define WINDOWCUSTOMISATION_H

#include <QObject>
#include <QAction>
#include <QList>
#include <QString>
#include <QStringList>
#include <QMainWindow>
#include <QDomDocument>
#include <QMenu>
#include <QEActionRequests.h>
#include <QMap>
#include <ContainerProfile.h>
#include <applicationLauncher.h>
#include <QEPluginLibrary_global.h>


// Class for building a log of the process of loading the customisation files
// Used for diagnosis.
// Available in QEGui help about.
// Output to console if it includes an error
class customisationLog
{
public:
    customisationLog(){ error = false; }
    ~customisationLog(){}

    void add( const QString message )                                            { log.append( QString( prefix ).append( message ) ); }
    void add( const QString message, const QString param1 )                      { log.append( QString( prefix ).append( message ).append( param1 ) ); }
    void add( const QString message, const QString param1, const QString param2 ){ log.append( QString( prefix ).append( message ).append( param1 ).append( param2 ) ); }

    void startLevel(){ prefix.append( "    " ); }
    void endLevel()  { prefix.truncate( prefix.length()-4 ); }
    void flagError() { error = true; add( "ERROR: ^^^^^^^^^^^^^^^^^^^^^"); }

    const QString getLog(){ QString s; for( int i = 0; i < log.count(); i++ ) s.append( log.at(i) ).append( "\n"); return s; }
    bool    getError(){ return error; }

private:
    QStringList  log;    // Log of customisaiton files loaded for diagnosis
    QString      prefix; // Current indentation, used while building customisationLog
    bool         error;  // Log reports an error (as well as normal processing)
};

// Class to determine if an item is checkable (check box or radio button) and if it is exclusive (a radio button)
class itemCheckInfo
{
public:
    itemCheckInfo();
    itemCheckInfo( QDomElement itemElement );
    itemCheckInfo( const itemCheckInfo &other );
    const QString getKey(){ return key; }
    const QString getValue(){ return value; }
    bool getCheckable(){ return checkable; }

private:
    QString key;        // Macro substitution key
    QString value;      // Macro substitution value
    bool checkable;     // True if checkable
};


// Class defining an individual item (base class for button or menu item)
class windowCustomisationItem : public QAction
{
    Q_OBJECT
public:
    windowCustomisationItem( // Construction
                      const QObject* launchRequestReceiver,                // Object (typically QEGui application) which will accept requests to launch a new GUI
                      const QList<windowCreationListItem>& windowsIn,      // Windows to display (centrals and docks)
                      const QString programIn,                             // Program to run
                      const QStringList argumentsIn );                     // Arguments for 'program'

    windowCustomisationItem(windowCustomisationItem* item);                 // Copy constructor
    windowCustomisationItem();                                              // Construct instance of class defining an individual item when none exists (for example, a menu placeholder)
    windowCustomisationItem( const QString builtInActionIn );               // Construct instance of class defining a built in application action
    windowCustomisationItem( const QString builtInActionIn,                 // Construct instance of class defining a built in application action
                             const QString widgetNameIn );                  // widget name if built in function is for a widget, not the application
    windowCustomisationItem( const QString dockTitleIn, bool unused );      // Construct instance of class defining a link to an existing dock

    void commonInit();

    QString getProgram(){return programLauncher.getProgram();}
    QStringList getArguments(){return programLauncher.getArguments();}

    QString getBuiltInAction(){return builtInAction;}

    QString getDockTitle(){ return dockTitle; }                             // Return the title of an existing dock (used to find the pre-existing dock)
    QString getGUITitle(){ return guiTitle; }                               // Return the title to be applied to a new GUI
    bool createsDocks();                                                    // Return true if at least one dock is created by this item

    void initialise();
    void logItem( customisationLog& log );
    void addUserLevelAccess( QDomElement element, customisationLog& log  ); // Note the user levels at which the item is enabled and visible
    void setUserLevelState( userLevelTypes::userLevels currentUserLevel );  // Set the visibility and enabled state of the item according to the user level

private:
    // Item action
    QList<windowCreationListItem> windows;          // Windows to create (.ui files and how to present them)
    QString dockTitle;                              // Title of dock to locate the associate with (not used when creating a new UI in a dock. In that case the dock to associate with is returned in the useDock() slot)

    QString builtInAction;                          // Identifier of action built in to the application

    QString widgetName;                             // Widget to locate if passing this action on to a widget in a GUI
    QString guiTitle;                               // Title to give GUI. This overrides any title specified in the GUI.

    ContainerProfile profile;                       // Profile to use while creating customisations.
    applicationLauncher programLauncher;            // Manage any program that needs to be started

    userLevelTypes::userLevels userLevelVisible;    // User level at which the item will be visible
    userLevelTypes::userLevels userLevelEnabled;    // User level at which the item will be enabled

public slots:
    void itemAction();                              // Slot to call when action is triggered

signals:
    void newGui( const QEActionRequests& request );

};


// Class defining an individual menu item
class windowCustomisationMenuItem : public windowCustomisationItem
{
public:
    enum menuObjectTypes { MENU_UNKNOWN, MENU_ITEM, MENU_PLACEHOLDER, MENU_BUILT_IN };
    windowCustomisationMenuItem( // Construction (menu item to create new GUI windows or docks)
                          customisationLog& log,                               // Log of customisation files loaded for diagnosis.

                          const QStringList menuHierarchyIn,                   // Location in menus to place this item. for example: 'Imaging'->'Region of interest'
                          const QString titleIn,                               // Name of this item. for example: 'Region 1'
                          const menuObjectTypes type,                          // type of menu object - must be MENU_ITEM
                          const bool separatorIn,                              // Separator required before this
                          const itemCheckInfo& checkInfoIn,                     // Information about the item's checkable state

                          const QObject* launchRequestReceiver,                // Object (typically QEGui application) which will accept requests to launch a new GUI
                          const QList<windowCreationListItem>& windowsIn,      // Windows to display (centrals and docks)
                          const QString programIn,                             // Program to run
                          const QStringList argumentsIn );                     // Arguments for 'program or for built in function


    windowCustomisationMenuItem( // Construction (placeholder menu item)
                          customisationLog& log,                               // Log of customisation files loaded for diagnosis.

                          const QStringList menuHierarchyIn,                   // Location in menus for application to place future items. for example: 'File' -> 'Recent'
                          const QString titleIn,                               // Identifier of placeholder. for example: 'Recent'
                          const menuObjectTypes typeIn,                        // type of menu object - must be MENU_PLACEHOLDER
                          const bool separatorIn,                              // Separator required before this
                          const itemCheckInfo& checkInfoIn );                   // Information about the item's checkable state

    windowCustomisationMenuItem( // Construction (menu item to pass a action request on to the application, or a QE widget inthe application)
                          customisationLog& log,                               // Log of customisation files loaded for diagnosis.

                          const QStringList menuHierarchyIn,                   // Location in menus for application to place future items. for example: 'File' -> 'Recent'
                          const QString titleIn,                               // Title for this item. for example: 'Region 1' Usually same as name of built in function. (for example, function='Copy' and title='Copy', but may be different (function='LaunchApplication1' and title='paint.exe')
                          const menuObjectTypes typeIn,                        // type of menu object - must be MENU_BUILT_IN
                          const bool separatorIn,                              // Separator required before this
                          const itemCheckInfo& checkInfoIn,                     // Information about the item's checkable state

                          const QString builtIn,                               // Name of built in function (built into the application or a QE widget). For example: 'Region 1'
                          const QString widgetNameIn );                        // widget name if built in function is for a widget, not the application

    windowCustomisationMenuItem( // Construction (menu item to pass a action request on to the application, or a QE widget inthe application)
                          customisationLog& log,                               // Log of customisation files loaded for diagnosis.

                          const QStringList menuHierarchyIn,                   // Location in menus for application to place future items. for example: 'File' -> 'Recent'
                          const QString titleIn,                               // Title for this item. for example: 'Region 1' Usually same as name of built in function. (for example, function='Copy' and title='Copy', but may be different (function='LaunchApplication1' and title='paint.exe')
                          const menuObjectTypes typeIn,                        // type of menu object - must be MENU_BUILT_IN
                          const bool separatorIn,                              // Separator required before this
                          const itemCheckInfo& checkInfoIn,                     // Information about the item's checkable state

                          const QString dockTitleIn );                         // Title of existing dock widget to assocaite the menu item with

    windowCustomisationMenuItem(windowCustomisationMenuItem* menuItem);

    QStringList getMenuHierarchy(){return menuHierarchy;}
    void prependMenuHierarchy( QStringList preMenuHierarchy );
    QString getTitle(){return title;}
    menuObjectTypes getType(){ return type; }

    bool hasSeparator(){ return separator; }
    const itemCheckInfo& getCheckInfo() { return checkInfo; }

private:
    menuObjectTypes type;
    // Menu bar details.
    // All details are optional.
    // A menu item is created if menuHierarchy contains at least one level and title exists
    QStringList menuHierarchy;  // Location in menus to place this item. for example: 'Imaging'->'Region of interest'
    QString title;              // Name of this item. for example: 'Region 1'
    bool separator;             // Separator should appear before this item
    itemCheckInfo checkInfo;    // Information about the item's checkable state
};

// Class defining an individual button item
class windowCustomisationButtonItem : public windowCustomisationItem
{
public:
    windowCustomisationButtonItem( // Construction
                            const QString buttonGroupIn,                         // Name of toolbar button group in which to place a button
                            const QString buttonToolbarIn,                       // Name of toolbar in which to place a button
                            const Qt::ToolBarArea buttonLocationIn,              // Location of toolbar in which to place a button
                            const QString buttonTextIn,                          // Text to place in button
                            const QString buttonIconIn,                          // Icon for button

                            const QObject* launchRequestReceiver,                // Object (typically QEGui application) which will accept requests to launch a new GUI
                            const QList<windowCreationListItem>& windowsIn,      // Windows to display (centrals and docks)
                            const QString programIn,                             // Program to run
                            const QStringList argumentsIn );                     // Arguments for 'program' and for action

    windowCustomisationButtonItem( // Construction
                            const QString buttonGroupIn,                         // Name of toolbar button group in which to place a button
                            const QString buttonToolbarIn,                       // Name of toolbar in which to place a button
                            const Qt::ToolBarArea buttonLocationIn,              // Location of toolbar in which to place a button
                            const QString buttonTextIn,                          // Text to place in button
                            const QString buttonIconIn,                          // Icon for button

                            const QString builtIn,                               // Name of built in function (built into the application or a QE widget). For example: 'Region 1'
                            const QString widgetNameIn );                        // widget name if built in function is for a widget, not the application

    windowCustomisationButtonItem( // Construction
                            const QString buttonGroupIn,                         // Name of toolbar button group in which to place a button
                            const QString buttonToolbarIn,                       // Name of toolbar in which to place a button
                            const Qt::ToolBarArea buttonLocationIn,              // Location of toolbar in which to place a button
                            const QString buttonTextIn,                          // Text to place in button
                            const QString buttonIconIn );                        // Icon for button

    windowCustomisationButtonItem(windowCustomisationButtonItem* buttonItem);

    QString getButtonGroup(){ return buttonGroup; }
    QString getButtonToolbar(){ return buttonToolbar; }
    Qt::ToolBarArea getButtonLocation(){ return buttonLocation; }
    QString getButtonText(){ return buttonText; }
    QString getButtonIcon(){ return buttonIcon; }

private:
    // Button details.
    // All details are optional.
    // A button is created if buttonText or buttonIcon is available
    QString buttonGroup;            // Name of toolbar button group in which to place a button
    QString buttonToolbar;          // Name of toolbar in which to place a button
    Qt::ToolBarArea buttonLocation; // Location of toolbar in which to place a button
    QString buttonText;             // Text to place in button
    QString buttonIcon;             // Icon for button
};

// Class defining the customisation of a window.
// Generated from an XML customisation file.
class windowCustomisation
{
public:
    windowCustomisation( const QString nameIn );       // Construction - create a named, empty, customisation
    ~windowCustomisation();                            // Destruction

    void addItem( windowCustomisationMenuItem* menuItem, QStringList preMenuHierarchy = QStringList() );      // Add a menu item to the customisation
    void addItem( windowCustomisationButtonItem* button );    // Add a button to the customisation

    QList<windowCustomisationMenuItem*> getMenuItems(){return menuItems;}      // get Menu items list
    QList<windowCustomisationButtonItem*> getButtons(){return buttons;}        // get Buttons list
    QString getName(){ return name; }

    static QEActionRequests::Options translateCreationOption( QString creationOption );

private:
    QString name;                                  // Customisation name
    QList<windowCustomisationMenuItem*> menuItems; // Menu items to be added to menu bar to implement customisation
    QList<windowCustomisationButtonItem*> buttons; // Buttons to be added to tool bar to implement customisation
};

// Window customisation information per Main Window
class QEPLUGINLIBRARYSHARED_EXPORT windowCustomisationInfo : public ContainerProfile
{
public:
    windowCustomisationInfo () {}
    ~windowCustomisationInfo () {}

    void userLevelChangedGeneral( userLevelTypes::userLevels ); // Repond to a user level change (this is an implementation for the base ContainerProfile class

    QMap<QString, QMenu*> placeholderMenus;    // Menus where application may insert items
    QMap<QString, QMenu*> menus;               // All menus added by customisation system
    QMap<QString, QToolBar*> toolbars;         // All tool bars added by customisation system
    QList<windowCustomisationItem*> items;     // All menu bar items and toolbar buttons. (These customisation
                                               // items are also the actual QActions used in the menus and
                                               // buttons, except where the customisation is a dock, in which
                                               // case the QAction is the sourced from the dock widget itself)
};

// Class to hold a relationship between a customisation menu item, and an actual QMenu.
// Used to build a transient list of menus that need to have dock 'toggle view' actions added.
class menuItemToBeActivated
{
public:
    menuItemToBeActivated( menuItemToBeActivated* other ){ item = other->item; menu = other->menu; }
    menuItemToBeActivated(){ item = NULL; menu = NULL; }
    menuItemToBeActivated( windowCustomisationMenuItem* itemIn, QMenu* menuIn ){ item = itemIn; menu = menuIn; }

    windowCustomisationMenuItem* item;  // Customisation item reference
    QMenu* menu;                        // Menu reference
    //!!! location required in item (or placeholder of some sort)
};

// Class managing all customisation sets
// Only one instance of this class is instantiated (unless groups of customisation sets are required)
// Multiple .xml files may be loaded, each defining one or more named customisations.
class QEPLUGINLIBRARYSHARED_EXPORT windowCustomisationList : public QObject, ContainerProfile
{
    Q_OBJECT
public:
    typedef QMap<QString, QDockWidget*> dockMap;                    // Used to pass a list of docks than may be linked to menu items based on the dock title

    windowCustomisationList();

    bool loadCustomisation( QString xmlFile );                      // Load a set of customisations
    void applyCustomisation( QMainWindow* mw, QString customisationName, windowCustomisationInfo* customisationInfo, dockMap dockedComponents = dockMap() ); // Add the named customisation set to a main window. Return true if named customisation found and loaded.

    windowCustomisation* getCustomisation(QString name);
    void initialise( windowCustomisationInfo* customisationInfo );

    customisationLog  log;                              // Log of customisaiton files loaded for diagnosis.

private:

    QMenu* buildMenuPath( windowCustomisationInfo* customisationInfo, QMenuBar* menuBar, const QStringList menuHierarchy );

    void addIncludeCustomisation( QDomElement includeCustomisationElement, windowCustomisation* customisation, QStringList menuHierarchy = QStringList() );
    void parseMenuElement( QDomElement element, windowCustomisation* customisation, QStringList menuHierarchy );          // Parse menu customisation data

    bool requiresSeparator( QDomElement itemElement );          // Determine if an item contains a 'separator' tag

    bool parseMenuAndButtonItem( QDomElement itemElement,
                                 QString& title,
                                 QList<windowCreationListItem>& windows,
                                 QString& builtIn,
                                 QString& program,
                                 QString& widgetName,
                                 QStringList& arguments,
                                 QString& dockTitle );
    void parseDockItem( QDomElement itemElement, QList<windowCreationListItem>& windows, QString& dockTitle, QString& guiTitle );

    windowCustomisationMenuItem* createMenuItem       ( QDomElement itemElement, QStringList menuHierarchy); // Create a custom menu item
    windowCustomisationMenuItem* createMenuPlaceholder( QDomElement itemElement, QStringList menuHierarchy); // Create a placeholder menu (for the application to add stuff to)

    windowCustomisationButtonItem* createButtonItem( // Create a button customisation item
                                              QDomElement itemElement);
    QList<windowCustomisation*> customisationList;                         // List of customisations

    // Variables to manage setting up 'toggle view' actions from docks created as a result of, but after, the window customisation has been applied.
    QList<menuItemToBeActivated> toBeActivatedList;     // Transient list of menus and customisation menu items
    QMainWindow* toBeActivatedMW;                       // Main Window being customised. Used to connect to to receive signals relating to newly created docks
    QMenu*       toBeActivatedMenu;                     // Menu currently currently waiting on a dock to be created (at which point the dock's 'toggle view' action will be added)

    QString      lastAppliedCustomisation;              // Last customisation applied by applyCustomisation() - successfully, or unsuccessfully

    void userLevelChangedGeneral( userLevelTypes::userLevels ); // Repond to a user level change (this is an implementation for the base ContainerProfile class

private slots:
    void activateDocks();                               // Slot to create any docks required to support dock menu items. Docked GUIs are created at the time customisation is applied.
    void useDock( QDockWidget* dock );                  // Slot to receive notification a docked GUI has been created. Used to then associate the dock's 'toggle view' action to be added to relevent menus
};


#endif // WINDOWCUSTOMISATION_H
