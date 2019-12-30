/* QEPvProperties.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at
 *  the Australian Synchrotron.
 *
 *  Copyright (c) 2012-2019 Australian Synchrotron
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
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_PV_PROPERTIES_H
#define QE_PV_PROPERTIES_H

#include <QAction>
#include <QLabel>
#include <QList>
#include <QMenu>
#include <QPoint>
#include <QScrollArea>
#include <QString>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QWidget>
#include <QComboBox>

#include <persistanceManager.h>
#include <QCaAlarmInfo.h>
#include <QEDragDrop.h>
#include <QCaObject.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QEAbstractDynamicWidget.h>
#include <QESingleVariableMethods.h>
#include <QELabel.h>
#include <QEResizeableFrame.h>
#include <QEInteger.h>
#include <QEIntegerFormatting.h>
#include <QEString.h>
#include <QEStringFormatting.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEWidget.h>
#include <QEQuickSort.h>
#include <QEOneToOne.h>

/// The QEPvProperties class allows user to view all the displayable fields
/// of the associated IOC record.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPvProperties :
      public QEAbstractDynamicWidget,
      public QESingleVariableMethods,
      public QEQuickSort {
   Q_OBJECT

   typedef QEAbstractDynamicWidget ParentWidgetClass;

   // BEGIN-SINGLE-VARIABLE-V2-PROPERTIES ===============================================
   // Single Variable properties
   // These properties should be identical for every widget using a single variable.
   // WHEN MAKING CHANGES: Use the update_widget_properties script in the resources
   // directory.
   //
   // Note, a property macro in the form 'Q_PROPERTY(QString variableName READ ...' doesn't work.
   // A property name ending with 'Name' results in some sort of string a variable being displayed,
   // but will only accept alphanumeric and won't generate callbacks on change.
public:
   /// EPICS variable name (CA PV)
   ///
   Q_PROPERTY (QString variable READ getVariableNameProperty WRITE setVariableNameProperty)

   /// Macro substitutions. The default is no substitutions. The format is NAME1=VALUE1[,] NAME2=VALUE2...
   /// Values may be quoted strings. For example, 'PUMP=PMP3, NAME = "My Pump"'
   /// These substitutions are applied to variable names for all QE widgets.
   /// In some widgets are are also used for other purposes.
   ///
   Q_PROPERTY (QString variableSubstitutions READ getVariableNameSubstitutionsProperty WRITE setVariableNameSubstitutionsProperty)

   /// The number of elements required to be subscribed for from the PV host (IOC).
   /// The default is 0 which means subscribed for all elements.
   /// Note: changing this value causes the unsubscribe/re-subscribe just as if the
   /// variable name changed.
   ///
   Q_PROPERTY (int elementsRequired READ getElementsRequired WRITE setElementsRequired)

   /// Index used to select a single item of data for processing. The default is 0.
   ///
   Q_PROPERTY (int arrayIndex READ getArrayIndex WRITE setArrayIndex)
   //
   // END-SINGLE-VARIABLE-V2-PROPERTIES =================================================

public:
   enum OwnContextMenuOptions {
      PVPROP_NONE = QEAbstractDynamicWidget::ADWCM_SUB_CLASS_WIDGETS_START_HERE,
      PVPROP_SORT_FIELD_NAMES,
      PVPROP_RESET_FIELD_NAMES,
      PVPROP_PROCESS_RECORD,
      PVPROP_SUB_CLASS_WIDGETS_START_HERE
   };

   // Constructors
   //
   QEPvProperties (QWidget*  parent = 0);
   QEPvProperties (const QString& variableName, QWidget* parent = 0);
   ~QEPvProperties ();

   QSize sizeHint () const;

   int addPvName (const QString& pvName);
   void addPvNameList (const QStringList& pvNameList);
   void clearAllPvNames ();

protected:
   void resizeEvent (QResizeEvent* event);
   void showEvent (QShowEvent* event);

   QMenu* buildContextMenu ();                        // Extend the QE generic context menu
   void contextMenuTriggered (int selectedItemNum);   // An action was selected from the context menu

   bool itemLessThan (const int a, const int b, QObject* context = NULL) const;
   void swapItems (const int a, const int b, QObject* context = NULL);

   void establishConnection (unsigned int variableIndex);

   // Override QCaObject/QEWidget functions.
   //
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);

   // Drop only. Dragging is from individual embedded QEWidgets.
   //
   // Override QEDragDrop functions.
   //
   void mousePressEvent (QMouseEvent* event)    { qcaMousePressEvent (event); }
   void dragEnterEvent (QDragEnterEvent* event) { qcaDragEnterEvent (event, false); }
   void dropEvent (QDropEvent* event)           { qcaDropEvent(event, true); }
   // This widget uses the setDrop/getDrop defined in QEWidget.

   void saveConfiguration (PersistanceManager* pm);
   void restoreConfiguration (PersistanceManager* pm, restorePhases restorePhase);

   // Copy paste
   //
   QString copyVariable ();
   QVariant copyData ();

   // Override QEAbstractDynamicWidget functions
   //
   void enableEditPvChanged ();

private:
   enum PVReadModes {
      StandardRead,      // no name qualification - read as is.
      ReadAsCharArray    // read field as array of chars to overcome 40 character DBF_STRING limit.
   };

   QEIntegerFormatting integerFormatting;
   QEStringFormatting rtypStringFormatting;
   QEStringFormatting valueStringFormatting;
   bool isFirstUpdate;

   // Internal widgets.
   //
   typedef QList<QLabel*> QLabelList;

   QFrame* topFrame;
   QLabel* label1;
   QLabel* label2;
   QLabel* label3;
   QLabel* label4;
   QLabel* label5;
   QLabel* label6;
   QComboBox* box;
   QLabel* valueLabel;
   QLabel* hostName;
   QLabel* fieldType;
   QLabel* timeStamp;
   QLabel* indexInfo;
   QVBoxLayout* topFrameVlayout;
   QHBoxLayout* hlayouts [6];

   QTableWidget* table;
   QFrame* enumerationFrame;
   QLabelList enumerationLabelList;
   QScrollArea* enumerationScroll;
   QEResizeableFrame*  enumerationResize;
   QVBoxLayout* vlayout;

   QString recordBaseName;
   QEStringFormatting fieldStringFormatting;

   QEInteger* recordProcField;       // PROC
   QEString* standardRecordType;     // RTYP
   QEString* alternateRecordType;    // RTYP$

   QString previousRecordBaseName;
   QString previousRecordType;

   QList<QEString *> fieldChannels;
   bool fieldsAreSorted;

   // Not used other than as quick sort contexts
   //
   QObject sortContext;
   QObject resetContext;

   // Map channel variable index  <==>  table row
   //
   QEOneToOne <unsigned int, int> variableIndexTableRowMap;

   // When not empty, this is the (context menu) selected field PV.
   // When empty, this is interpretted as the main PV name.
   //
   QString contextMenuPvName;

   // common constructor function.
   void common_setup ();
   void createInternalWidgets ();
   void clearFieldChannels ();

   void setUpRecordProcChannel (QEInteger* &qca);
   void setUpRecordTypeChannels (QEString* &qca, const PVReadModes readMode);

   // Override standardProperties::setApplicationEnabled()
   void setApplicationEnabled (const bool & state);

   // Insert name into the (top) of the combo box drop down list.
   //
   void insertIntoDropDownList (const QString& pvName);

   // Set pvName.
   //
   void setPvName (const QString& pvName);

private slots:
   void postCreationSetup ();
   void useNewVariableNameProperty (QString variableNameIn,
                                    QString variableNameSubstitutionsIn,
                                    unsigned int variableIndex);

   // Basic widgit PV related slots (used for RTYP pseudo field).
   //
   void setRecordTypeConnection (QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex);
   void setRecordTypeValue (const QString & rtypeValue, QCaAlarmInfo&, QCaDateTime&, const unsigned int& );

   // The value item slots.
   //
   void setValueConnection (QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex);
   void setValueValue (const QVariant& valueValue, QCaAlarmInfo&, QCaDateTime&, const unsigned int& variableIndex);

   // Field related slots
   //
   void setFieldConnection (QCaConnectionInfo& connectionInfo,
                            const unsigned int &variableIndex);

   void setFieldValue (const QString &value,
                       QCaAlarmInfo & alarmInfo,
                       QCaDateTime & dateTime,
                       const unsigned int & variableIndex);

   void boxCurrentIndexChanged (int index);                    // From own combo box.
   void customTableContextMenuRequested (const QPoint & pos);  // Form the table.
   void tableHeaderClicked (int index);

signals:
   void setCurrentBoxIndex (int index);           // connected to own combo box
};

# endif  // QE_PV_PROPERTIES_H
