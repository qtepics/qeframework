/*  ContainerProfile.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2018 Australian Synchrotron
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

/* Description:
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

#ifndef QE_CONTAINER_PROFILE_H
#define QE_CONTAINER_PROFILE_H

#include <QSharedMemory>
#include <QObject>
#include <QMutex>
#include <QList>
#include <QStringList>
#include <QDebug>
#include <QEFrameworkLibraryGlobal.h>
#include <persistanceManager.h>

class QEWidget;             // differed
class ContainerProfile;     // differed

// Define the user levels
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT userLevelTypes : public QObject
{
    Q_OBJECT
public:
    // Define the user levels
    // NOTE: order must remain least privileged to most privileged
    /// \public
    /// \enum userLevels
    /// User levels set by widgets such as QELogin and used by many widgets to determine visibility, enabled state, and style.
    enum userLevels { USERLEVEL_USER,       ///< User level - least privilaged
                      USERLEVEL_SCIENTIST,  ///< User level - more privilaged than user, less than engineer
                      USERLEVEL_ENGINEER    ///< User level - most privilaged
                    };
    Q_ENUMS (userLevels)
};


// Published profile, and the shared memory to reference it,
// These static variables are instantiated twice on windows - once when this code
// is loaded by an application (QEGui); and once when the Qt .ui loaded loads
// this code to support creation of QE widgets.
//
class QEPublishedProfile;

class QEEnvironmentShare
{
public:
    explicit QEEnvironmentShare();
    ~QEEnvironmentShare();

    QSharedMemory* sharedMemory;            // Memory to hold a reference to the application wide PublishedProfile
    QEPublishedProfile* publishedProfile;   // Reference to the application wide PublishedProfile
    bool publishedProfileCreatedByMe;       // True of this instance of the QEEnvironmentShare class allocated the memory for the publishedProfile
};


// Class used to generate signals that the user level has changed.
// A single instance of this class is shared by all instances of
// the ContainerProfile class.
// The ContainerProfile class can't generate signals directly as it
// is not based on QObject and can't be as it is a base class for
// Widgets that may also be based on Qwidgets and only one base
// class can be based on a QObject.
//
class QEProfileUserLevelSignal : public QObject
{
    Q_OBJECT
public:
    // Constructor, destructor
    // Default to 'user'
    explicit QEProfileUserLevelSignal();
    ~QEProfileUserLevelSignal();

    // Set the application wide user level
    // When level is set in the single instance of this class, all ContainerProfile
    // classes are signaled
    void setLevel( userLevelTypes::userLevels level );

    // Get the application wide user level
    // Each widget can reimplement ContainerProfile::userLevelChanged() to be
    // notified of user level changes, but this function can be used to
    // determine the user level when a widget is first created
    userLevelTypes::userLevels getLevel() const;

signals:
    /// Internal use only. Send when the user level has changed
    void userChanged( userLevelTypes::userLevels level );   // User level change signal

private:
    userLevelTypes::userLevels level;    // Current user level
};

// Class used to recieve signals that the user level has changed.
// An instance of this class is used by each instance of the
// ContainerProfile class.
// The ContainerProfile class can't recieve signals directly as it
// is not based on QObject and can't be as it is a base class for
// Widgets that may also be based on Qwidgets and only one base
// class can be based on a QObject.
//
class QEProfileUserLevelSlot : public QObject
{
    Q_OBJECT

public:
    // Constructor, destructor
    // Default to no owner
    explicit QEProfileUserLevelSlot();
    ~QEProfileUserLevelSlot();

    // Set the ContainerProfile class that this instance is a part of
    void setOwner( ContainerProfile* owner );

public slots:
    void userChanged( userLevelTypes::userLevels level );  // Receive user level change signals

private:
    ContainerProfile* owner;                                // ContainerProfile class that this instance is a part of
};

// Class to allow a truly unique instance of the published profile.
// This is a singleton class - all operations are via static ContainerProfile methods.
//
// On Windows, static variables are defined twice when the QE plugin library is
// loaded: once by QEGui and once by the Qt .ui loader when creating forms.
//
// NOTE: Since the QEPlugin library was split into the functional QEFramework library
// and a minimalist/pure plugin QEPlugin library (with no static variables) I strongly
// suspect that two instance is no longer occurs; however even if that is the case
// I have left this mechanism as is (at least until it ever becomes an issue).
//
class QEPublishedProfile
{
public:
    explicit QEPublishedProfile();      // Construction
    ~QEPublishedProfile();              // Destruction

    QObject* guiLaunchConsumer;         // Object to send GUI launch requests to
    QStringList pathList;               // Path list used for file operations (scope: application wide)
    QString parentPath;                 // Path used for file operations (scope: Parent object, if any. This is set up by the application, but is temporarily overwritten and then reset by each level of sub object (sub form)
    QList<QString> macroSubstitutions;  // list of variable name macro substitution strings. Extended by each sub form created
    unsigned int messageFormId;         // Current form ID. Used to group forms with their widgets for messaging

    QList<QEWidget*> containedWidgets;  // List of QE widgets created with this profile

    QEProfileUserLevelSignal userSignal;   // Current user level signal object. One instance to signal all QE Widgets

    QString userLevelPassword;          // User Level password for 'user'
    QString scientistLevelPassword;     // User Level password for 'scientist'
    QString engineerLevelPassword;      // User Level password for 'engineer'

    bool profileDefined;                // Flag true if a profile has been setup. Set between calling setupProfile() and releaseProfile()

    PersistanceManager persistanceManager;  // Persistance manager to manage configuration save and restore
    bool dontActivateYet;               // Flag true if QE widgets should hold of activating (connection to data) until told to do so

    bool userLevelPasswordsSet;         // One or more user level passwords have been set. Use passwords defined in the profile
};


// Class to provide a communication mechanism from the code creating QE widgets to the QE widgets.
// See ContainerProfile.cpp for details
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT ContainerProfile
{
public:
    explicit ContainerProfile();    // Construction. Sets up local copies of any profile defined by setupProfile() in an earlier instance
    virtual ~ContainerProfile();    // Destruction

    // Setup local profile based on published profile
    void takeLocalCopy();

    // Setup a local environmental profile  and publishe for all QEWidgets to use on creation.
    void setupProfile( QObject* guiLaunchConsumer,
                       const QStringList& pathList,
                       const QString& parentPath,
                       const QString& macroSubstitutions );

    // Setup the local environmental profile for this instance only
    void setupLocalProfile( QObject* guiLaunchConsumer,
                            const QStringList& pathList,
                            const QString& parentPath,
                            const QString& macroSubstitutions );

    void publishOwnProfile();                 // Set the published profile to whatever is saved in our local copy

    // Update the local and published signal consumer objects
    void updateConsumers( QObject* guiLaunchConsumer );

    // Update the local and published signal consumer objects
    // Returns what was the local copy of the object that will recieve GUI launch requests
    QObject* replaceGuiLaunchConsumer( QObject* newGuiLaunchConsumer );  // Override the current GUI launch consumer

    QObject* getGuiLaunchConsumer() const;    // Get the local copy of the object that will recieve GUI launch requests
    QString getPath() const;                  // Get the local copy of the first entry in the application path list used for file operations
    QStringList getPathList() const;          // Get the local copy of the application path list used for file operations
    QString getParentPath() const;            // Get the local copy of the current object path used for file operations
    QString getMacroSubstitutions() const;    // Get the local copy of the variable name macro substitutions
    unsigned int getMessageFormId() const;    // Get the local copy of the message form ID

    // The following methods are static - they operate on  share.publishedProfile
    //
    static void addMacroSubstitutions( QString macroSubstitutions ); // Add another set of macro substitutions to those setup by setupProfile(). Used as sub forms are created
    static void removeMacroSubstitutions();                            // Remove the last set of macro substitutions added by addMacroSubstitutions(). Used after sub forms are created

    static void addPriorityMacroSubstitutions( QString macroSubstitutions ); // Add another set of macro substitutions to those setup by setupProfile(). Used as sub forms are created. These macros take priority
    static void removePriorityMacroSubstitutions();                            // Remove the last set of macro substitutions added by addPriorityMacroSubstitutions(). Used after sub forms are created

    static void setPublishedParentPath( QString publishedParentPath ); // Set the published current object path used for file operations
    static bool isProfileDefined();           // Returns true if a profile has been setup by setupProfile()
    static bool areUserLevelPasswordsSet();   // Return true if one or more user level passwords have been set in the profile

    static QString getUserLevelPassword( userLevelTypes::userLevels level );  // Get the local copy of the user level password for the specified user level
    static void setUserLevelPassword( userLevelTypes::userLevels level, QString password );  // Set the local copy of the user level password for the specified user level

    static void addContainedWidget( QEWidget* containedWidget );    // Adds a reference to the list of QE widgets created with this profile
    static QEWidget* getNextContainedWidget();               // Returns a reference to the next QE widgets in the list of QE widgets created with this profile
    static void removeContainedWidget( QEWidget* containedWidget ); // Remove a reference from the list of QE widgets created with this profile

    static unsigned int getPublishedMessageFormId();         // Get the currently published message form ID
    static void setPublishedMessageFormId( unsigned int publishedMessageFormId );  // Set the currently published message form ID

    static QStringList getEnvPathList();                     // Get the path list from the environment variable

    static bool setDontActivateYet( bool dontActivate );   // Flag newly created QE widgets should hold of activating until told to do so
    static bool getDontActivateYet();                        // Get flag indicating QE widgets should hold of activating until told to do so

    static void releaseProfile();                            // Clears the context setup by setupProfile(). Local data in all instances is still valid

    static void setUserLevel( userLevelTypes::userLevels level );        // Set the current user level
    static userLevelTypes::userLevels getUserLevel();                    // Return the current user level

    static PersistanceManager* getPersistanceManager();  // Return a reference to the single persistance manager

    static QChar platformSeperator();                   // Return the platform dependant path separator (between paths, not directories in a path). Qt only provides a platform directory separator (\ or /)

    static QString getUserLevelName( userLevelTypes::userLevels userLevelValue );   // Return the user level string given the user level
    static userLevelTypes::userLevels getUserLevelValue( QString userLevelName );   // Return the user level value given the user level string name

protected:
    // Virtual function implemented by QEWidget that manages general aspects of
    // user level change, then calls optional QE widget specific virtual functions.
    //
    virtual void userLevelChangedGeneral( userLevelTypes::userLevels );

private:
    // Publish an environmental profile for all QEWidgets to use on creation
    static void publishProfile( QObject* guiLaunchConsumer,
                                const QStringList& pathList,
                                const QString& publishedParentPath,
                                const QString& macroSubstitutions );

    static QEPublishedProfile* getPublishedProfile();     // Get the single instance of the published profile (via share)

    QEProfileUserLevelSlot userSlot;                             // Current user level slot object. An instance per ContainerProfile to recieve level changes

    QObject* guiLaunchConsumer;      // Local copy of GUI launch consumer. Still valid after the profile has been released by releaseProfile()
    QStringList pathList;            // Local copy of application path list used for file operations
    QString parentPath;              // Local copy of parent object path used for file operations
    QString macroSubstitutions;      // Local copy of macro substitutions (converted to a single string) Still valid after the profile has been released by releaseProfile()

    unsigned int messageFormId;      // Local copy of current form ID. Used to group forms with their widgets for messaging

    static QEEnvironmentShare share;

    friend class QEProfileUserLevelSlot;
};


//------------------------------------------------------------------------------
// Just by declaring an object of this class, the following is performed during construction
//
//    owner->publishOwnProfile ()                  - called if a profile not defined.
//    owner->addMacroSubstitutions (...)           - extracted from the specified owner
//    owner->addPriorityMacroSubstitutions (...)   - using value supplied to the constructor.
//
// And the following is performed during destruction
//
//    owner->removePriorityMacroSubstitutions()
//    owner->removeMacroSubstitutions ()
//    owner->releaseProfile ()                     - called if required
//
// Use as:
//   {
//      ProfilePublishiser p (qewidget, substitutions);
//      'stuff' requiring a profile.
//
//   }  p is destroyed when it goes out of scope.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT ProfilePublisher {
public:
    explicit ProfilePublisher( QEWidget* owner,
                               const QString& prioritySubstitutions );
    ~ProfilePublisher();
private:
    QEWidget* owner;
    bool localProfileWasPublished;
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (userLevelTypes::userLevels)
#endif

#endif // QE_CONTAINER_PROFILE_H
