/*  QEGeneralEdit.h
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
 *  Copyright (c) 2014,2016 Australian Synchrotron.
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

#include "QEGroupBox.h"
#include "QELabel.h"
#include "QELineEdit.h"
#include "QENumericEdit.h"
#include "QERadioGroup.h"

#include <QCaObject.h>
#include <QEWidget.h>
#include <QEInteger.h>
#include <QEIntegerFormatting.h>
#include <QESingleVariableMethods.h>
#include <QELocalEnumeration.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEPluginLibrary_global.h>
#include <QEFrame.h>

/*!
 * \brief The QEGeneralEdit class
 * This class provides a general PV edit widget, presenting one off a QELineEdit,
 * a QENumericEdit or a QERadioGroup for string, numerical and enumeration data
 * kinds respectively.
 *
 * This widget is intented for use within a qegui predefined form, displayed in
 * response to context menu request to edit an arbitary PV.
 */
class QEPLUGINLIBRARYSHARED_EXPORT QEGeneralEdit :
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

   /// Index used to select a single item of data for processing. The default is 0.
   ///
   Q_PROPERTY (int arrayIndex READ getArrayIndex WRITE setArrayIndex)
   //
   // END-SINGLE-VARIABLE-V2-PROPERTIES =================================================

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

   // Override/hide paraent function.
   //
   void setArrayIndex (const int arrayIndex);

protected:
   QSize sizeHint () const;

   // override QEWidget fnctions.
   //
   void establishConnection (unsigned int variableIndex);
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);

private:
   void commonSetup ();
   void createInternalWidgets ();

   QVBoxLayout *verticalLayout;
   QLabel *pvNameLabel;
   QELabel *valueLabel;
   QENumericEdit *numericEditWidget;
   QERadioGroup *radioGroupPanel;
   QELineEdit *stringEditWidget;

   bool isFirstUpdate;

private slots:

   void useNewVariableNameProperty (QString variableNameIn,
                                    QString variableNameSubstitutionsIn,
                                    unsigned int variableIndex);

   void connectionChanged (QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex);
   void dataChanged (const QVariant& value, QCaAlarmInfo& alarmInfo, QCaDateTime& timeStamp, const unsigned int& variableIndex);


protected:
   // Drag and Drop
   void dragEnterEvent(QDragEnterEvent *event) { qcaDragEnterEvent( event );  }
   void dropEvent(QDropEvent *event)           { qcaDropEvent( event );       }
   void mousePressEvent(QMouseEvent *event)    { qcaMousePressEvent( event ); }
   void setDrop( QVariant drop );
   QVariant getDrop();

   // Copy paste
   QString copyVariable ();
   QVariant copyData ();
   void paste (QVariant s);
};

#endif // QE_GENERAL_EDIT_H
