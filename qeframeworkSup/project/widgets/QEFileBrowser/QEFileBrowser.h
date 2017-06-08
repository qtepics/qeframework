/*  QEFileBrowser.h
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
 *  Copyright (c) 2012,2017 Australian Synchrotron
 *
 *  Author:
 *    Ricardo Fernandes
 *  Contact details:
 *    ricardo.fernandes@synchrotron.org.au
 */

#ifndef QE_FILE_BROWSER_H
#define QE_FILE_BROWSER_H

#include <QTableWidget>
#include <QPushButton>
#include <QEWidget.h>
#include <QELineEdit.h>

// ============================================================
//  _QTABLEWIDGETFILEBROWSER CLASS
// ============================================================
class _QTableWidgetFileBrowser:public QTableWidget
{

    Q_OBJECT

    private:
        bool initialized;

    protected:

    public:
        _QTableWidgetFileBrowser(QWidget * pParent = 0);
        void refreshSize();
        void resizeEvent(QResizeEvent *);
        void resize(int w, int h);

    private slots:

};


// ============================================================
//  QEFILEBROWSER METHODS
// ============================================================
/*!
  This class is a EPICS aware widget.
  The QEFileBrowser widget allows the user to browse existing files from a certain directory.
*/
class QEPLUGINLIBRARYSHARED_EXPORT QEFileBrowser:public QWidget, public QEWidget
{

    Q_OBJECT

    private:

    protected:
        QELineEdit *qeLineEditDirectoryPath;
        QPushButton *qPushButtonDirectoryBrowser;
        QPushButton *qPushButtonRefresh;
        _QTableWidgetFileBrowser *qTableWidgetFileBrowser;
        QString fileFilter;
        bool showFileExtension;
        bool fileDialogDirectoriesOnly;
        int optionsLayout;

    public:

        QEFileBrowser(QWidget *pParent = 0);
        virtual ~QEFileBrowser(){}

        void setVariableName(QString pValue);
        QString getVariableName();

        void setVariableNameSubstitutions(QString pValue);
        QString getVariableNameSubstitutions();

        void setDirectoryPath(QString pValue);
        QString getDirectoryPath();

        void setShowDirectoryPath(bool pValue);
        bool getShowDirectoryPath();

        void setShowDirectoryBrowser(bool pValue);
        bool getShowDirectoryBrowser();

        void setShowRefresh(bool pValue);
        bool getShowRefresh();

        void setShowTable(bool pValue);
        bool getShowTable();

        void setShowColumnTime(bool pValue);
        bool getShowColumnTime();

        void setShowColumnSize(bool pValue);
        bool getShowColumnSize();

        void setShowColumnFilename(bool pValue);
        bool getShowColumnFilename();

        void setShowFileExtension(bool pValue);
        bool getShowFileExtension();

        void setFileFilter(QString pValue);
        QString getFileFilter();

        void setFileDialogDirectoriesOnly(bool pValue);
        bool getFileDialogDirectoriesOnly();

        void setOptionsLayout(int pValue);
        int getOptionsLayout();

        void updateTable();


        /// EPICS variable name (CA PV).
        /// This variable is used for both writing and reading the directory to be used by the widget.
        Q_PROPERTY(QString variable READ getVariableName WRITE setVariableName)

        /// Macro substitutions. The default is no substitutions. The format is NAME1=VALUE1[,] NAME2=VALUE2... Values may be quoted strings. For example, 'PUMP=PMP3, NAME = "My Pump"'
        /// These substitutions are applied to variable names for all QE widgets. In some widgets are are also used for other purposes.
        Q_PROPERTY(QString variableSubstitutions READ getVariableNameSubstitutions WRITE setVariableNameSubstitutions)

        /// Default directory where to browse files when QEFileBrowser is launched for the first time
        Q_PROPERTY(QString directoryPath READ getDirectoryPath WRITE setDirectoryPath)

        /// Show/hide directory path line edit where the user can specify the directory to browse files
        Q_PROPERTY(bool showDirectoryPath READ getShowDirectoryPath WRITE setShowDirectoryPath)

        /// Show/hide button to open the dialog window to browse for directories and files
        Q_PROPERTY(bool showDirectoryBrowser READ getShowDirectoryBrowser WRITE setShowDirectoryBrowser)

        /// Show/hide button to refresh the table containing the list of files being browsed
        Q_PROPERTY(bool showRefresh READ getShowRefresh WRITE setShowRefresh)

        /// Show/hide table containing the list of files being browsed
        Q_PROPERTY(bool showTable READ getShowTable WRITE setShowTable)

        /// Show/hide column containing the time of creation of files
        Q_PROPERTY(bool showColumnTime READ getShowColumnTime WRITE setShowColumnTime)

        /// Show/hide column containing the size (in bytes) of files
        Q_PROPERTY(bool showColumnSize READ getShowColumnSize WRITE setShowColumnSize)

        /// Show/hide column containing the name of files
        Q_PROPERTY(bool showColumnFilename READ getShowColumnFilename WRITE setShowColumnFilename)

        /// Show/hide the extension of files
        Q_PROPERTY(bool showFileExtension READ getShowFileExtension WRITE setShowFileExtension)

        /// Enable/disable the browsing of directories-only when opening the dialog window
        Q_PROPERTY(bool fileDialogDirectoriesOnly READ getFileDialogDirectoriesOnly WRITE setFileDialogDirectoriesOnly)

        /// Specify which files to browse. To specify more than one filter, please separate them with a ";".
        /// Example: *.py;*.ui (this will only display files with an extension .py or .ui).
        Q_PROPERTY(QString fileFilter READ getFileFilter WRITE setFileFilter)

        /// Change the order of the widgets. Valid orders are: TOP, BOTTOM, LEFT and RIG
        Q_ENUMS(optionsLayoutProperty)
        Q_PROPERTY(optionsLayoutProperty optionsLayout READ getOptionsLayoutProperty WRITE setOptionsLayoutProperty)
        enum optionsLayoutProperty
        {
            Top,
            Bottom,
            Left,
            Right
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
        void lineEditDirectoryPathChanged(QString);

        void buttonDirectoryBrowserClicked();

        void buttonRefreshClicked();

        void itemActivated(QTableWidgetItem *);

    signals:
        /// Signal that is generated every time the user double-clicks a certain file.
        /// This signals emits a string that contains the full path and the name of the selected file.
        /// This signal may be captured by other widgets that perform further operations (for instance,
        /// the QEImage displays the content of this file if it is a graphical one).
        void selected(QString pFilename);

};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEFileBrowser::optionsLayoutProperty)
Q_DECLARE_METATYPE (QEFileBrowser::UserLevels)
Q_DECLARE_METATYPE (QEFileBrowser::DisplayAlarmStateOptions)
#endif

#endif // QE_FILE_BROWSER_H
