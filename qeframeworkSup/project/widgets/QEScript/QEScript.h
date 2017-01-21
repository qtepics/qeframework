/*  QEScript.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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
 *  Copyright (c) 2012 Australian Synchrotron
 *
 *  Author:
 *    Ricardo Fernandes
 *  Contact details:
 *    ricardo.fernandes@synchrotron.org.au
 */

#ifndef QE_SCRIPT_H
#define QE_SCRIPT_H

#include <QTableWidgetItem>
#include <QPushButton>
#include <QComboBox>
#include <QEWidget.h>


enum details
{
    TOP,
    BOTTOM,
    LEFT,
    RIGHT
};


enum script
{
    FROM_FILE,
    FROM_TEXT
};


// ============================================================
//  _QTABLEWIDGETSCRIPT CLASS
// ============================================================
class _QTableWidgetScript:public QTableWidget
{

    Q_OBJECT

    private:
        bool initialized;

    protected:

    public:
        _QTableWidgetScript(QWidget * pParent = 0);
        void refreshSize();
        void resizeEvent(QResizeEvent *);
        void resize(int w, int h);

    private slots:

};


// ============================================================
//  _QCOPYPASTE CLASS
// ============================================================
class _CopyPaste
{
    private:
        bool enable;
        QString program;
        QString parameters;
        QString workingDirectory;
        int timeOut;
        bool stop;
        bool log;

    public:
        _CopyPaste();

        _CopyPaste(bool pEnable, QString pProgram, QString pParameters, QString pWorkingDirectory, int pTimeOut, bool pStop, bool pLog);

        void setEnable(bool pEnable);
        bool getEnable();

        void setProgram(QString pProgram);
        QString getProgram();

        void setParameters(QString pParameters);
        QString getParameters();

        void setWorkingDirectory(QString pWorkingDirectory);
        QString getWorkingDirectory();

        void setTimeOut(int pTimeOut);
        int getTimeOut();

        void setStop(bool pStop);
        bool getStop();

        void setLog(bool pLog);
        bool getLog();
};


// ============================================================
//  QESCRIPT METHODS
// ============================================================
/*!
  This class is a EPICS aware widget.
  The QEScript widget allows the user to define a certain sequence of external programs to be executed. This sequence may be saved, modified or loaded for future usage.
*/
class QEPLUGINLIBRARYSHARED_EXPORT QEScript:public QWidget, public QEWidget
{

    Q_OBJECT

    private:

    protected:
        QComboBox *qComboBoxScriptList;
        QPushButton *qPushButtonNew;
        QPushButton *qPushButtonSave;
        QPushButton *qPushButtonDelete;
        QPushButton *qPushButtonExecute;
        QPushButton *qPushButtonAbort;
        QPushButton *qPushButtonAdd;
        QPushButton *qPushButtonRemove;
        QPushButton *qPushButtonUp;
        QPushButton *qPushButtonDown;
        QPushButton *qPushButtonCopy;
        QPushButton *qPushButtonPaste;
        _QTableWidgetScript *qTableWidgetScript;
        QString scriptFile;
        QString scriptText;
        QString scriptDefault;
        int scriptType;
        int optionsLayout;
        QDomDocument document;
        QString filename;
        QList<_CopyPaste *> copyPasteList;
        bool editableTable;
        bool isExecuting;

    public:
        QEScript(QWidget *pParent = 0);
        virtual ~QEScript(){}

        void setShowScriptList(bool pValue);
        bool getShowScriptList();

        void setShowNew(bool pValue);
        bool getShowNew();

        void setShowSave(bool pValue);
        bool getShowSave();

        void setShowDelete(bool pValue);
        bool getShowDelete();

        void setShowExecute(bool pValue);
        bool getShowExecute();

        void setShowAbort(bool pValue);
        bool getShowAbort();

        void setEditableTable(bool pValue);
        bool getEditableTable();

        void setShowTable(bool pValue);
        bool getShowTable();

        void setShowTableControl(bool pValue);
        bool getShowTableControl();

        void setShowColumnNumber(bool pValue);
        bool getShowColumnNumber();

        void setShowColumnEnable(bool pValue);
        bool getShowColumnEnable();

        void setShowColumnProgram(bool pValue);
        bool getShowColumnProgram();

        void setShowColumnParameters(bool pValue);
        bool getShowColumnParameters();

        void setShowColumnWorkingDirectory(bool pValue);
        bool getShowColumnWorkingDirectory();

        void setShowColumnTimeout(bool pValue);
        bool getShowColumnTimeout();

        void setShowColumnStop(bool pValue);
        bool getShowColumnStop();

        void setShowColumnLog(bool pValue);
        bool getShowColumnLog();

        void setScriptType(int pValue);
        int getScriptType();

        void setScriptFile(QString pValue);
        QString getScriptFile();

        void setScriptText(QString pValue);
        QString getScriptText();

        void setScriptDefault(QString pValue);
        QString getScriptDefault();

        void setExecuteText(QString pValue);
        QString getExecuteText();

        void setOptionsLayout(int pValue);
        int getOptionsLayout();

        void insertRow(bool pEnable, QString pProgram, QString pParameter, QString pWorkingDirectory, int pTimeOut, bool pStop, bool pLog);

        bool saveScriptList();

        void refreshScriptList();

        void refreshWidgets();

        /// Show/hide combobox that contains the list of existing scripts created by the user
        Q_PROPERTY(bool showScriptList READ getShowScriptList WRITE setShowScriptList)

        /// Show/hide button to reset (initialize) the table that contains the sequence of programs to be executed
        Q_PROPERTY(bool showNew READ getShowNew WRITE setShowNew)

        /// Show/hide button to save/overwrite a new/existing script
        Q_PROPERTY(bool showSave READ getShowSave WRITE setShowSave)

        /// Show/hide button to delete an existing script
        Q_PROPERTY(bool showDelete READ getShowDelete WRITE setShowDelete)

        /// Show/hide button to execute a sequence of programs
        Q_PROPERTY(bool showExecute READ getShowExecute WRITE setShowExecute)

        /// Show/hide button to abort the execution of a sequence of programs
        Q_PROPERTY(bool showAbort READ getShowAbort WRITE setShowAbort)

        /// Show/hide table that contains a sequence of programs to be executed
        Q_PROPERTY(bool showTable READ getShowTable WRITE setShowTable)

        /// Enable/disable table edition
        Q_PROPERTY(bool editableTable READ getEditableTable WRITE setEditableTable)

        /// Show/hide the controls of the table that contains a sequence of programs to be executed
        Q_PROPERTY(bool showTableControl READ getShowTableControl WRITE setShowTableControl)

        /// Show/hide the column '#' that displays the sequential number of programs
        Q_PROPERTY(bool showColumnNumber READ getShowColumnNumber WRITE setShowColumnNumber)

        /// Show/hide the column 'Enable' that enables the execution of programs
        Q_PROPERTY(bool showColumnEnable READ getShowColumnEnable WRITE setShowColumnEnable)

        /// Show/hide the column 'Program' that contains the external programs to be executed
        Q_PROPERTY(bool showColumnProgram READ getShowColumnProgram WRITE setShowColumnProgram)

        /// Show/hide the column 'Parameters' that contains the parameters that are passed to external programs to be executed
        Q_PROPERTY(bool showColumnParameters READ getShowColumnParameters WRITE setShowColumnParameters)

        /// Show/hide the column 'Directory' that defines the working directory to be used when external programs are executed
        Q_PROPERTY(bool showColumnWorkingDirectory READ getShowColumnWorkingDirectory WRITE setShowColumnWorkingDirectory)

        /// Show/hide the column 'Timeout' that defines a time out period in seconds (if equal to 0 then the program runs until it finishes; otherwise if greater than 0 then the program will only run during this amount of seconds and will be aborted beyond this time)
        Q_PROPERTY(bool showColumnTimeout READ getShowColumnTimeout WRITE setShowColumnTimeout)

        /// Show/hide the column 'Stop' that enables stopping the execution of subsequent programs when the current one exited with an error code different from 0
        Q_PROPERTY(bool showColumnStop READ getShowColumnStop WRITE setShowColumnStop)

        /// Show/hide the column 'Log' that enables the generation of log messages (these messages may be displayed using the QELog widget)
        Q_PROPERTY(bool showColumnLog READ getShowColumnLog WRITE setShowColumnLog)

        /// Select if the scripts are to be loaded/saved from an XML file or from an XML text
        Q_ENUMS(scriptTypesProperty)
        Q_PROPERTY(scriptTypesProperty scriptType READ getScriptTypeProperty WRITE setScriptTypeProperty)
        enum scriptTypesProperty
        {
            File = FROM_FILE,
            Text = FROM_TEXT
        };

        void setScriptTypeProperty(scriptTypesProperty pScriptType)
        {
            setScriptType((scriptTypesProperty) pScriptType);
        }
        scriptTypesProperty getScriptTypeProperty()
        {
            return (scriptTypesProperty) getScriptType();
        }

        /// Define the file where to save the scripts (if not defined then the scripts will be saved in a file named "QEScript.xml")
        Q_PROPERTY(QString scriptFile READ getScriptFile WRITE setScriptFile)

        /// Define the XML text that contains the scripts
        Q_PROPERTY(QString scriptText READ getScriptText WRITE setScriptText)

        /// Define the script (previously saved by the user) that will be load as the default script when the widget starts
        Q_PROPERTY(QString scriptDefault READ getScriptDefault WRITE setScriptDefault)

        /// Define the caption of the button responsible for starting the execution of external programs (if not defined then the caption will be "Execute")
        Q_PROPERTY(QString executeText READ getExecuteText WRITE setExecuteText)

        /// Change the order of the widgets. Valid orders are: TOP, BOTTOM, LEFT and RIG
        Q_ENUMS(optionsLayoutProperty)
        Q_PROPERTY(optionsLayoutProperty optionsLayout READ getOptionsLayoutProperty WRITE setOptionsLayoutProperty)
        enum optionsLayoutProperty
        {
            Top = TOP,
            Bottom = BOTTOM,
            Left = LEFT,
            Right = RIGHT
        };        

        void setOptionsLayoutProperty(optionsLayoutProperty pOptionsLayout)
        {
            setOptionsLayout((optionsLayoutProperty) pOptionsLayout);
        }
        optionsLayoutProperty getOptionsLayoutProperty()
        {
            return (optionsLayoutProperty) getOptionsLayout();
        }


    // BEGIN-STANDARD-PROPERTIES ======================================================
    // Standard properties
    // These properties should be identical for every widget using them.
    // WHEN MAKING CHANGES: Use the update_widget_properties script in the
    // resources directory.
public slots:
    /// Slot to set the visibility of a QE widget, taking into account the user level.
    /// Widget will be hidden if hidden by a call this slot, by will only be made visible by a calll to this slot if the user level allows.
    void setManagedVisible( bool v ){ setRunVisible( v ); }
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

    /// Hide style sheet from designer as style calculation by the styleManager and not directly setable per se.
    /// This also stops transient styles being saved to the ui file.
    Q_PROPERTY(QString styleSheet   READ styleSheet       WRITE setStyleSheet  DESIGNABLE false)

    /// Style Sheet string to be applied before, i.e. lower priority than, any other style, e.g. alarm style and/or user level style.
    /// Default is an empty string.
    Q_PROPERTY(QString defaultStyle READ getStyleDefault  WRITE setStyleDefault)

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

    private slots:
        void comboBoxScriptSelected(int);

        void buttonNewClicked();

        void buttonSaveClicked();

        void buttonDeleteClicked();

        void buttonExecuteClicked();

        void buttonAbortClicked();

        void buttonAddClicked();

        void buttonRemoveClicked();

        void buttonUpClicked();

        void buttonDownClicked();

        void buttonCopyClicked();

        void buttonPasteClicked();

        void selectionChanged(const QItemSelection &, const QItemSelection &);

    signals:
        void selected(QString pFilename);
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEScript::scriptTypesProperty)
Q_DECLARE_METATYPE (QEScript::optionsLayoutProperty)
Q_DECLARE_METATYPE (QEScript::UserLevels)
Q_DECLARE_METATYPE (QEScript::DisplayAlarmStateOptions)
#endif

#endif // QE_SCRIPT_H
