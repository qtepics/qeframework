/*  QEScript.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Ricardo Fernandes
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_SCRIPT_H
#define QE_SCRIPT_H

#include <QTableWidgetItem>
#include <QPushButton>
#include <QComboBox>
#include <QEEnums.h>
#include <QEWidget.h>

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
  The QEScript widget allows the user to define a certain sequence of external
  programs to be executed. This sequence may be saved, modified or loaded for
  future usage.
*/
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEScript : public QWidget, public QEWidget
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
   QE::SourceOptions scriptType;
   QE::LayoutOptions optionsLayout;
   QDomDocument document;
   QString filename;
   QList<_CopyPaste *> copyPasteList;
   bool editableTable;
   bool isExecuting;

public:
   explicit QEScript(QWidget *pParent = 0);
   virtual ~QEScript();

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

   void setScriptType(QE::SourceOptions pValue);
   QE::SourceOptions getScriptType();

   void setScriptFile(QString pValue);
   QString getScriptFile();

   void setScriptText(QString pValue);
   QString getScriptText();

   void setScriptDefault(QString pValue);
   QString getScriptDefault();

   void setExecuteText(QString pValue);
   QString getExecuteText();

   void setOptionsLayout(QE::LayoutOptions pValue);
   QE::LayoutOptions getOptionsLayout();

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
   Q_PROPERTY(QE::SourceOptions scriptType READ getScriptType WRITE setScriptType)

   /// Define the file where to save the scripts (if not defined then the scripts will be saved in a file named "QEScript.xml")
   Q_PROPERTY(QString scriptFile READ getScriptFile WRITE setScriptFile)

   /// Define the XML text that contains the scripts
   Q_PROPERTY(QString scriptText READ getScriptText WRITE setScriptText)

   /// Define the script (previously saved by the user) that will be load as the default script when the widget starts
   Q_PROPERTY(QString scriptDefault READ getScriptDefault WRITE setScriptDefault)

   /// Define the caption of the button responsible for starting the execution of external programs (if not defined then the caption will be "Execute")
   Q_PROPERTY(QString executeText READ getExecuteText WRITE setExecuteText)

   /// Change the order of the widgets. Valid orders are: TOP, BOTTOM, LEFT and RIG
   Q_PROPERTY(QE::LayoutOptions optionsLayout READ getOptionsLayout WRITE setOptionsLayout)

   // BEGIN-STANDARD-V2-PROPERTIES ===================================================
   // Standard properties
   // These properties should be identical for every widget using them.
   // WHEN MAKING CHANGES: Use the update_widget_properties script in the
   // resources directory.
public slots:
   /// Slot to set the visibility of a QE widget, taking into account the user level.
   /// Widget will be hidden if hidden by a call this slot, by will only be made
   /// visible by a calll to this slot if the user level allows.
   ///
   void setManagedVisible (bool v) { this->setRunVisible(v); }

public:
   /// variableAsToolTip not applicale to V2 options.
   /// Class should ensure this option is initialised to false.

   /// allowDrop not applicale to V2 options.
   /// Class should ensure this option is initialised to false.

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

   /// Lowest user level at which the widget is visible. Default is 'User'.
   /// Used when designing GUIs that display more and more detail according to the user mode.
   /// The user mode is set application wide through the QELogin widget, or programatically through setUserLevel()
   /// Widgets that are always visible should be visible at 'User'.
   /// Widgets that are only used by scientists managing the facility should be visible at 'Scientist'.
   /// Widgets that are only used by engineers maintaining the facility should be visible at 'Engineer'.
   ///
   Q_PROPERTY(QE::UserLevels userLevelVisibility READ getUserLevelVisibility WRITE setUserLevelVisibility)

   /// Lowest user level at which the widget is enabled. Default is 'User'.
   /// Used when designing GUIs that allow access to more and more detail according to the user mode.
   /// The user mode is set application wide through the QELogin widget, or programatically through setUserLevel()
   /// Widgets that are always accessable should be visible at 'User'.
   /// Widgets that are only accessable to scientists managing the facility should be visible at 'Scientist'.
   /// Widgets that are only accessable to engineers maintaining the facility should be visible at 'Engineer'.
   ///
   Q_PROPERTY(QE::UserLevels userLevelEnabled READ getUserLevelEnabled WRITE setUserLevelEnabled)

   /// displayAlarmStateOption not applicale to V2 options.
   /// Class should ensure this option is initialised to DISPLAY_ALARM_STATE_NEVER.

public:
   // END-STANDARD-V2-PROPERTIES =====================================================

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

#endif // QE_SCRIPT_H
