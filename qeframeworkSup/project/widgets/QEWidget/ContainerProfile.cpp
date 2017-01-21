/*  ContainerProfile.cpp
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
 *  Copyright (c) 2009, 2010 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
 * Description:
 *
 * This class provides a communication mechanism from the code creating QE widgets to the QE widgets.
 *
 * When QE widgets, such as QELabel, are created, they need to know environmental
 * information such as what macro substitutions to apply, or where to signal error messages.
 * Also, the code creating the QE widgets may require a reference to all the created QE widgets.
 * In some cases this information cannot be passed during construction or set up post construction
 * via a method. For example, when the object is being created from a UI file by Qt. In this case
 * the application code asks Qt to generate objects from a UI file and has no idea what
 * QE widgets if any have been created.
 *
 * To use this class, an instance of this class is instantiated prior to creating the QE widgets.
 * Information to be communicated such as message handlers and macro substitutions is set up within this class.
 * Then the QE widgets are created using a mechanism such as the QUiLoader class.
 *
 * As each QE widgets is created it also instantiates an instance of the ContainerProfile class.
 * If any information has been provided, it can then be used.
 *
 * Note, a local copy of the environment profile is saved per instance, so an application
 * creating QE widgets (the container) can define a profile, create QE widgets, then release the profile.
 *
 * To use this class
 *         - Instantiate a ContainerProfile class
 *         - Call setupProfile()
 *         - Create QE widgets
 *         - Call releaseProfile()
 *
 * This class also communicates the current user level between the application and contained widgets.
 * This differs from other environmental information described above in the following ways:
 *
 * - Widgets based on the QEWidget class (and therefore this ContainerProfile class) can be
 *   notified of user level changes by reimplementing QEWidget::userLevelChanged()
 *   Note, Widgets can also determine the current user level by calling ContainerProfile::getUserLevel()
 *
 * - Both the application and any widgets based on the QEWidget class can set the user level by
 *   calling ContainerProfile::setUserLevel().
 *   For example, the QELogin widgt can alter the user level from within a GUI, alternatively
 *   the application can manage the user level.
 *
 * Notes:
 * - If an application creates the ContainerProfile class early, before the widgets that are published in the
 *   profile, or if the published widgets change the widgets in the profile can be updated by calling updateConsumers().
 *   Alternatively, just the widget that launches new GUIs can be updated with replaceGuiLaunchConsumer().
 *
 * - An application may need to temprarily extend the the macro substitutions. For example, when creating an QEForm
 *   widget as a sub form within another QEForm widget. Macro substitutions can be extended by calling addMacroSubstitutions()
 *   then restored using removeMacroSubstitutions().
 *
 */

#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QDir>
#include <QEWidget.h>
#include <QtDebug>
#include <ContainerProfile.h>

// Forward declaration
QEEnvironmentShare ContainerProfile::share;

// Single static instance of a QEEnvironmentShare class.
//
// The details contained in the PublishedProfile class need to be shared across the application, but they can't
// be just static since QE framework library QEPlugin may be loaded twice: Once as a run time loaded dll
// for an application (example QEGui) and once when a QEForm is loaded by the Qt .ui loader. On Linux this
// doesn't matter - the library is mapped once and the same mapping is returned in both cases. So any static
// variables only exist once.
// In windows, however, the library (QEPlugin.dll) is mapped twice and static variables are created twice.
// This can be observed by printing out the address of a static variable. The 'same' static variable will
// have different addresses depening on whether it is being referenced by the application or the Qt .ui form loader.
//
// This static instance of the QEEnvironmentShare class will be mapped once for each mapping of the library and will
// manage sharing the PublishedProfile across all mappings through shared memory.
//
// The sharing is limited to the current process by including the process ID in the shared memory key.
// Note, the Qt objects in the single published profile class are all being access by the same thread, just through
// different mappings of the same dll.
QEEnvironmentShare::QEEnvironmentShare()
{
    // Generate a shared memory key.
    // Keep it unique to this process by including the pid
    qint64 pid = QCoreApplication::applicationPid();
    QString key = QString( "QEFramework:" ).append( QString("%1").arg( pid ) );

    // Create the shared memory
    sharedMemory = new QSharedMemory( key );

    // Get the shared memory data
    // If the shared memory data is there, it has also been set up to contain a reference
    // to the unique published profile by another instance of this class.
    void* sharedMemoryData = NULL;
    if( sharedMemory->attach() )
    {
        sharedMemoryData = sharedMemory->data();
        if( sharedMemoryData )
        {
            // Get the other's reference to it's Published Profile
            sharedMemory->lock();
            publishedProfile = *(PublishedProfile**)(sharedMemoryData);
            sharedMemory->unlock();

            // Note we don't own the PublishedProfile (we should never deleted it)
            publishedProfileCreatedByMe = false;
            return;
        }
    }

    // There is no shared memory data. (or we couldn't attach)
    // Create a unique published profile class, create the shared memory data (big enough to
    // hold a reference to the published profile), and load the reference to the published profile
    // into the shared memory data.
    // (Note we own the PublishedProfile (we should deleted it on exit)
    publishedProfile = new PublishedProfile;
    publishedProfileCreatedByMe = true;

    if( !sharedMemory->create( sizeof( PublishedProfile* ) ) )
    {
        qDebug() << "Error (1) setting up ContainerProfile in shared memory. (on Linux, check if there are old shared memory sections or semaphores using the ipcs command.)" << sharedMemory->error() << sharedMemory->errorString();
    }
    else
    {
        sharedMemoryData = sharedMemory->data();
        if( sharedMemoryData == NULL )
        {
            qDebug() << "Error (2) setting up ContainerProfile in shared memory." << sharedMemory->error() << sharedMemory->errorString();
        }
        else
        {
            // Lock the shared memory
            sharedMemory->lock();
            *(PublishedProfile**)(sharedMemoryData) = publishedProfile;
            sharedMemory->unlock();
        }
    }
}

// Release shared profile
QEEnvironmentShare::~QEEnvironmentShare()
{
    // Free the profile if we created it.
/* How about we dont. Other instances take a copy and although we may have created it
   we may not be the last to reference it.
   Instead, just leave it. We are talking about a few bytes that is meant to last for the
   duration of the applcation anyway.
   The best reason to do this better is so memory leak tools dont complain.
   To do this properly, the QEEnvironmentShare class should get the published profile from
   the (locked) shared memory each time it is required.

    if( publishedProfileCreatedByMe )
    {
        void* sharedMemoryData = sharedMemory->data();
        if( sharedMemoryData == NULL )
        {
            qDebug() << "Error (3) accessing ContainerProfile in shared memory." << sharedMemory->error() << sharedMemory->errorString();
        }
        else
        {
            sharedMemory->lock();
            *sharedMemoryData = NULL;
            delete publishedProfile;
            sharedMemory->unlock();
        }
    }
*/

    // Ensure shared memory is released. Without this lock files may persist on Linux
    delete sharedMemory;

}

// Constructor.
// A local copy of the defined profile (if any) is made.
// Note, this does not define a profile. A profile is defined only when ContainerProfile::setupProfile() is called
ContainerProfile::ContainerProfile()
{
    // Set up the object that will recieve signals that the user level has changed
    userSlot.setOwner( this );
    QObject::connect( &(getPublishedProfile()->userSignal),  SIGNAL( userChanged( userLevelTypes::userLevels ) ),
                      &userSlot,    SLOT  ( userChanged( userLevelTypes::userLevels ) ) );

    // Take a local copy of the defined profile
    takeLocalCopy();
}

// Destructor
// Note, if the profile has been defined (ContainerProfile::setupProfile() has been
// called) this does not release the profile. A profile is released only when
// ContainerProfile::releaseProfile() is called.
ContainerProfile::~ContainerProfile()
{
}

// Get a unique instance of the published profile.
// The details contained in the PublishedProfile class need to be shared across the application, but they can't
// be just static since QE framework library QEPlugin may be loaded twice: Once as a run time loaded dll
// for an application (example QEGui) and once when a QEForm is loaded by the Qt .ui loader. On Linux this
// doesn't matter - the library is mapped once and the same mapping is returned in both cases. So any static
// variables only exist once.
// In windows, however, the library (QEPlugin.dll) is mapped twice and static variables are created twice.
// This can be observed by printing out the address of a static variable. The 'same' static variable will
// have different addresses depening on whether it is being referenced by the application or the Qt .ui form loader.
//
// This function uses shared memory to hold a reference to a single published profile. It is limited to the
// current process by including the process ID in the shared memory key.
// Note, the Qt objects in the single published profile class are all being access by the same thread, just through
// different mappings of the same dll.
PublishedProfile* ContainerProfile::getPublishedProfile()
{
    return share.publishedProfile;
}

/*
  Setup the environmental profile prior to creating some QE widgets.
  The new widgets will use this profile to determine their external environment.

  This method locks access to the envionmental profile. ReleaseProfile() must be
  called to release the lock once all QE widgets have been created.
  */
void ContainerProfile::setupProfile( QObject* guiLaunchConsumerIn,
                                             QStringList pathListIn,
                                             QString parentPathIn,
                                             QString macroSubstitutionsIn )
{
    // Publish the profile supplied
    publishProfile(guiLaunchConsumerIn,
                   pathListIn,
                   parentPathIn,
                   macroSubstitutionsIn );

    // Save a local copy of what has been published
    takeLocalCopy();
}

/*
  Update published signal consumer objects.
  This is used if the signal consumer objects were not available when the profile was
  first set up, or if the objects are changing
  */
void ContainerProfile::updateConsumers( QObject* guiLaunchConsumerIn )
{
    // If no profile has been defined, then can't update it
    if( !isProfileDefined() )
    {
        qDebug() << "Can't update consumers as a published profile has not yet been defined";
    }

    // Update the published profile
    getPublishedProfile()->guiLaunchConsumer = guiLaunchConsumerIn;

    // Keep the local copy matching what has been published
    takeLocalCopy();
}

/*
  Update just the published signal consumer object that is used to launch new GUIs.
  The previous object is returned so it can be reinstated later.
  */
QObject* ContainerProfile::replaceGuiLaunchConsumer( QObject* newGuiLaunchConsumerIn )
{
    QObject* savedGuiLaunchConsumer = guiLaunchConsumer;

    PublishedProfile* publishedProfile = getPublishedProfile();

    publishedProfile->guiLaunchConsumer = newGuiLaunchConsumerIn;
    guiLaunchConsumer = publishedProfile->guiLaunchConsumer;

    return savedGuiLaunchConsumer;
}

PersistanceManager* ContainerProfile::getPersistanceManager()
{
    PublishedProfile* publishedProfile = getPublishedProfile();
    return& publishedProfile->persistanceManager;
}

/*
  Set up the published profile.
  All instances of ContainerProfile will be able to see the published profile.
  */
void ContainerProfile::publishProfile( QObject* guiLaunchConsumerIn,
                                       QStringList pathListIn,
                                       QString parentPathIn,
                                       QString macroSubstitutionsIn )
{
    PublishedProfile* publishedProfile = getPublishedProfile();

    // Do nothing if a profile has already been published
    if( publishedProfile->profileDefined )
    {
        qDebug() << "Can't publish a profile as one is already published";
        return;
    }

    // Publish the profile
    publishedProfile->guiLaunchConsumer = guiLaunchConsumerIn;

    publishedProfile->pathList = pathListIn;
    publishedProfile->parentPath = parentPathIn;

    publishedProfile->macroSubstitutions.clear();
    if( !macroSubstitutionsIn.isEmpty() )
    {
        publishedProfile->macroSubstitutions.append( macroSubstitutionsIn );
    }

    // flag a published profile now exists
    publishedProfile->profileDefined = true;
}

/*
 Take a local copy of the profile visable to all instances of ContainerProfile
 */
void ContainerProfile::takeLocalCopy()
{
    PublishedProfile* publishedProfile = getPublishedProfile();

    QString subs;
    for( int i = 0; i < publishedProfile->macroSubstitutions.size(); i++ )
    {
        QString nextSubs = publishedProfile->macroSubstitutions[i];
        // Only add if anything present
        if( !nextSubs.isEmpty() )
        {
            // Only insert delimiter if required
            if( !subs.isEmpty() )
            {
                subs.append( "," );
            }
            subs.append( nextSubs );
        }
    }

    setupLocalProfile( publishedProfile->guiLaunchConsumer,
                       publishedProfile->pathList,
                       publishedProfile->parentPath,
                       subs );

    messageFormId = publishedProfile->messageFormId;
}

/*
  Set up the local profile only (don't refer to any published profile)
  This is used when a QE widgets needs a profile, but none has been published.
  A default local profile can be set up using this method.
  The local profile can then be made public if required by calling publishOwnProfile()
  */
void ContainerProfile::setupLocalProfile( QObject* guiLaunchConsumerIn,
                                          QStringList pathListIn,
                                          QString parentPathIn,
                                          QString macroSubstitutionsIn )
{
    // Set up the local profile as specified
    guiLaunchConsumer = guiLaunchConsumerIn;

    macroSubstitutions = macroSubstitutionsIn;

    pathList = pathListIn;
    parentPath = parentPathIn;

    messageFormId = 0;
}

/*
  Extend the macro substitutions currently being used by all new QEWidgets.
  This is used when a form is created. This allow a form to pass on macro substitutions to the QE widgets it contains.
  Since it adds to the end of the existing macro substitutions, any substitutions already added by the originating
  container or higher forms take precedence.
  Use removeMacroSubstitutions() to remove macro substitutions added by this method.
  */
void ContainerProfile::addMacroSubstitutions( QString macroSubstitutionsIn )
{
    PublishedProfile* publishedProfile = getPublishedProfile();

    if( publishedProfile->profileDefined  )
        publishedProfile->macroSubstitutions.append( macroSubstitutionsIn );
}

/*
  Extend the macro substitutions currently being used by all new QEWidgets.
  Same as addMacroSubstitutions, but these take precedence over existing macro substitutions.
  This is used when a form is created by a button. In particular, this allow a button to
  reload the same form but with different macro substitutions.
  Use removePriorityMacroSubstitutions() to remove macro substitutions added by this method.
  */
void ContainerProfile::addPriorityMacroSubstitutions( QString macroSubstitutionsIn )
{
    PublishedProfile* publishedProfile = getPublishedProfile();

    if( publishedProfile->profileDefined  )
        publishedProfile->macroSubstitutions.prepend( macroSubstitutionsIn );
}
/*
  Reduce the macro substitutions currently being used by all new QEWidgets.
  This is used after a form is created. Any macro substitutions passed on by the form being created are no longer relevent.
  */
void ContainerProfile::removeMacroSubstitutions()
{
    PublishedProfile* publishedProfile = getPublishedProfile();

    if( publishedProfile->profileDefined && !publishedProfile->macroSubstitutions.isEmpty() )
        publishedProfile->macroSubstitutions.removeLast();
}

/*
  Reduce the macro substitutions currently being used by all new QEWidgets.
  Same as removeMacroSubstitutions(), but removes substitutions added by addPriorityMacroSubstitutions().
  This is used after a form is created. Any macro substitutions passed on by the form being created are no longer relevent.
  */
void ContainerProfile::removePriorityMacroSubstitutions()
{
    PublishedProfile* publishedProfile = getPublishedProfile();

    if( publishedProfile->profileDefined && !publishedProfile->macroSubstitutions.isEmpty() )
        publishedProfile->macroSubstitutions.removeFirst();
}
/*
  Set the published profile to whatever is saved in our local copy
  */
void ContainerProfile::publishOwnProfile()
{
    publishProfile( guiLaunchConsumer,
                    pathList,
                    parentPath,
                    macroSubstitutions );
}

/*
  Clears any profile context. Must be called by any code that calls setupProfile() once the profile should no longer be used
  */
void ContainerProfile::releaseProfile()
{
    PublishedProfile* publishedProfile = getPublishedProfile();

    // Clear the profile
    publishedProfile->guiLaunchConsumer = NULL;

    publishedProfile->pathList.clear();
    publishedProfile->parentPath.clear();

    publishedProfile->macroSubstitutions.clear();

    publishedProfile->containedWidgets.clear();

    // Indicate no profile is defined
    publishedProfile->profileDefined = false;
}

/*
  Return the object to emit GUI launch request signals to.
  If NULL, there is no object available.
  */
QObject* ContainerProfile::getGuiLaunchConsumer()
{
    return guiLaunchConsumer;
}

/*
  Return the application path list to use for file operations.
  */
QStringList ContainerProfile::getPathList()
{
    return pathList;
}

/*
  Return the environment path list to use for file operations.
  */
QStringList ContainerProfile::getEnvPathList()
{
    QStringList envPathList;

    QProcessEnvironment sysEnv = QProcessEnvironment::systemEnvironment();
    QString pathVar = sysEnv.value ( "QE_UI_PATH" );
    if( !pathVar.isEmpty() )
    {
        envPathList = pathVar.split( platformSeperator() );
    }

    return envPathList;
}

/*
  Return the first entry from the application path list to use for file operations.
  */
QString ContainerProfile::getPath()
{
    if( pathList.count() )
    {
        return pathList[0];
    }
    else
    {
        QString emptyString;
        return emptyString;
    }
}

/*
  Return the current object path to use for file operations.
  */
QString ContainerProfile::getParentPath()
{
    return parentPath;
}

/*
  Set the current object path to use for file operations.
  */
void ContainerProfile::setPublishedParentPath( QString publishedParentPathIn )
{
    getPublishedProfile()->parentPath = publishedParentPathIn;
}

/*
  Return the current macro substitutions
  */
QString ContainerProfile::getMacroSubstitutions()
{
    return macroSubstitutions;
}

/*
  Return the message form ID
  */
unsigned int ContainerProfile::getMessageFormId()
{
    return messageFormId;
}

unsigned int ContainerProfile::getPublishedMessageFormId()
{
    return getPublishedProfile()->messageFormId;
}

void ContainerProfile::setPublishedMessageFormId( unsigned int publishedMessageFormIdIn )
{
    getPublishedProfile()->messageFormId = publishedMessageFormIdIn;
}

// Set the flag indicating newly created QE widgets should hold of activating until told to do so
// Return the previous value so it can be reset
bool ContainerProfile::setDontActivateYet( bool dontActivateYetIn )
{
    bool oldDontActivate = getPublishedProfile()->dontActivateYet;
    getPublishedProfile()->dontActivateYet = dontActivateYetIn;
    return oldDontActivate;
}

// Get the flag indicating newly created QE widgets should hold of activating until told to do so
bool ContainerProfile::getDontActivateYet()
{
    return getPublishedProfile()->dontActivateYet;
}

// Return flag if one or more user level passwords have been set in the profile.
// If the any passwords have been set up in the profile, then they should be used in preference to any others,
// such as those local to a QELogin widget
bool ContainerProfile::areUserLevelPasswordsSet()
{
    return getPublishedProfile()->userLevelPasswordsSet;
}

/*
  Return the flag indicating true if a profile is currently being published.
  */
bool ContainerProfile::isProfileDefined()
{
    return getPublishedProfile()->profileDefined;
}

/*
  Add a QE widgets to the list of QE widgets created under the currently published profile.
  This provides the application with a list of its QE widgets without having to trawl through
  the widget hierarchy looking for them. Note, in some applications the application may know
  exactly what QE widgets have been created, but if the application has loaded a .ui file
  unrelated to the application development (for example, a user created control GUI), then the
  application will not know how many, if any, QE widgets it owns.
  */
void ContainerProfile::addContainedWidget( QEWidget* containedWidget )
{
    getPublishedProfile()->containedWidgets.append( WidgetRef( containedWidget ) );
}

/*
  Remove a QE widgets to the list of QE widgets created under the currently published profile.
  This list provides the application with a list of its QE widgets without having to trawl through
  the widget hierarchy looking for them. Generally the entire list is discarded after it has
  been used and is no longer relevent when the widgets are deleted. Some QEWidgets can be
  destroyed, however, while a form is being created (for example, QELabel widgets are used
  within QEMotor widgets and are created and sometimes destroyed during the creation of a QEMotor
  record) For this reason, QEWidgets are removed from this list on destruction.

  This operation is reasonably expensive, but only when there is a large number of widgets in the list.
  Generally, the list is empty.
  */
void ContainerProfile::removeContainedWidget( QEWidget* containedWidget )
{
    PublishedProfile* publishedProfile = getPublishedProfile();

    // Search for the widget in the list
    int s = publishedProfile->containedWidgets.size();
    for( int i = 0; i < s; i++ )
    {
        // If found, remove the widget and finish
        if( publishedProfile->containedWidgets[i].getRef() == containedWidget )
        {
            publishedProfile->containedWidgets.removeAt( i );
            break;
        }
    }
}

/*
  Return the next QE widgets from the list of QE widgets built using addContainedWidget().
  Note, this is destructive to the list. It is fine if the application only needs to get the
  widgets from the list once, such as when activating QE widgets after creating a form.
  */
QEWidget* ContainerProfile::getNextContainedWidget()
{
    PublishedProfile* publishedProfile = getPublishedProfile();

    // Remove and return the first widget in the list, or return NULL if no more
    if( !publishedProfile->containedWidgets.isEmpty() )
        return publishedProfile->containedWidgets.takeFirst().getRef();
    else
        return NULL;
}

/*
  Get the local copy of the user level password for the specified user level
  */
QString ContainerProfile::getUserLevelPassword( userLevelTypes::userLevels level )
{
    switch( level )
    {
        case userLevelTypes::USERLEVEL_USER:      return getPublishedProfile()->userLevelPassword;      break;
        case userLevelTypes::USERLEVEL_SCIENTIST: return getPublishedProfile()->scientistLevelPassword; break;
        case userLevelTypes::USERLEVEL_ENGINEER:  return getPublishedProfile()->engineerLevelPassword;  break;
        default: return QString();
    }
}

/*
  Set the local copy of the user level password for the specified user level
  */
void ContainerProfile::setUserLevelPassword( userLevelTypes::userLevels level, QString passwordIn )
{
    switch( level )
    {
        case userLevelTypes::USERLEVEL_USER:      getPublishedProfile()->userLevelPassword      = passwordIn; break;
        case userLevelTypes::USERLEVEL_SCIENTIST: getPublishedProfile()->scientistLevelPassword = passwordIn; break;
        case userLevelTypes::USERLEVEL_ENGINEER:  getPublishedProfile()->engineerLevelPassword  = passwordIn; break;
    }
    getPublishedProfile()->userLevelPasswordsSet = true;
}


/*
  Set the application user type (user/scientist/engineer)
  */
void ContainerProfile::setUserLevel( userLevelTypes::userLevels level )
{
    // Update the user level (this will result in a signal being emited
    getPublishedProfile()->userSignal.setLevel( level );
}


// Return the platform dependant path separator (between paths, not directories in a path).
// Qt only provides a platform directory separator (\ or /)
QChar ContainerProfile::platformSeperator()
{
    // If directory seperator is '/' platform is linux, path seperator is ':'
    // else (directory seperator is '\') platform assumed windows, path seperator is ';'
    return( QDir::separator() == '/' )?':':';';
}

// Return the user level string name given the user level value
QString ContainerProfile::getUserLevelName( userLevelTypes::userLevels userLevelValue )
{
    switch( userLevelValue )
    {
        case userLevelTypes::USERLEVEL_USER:      return "User";
        case userLevelTypes::USERLEVEL_SCIENTIST: return "Scientist";
        case userLevelTypes::USERLEVEL_ENGINEER:  return "Engineer";
        default: return "Unknown";
    }
}

// Return the user level value given the user level string name
userLevelTypes::userLevels ContainerProfile::getUserLevelValue( QString userLevelName )
{
    if( userLevelName == "User" )
    {
        return userLevelTypes::USERLEVEL_USER;
    }
    else if ( ( userLevelName == "Scientist" ) )
    {
        return userLevelTypes::USERLEVEL_SCIENTIST;
    }
    else if ( ( userLevelName == "Engineer" ) )
    {
        return userLevelTypes::USERLEVEL_ENGINEER;
    }
    else // default
    {
        return userLevelTypes::USERLEVEL_USER;
    }
}

void userLevelSignal::setLevel( userLevelTypes::userLevels levelIn )
{
    level = levelIn;
    emit userChanged( level );
}

/*
  Get the application user type (user/scientist/engineer)
  */
userLevelTypes::userLevels ContainerProfile::getUserLevel()
{
    return getPublishedProfile()->userSignal.getLevel();
}

userLevelSlot::userLevelSlot()
{
    owner = NULL;
}
userLevelSlot::~userLevelSlot()
{
}

void userLevelSlot::setOwner( ContainerProfile* ownerIn )
{
    owner = ownerIn;
}

void userLevelSlot::userChanged( userLevelTypes::userLevels level )
{
    if( owner )
        owner->userLevelChangedGeneral( level );
}

userLevelSignal::userLevelSignal()
{
    level = userLevelTypes::USERLEVEL_USER;
}
userLevelSignal::~userLevelSignal()
{
}

userLevelTypes::userLevels userLevelSignal::getLevel()
{
    return level;
}


// Published own profile if and only if required.
//
ProfilePublisher::ProfilePublisher( QEWidget* ownerIn,
                                    const QString& prioritySubstitutions )
{
    owner = ownerIn;
    if (!owner) return;

    // Do we need to publish a local profile?
    //
    if( owner->isProfileDefined() ){
        // No - one is already published.
        localProfileWasPublished = false;
    } else {
        // Flag the profile was set up in this function (and so should be released
        // in the destructor).
        //
        owner->publishOwnProfile();
        localProfileWasPublished = true;
    }

    // Extend any variable name substitutions with this button's substitutions
    // Like most other macro substitutions, the substitutions already present
    // take precedence.
    //
    owner->addMacroSubstitutions( owner->getVariableNameSubstitutions () );

    // Extend any variable name substitutions with this the priority substitutions.
    // Unlike most other macro substitutions, these macro substitutions take
    // precedence over substitutions already present.
    //
    owner->addPriorityMacroSubstitutions( prioritySubstitutions );
}

// Reverse the operations applied during construction.
//
ProfilePublisher::~ProfilePublisher()
{
    if (!owner) return;

    // Remove this the priority macro substitutions now all its children are created.
    //
    owner->removePriorityMacroSubstitutions ();

    // Remove this button's normal macro substitutions now all its children are created.
    //
    owner->removeMacroSubstitutions ();

    // Release the profile, if we defined one, now that all QE widgets have been created.
    //
    if( localProfileWasPublished ){
        owner->releaseProfile ();
    }
}

// end
