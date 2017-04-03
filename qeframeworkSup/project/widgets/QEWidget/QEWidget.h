/*  QEWidget.h
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
 *  Copyright (c) 2009, 2010, 2016 Australian Synchrotron
 *
 *  Author:
 *    Anthony Owen
 *  Contact details:
 *    anthony.owen@gmail.com
 */

#ifndef QEWIDGET_H
#define QEWIDGET_H

#include <QList>
#include <QObject>
#include <VariableManager.h>
#include <ContainerProfile.h>
#include <QEEmitter.h>
#include <QEToolTip.h>
#include <QEPluginLibrary_global.h>
#include <QEDragDrop.h>
#include <styleManager.h>
#include <contextMenu.h>
#include <standardProperties.h>

/// Class to pass summary information about a QCaObject
class QCaInfo
{
public:
    enum ACCESS_MODES {NO_ACCESS, READ_ONLY, READ_WRITE, UNKNOWN };

    // The constructor bundles up a set of CA related information, ensuring every item is set
    QCaInfo(
            QString variableIn,
            QString typeIn,
            QString valueIn,
            QString severityIn,
            QString statusIn,
            QString hostIn,
            int precisionIn,
            int precisionUserIn,
            double alarmUserMinIn,
            double alarmUserMaxIn,
            double controlLimitLowerIn,
            double controlLimitUpperIn,
            double alarmLimitLowerIn,
            double alarmLimitUpperIn,
            double warningLimitLowerIn,
            double warningLimitUpperIn,
            double driveLimitLowIn,
            double driveLimitHighIn,
            bool   alarmSensitiveIn,
            ACCESS_MODES accessModeIn )
    {
         variable          = variableIn;
         type              = typeIn;
         value             = valueIn;
         severity          = severityIn;
         status            = statusIn;
         host              = hostIn;
         precision         = precisionIn;
         precisionUser     = precisionUserIn;
         alarmUserMin      = alarmUserMinIn;
         alarmUserMax      = alarmUserMaxIn;
         controlLimitLower = controlLimitLowerIn;
         controlLimitUpper = controlLimitUpperIn;
         alarmLimitLower   = alarmLimitLowerIn;
         alarmLimitUpper   = alarmLimitUpperIn;
         warningLimitLower = warningLimitLowerIn;
         warningLimitUpper = warningLimitUpperIn;
         driveLimitLow     = driveLimitLowIn;
         driveLimitHigh    = driveLimitHighIn;
         alarmSensitive    = alarmSensitiveIn;
         accessMode        = accessModeIn;
    }

    // CA informational items
    QString variable;
    QString type;
    QString value;
    QString severity;
    QString status;
    QString host;
    int precision;
    int precisionUser;
    double alarmUserMin;
    double alarmUserMax;
    double controlLimitLower;   // CA lopr
    double controlLimitUpper;   // CA hopr
    double alarmLimitLower;     // CA lolo
    double alarmLimitUpper;     // CA hihi
    double warningLimitLower;   // CA low
    double warningLimitUpper;   // CA high
    double driveLimitLow;       // CA drvl
    double driveLimitHigh;      // CA drvh
    bool   alarmSensitive;
    ACCESS_MODES accessMode;
};

// Forward declaration
class QEWidget;

// Class used to recieve save and restore signals from persistance manager,
// and emit and receive signals for customisation actions.
// An instance of this class is used by each QEWidget class.
// The QEWidget class can't recieve signals directly as it
// is not based on QObject and can't be as it is a base class for
// widgets that may also be based on Qwidgets and only one base
// class can be based on a QObject.
//
class signalSlotHandler: public QObject
{
    Q_OBJECT

public:
    // Constructor, destructor
    // Default to no owner
    signalSlotHandler();
    ~signalSlotHandler();

    // Set the QEWidget class that this instance is a part of
    void setOwner( QEWidget* ownerIn );     // Set the owner of this class which will be called when a signal is received

public slots:
    void saveRestore( SaveRestoreSignal::saveRestoreOptions option );

// none yet  private slots:

// none yet  signals:

private:
    QEWidget* owner;                                // QEWidget class that this instance is a part of
};

/**
  This class is used as a base for all CA aware wigets, such as QELabel, QESpinBox, etc.
  It manages common issues including creating a source of CA data updates, handling error,
  warning and status messages, and setting tool tips based on variable names.

  Note, there is tight integration between the CA aware widget classes, this class, and its
  base classes, especially VariableManager, VariableNameManager and QEToolTip.

  In particular, the VariableManager class manages QCaObject classes that stream updates to the
  CA aware widgets. But the VariableManager class, however, doesn't know how to format the data,
  or how the updates will be used.
  To resolve this, the VariableManager class asks a parent class (such as QELabel) to create the
  QCaObject class in what ever flavour it wants, by calling the virtual function createQcaItem.
  A QELabel, for example, wants string updates so it creates a QEString which is based on a
  QCaObject class and formats all updates as strings.

  The CA aware parent class (such as QELabel) defines a variable by calling VariableNameManager::setVariableName().
  The VariableNamePropertyManager class calls the establishConnection function of the CA aware parent class, such as QELabel
  when it has a new variable name.

  This class uses its base QEToolTip class to format tool tips. that class in turn calls the CA aware parent class
  (such as QELabel) directly to make use of a new tool tip.


  After construction, a CA aware widget is activated (starts updating) by VariableManager calling its
  establishConnection() function in one of two ways:

   1) The variable name or variable name substitutions is changed by calling setVariableName
      or setVariableNameSubstitutions respectively. These functions are in the VariableNameManager class.
      The VariableNamePropertyManager calls a virtual function establishConnection() which is implemented by the CA aware widget.
      This is how a CA aware widget is activated in 'designer'. It occurs when 'designer' updates the
      variable name property or variable name substitution property.

   2) When an QEForm widget is created, resulting in a set of CA aware widgets being created by loading a UI file
      contining plugin definitions.
      After loading the plugin widgets, code in the QEForm class calls the activate() function in the VariableManager base of
      this class (QEWiget).
      The activate() function calls  establishConnection() in the CA aware widget for each variable. This simulates
      what the VariableNamePropertyManager does as each variable name is entered (see 1, above, for details)

  No matter which way a CA aware widget is activated, the establishConnection() function in the CA aware widget is called
  for each variable. The establishConnection() function asks this QEWidget base class, by calling the createConnection()
  function, to perform the tasks common to all CA aware widgets for establishing a stream of CA data.

  The createConnection() function sets up the widget 'tool tip', then immedietly calls (through VariableManager::createVariable)
  the CA aware widget back asking it to create an object based on QCaObject. This object will supply a stream of CA update
  signals to the CA aware object in a form that it needs. For example a QELabel creates a QEString object. The QEString class
  is based on the QCaObject class and converts all update data to a strings which is required for updating a Qt label widget.
  This class stores the QCaObject based class.

  After the establishConnection() function in the CA aware widget has called createConnection(), the remaining task of the
  establishConnection() function is to connect the signals of the newly created QCaObject based classes to its own slots
  so that data updates can be used. For example, a QELabel connects the 'stringChanged' signal
  from the QEString object to its setLabelText slot.

 */

class QEPLUGINLIBRARYSHARED_EXPORT  QEWidget :
                 public VariableManager,
                 public QEToolTip,
                 public ContainerProfile,
                 public QEDragDrop,
                 public styleManager,
                 public UserMessage,
                 public contextMenu,
                 public standardProperties,
                 public QEEmitter
{
public:
    /// Restore phases. When a widget's persistant data is restored, the restore occurs in two phases.
    enum restorePhases { APPLICATION = SaveRestoreSignal::RESTORE_APPLICATION,
                         FRAMEWORK   = SaveRestoreSignal::RESTORE_QEFRAMEWORK };

    /// Defines a list or set of variable indices. Used to indicate those variables
    /// that are considered control variables, and thus have their cursor updated
    /// to the forbidden cursor if/when write access is denied.
    typedef QList <unsigned int> ControlVariableIndicesSet;


    /// Constructor
    QEWidget( QWidget* ownerIn );

    /// Destructor
    virtual ~QEWidget();

    /// Get the message source ID.
    /// The message source ID is used as part of the system where QE widgets can emit
    /// a message and have the right QE widget in the right form catch the message.
    /// Refer to the UserMessage class for further details.
    unsigned int getMessageSourceId(){ return getSourceId(); }

    /// Set the message source ID.
    /// The message source ID is used as part of the system where QE widgets can emit
    /// a message and have the right QE widget in the right form catch the message.
    /// Refer to the UserMessage class for further details.
    void setMessageSourceId( unsigned int messageSourceId ){ setSourceId( messageSourceId ); }

    /// Static method to return a colour to update the widget's look to reflect the current alarm state
    /// Note, the color is determined by the alarmInfo class, but since that class is used in non
    /// gui applications, it can't return a QColor
    static QColor getColor( QCaAlarmInfo& alarmInfo, const int saturation );

    /// This convenience function updates the alarm tool tip, and alarm status style if the displayAlarmState
    /// property is set to true - assumes the widget uses standard properties. This function is perhaps
    /// most usefull for single-variable widgets.
    void processAlarmInfo( QCaAlarmInfo& alarmInfo, const unsigned int variableIndex = 0 );

    /// Virtual function that may be implimented by users of QEWidget to update variable names and macro substitutions.
    /// A default is provided that is suitible in most cases.
    virtual void setVariableNameAndSubstitutions( QString variableNameIn, QString variableNameSubstitutionsIn, unsigned int variableIndex ); // Generally don't need to override

    /// Looks for a file in a standard set of locations (and opens the file)
    ///
    QFile* openQEFile( QString name, QFile::OpenModeFlag mode );

    /// Static method that looks for a file in a standard set of locations
    /// Returns a pointer to a QFile which is the caller's responsibility to delete, or NULL if the file was not found.
    static QFile* findQEFile( QString name, ContainerProfile* profile );

    /// Static method that looks for a file in a standard set of locations and assumes a current published profile.
    /// Returns a pointer to a QFile which is the caller's responsibility to delete, or NULL if the file was not found.
    static QFile* findQEFile( QString name );

    /// Returns the default location to create files.
    /// Use this to create files in a consistant location
    QString defaultFileLocation();

    /// Returns the QE framework that built this instance of the widget.
    /// On windows, the QEFramework DLL may be loaded twice with potentially different versions of it.
    QString getFrameworkVersion();

    /// Service a request to save the QE widget's current configuration.
    /// A widget may save any configuration details through the PersistanceManager.
    /// For example, a QEStripChart may save the variables being plotted.
    /// Many QE widgets do not have any persistant data requirements and do not implement this method.
    virtual void saveConfiguration( PersistanceManager* ){}

    /// Service a request to restore the QE widget's configuration.
    /// A QE widget recover any configuration details from the PersistanceManager.
    /// For example, a QEStripChart may restore the variables being plotted.
    /// Many QE widgets do not have any persistant data requirements and do not implement this method.
    /// This is called twice with an incrementing restorePhase. Most widgets will miss the first call
    /// as they don't exist yet (they are created as part of the first phase)
    virtual void restoreConfiguration( PersistanceManager*, restorePhases ){}

    /// Any QEWidget that requires additional scaling, i.e. above and beyond the standard scaling
    /// applied to size, minimum size, maximum size and font size, may override this function in
    /// order to perform any bespoke scaling need by the widget (for example see QEShape).
    /// The scaling is defined using a rational number specifed by two integers (m, d).
    /// The first (m) parameter is the multiplier and the second (d) parameter is the divisor.
    /// For example, if m = 4 and d = 5, then an 80% scaling should be applied.
    /// And if m = 5 and d = 4, and a 125% scaling is required.
    virtual void scaleBy( const int, const int ) {}

    /// Get the QWidget that the parent of this QEWidget instance is based on.
    /// For example, the parent of a QEWidget might be a QELabel, which is based on QLabel which is based on QWidget.
    QWidget* getQWidget();

    /// Find a QE widget and request an action.
    /// The widget hierarchy under a supplied widget is searched for a QE widget with a given name.
    /// If found the QE widget will attecjpt to carry out the requested action which consists of an action string and an argument list.
    /// This method allows an application to initiate QE widget activity. The QEGui application uses this mechanism when providing custom menus defined in XML files.
    /// The method returns true if the named widget was found. (The action was not nessesarily performed, or even recognised by the widget)
    static void doAction( QWidget* searchPoint, QString widgetName, QString action, QStringList arguments, bool initialise, QAction* originator );

    /// Return information about the data sources for this widget
    ///
    const QList<QCaInfo> getQCaInfo();

protected:
    qcaobject::QCaObject* createConnection( unsigned int variableIndex ); ///< Create a CA connection. Use default subscribe option. Return a QCaObject if successfull
    qcaobject::QCaObject* createConnection( unsigned int variableIndex,
                                            const bool do_subscribe );    ///< Create a CA connection. Use specified subscribe option. Return a QCaObject if successfull

    QString persistantName( QString prefix ) const;                       ///< Returns a string that will not change between runs of the application (given the same configuration)

    virtual void actionRequest( QString, QStringList, bool, QAction* ){}  ///< Perform a named action

    // Default drag/drop actions.
    void setDrop (QVariant drop) { if( getAllowDrop() ){ paste (drop); } }                           ///< Default get drop action
    QVariant getDrop () { return isDraggingVariable () ? QVariant( copyVariable() ) : copyData(); }  ///< Default set drop action

    void setControlPV (const unsigned int variableIndex);                     /// Nominate a single variable index as the control variable index.
    void setControlPVs (const ControlVariableIndicesSet& variableIndexList);  /// Nominate a variable number of variable indicis as the control variable indies list.
    ControlVariableIndicesSet getControlPVs () const;

    void setAccessCursorStyle ();                                         /// Update cursor style if all control variable indices are write inhibited.
    void startGui( const QEActionRequests & request );                    /// Support function for slot for launching a new gui. Used by QE buttons and QEForm as the default action for launching a gui.

private:
    void userLevelChangedGeneral( userLevelTypes::userLevels level );     // Manage general aspects of user level change, then call optional QE widget specific virtual functions
    virtual void userLevelChanged( userLevelTypes::userLevels ){}         // Virtual function implemented by QE widgets when the user level changes

    void setToolTipFromVariableNames();                                   // Update the variable name list used in tool tips if requried

    signalSlotHandler signalSlot;                                         // QObject based class a save/restore signal can be delivered to

    void buildPersistantName( QWidget* w, QString& name ) const;          // make a function??

    QCAALARMINFO_SEVERITY lastSeverity;                                  // Used as low pass tool tip filter.
    standardProperties::displayAlarmStateOptions lastDisplayAlarmState;  // Last alarm state. Kept to identify when the alarm state (included in a data update signal) changes

    static void addPathToSearchList( QString path, QString name, QStringList& searchList ); // Add a path and filename to a search list. (add sub dirs if path ends with '...')

    QWidget* owner;                                                      // QE Widget using this base class. for example, QELabel

    //
    ControlVariableIndicesSet controlVariableIndices;
    bool isWriteAllowed;
    QCursor savedAllowedCursor;

public:
    static bool inDesigner();                               /// Flag indicating this widget is running inside Qt's 'designer'
    virtual QMenu* getDefaultContextMenu(){ return NULL; }  /// Return the Qt default context menu to add to the QE context menu
    virtual int getUserPrecision(){ return 0; }             /// Return the widgets precision property if any (otherwise zero)
    virtual int getUserAlarmMin(){ return 0; }              /// Return the widget alarm minimum if any (otherwise zero)
    virtual int getUserAlarmMax(){ return 0; }              /// Return the widget alarm maximum if any (otherwise zero)
    virtual bool getAlarmSensitive(){ return false; }       /// Return the widget's alarm sensitivity (default to not sensitive)
};

#endif // QEWIDGET_H
