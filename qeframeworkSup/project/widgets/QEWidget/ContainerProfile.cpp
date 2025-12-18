/*  ContainerProfile.cpp
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

#include "ContainerProfile.h"
#include <QCoreApplication>
#include <QProcessEnvironment>
#include <QDebug>
#include <QDir>
#include <QEWidget.h>
#include <QtDebug>
#include <macroSubstitution.h>

#define DEBUG qDebug() << "ContainerProfile" << __LINE__ << __FUNCTION__ << "  "


// static singleton actual declaration
//
QEPublishedProfile ContainerProfile::publishedProfile;

//==============================================================================
// Construction
//
QEPublishedProfile::QEPublishedProfile()
{
    this->guiLaunchConsumer = NULL;
    this->messageFormId = 0;
    this->profileDefined = false;
    this->dontActivateYet = false;
    this->userLevelPasswordsSet = false;
}

//------------------------------------------------------------------------------
//
QEPublishedProfile::~QEPublishedProfile() { }


//==============================================================================
// Constructor.
// A local copy of the defined profile (if any) is made.
// Note, this does not define a profile. A profile is defined only when ContainerProfile::setupProfile() is called
ContainerProfile::ContainerProfile()
{
    // Set up the object that will recieve signals that the user level has changed
    this->userSlot.setOwner( this );

    QEPublishedProfile* publishedProfile = ContainerProfile::getPublishedProfile();
    QObject::connect( &(publishedProfile->userSignal), SIGNAL( userChanged( QE::UserLevels ) ),
                      &this->userSlot,                 SLOT  ( userChanged( QE::UserLevels ) ) );

    // Take a local copy of the defined profile
    this->takeLocalCopy();
}

// Destructor
// Note, if the profile has been defined (ContainerProfile::setupProfile() has been
// called) this does not release the profile. A profile is released only when
// ContainerProfile::releaseProfile() is called.
//
ContainerProfile::~ContainerProfile() { }

//------------------------------------------------------------------------------
// Get a pointer to the unique instance of the published profile.
//
QEPublishedProfile* ContainerProfile::getPublishedProfile()
{
    return &ContainerProfile::publishedProfile;
}

/* -----------------------------------------------------------------------------
  Setup the environmental profile prior to creating some QE widgets.
  The new widgets will use this profile to determine their external environment.

  This method locks access to the envionmental profile. ReleaseProfile() must be
  called to release the lock once all QE widgets have been created.
  */
void ContainerProfile::setupProfile( QObject* guiLaunchConsumerIn,
                                     const QStringList& pathListIn,
                                     const QString& parentPathIn,
                                     const QString& macroSubstitutionsIn )
{
    // Publish the profile supplied
    this->publishProfile( guiLaunchConsumerIn,
                          pathListIn,
                          parentPathIn,
                          macroSubstitutionsIn );

    // Save a local copy of what has just been published
    this->takeLocalCopy();
}

/* -----------------------------------------------------------------------------
  Update published signal consumer objects.
  This is used if the signal consumer objects were not available when the profile was
  first set up, or if the objects are changing
  */
void ContainerProfile::updateConsumers( QObject* guiLaunchConsumerIn )
{
    // If no profile has been defined, then can't update it
    if( !ContainerProfile::isProfileDefined() )
    {
        qDebug() << "Can't update consumers as a published profile has not yet been defined";
    }

    // Update the published profile
    ContainerProfile::getPublishedProfile()->guiLaunchConsumer = guiLaunchConsumerIn;

    // Keep the local copy matching what has been published
    this->takeLocalCopy();
}

/* -----------------------------------------------------------------------------
  Update just the published signal consumer object that is used to launch new GUIs.
  The previous object is returned so it can be reinstated later.
  */
QObject* ContainerProfile::replaceGuiLaunchConsumer( QObject* newGuiLaunchConsumerIn )
{
    QObject* savedGuiLaunchConsumer = this->guiLaunchConsumer;

    QEPublishedProfile* publishedProfile = ContainerProfile::getPublishedProfile();

    publishedProfile->guiLaunchConsumer = newGuiLaunchConsumerIn;
    this->guiLaunchConsumer = publishedProfile->guiLaunchConsumer;

    return savedGuiLaunchConsumer;
}

/* -----------------------------------------------------------------------------
   Virtual function default/null implementation
 */
void ContainerProfile::userLevelChangedGeneral( QE::UserLevels ) {}

/* -----------------------------------------------------------------------------
   Return a reference to the single persistance manager
 */
PersistanceManager* ContainerProfile::getPersistanceManager()
{
    QEPublishedProfile* publishedProfile = ContainerProfile::getPublishedProfile();
    return & publishedProfile->persistanceManager;
}

/* -----------------------------------------------------------------------------
  Set up the published profile.
  All instances of ContainerProfile will be able to see the published profile.
  */
void ContainerProfile::publishProfile( QObject* guiLaunchConsumerIn,
                                       const QStringList& pathListIn,
                                       const QString& parentPathIn,
                                       const QString& macroSubstitutionsIn )
{
    QEPublishedProfile* publishedProfile = ContainerProfile::getPublishedProfile();

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

/* -----------------------------------------------------------------------------
 Take a local copy of the profile visable to all instances of ContainerProfile
 */
void ContainerProfile::takeLocalCopy()
{
    QEPublishedProfile* publishedProfile = ContainerProfile::getPublishedProfile();

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

    this->setupLocalProfile( publishedProfile->guiLaunchConsumer,
                             publishedProfile->pathList,
                             publishedProfile->parentPath,
                             subs );

    this->messageFormId = publishedProfile->messageFormId;
}

/* -----------------------------------------------------------------------------
  Set up the local profile only (don't refer to any published profile)
  This is used when a QE widgets needs a profile, but none has been published.
  A default local profile can be set up using this method.
  The local profile can then be made public if required by calling publishOwnProfile()
  */
void ContainerProfile::setupLocalProfile( QObject* guiLaunchConsumerIn,
                                          const QStringList& pathListIn,
                                          const QString& parentPathIn,
                                          const QString& macroSubstitutionsIn )
{
    // Set up the local profile as specified
    //
    this->guiLaunchConsumer = guiLaunchConsumerIn;
    this->macroSubstitutions = macroSubstitutionsIn;

    this->pathList = pathListIn;
    this->parentPath = parentPathIn;

    this->messageFormId = 0;
}

/* -----------------------------------------------------------------------------
  Extend the macro substitutions currently being used by all new QEWidgets.
  This is used when a form is created. This allow a form to pass on macro substitutions to the QE widgets it contains.
  Since it adds to the end of the existing macro substitutions, any substitutions already added by the originating
  container or higher forms take precedence.
  Use removeMacroSubstitutions() to remove macro substitutions added by this method.
  static
  */
void ContainerProfile::addMacroSubstitutions( QString macroSubstitutionsIn )
{
    QEPublishedProfile* publishedProfile = ContainerProfile::getPublishedProfile();

    if( publishedProfile->profileDefined  )
        publishedProfile->macroSubstitutions.append( macroSubstitutionsIn );
}

/* -----------------------------------------------------------------------------
  Extend the macro substitutions currently being used by all new QEWidgets.
  Same as addMacroSubstitutions, but these take precedence over existing macro substitutions.
  This is used when a form is created by a button. In particular, this allow a button to
  reload the same form but with different macro substitutions.
  Use removePriorityMacroSubstitutions() to remove macro substitutions added by this method.
  */
void ContainerProfile::addPriorityMacroSubstitutions( QString macroSubstitutionsIn )
{
    QEPublishedProfile* publishedProfile = ContainerProfile::getPublishedProfile();

    QString presubPriority = macroSubstitutionsIn;

    // Pre substitute priority substitutions in case user has specified AA=$(AA)
    //
    for (int j = 0 ; j < publishedProfile->macroSubstitutions.count(); j++) {
       macroSubstitutionList parts = macroSubstitutionList (publishedProfile->macroSubstitutions.value(j));
       presubPriority = parts.substitute(presubPriority);
    }

    if( publishedProfile->profileDefined  )
        publishedProfile->macroSubstitutions.prepend( presubPriority );
}

/* -----------------------------------------------------------------------------
  Reduce the macro substitutions currently being used by all new QEWidgets.
  This is used after a form is created. Any macro substitutions passed on by
  the form being created are no longer relevent.
  */
void ContainerProfile::removeMacroSubstitutions()
{
    QEPublishedProfile* publishedProfile = ContainerProfile::getPublishedProfile();

    if( publishedProfile->profileDefined && !publishedProfile->macroSubstitutions.isEmpty() )
        publishedProfile->macroSubstitutions.removeLast();
}

/* -----------------------------------------------------------------------------
  Reduce the macro substitutions currently being used by all new QEWidgets.
  Same as removeMacroSubstitutions(), but removes substitutions added by addPriorityMacroSubstitutions().
  This is used after a form is created. Any macro substitutions passed on by the form being created are no longer relevent.
  */
void ContainerProfile::removePriorityMacroSubstitutions()
{
    QEPublishedProfile* publishedProfile = ContainerProfile::getPublishedProfile();

    if( publishedProfile->profileDefined && !publishedProfile->macroSubstitutions.isEmpty() )
        publishedProfile->macroSubstitutions.removeFirst();
}

/* -----------------------------------------------------------------------------
  Set the published profile to whatever is saved in our local copy
  */
void ContainerProfile::publishOwnProfile()
{
    publishProfile( this->guiLaunchConsumer,
                    this->pathList,
                    this->parentPath,
                    this->macroSubstitutions );
}

/* -----------------------------------------------------------------------------
  Clears any profile context. Must be called by any code that calls setupProfile()
  once the profile should no longer be used
  */
void ContainerProfile::releaseProfile()
{
    QEPublishedProfile* publishedProfile = ContainerProfile::getPublishedProfile();

    // Clear the profile
    publishedProfile->guiLaunchConsumer = NULL;

    publishedProfile->pathList.clear();
    publishedProfile->parentPath.clear();

    publishedProfile->macroSubstitutions.clear();

    publishedProfile->containedWidgets.clear();

    // Indicate no profile is defined
    publishedProfile->profileDefined = false;
}

/* -----------------------------------------------------------------------------
  Return the object to emit GUI launch request signals to.
  If NULL, there is no object available.
  */
QObject* ContainerProfile::getGuiLaunchConsumer() const
{
    return this->guiLaunchConsumer;
}

/* -----------------------------------------------------------------------------
  Return the application path list to use for file operations.
  */
QStringList ContainerProfile::getPathList() const
{
    return this->pathList;
}

/* -----------------------------------------------------------------------------
  Return the environment path list to use for file operations.
  */
QStringList ContainerProfile::getEnvPathList()
{
    QStringList envPathList;

    QProcessEnvironment sysEnv = QProcessEnvironment::systemEnvironment();
    QString pathVar = sysEnv.value( "QE_UI_PATH" );
    if( !pathVar.isEmpty() )
    {
        envPathList = pathVar.split( ContainerProfile::platformSeperator() );
    }

    return envPathList;
}

/* -----------------------------------------------------------------------------
  Return the first entry from the application path list to use for file operations.
  */
QString ContainerProfile::getPath() const
{
    QString result;   // emptyString
    if( this->pathList.count() )
    {
        result =  this->pathList[0];
    }
    return result;
}

/* -----------------------------------------------------------------------------
  Return the current object path to use for file operations.
  */
QString ContainerProfile::getParentPath() const
{
    return this->parentPath;
}

/* -----------------------------------------------------------------------------
  Set the current object path to use for file operations.
  */
void ContainerProfile::setPublishedParentPath( QString publishedParentPathIn )
{
    ContainerProfile::getPublishedProfile()->parentPath = publishedParentPathIn;
}

/* -----------------------------------------------------------------------------
  Return the current macro substitutions
  */
QString ContainerProfile::getMacroSubstitutions() const
{
    return this->macroSubstitutions;
}

/* -----------------------------------------------------------------------------
  Return the message form ID
  */
unsigned int ContainerProfile::getMessageFormId() const
{
    return this->messageFormId;
}

unsigned int ContainerProfile::getPublishedMessageFormId()
{
    return ContainerProfile::getPublishedProfile()->messageFormId;
}

void ContainerProfile::setPublishedMessageFormId( unsigned int publishedMessageFormIdIn )
{
    ContainerProfile::getPublishedProfile()->messageFormId = publishedMessageFormIdIn;
}

// Set the flag indicating newly created QE widgets should hold off activating
// until told to do so. Return the previous value so it can be reset.
bool ContainerProfile::setDontActivateYet( bool dontActivateYetIn )
{
    QEPublishedProfile* publishedProfile = ContainerProfile::getPublishedProfile();

    const bool oldDontActivate = publishedProfile->dontActivateYet;
    publishedProfile->dontActivateYet = dontActivateYetIn;

    return oldDontActivate;
}

// Get the flag indicating newly created QE widgets should hold off activating
// until told to do so.
bool ContainerProfile::getDontActivateYet()
{
    return ContainerProfile::getPublishedProfile()->dontActivateYet;
}

// Return flag if one or more user level passwords have been set in the profile.
// If the any passwords have been set up in the profile, then they should be
// used in preference to any others, such as those local to a QELogin widget.
bool ContainerProfile::areUserLevelPasswordsSet()
{
    return ContainerProfile::getPublishedProfile()->userLevelPasswordsSet;
}

/* -----------------------------------------------------------------------------
  Return the flag indicating true if a profile is currently being published.
  */
bool ContainerProfile::isProfileDefined()
{
    return ContainerProfile::getPublishedProfile()->profileDefined;
}

/* -----------------------------------------------------------------------------
  Add a QE widgets to the list of QE widgets created under the currently published profile.
  This provides the application with a list of its QE widgets without having to trawl through
  the widget hierarchy looking for them. Note, in some applications the application may know
  exactly what QE widgets have been created, but if the application has loaded a .ui file
  unrelated to the application development (for example, a user created control GUI), then the
  application will not know how many, if any, QE widgets it owns.
  */
void ContainerProfile::addContainedWidget( QEWidget* containedWidget )
{
    ContainerProfile::getPublishedProfile()->containedWidgets.append( containedWidget  );
}

/* -----------------------------------------------------------------------------
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
    QEPublishedProfile* publishedProfile = ContainerProfile::getPublishedProfile();

    // Search for the widget in the list
    const int s = publishedProfile->containedWidgets.size();
    for( int i = 0; i < s; i++ )
    {
        // If found, remove the widget and finish
        if( publishedProfile->containedWidgets.value( i, NULL ) == containedWidget )
        {
            publishedProfile->containedWidgets.removeAt( i );
            break;
        }
    }
}

/* -----------------------------------------------------------------------------
  Return the next QE widgets from the list of QE widgets built using addContainedWidget().
  Note, this is destructive to the list. It is fine if the application only needs to get the
  widgets from the list once, such as when activating QE widgets after creating a form.
  */
QEWidget* ContainerProfile::getNextContainedWidget()
{
    QEPublishedProfile* publishedProfile = ContainerProfile::getPublishedProfile();

    // Remove and return the first widget in the list, or return NULL if no more
    if( !publishedProfile->containedWidgets.isEmpty() )
        return publishedProfile->containedWidgets.takeFirst();
    else
        return NULL;
}

/* -----------------------------------------------------------------------------
  Get the local copy of the user level password for the specified user level
  */
QString ContainerProfile::getUserLevelPassword( QE::UserLevels level )
{
    QEPublishedProfile* publishedProfile = ContainerProfile::getPublishedProfile();

    QString result;
    switch( level )
    {
        case QE::User:      result = publishedProfile->userLevelPassword;      break;
        case QE::Scientist: result = publishedProfile->scientistLevelPassword; break;
        case QE::Engineer:  result = publishedProfile->engineerLevelPassword;  break;
    }
    return result;
}

/* -----------------------------------------------------------------------------
  Set the local copy of the user level password for the specified user level
  */
void ContainerProfile::setUserLevelPassword( QE::UserLevels level, QString passwordIn )
{
    QEPublishedProfile* publishedProfile = ContainerProfile::getPublishedProfile();

    switch( level )
    {
        case QE::User:      publishedProfile->userLevelPassword      = passwordIn; break;
        case QE::Scientist: publishedProfile->scientistLevelPassword = passwordIn; break;
        case QE::Engineer:  publishedProfile->engineerLevelPassword  = passwordIn; break;
    }

    publishedProfile->userLevelPasswordsSet = true;
}

/* -----------------------------------------------------------------------------
  Set the application user type (user/scientist/engineer)
  */
void ContainerProfile::setUserLevel( QE::UserLevels level )
{
    // Update the user level (this will result in a signal being emited
    ContainerProfile::getPublishedProfile()->userSignal.setLevel( level );
}

//------------------------------------------------------------------------------
// Return the platform dependant path separator (between paths, not directories in a path).
// Qt only provides a platform directory separator (\ or /)
QChar ContainerProfile::platformSeperator()
{
    // If directory seperator is '/' platform is linux, path seperator is ':'
    // else (directory seperator is '\') platform assumed windows, path seperator is ';'
    return( QDir::separator() == '/' )?':':';';
}

//------------------------------------------------------------------------------
// Return the user level string name given the user level value
QString ContainerProfile::getUserLevelName( QE::UserLevels userLevelValue )
{
    switch( userLevelValue )
    {
        case QE::User:      return "User";
        case QE::Scientist: return "Scientist";
        case QE::Engineer:  return "Engineer";
        default: return "Unknown";
    }
}

//------------------------------------------------------------------------------
// Return the user level value given the user level string name
QE::UserLevels ContainerProfile::getUserLevelValue( QString userLevelName )
{
    if( userLevelName == "User" )
    {
        return QE::User;
    }
    else if ( ( userLevelName == "Scientist" ) )
    {
        return QE::Scientist;
    }
    else if ( ( userLevelName == "Engineer" ) )
    {
        return QE::Engineer;
    }
    else // default
    {
        return QE::User;
    }
}

/* -----------------------------------------------------------------------------
  Get the application user type (user/scientist/engineer)
  */
QE::UserLevels ContainerProfile::getUserLevel()
{
    return ContainerProfile::getPublishedProfile()->userSignal.getLevel();
}

//==============================================================================
//
QEProfileUserLevelSlot::QEProfileUserLevelSlot() : QObject( NULL )
{
    this->owner = NULL;
}

//------------------------------------------------------------------------------
//
QEProfileUserLevelSlot::~QEProfileUserLevelSlot() { }

//------------------------------------------------------------------------------
//
void QEProfileUserLevelSlot::setOwner( ContainerProfile* ownerIn )
{
    this->owner = ownerIn;
}

//------------------------------------------------------------------------------
//
void QEProfileUserLevelSlot::userChanged( QE::UserLevels level )
{
    if( this->owner )
        this->owner->userLevelChangedGeneral( level );
}

//==============================================================================
//
QEProfileUserLevelSignal::QEProfileUserLevelSignal() : QObject( NULL )
{
    this->level = QE::User;
}

//------------------------------------------------------------------------------
//
QEProfileUserLevelSignal::~QEProfileUserLevelSignal() { }

//------------------------------------------------------------------------------
//
void QEProfileUserLevelSignal::setLevel( QE::UserLevels levelIn )
{
    this->level = levelIn;
    emit userChanged( level );
}

//------------------------------------------------------------------------------
//
QE::UserLevels QEProfileUserLevelSignal::getLevel() const
{
    return this->level;
}


//==============================================================================
// Published own profile if and only if required.
//
ProfilePublisher::ProfilePublisher( QEWidget* ownerIn,
                                    const QString& prioritySubstitutions )
{
    this->owner = ownerIn;
    if (!this->owner) return;

    // Do we need to publish a local profile?
    //
    if( this->owner->isProfileDefined() ){
        // No - one is already published.
        this->localProfileWasPublished = false;
    } else {
        // Flag the profile was set up in this function (and so should be released
        // in the destructor).
        //
        this->owner->publishOwnProfile();
        this->localProfileWasPublished = true;
    }

    // Extend any variable name substitutions with this button's substitutions
    // Like most other macro substitutions, the substitutions already present
    // take precedence.
    //
    this->owner->addMacroSubstitutions( owner->getVariableNameSubstitutions () );

    // Extend any variable name substitutions with this the priority substitutions.
    // Unlike most other macro substitutions, these macro substitutions take
    // precedence over substitutions already present.
    //
    this->owner->addPriorityMacroSubstitutions( prioritySubstitutions );
}

//------------------------------------------------------------------------------
// Reverse the operations applied during construction.
//
ProfilePublisher::~ProfilePublisher()
{
    if (!this->owner) return;

    // Remove this the priority macro substitutions now all its children are created.
    //
    this->owner->removePriorityMacroSubstitutions ();

    // Remove this button's normal macro substitutions now all its children are created.
    //
    this->owner->removeMacroSubstitutions ();

    // Release the profile, if we defined one, now that all QE widgets have been created.
    //
    if( this->localProfileWasPublished ){
        this->owner->releaseProfile ();
    }
}

// end
