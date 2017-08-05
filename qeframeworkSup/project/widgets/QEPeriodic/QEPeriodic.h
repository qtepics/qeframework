/*  QEPeriodic.h
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
 *  Copyright (c) 2011 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef QE_PERIODIC_H
#define QE_PERIODIC_H

#include <QFrame>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QEWidget.h>
#include <QEFloating.h>
#include <QEFloatingFormatting.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEFrameworkLibraryGlobal.h>

#define NUM_ELEMENTS 118

class QEPeriodicComponentData
{
public:

    QEPeriodicComponentData()
    {
        variableIndex1 = 0;
        lastData1 = 0.0;
        haveLastData1 = false;

        variableIndex2 = 0;
        lastData2 = 0.0;
        haveLastData2 = false;
    }

    unsigned int variableIndex1;
    double lastData1;
    bool haveLastData1;

    unsigned int variableIndex2;
    double lastData2;
    bool haveLastData2;
};

// Dynamic element information structure
class userInfoStruct
{
public:
    userInfoStruct() { enable = false; value1 = 0.0; value2 = 0.0; }
    bool    enable;         // True if element is available for the user to select
    double  value1;         // User value to be written to and compared against the first variable
    double  value2;         // User value to be written to and compared against the second variable
    QString elementText;    // User text associated with element (emitted on element change)
};

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPeriodic : public QFrame, public QEWidget {
    Q_OBJECT

  public:
    // Static element information structure
    struct elementInfoStruct
    {
        unsigned int number;            // Atomic number:     1
        double       atomicWeight;      // Atomic weight:     1.0079
        QString      name;              // Element name:      Hydrogen
        QString      symbol;            // Element symbol:    H
        double       meltingPoint;      // Melting point:     -259 deg C
        double       boilingPoint;      // Boiling point:     -253 deg C
        double       density;           // Density:           0.09
        unsigned int group;             // Periodic group:    1
        double       ionizationEnergy;  // Ionization energy: 13.5984 eV
        unsigned int tableRow;          // Index into table row representing periodic table (related to user interface, not chemistry)
        unsigned int tableCol;          // Index into table column representing periodic table (related to user interface, not chemistry)
    };

    QEPeriodic( QWidget *parent = 0 );
    QEPeriodic( const QString& variableName, QWidget *parent = 0 );

    // Array of dynamic element information structure
    struct userInfoStructArray
    {
        userInfoStruct array[NUM_ELEMENTS];
    };

    static elementInfoStruct elementInfo[NUM_ELEMENTS];      // Array of static element information
    userInfoStruct userInfo[NUM_ELEMENTS];                   // Array of dynamic element information

    // Element information options
    enum variableTypes { VARIABLE_TYPE_NUMBER,
                         VARIABLE_TYPE_ATOMIC_WEIGHT,
                         VARIABLE_TYPE_MELTING_POINT,
                         VARIABLE_TYPE_BOILING_POINT,
                         VARIABLE_TYPE_DENSITY,
                         VARIABLE_TYPE_GROUP,
                         VARIABLE_TYPE_IONIZATION_ENERGY,
                         VARIABLE_TYPE_USER_VALUE_1,
                         VARIABLE_TYPE_USER_VALUE_2 };

    // Property convenience functions

    // subscribe
    void setSubscribe( bool subscribe );
    bool getSubscribe();

    // presentation options
    enum presentationOptions { PRESENTATION_BUTTON_AND_LABEL,
                               PRESENTATION_BUTTON_ONLY,
                               PRESENTATION_LABEL_ONLY };
    void setPresentationOption( presentationOptions presentationOptionIn );
    presentationOptions getPresentationOption();

    // variable 1 type
    void setVariableType1( variableTypes variableType1In );
    variableTypes getVariableType1();

    // variable 2 type
    void setVariableType2( variableTypes variableType2In );
    variableTypes getVariableType2();


    // variable 1 tolerance
    void setVariableTolerance1( double variableTolerance1In );
    double getVariableTolerance1();

    // variable 2 tolerance
    void setVariableTolerance2( double variableTolerance2In );
    double getVariableTolerance2();

    // user info
    // Used regardless of the user info source (text property or a file)
    // This is the entire set of user information as an XML string. Not an individual element's user information
    void setUserInfo( QString userInfo );
    QString getUserInfo();

    // User info text.
    // Used to manage the user info text property
    // This is the entire set of user information as an XML string. Not an individual element's user information
    void setUserInfoText( QString userInfo );
    QString getUserInfoText();

    // user info filename
    // This is name of a file containing the entire set of user information as an XML string.
    void setUserInfoFile( QString userInfoFileIn );
    QString getUserInfoFile();

    // user info source options
    enum userInfoSourceOptions { USER_INFO_SOURCE_TEXT,
                                 USER_INFO_SOURCE_FILE, };
    void setUserInfoSourceOption( userInfoSourceOptions userInfoSourceOptionIn );
    userInfoSourceOptions getUserInfoSourceOption();

    // The user info has changed (from the user info setup dialog), so update the current user info source
    void updateUserInfoSource();

    // Get user values for an element
    bool getElementValues( QString symbol, double* value1, double* value2 );

    // Get user selected symbol for element
    QString getSelectedSymbol(){ return selectedSymbol; }

  private slots:
    void connectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex );
    void setElement( const double& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& );
    void userClicked();

public slots:
    void setElement( const QString symbol );

  signals:
    /// Sent when the element is changed by the user selecting an element
    void userElementChanged( const QString& symbol );
    // Note, the following signals are common to many QE widgets,
    // if changing the doxygen comments, ensure relevent changes are migrated to all instances
    /// Sent when the widget is updated following a data change
    /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
    /// For example a QList widget could log updates from this widget.
    void dbValueChanged( const double& out );
    /// Sent when the widget is updated following a data change
    /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
    /// For example a QList widget could log updates from this widget.
    void dbElementChanged( const QString& out );
    /// Internal use only. Used when changing a property value to force a re-display to reflect the new property value.
    void requestResend();

  protected:
    QEFloatingFormatting floatingFormatting;
    bool localEnabled;
    bool allowDrop;

    variableTypes variableType1;
    variableTypes variableType2;

    double variableTolerance1;
    double variableTolerance2;


    void establishConnection( unsigned int variableIndex );

  private:
    void setup();
    qcaobject::QCaObject* createQcaItem( unsigned int variableIndex  );

    void writeUserInfoFile();
    void readUserInfoFile();


    bool isConnected;

    QEPeriodicComponentData writeButtonData;
    QEPeriodicComponentData readbackLabelData;
    QPushButton* writeButton;
    QLabel* readbackLabel;
    QHBoxLayout *layout;
    QString selectedSymbol;

    bool getElementTextForValue( const double& value, const unsigned int& variableIndex, QEPeriodicComponentData& componentData, const QString& currentText, QString& newText );

    presentationOptions presentationOption;
    void updatePresentationOptions();

    float elementMatch( int i, bool haveFirstVariable, double lastData1, bool haveSecondVariable, double lastData2 );

    QString userInfoText;
    QString userInfoFile;
    userInfoSourceOptions userInfoSourceOption;

    // Drag and Drop
protected:
    void dragEnterEvent(QDragEnterEvent *event) { qcaDragEnterEvent( event ); }
    void dropEvent(QDropEvent *event)           { qcaDropEvent( event ); }
    void mousePressEvent(QMouseEvent *event)    { qcaMousePressEvent( event ); }
    void setDrop( QVariant drop );
    QVariant getDrop();

    // Copy paste
    QString copyVariable();
    QVariant copyData();
    void paste( QVariant s );

    enum variableIndexes{ WRITE_VARIABLE_1,
                          WRITE_VARIABLE_2,
                          READ_VARIABLE_1,
                          READ_VARIABLE_2,
                          QEPERIODIC_NUM_VARIABLES /*Must be last*/ };

    //=================================================================================
    // Multiple Variable properties
    // These properties should be similar for every widget using multiple variables (The number of variables may vary).
    // WHEN MAKING CHANGES: search for MULTIPLEVARIABLEPROPERTIESBASE and change all occurances.
    private:
        QCaVariableNamePropertyManager variableNamePropertyManagers[QEPERIODIC_NUM_VARIABLES];
    public:

    // Define a variable
    // Note, the QPROPERTY declaration itself can't be in this macro
#define VARIABLE_PROPERTY_ACCESS(VAR_INDEX) \
    void    setVariableName##VAR_INDEX##Property( QString variableName ){ variableNamePropertyManagers[VAR_INDEX].setVariableNameProperty( variableName ); } \
    QString getVariableName##VAR_INDEX##Property(){ return variableNamePropertyManagers[VAR_INDEX].getVariableNameProperty(); }

    VARIABLE_PROPERTY_ACCESS(0)
    /// EPICS variable name (CA PV).
    /// This variable is used to write a value to the first of two positioners that will position the select element.
    Q_PROPERTY(QString writeButtonVariable1 READ getVariableName0Property WRITE setVariableName0Property)

    VARIABLE_PROPERTY_ACCESS(1)
    /// EPICS variable name (CA PV).
    /// This variable is used to write a value to the second of two positioners that will position the select element.
    Q_PROPERTY(QString writeButtonVariable2 READ getVariableName1Property WRITE setVariableName1Property)

    VARIABLE_PROPERTY_ACCESS(2)
    /// EPICS variable name (CA PV).
    /// This variable is used to read the value to the first of two positioners to determine which (if any) element is currently selected.
    Q_PROPERTY(QString readbackLabelVariable1 READ getVariableName2Property WRITE setVariableName2Property)

    VARIABLE_PROPERTY_ACCESS(3)
    /// EPICS variable name (CA PV).
    /// This variable is used to read the value to the second of two positioners to determine which (if any) element is currently selected.
    Q_PROPERTY(QString readbackLabelVariable2 READ getVariableName3Property WRITE setVariableName3Property)

#undef VARIABLE_PROPERTY_ACCESS

    /// Macro substitutions. The default is no substitutions. The format is NAME1=VALUE1[,] NAME2=VALUE2... Values may be quoted strings. For example, 'SAMPLE=SAM1, NAME = "Ref foil"'
    /// These substitutions are applied to all the variable names.
    Q_PROPERTY(QString variableSubstitutions READ getVariableNameSubstitutionsProperty WRITE setVariableNameSubstitutionsProperty)

    /// Property access function for #variableSubstitutions property. This has special behaviour to work well within designer.
    void    setVariableNameSubstitutionsProperty( QString variableNameSubstitutions )
    {
        for( int i = 0; i < QEPERIODIC_NUM_VARIABLES; i++ )
        {
            variableNamePropertyManagers[i].setSubstitutionsProperty( variableNameSubstitutions );
        }
    }
    /// Property access function for #variableSubstitutions property. This has special behaviour to work well within designer.
    QString getVariableNameSubstitutionsProperty()
    {
        return variableNamePropertyManagers[0].getSubstitutionsProperty();
    }
public:

private slots:
  void useNewVariableNameProperty( QString variableNameIn, QString variableNameSubstitutionsIn, unsigned int variableIndex )
  {
      setVariableNameAndSubstitutions(variableNameIn, variableNameSubstitutionsIn, variableIndex);
  }

public:
    //=================================================================================

  //=================================================================================
  // Control widget properties
  // These properties should be similar for all widgets with control functionality (writing to variables, not just displaying).
  // WHEN MAKING CHANGES: search for CONTROLVARIABLEPROPERTIES and change all relevent occurances.
public:
  /// Sets if this widget subscribes for data updates and displays current data.
  /// Default is 'true' (subscribes for and displays data updates)
  Q_PROPERTY(bool subscribe READ getSubscribe WRITE setSubscribe)
public:
  //=================================================================================

    //=================================================================================
    // Standard properties
    // These properties should be identical for every widget using them.
    // WHEN MAKING CHANGES: search for STANDARDPROPERTIES and change all occurances.
public:
    /// Use the variable as the tool tip. Default is true. Tool tip property will be overwritten by the variable name.
    ///
    Q_PROPERTY(bool variableAsToolTip READ getVariableAsToolTip WRITE setVariableAsToolTip)

    /// Allow drag/drops operations to this widget. Default is false. Any dropped text will be used as a new variable name.
    ///
    Q_PROPERTY(bool allowDrop READ getAllowDrop WRITE setAllowDrop)

    /// Display the widget. Default is true.
    /// Setting this property false is usefull if widget is only used to provide a signal - for example, when supplying data to a QELink widget.
    /// Note, when false the widget will still be visible in Qt Designer.
    Q_PROPERTY(bool visible READ getRunVisible WRITE setRunVisible)

    /// Set the ID used by the message filtering system. Default is zero.
    /// Widgets or applications that use messages from the framework have the option of filtering on this ID.
    /// For example, by using a unique message source ID a QELog widget may be set up to only log messages from a select set of widgets.
    Q_PROPERTY(unsigned int messageSourceId READ getMessageSourceId WRITE setMessageSourceId )

    /// Style Sheet string to be applied when the widget is displayed in 'User' mode. Default is an empty string.
    /// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
    /// This Style Sheet string will be applied by the styleManager class.
    /// Refer to the styleManager class for details about how this Style Sheet string will be merged with any pre-existing Style Sheet string
    /// and any Style Sheet strings generated during the display of data.
    Q_PROPERTY(QString userLevelUserStyle READ getStyleUser WRITE setStyleUser)

    /// Style Sheet string to be applied when the widget is displayed in 'Scientist' mode. Default is an empty string.
    /// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
    /// This Style Sheet string will be applied by the styleManager class.
    /// Refer to the styleManager class for details about how this Style Sheet string will be merged with any pre-existing Style Sheet string
    /// and any Style Sheet strings generated during the display of data.
    Q_PROPERTY(QString userLevelScientistStyle READ getStyleScientist WRITE setStyleScientist)

    /// Style Sheet string to be applied when the widget is displayed in 'Engineer' mode. Default is an empty string.
    /// The syntax is the standard Qt Style Sheet syntax. For example, 'background-color: red'
    /// This Style Sheet string will be applied by the styleManager class.
    /// Refer to the styleManager class for details about how this Style Sheet string will be merged with any pre-existing Style Sheet string
    /// and any Style Sheet strings generated during the display of data.
    Q_PROPERTY(QString userLevelEngineerStyle READ getStyleEngineer WRITE setStyleEngineer)

    /// \enum UserLevels
    /// User friendly enumerations for #userLevelVisibility and #userLevelEnabled properties - refer to #userLevelVisibility and #userLevelEnabled properties and userLevel enumeration for details.
    enum UserLevels { User      = userLevelTypes::USERLEVEL_USER,          ///< Refer to USERLEVEL_USER for details
                      Scientist = userLevelTypes::USERLEVEL_SCIENTIST,     ///< Refer to USERLEVEL_SCIENTIST for details
                      Engineer  = userLevelTypes::USERLEVEL_ENGINEER       ///< Refer to USERLEVEL_ENGINEER for details
                              };
    Q_ENUMS(UserLevels)

    /// Lowest user level at which the widget is visible. Default is 'User'.
    /// Used when designing GUIs that display more and more detail according to the user mode.
    /// The user mode is set application wide through the QELogin widget, or programatically through setUserLevel()
    /// Widgets that are always visible should be visible at 'User'.
    /// Widgets that are only used by scientists managing the facility should be visible at 'Scientist'.
    /// Widgets that are only used by engineers maintaining the facility should be visible at 'Engineer'.
    Q_PROPERTY(UserLevels userLevelVisibility READ getUserLevelVisibilityProperty WRITE setUserLevelVisibilityProperty)

    /// Lowest user level at which the widget is enabled. Default is 'User'.
    /// Used when designing GUIs that allow access to more and more detail according to the user mode.
    /// The user mode is set application wide through the QELogin widget, or programatically through setUserLevel()
    /// Widgets that are always accessable should be visible at 'User'.
    /// Widgets that are only accessable to scientists managing the facility should be visible at 'Scientist'.
    /// Widgets that are only accessable to engineers maintaining the facility should be visible at 'Engineer'.
    Q_PROPERTY(UserLevels userLevelEnabled READ getUserLevelEnabledProperty WRITE setUserLevelEnabledProperty)

    UserLevels getUserLevelVisibilityProperty() { return (UserLevels)getUserLevelVisibility(); }            ///< Access function for #userLevelVisibility property - refer to #userLevelVisibility property for details
    void setUserLevelVisibilityProperty( UserLevels level ) { setUserLevelVisibility( (userLevelTypes::userLevels)level ); }///< Access function for #userLevelVisibility property - refer to #userLevelVisibility property for details
    UserLevels getUserLevelEnabledProperty() { return (UserLevels)getUserLevelEnabled(); }                  ///< Access function for #userLevelEnabled property - refer to #userLevelEnabled property for details
    void setUserLevelEnabledProperty( UserLevels level ) { setUserLevelEnabled( (userLevelTypes::userLevels)level ); }      ///< Access function for #userLevelEnabled property - refer to #userLevelEnabled property for details

    /// DEPRECATED. USE displayAlarmStateOption INSTEAD.
    /// If set (default) widget will indicate the alarm state of any variable data it is displaying.
    /// If clear widget will never indicate the alarm state of any variable data it is displaying.
    /// Typically the background colour is set to indicate the alarm state.
    /// Note, this property is included in the set of standard properties as it applies to most widgets. It
    /// will do nothing for widgets that don't display data.
    Q_PROPERTY(bool displayAlarmState READ getDisplayAlarmState WRITE setDisplayAlarmState DESIGNABLE false)

    /// \enum DisplayAlarmStateOptions
    /// User friendly enumerations for #displayAlarmStateOption property - refer to #displayAlarmStateOption property and displayAlarmStateOptions enumeration for details.
    enum DisplayAlarmStateOptions { Never       = standardProperties::DISPLAY_ALARM_STATE_NEVER,          ///< Refer to DISPLAY_ALARM_STATE_NEVER for details
                                    Always      = standardProperties::DISPLAY_ALARM_STATE_ALWAYS,         ///< Refer to DISPLAY_ALARM_STATE_ALWAYS for details
                                    WhenInAlarm = standardProperties::DISPLAY_ALARM_STATE_WHEN_IN_ALARM   ///< Refer to DISPLAY_ALARM_STATE_WHEN_IN_ALARM for details
                              };
    Q_ENUMS(DisplayAlarmStateOptions)
    /// If 'Always' (default) widget will indicate the alarm state of any variable data it is displaying, including 'No Alarm'
    /// If 'Never' widget will never indicate the alarm state of any variable data it is displaying.
    /// If 'WhenInAlarm' widget only indicate the alarm state of any variable data it is displaying if it is 'in alarm'.
    /// Typically the background colour is set to indicate the alarm state.
    /// Note, this property is included in the set of standard properties as it applies to most widgets. It
    /// will do nothing for widgets that don't display data.
    Q_PROPERTY(DisplayAlarmStateOptions displayAlarmStateOption READ getDisplayAlarmStateOptionProperty WRITE setDisplayAlarmStateOptionProperty)

    DisplayAlarmStateOptions getDisplayAlarmStateOptionProperty() { return (DisplayAlarmStateOptions)getDisplayAlarmStateOption(); }            ///< Access function for #displayAlarmStateOption property - refer to #displayAlarmStateOption property for details
    void setDisplayAlarmStateOptionProperty( DisplayAlarmStateOptions option ) { setDisplayAlarmStateOption( (displayAlarmStateOptions)option ); }///< Access function for #displayAlarmStateOption property - refer to #displayAlarmStateOption property for details

public:
   // END-STANDARD-PROPERTIES ========================================================


    // Widget specific properties

    Q_ENUMS(PresentationOptions)
    Q_PROPERTY(PresentationOptions presentationOption READ getPresentationOptionProperty WRITE setPresentationOptionProperty)
    enum PresentationOptions { buttonAndLabel = QEPeriodic::PRESENTATION_BUTTON_AND_LABEL,
                               buttonOnly     = QEPeriodic::PRESENTATION_BUTTON_ONLY,
                               labelOnly      = QEPeriodic::PRESENTATION_LABEL_ONLY };
    void setPresentationOptionProperty( PresentationOptions presentationOption ){ setPresentationOption( (QEPeriodic::presentationOptions)presentationOption ); }
    PresentationOptions getPresentationOptionProperty(){ return (PresentationOptions)getPresentationOption(); }



    Q_ENUMS(VariableTypes)
    Q_PROPERTY(VariableTypes variableType1 READ getVariableType1Property WRITE setVariableType1Property)
    Q_PROPERTY(VariableTypes variableType2 READ getVariableType2Property WRITE setVariableType2Property)
    enum VariableTypes { Number           = QEPeriodic::VARIABLE_TYPE_NUMBER,
                         atomicWeight     = QEPeriodic::VARIABLE_TYPE_ATOMIC_WEIGHT,
                         meltingPoint     = QEPeriodic::VARIABLE_TYPE_MELTING_POINT,
                         boilingPoint     = QEPeriodic::VARIABLE_TYPE_BOILING_POINT,
                         density          = QEPeriodic::VARIABLE_TYPE_DENSITY,
                         group            = QEPeriodic::VARIABLE_TYPE_GROUP,
                         ionizationEnergy = QEPeriodic::VARIABLE_TYPE_IONIZATION_ENERGY,
                         userValue1       = QEPeriodic::VARIABLE_TYPE_USER_VALUE_1,
                         userValue2       = QEPeriodic::VARIABLE_TYPE_USER_VALUE_2 };
    void setVariableType1Property( VariableTypes variableType ){ setVariableType1( (QEPeriodic::variableTypes)variableType ); }
    void setVariableType2Property( VariableTypes variableType ){ setVariableType2( (QEPeriodic::variableTypes)variableType ); }
    VariableTypes getVariableType1Property(){ return (VariableTypes)getVariableType1(); }
    VariableTypes getVariableType2Property(){ return (VariableTypes)getVariableType2(); }


    Q_PROPERTY(double variableTolerance1 READ getVariableTolerance1 WRITE setVariableTolerance1)
    Q_PROPERTY(double variableTolerance2 READ getVariableTolerance2 WRITE setVariableTolerance2)

    // This property should be called userInfoText. It was named userInfo before there was a choise of user info sources (text property or file)
    Q_PROPERTY(QString userInfo READ getUserInfoText WRITE setUserInfoText)

    Q_PROPERTY(QString userInfoFile READ getUserInfoFile WRITE setUserInfoFile)

    Q_ENUMS(UserInfoSourceOptions)
    Q_PROPERTY(UserInfoSourceOptions userInfoSourceOption READ getUserInfoSourceOptionProperty WRITE setUserInfoSourceOptionProperty)
    enum UserInfoSourceOptions { userInfoSourceText = QEPeriodic::USER_INFO_SOURCE_TEXT,
                               userInfoSourceFile = QEPeriodic::USER_INFO_SOURCE_FILE };
    void setUserInfoSourceOptionProperty( UserInfoSourceOptions userInfoSourceOption ){ setUserInfoSourceOption( (QEPeriodic::userInfoSourceOptions)userInfoSourceOption ); }
    UserInfoSourceOptions getUserInfoSourceOptionProperty(){ return (UserInfoSourceOptions)getUserInfoSourceOption(); }
};

Q_DECLARE_METATYPE(QEPeriodic::userInfoStructArray)

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEPeriodic::UserLevels)
Q_DECLARE_METATYPE (QEPeriodic::DisplayAlarmStateOptions)
Q_DECLARE_METATYPE (QEPeriodic::PresentationOptions)
Q_DECLARE_METATYPE (QEPeriodic::VariableTypes)
Q_DECLARE_METATYPE (QEPeriodic::UserInfoSourceOptions)
#endif

#endif // QE_PERIODIC_H
