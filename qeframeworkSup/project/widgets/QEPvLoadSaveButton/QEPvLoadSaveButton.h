/*  QEPvLoadSaveButton.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2020-2024 Australian Synchrotron
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
 *    Andraz Pozar
 *  Contact details:
 *    andraz.pozar@ansto.gov.au
 */

#ifndef QE_PV_LOAD_SAVE_BUTTON_H
#define QE_PV_LOAD_SAVE_BUTTON_H

#include <QPushButton>
#include <QString>
#include <QProgressDialog>
#include <QTreeView>
#include <QLabel>

#include <QEEnums.h>
#include <QEWidget.h>
#include <QEPvLoadSaveModel.h>
#include <QCaVariableNamePropertyManager.h>


class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPvLoadSaveButton : public QPushButton, public QEWidget {

   Q_OBJECT
   // QEPvLoadSave specific properties ==============================================
   //
public:
   enum Actions { LoadToPVs, SaveToFile };
   Q_ENUM (Actions)

   /// configurationFile
   ///
   Q_PROPERTY (QString configurationFile  READ getConfigurationFile  WRITE setConfigurationFile)

   /// Macro substitutions. The default is no substitutions. The format is NAME1=VALUE1[,] NAME2=VALUE2...
   /// Values may be quoted strings. For example, 'PUMP=PMP3, NAME = "My Pump"'
   /// These substitutions are applied to variable names for all QE widgets.
   /// In some widgets are are also used for other purposes.
   ///
   Q_PROPERTY (QString defaultSubstitutions READ getSubstitutions WRITE setSubstitutions)

   /// If true, a dialog will be presented asking the user to confirm if the PV write actions
   /// should be carried out. Defaults to true.
   ///
   Q_PROPERTY (Actions action READ getAction WRITE setAction)

   /// If true, a dialog will be presented asking the user to confirm if the PV write actions
   /// should be carried out. Defaults to true.
   ///
   Q_PROPERTY (bool confirmAction READ getConfirmAction WRITE setConfirmAction)

   /// Text used to confirm acion if confirmation dialog is presented
   ///
   Q_PROPERTY (QString confirmText READ getConfirmText WRITE setConfirmText)

   /// If true, progress dialog will be shown while the data is being read or written
   ///
   Q_PROPERTY (bool showProgressDialog READ getShowProgressDialog WRITE setShowProgressDialog)

   // BEGIN-STANDARD-V2-PROPERTIES ===================================================
   // Standard properties
   // These properties should be identical for every widget using them.
   // WHEN MAKING CHANGES: Use the update_widget_properties script in the
   // resources directory.
public slots:
   /// Slot to set the visibility of a QE widget, taking into account the user level.
   /// Widget will be hidden if hidden by a call this slot, by will only be made visible by a calll to this slot if the user level allows.
   void setManagedVisible( bool v ){ setRunVisible( v ); }
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

public:
   /// Create without a nominated config file.
   ///
   QEPvLoadSaveButton (QWidget *parent = NULL);

   /// Destruction
   virtual ~QEPvLoadSaveButton ();

   // We use same mechanism that is used to manage variable names to manage the configuration file name.
   //
   void    setConfigurationFile (QString configurationFile)    { this->vnpm.setVariableNameProperty(configurationFile); }
   QString getConfigurationFile ()                             { return this->vnpm.getVariableNameProperty(); }

   // Property access functions.
   //
   void    setSubstitutions (QString configurationFileSubstitutions) { this->vnpm.setSubstitutionsProperty(configurationFileSubstitutions); }
   QString getSubstitutions () const                                 { return this->vnpm.getSubstitutionsProperty(); }

   // load or save
   void setAction (Actions loadOrSaveIn )   { this->loadSaveAction = loadOrSaveIn; }
   Actions getAction () const               { return this->loadSaveAction; }

   // confirm
   void setConfirmAction (bool confirmRequiredIn ) {this->confirmRequired = confirmRequiredIn;}
   bool getConfirmAction () const                  {return this->confirmRequired;}

   // confirm text
   void setConfirmText( QString confirmTextIn ) { this->confirmText = confirmTextIn; }
   QString getConfirmText() const               { return this->confirmText; }

   // show progress
   void setShowProgressDialog (bool showProgressDialogIn ) {this->showProgressDialog = showProgressDialogIn;}
   bool getShowProgressDialog () const                  {return this->showProgressDialog;}

private:
   QCaVariableNamePropertyManager vnpm;    // manages filenames
   QEPvLoadSaveModel* model;               // manages tree data
   QEPvLoadSaveItem* rootItem;

   Actions loadSaveAction;
   bool confirmRequired;
   QString confirmText;
   bool showProgressDialog;

   QProgressDialog* progressDialog;

   bool actionIsPermitted (Actions);

private slots:
   void useNewConfigurationFileProperty (QString configurationFileIn,
                                         QString configurationFileSubstitutionsIn,
                                         unsigned int variableIndex );

   void acceptActionComplete (const QEPvLoadSaveItem*, const QEPvLoadSaveCommon::ActionKinds, const bool);

   void delayedSaveToFile ();
   void delayedLoadToPVs ();
   void userClicked(bool);
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEPvLoadSaveButton::Actions)
#endif

#endif // QE_PV_LOAD_SAVE_BUTTON_H
