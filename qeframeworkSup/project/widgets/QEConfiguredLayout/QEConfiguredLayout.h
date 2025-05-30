/*  QEConfiguredLayout.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2025 Australian Synchrotron
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
 *  Author:
 *    Ricardo Fernandes
 *  Contact details:
 *    ricardo.fernandes@synchrotron.org.au
 */

#ifndef QE_CONFIGURED_LAYOUT_H
#define QE_CONFIGURED_LAYOUT_H

#include <QDialog>
#include <QDomDocument>
#include <QVBoxLayout>
#include <QEEnums.h>
#include <QELabel.h>
#include <QEBitStatus.h>
#include <QEPushButton.h>
#include <QELineEdit.h>
#include <QEComboBox.h>
#include <QESpinBox.h>

// =============================================================================
//  FIELD CLASS
// =============================================================================
class _Field
{
private:
   QString name;
   QString processVariable;
   QString group;
   QString visible;
   QString editable;
   bool visibility;
   bool join;
   int type;

public:
   _Field();

   QEWidget *getWidget();
   void setWidget(QString *pValue);

   QString getName();
   void setName(QString pValue);

   QString getProcessVariable();
   void setProcessVariable(QString pValue);

   void setJoin(bool pValue);
   bool getJoin();

   int getType();
   void setType(int pValue);

   QString getGroup();
   void setGroup(QString pValue);

   QString getVisible();
   void setVisible(QString pValue);

   QString getEditable();
   void setEditable(QString pValue);

   bool getVisibility();
   void setVisibility(bool pValue);

   QEWidget *qeWidget;  // TODO: this attribute should be private

};


// =============================================================================
//  _ITEM CLASS
// =============================================================================
class _Item
{
private:
   QString name;
   QString substitution;
   QString visible;

public:
   _Item();

   void setName(QString pValue);
   QString getName();

   void setSubstitution(QString pValue);
   QString getSubstitution();

   void setVisible(QString pValue);
   QString getVisible();

   QList <_Field *> fieldList;  // TODO: this attribute should be private

};


// =============================================================================
//  _QPUSHBUTTONGROUP CLASS
// =============================================================================
class _QPushButtonGroup:public QPushButton
{
   Q_OBJECT

private:
   QList <_Field *> *currentFieldList;
   QString itemName;
   QString groupName;

public:
   _QPushButtonGroup(QWidget * pParent = 0,
                     QString pItemName = "",
                     QString pGroupName = "",
                     QList <_Field *> *pCurrentFieldList = 0);

   void mouseReleaseEvent(QMouseEvent *qMouseEvent);
   void keyPressEvent(QKeyEvent *pKeyEvent);
   void showDialogGroup();

public slots:
   void buttonGroupClicked();

};


// =============================================================================
//  _QDIALOGITEM CLASS
// =============================================================================
class _QDialogItem:public QDialog
{
   Q_OBJECT

private:
   QPushButton *qPushButtonClose;

public:
   _QDialogItem(QWidget *pParent = 0,
                QString pItemName = "",
                QString pGroupName = "",
                QList <_Field *> *pCurrentFieldList = 0,
                Qt::WindowFlags pF = Qt::Widget);

private slots:
   void buttonCloseClicked();

};


// =============================================================================
//  QECONFIGUREDLAYOUT CLASS
// =============================================================================
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEConfiguredLayout :
      public QWidget, public QEWidget
{
   Q_OBJECT

private:
   void setConfiguration(QString pValue);

protected:
   QLabel *qLabelItemDescription;
   QComboBox *qComboBoxItemList;
   QVBoxLayout *qVBoxLayoutFields;
   QScrollArea *qScrollArea;
   QString configurationFile;
   QString configurationText;
   QE::SourceOptions configurationType;
   QE::LayoutOptions optionsLayout;
   int currentUserType;
   bool subscription;

public:
   enum types { LABEL, LINEEDIT, COMBOBOX, SPINBOX, BUTTON };

   QEConfiguredLayout(QWidget *pParent = 0, bool pSubscription = true);
   virtual ~QEConfiguredLayout(){}

   void setItemDescription(QString pValue);
   QString getItemDescription();

   void setShowItemList(bool pValue);
   bool getShowItemList();

   void setConfigurationType(QE::SourceOptions pValue);
   QE::SourceOptions getConfigurationType();

   void setConfigurationFile(QString pValue);
   QString getConfigurationFile();

   void setConfigurationText(QString pValue);
   QString getConfigurationText();

   void setOptionsLayout(QE::LayoutOptions pValue);
   QE::LayoutOptions getOptionsLayout();

   void setCurrentUserType(int pValue);
   int getCurrentUserType();


   void refreshFields();

   void userLevelChanged( QE::UserLevels pValue );

   QList <_Item *> itemList;    // TODO: this attribute shoule be private

   QList <_Field *> currentFieldList;   //TODO: this attribute should be private

   Q_PROPERTY(QString itemDescription READ getItemDescription WRITE setItemDescription)

   Q_PROPERTY(bool showItemList READ getShowItemList WRITE setShowItemList)

   Q_PROPERTY(QE::SourceOptions configurationType READ getConfigurationType WRITE setConfigurationType)

   Q_PROPERTY(QString configurationFile READ getConfigurationFile WRITE setConfigurationFile)

   Q_PROPERTY(QString configurationText READ getConfigurationText WRITE setConfigurationText)

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
   void comboBoxItemSelected(int);

   void valueWritten(const QString &pNewValue, const QString &pOldValue, const QString&);

};

#endif // QE_CONFIGURED_LAYOUT_H
