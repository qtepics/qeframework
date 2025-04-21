/*  QEGeneralEdit.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2014-2022 Australian Synchrotron.
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

#ifndef QE_GENERAL_EDIT_H
#define QE_GENERAL_EDIT_H

#include <QString>
#include <QVariant>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QWidget>
#include <QSize>

#include <QEEnums.h>
#include <QRadioGroup.h>
#include <QCaObject.h>
#include <QEWidget.h>
#include <QEInteger.h>
#include <QEIntegerFormatting.h>
#include <QESingleVariableMethods.h>
#include <QELocalEnumeration.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QEFrame.h>


namespace Ui {
   class General_Edit_Form;   // differed
}

/*!
 * \brief The QEGeneralEdit class
 * This class provides a general PV edit widget, presenting one off a QELineEdit,
 * a QENumericEdit or a QERadioGroup for string, numerical and enumeration data
 * kinds respectively.
 *
 * This widget is intented for use within a qegui predefined form, displayed in
 * response to context menu request to edit an arbitary PV.
 */
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEGeneralEdit :
   public QEFrame,
   public QESingleVariableMethods
{

   Q_OBJECT

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

   // Widget specific properties.
   //
   /// Button style for enumerations edits.
   ///
   Q_PROPERTY (QRadioGroup::ButtonStyles buttonStyle READ getButtonStyle  WRITE setButtonStyle)

   /// Button order for enumerations edits.
   ///
   Q_PROPERTY (QE::GridOrders buttonOrder READ getButtonOrder  WRITE setButtonOrder)

   /// For numeric and string edits, the cnage can be applied on enter/lose focus, or
   /// applied via explicity clicking an apply button. Setting this proerty false
   /// uses the former, true the latter. The default property value is false.
   ///
   Q_PROPERTY (bool  useApplyButton   READ getUseApplyButton       WRITE setUseApplyButton)

public:
   /// Create without a variable.
   /// Use setVariableNameProperty() and setSubstitutionsProperty() to define a
   /// variable and, optionally, macro substitutions later.
   ///
   explicit QEGeneralEdit (QWidget* parent = 0);

   /// Create with a variable.
   /// A connection is automatically established.
   /// If macro substitutions are required, create without a variable and set the
   /// variable and macro substitutions after creation.
   ///
   explicit QEGeneralEdit (const QString& variableName, QWidget* parent = 0);

   /// Destruction
   virtual ~QEGeneralEdit () {}

   // Override/hide parent function.
   //
   void setArrayIndex (const int arrayIndex);

   // Property value setter and getter functions
   //
   void setButtonStyle (const QRadioGroup::ButtonStyles style);
   QRadioGroup::ButtonStyles getButtonStyle () const;

   void setButtonOrder (const QE::GridOrders order);
   QE::GridOrders getButtonOrder () const;

   void setUseApplyButton (const bool useApplyButton);
   bool getUseApplyButton () const;

signals:
   // Note, the following signals are common to many QE widgets,
   // if changing the doxygen comments, ensure relevent changes are migrated to all instances
   // These signals are emitted using the QEEmitter::emitDbValueChanged function.
   // This signal emiited as a result of changes to the primary PV variable only,
   // i.e. excludes the edgeVariable.
   /// Sent when the widget is updated following a data change
   /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
   /// For example a QList widget could log updates from this widget.
   ///
   void dbValueChanged ();                     // signal event
   void dbValueChanged (const QString& out);   // signal as formatted text
   void dbValueChanged (const int& out);       // signal as int if applicable
   void dbValueChanged (const long& out);      // signal as long if applicable
   void dbValueChanged (const qlonglong& out); // signal as qlonglong if applicable
   void dbValueChanged (const double& out);    // signal as floating if applicable
   void dbValueChanged (const bool& out);      // signal as bool: value != 0 if applicable

   // This signal is emitted using the QEEmitter::emitDbConnectionChanged function.
   /// Sent when the widget state updated following a channel connection change
   /// Applied to provary varible.
   ///
   void dbConnectionChanged (const bool& isConnected);

protected:
   QSize sizeHint () const;

   // override QEWidget fnctions.
   //
   void establishConnection (unsigned int variableIndex);
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);

private:
   void commonSetup ();

   Ui::General_Edit_Form* ui;
   bool isFirstUpdate;
   bool useApplyButton;

private slots:
   void useNewVariableNameProperty (QString variableNameIn,
                                    QString variableNameSubstitutionsIn,
                                    unsigned int variableIndex);

   void connectionChanged (QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex);
   void dataChanged (const QVariant& value, QCaAlarmInfo& alarmInfo, QCaDateTime& timeStamp, const unsigned int& variableIndex);

   // Handles signals from within the General_Edit_Form itself.
   //
   void onStringEditApply (bool checked);
   void onNumericEditApply (bool checked);
   void onZerosValueChanged (const int value);
   void onPrecisionValueChanged (const int value);

protected:
   // Drag and Drop
   void dragEnterEvent(QDragEnterEvent *event) { qcaDragEnterEvent( event );  }
   void dropEvent(QDropEvent *event)           { qcaDropEvent( event );       }
   void mousePressEvent(QMouseEvent *event)    { qcaMousePressEvent( event ); }

   // Use default getDrop/setDrop

   // Copy paste
   QString copyVariable ();
   QVariant copyData ();
   void paste (QVariant s);
};

#endif // QE_GENERAL_EDIT_H
