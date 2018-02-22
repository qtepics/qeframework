/*  QERadioGroup.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2013,2014,2016,2017,2018 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_RADIO_GROUP_H
#define QE_RADIO_GROUP_H

#include <QHBoxLayout>
#include <QList>
#include <QString>
#include <QVector>
#include <QSize>

#include <QECommon.h>
#include <QEAbstractWidget.h>
#include <QRadioGroup.h>

#include <QCaObject.h>
#include <QEInteger.h>
#include <QEIntegerFormatting.h>
#include <QELocalEnumeration.h>
#include <QESingleVariableMethods.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QEOneToOne.h>

// QEAbstractWidget provides all standard QEWidget properties
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QERadioGroup :
      public QEAbstractWidget,
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

   // QERadioGroup specific properties ===============================================
   //
   /// Group box title text to be substituted.
   /// This text will be copied to the group box title text after applying any macro substitutions
   /// from the variableSubstitutions property.
   /// The former is depricated and included for backward compatabilty only and not presented on designer.
   ///
   Q_PROPERTY (QString substitutedTitle READ getSubstitutedTitleProperty WRITE setSubstitutedTitleProperty  DESIGNABLE false)
   Q_PROPERTY (QString title            READ getSubstitutedTitleProperty WRITE setSubstitutedTitleProperty)

   /// Number of colums - defaults to two.
   ///
   Q_PROPERTY (int columns READ getColumns WRITE setColumns)

   /// Interay layout margins and spacing - defaults to 4.
   ///
   Q_PROPERTY (int spacing READ getSpacing WRITE setSpacing)

   /// Use database enumerations - defaults to true.
   /// False implies use of local enumeration.
   ///
   Q_PROPERTY (bool useDbEnumerations READ getUseDbEnumerations WRITE setUseDbEnumerations)

   /// Enumrations values used when useDbEnumerations is false.
   ///
   Q_PROPERTY (QString localEnumeration READ getLocalEnumerations WRITE setLocalEnumerations)

   /// Allows selection of buttom style (Radio (default) or Push)
   ///
   Q_PROPERTY (QRadioGroup::ButtonStyles buttonStyle READ getButtonStyle WRITE setButtonStyle)

   /// Allows selection of buttom order (rowMajor (default) or colMajor)
   ///
   Q_PROPERTY (QRadioGroup::ButtonOrders buttonOrder READ getButtonOrder WRITE setButtonOrder)
   //
   // End of QERadioGroup specific properties =========================================

public:
   /// Create without a variable.
   /// Use setVariableNameProperty() and setSubstitutionsProperty() to define a
   /// variable and, optionally, macro substitutions later.
   ///
   explicit QERadioGroup (QWidget* parent = 0);

   /// Create with a variable.
   /// A connection is automatically established.
   /// If macro substitutions are required, create without a variable and set the
   /// variable and macro substitutions after creation.
   ///
   explicit QERadioGroup (const QString& variableName, QWidget* parent = 0);

   /// Create with a group title and a variable.
   /// A connection is automatically established.
   /// If macro substitutions are required, create without a variable and set the
   /// variable and macro substitutions after creation.
   ///
   explicit QERadioGroup (const QString& title, const QString& variableName, QWidget* parent = 0);

   /// Destruction
   virtual ~QERadioGroup ();

   int getCurrentIndex () const;

   // Hide/override parent function out of QESingleVariableMethods.
   void setVariableNameSubstitutionsProperty (const QString& substitutions);

   void setSubstitutedTitleProperty (const QString& substitutedTitle);
   QString getSubstitutedTitleProperty () const;

   void setUseDbEnumerations (bool useDbEnumerations);
   bool getUseDbEnumerations () const;

   void setLocalEnumerations (const QString& localEnumerations);
   QString getLocalEnumerations () const;

   // Expose access to the internal widget's set/get functions.
   //
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, int, getColumns, setColumns)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, int, getSpacing, setSpacing)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, QRadioGroup::ButtonStyles, getButtonStyle, setButtonStyle)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, QRadioGroup::ButtonOrders, getButtonOrder, setButtonOrder)

signals:
   // Note, the following signals are common to many QE widgets,
   // if changing the doxygen comments, ensure relevent changes are migrated to all instances
   // These signals are emitted using the QEEmitter::emitDbValueChanged function.
   // This signal emiited as a result of changes to the primary PV variable only,
   // i.e. excludes the edgeVariable.
   /// Sent when the widget is updated following a data change
   /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
   /// For example a QList widget could log updates from this widget.
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
   void dbConnectionChanged (const bool& isConnected);

public slots:
   /// Update the default style applied to this widget.
   void setDefaultStyle (const QString& style) { this->setStyleDefault (style); }

protected:
   QSize sizeHint () const;

   // override QEWidget fnctions.
   //
   void activated ();
   void establishConnection (unsigned int variableIndex);
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);

   // Drag and Drop
   void dragEnterEvent (QDragEnterEvent *event) { qcaDragEnterEvent( event ); }
   void dropEvent (QDropEvent *event)           { qcaDropEvent( event ); }
   void mousePressEvent (QMouseEvent *event)    { qcaMousePressEvent( event ); }

   // Use default getDrop/setDrop

   // Copy paste
   QString copyVariable ();
   QVariant copyData ();
   void paste (QVariant s);

private:
   void commonSetup (const QString& title);
   void setRadioGroupText ();

   QRadioGroup* internalWidget;
   QHBoxLayout* layout;         // holds the internal widget - any layout type will do

   // Use of the local enumerations means that we could have sparce mapping,
   // e.g.: 1 => Red, 5 => Blue, 63 => Green.  Therefore we need to create
   // and maintain a two way value to index association.
   // Using the above example:
   //   value  1  <==> radio group index 0 (text "Red")
   //   value  5  <==> radio group index 1 (text "Blue")
   //   value 63  <==> radio group index 2 (text "Green")
   //
   typedef QEOneToOne<int, int> ValueIndexAssociations;
   ValueIndexAssociations valueToIndex;

   bool useDbEnumerations;
   int  currentIndex;
   bool isFirstUpdate;
   QEIntegerFormatting integerFormatting;
   QELocalEnumeration localEnumerations;

   QCaVariableNamePropertyManager titleVnpm;

private slots:
   void connectionChanged (QCaConnectionInfo& connectionInfo,
                           const unsigned int &variableIndex);
   void valueUpdate (const long& value, QCaAlarmInfo&, QCaDateTime&, const unsigned int&);

   void useNewVariableNameProperty (QString variableNameIn,
                                    QString variableNameSubstitutionsIn,
                                    unsigned int variableIndex);

   void internalValueChanged (const int value);
};

#endif // QE_RADIO_GROUP_H
