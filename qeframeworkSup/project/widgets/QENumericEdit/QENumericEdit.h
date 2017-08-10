/*  QENumericEdit.h
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
 *  Copyright (c) 2013,2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_NUMERIC_EDIT_H
#define QE_NUMERIC_EDIT_H

#include <QHBoxLayout>
#include <QList>
#include <QString>
#include <QVector>
#include <QSize>

#include <QECommon.h>
#include <QEAbstractWidget.h>
#include <QNumericEdit.h>
#include <QEFloating.h>
#include <QEFloatingFormatting.h>
#include <QESingleVariableMethods.h>
#include <QEFrameworkLibraryGlobal.h>

// QEAbstractWidget provides all standard QEWidget properties
// QENumericEdit holds a single QNumericEdit widget
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QENumericEdit :
      public QEAbstractWidget,
      public QESingleVariableMethods {

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


   // QENumericEdit specific properties ===============================================
   //
   /// This property holds whether the numeric edit draws itself with a frame.
   /// If enabled (the default) the numeric edit draws itself inside a frame,
   /// otherwise the line edit draws itself without any frame.
   ///
   Q_PROPERTY (bool frame              READ hasFrame            WRITE setFrame)

   /// This property holds the alignment of the numeric edit.
   /// Both horizontal and vertical alignment is allowed here, Qt::AlignJustify will map to Qt::AlignLeft.
   /// By default, this property contains a combination of Qt::AlignRight and Qt::AlignVCenter.
   ///
   Q_PROPERTY (Qt::Alignment alignment READ alignment           WRITE setAlignment)

   /// This property holds the displayed text. Not a property available to designer.
   ///
   Q_PROPERTY (QString cleanText       READ getCleanText)

   /// If true (default), display and editing of numbers using the PV's precision and control
   /// limits supplied with the data. If false, the precision, leadingZeros, minimum and maximum
   /// propertie values are used.
   ///
   Q_PROPERTY (bool autoScale          READ getAutoScale        WRITE setAutoScale)

   // Note: the order of declaration affects the order in which these are applied
   //       which is important given the nature of how radix, leading zeros,
   //       precision, min and max are related.
   /// Notation used for formatting/editing. Default is fixed.
   ///
   Q_PROPERTY (QNumericEdit::Notations notation     READ getNotation       WRITE setNotation)

   /// Specify radix, default is Decimal.
   ///
   Q_PROPERTY (QEFixedPointRadix::Radicies radix        READ getRadix       WRITE setRadix)

   /// Specify digit 'thousands' separator character, default is none.
   ///
   Q_PROPERTY (QEFixedPointRadix::Separators separator  READ getSeparator   WRITE setSeparator)

   /// Speficies the number of leading zeros. The default is 3.
   /// This is only used if autoScale is false. When autoScale is true the PV's control range is used
   /// to determine the number of required leading zeros.
   /// Stictly speaking, this should be an unsigned int, but designer properties editor much 'nicer' with ints.
   ///
   Q_PROPERTY (int leadingZeros        READ getLeadingZeros     WRITE setLeadingZeros)

   /// Precision used for the display and editing of numbers. The default is 2.
   /// This is only used if autoScale is false. When autoScale is true the PV's precision is used.
   /// Stictly speaking, this should be an unsigned int, but designer properties editor much 'nicer' with ints.
   ///
   Q_PROPERTY (int precision           READ getPrecision        WRITE setPrecision)

   /// Speficies the mimimum allowed value.
   /// This is only used if autoScale is false.
   ///
   Q_PROPERTY (double minimum          READ getMinimum          WRITE setMinimum)

   /// Speficies the maximum allowed value.
   /// This is only used if autoScale is false.
   ///
   Q_PROPERTY (double maximum          READ getMaximum          WRITE setMaximum)

   /// If true (default), add engineering units supplied with the data.
   ///
   Q_PROPERTY (bool addUnits           READ getAddUnits         WRITE setAddUnits)

   /// Sets if this widget automatically writes any changes when it loses focus.
   /// Default is 'false' (does not write any changes when it loses focus).
   ///
   Q_PROPERTY (bool writeOnLoseFocus   READ getWriteOnLoseFocus WRITE setWriteOnLoseFocus)

   /// Sets if this widget writes any changes when the user presses 'enter'.
   /// Note, the current value will be written even if the user has not changed it.
   /// Default is 'true' (writes any changes when the user presses 'enter').
   ///
   Q_PROPERTY (bool writeOnEnter       READ getWriteOnEnter     WRITE setWriteOnEnter)

   /// Sets if this widget writes any changes when the user finished editing (the underlying
   /// QLineEdit 'editingFinished' signal is emitted). No writing occurs if no changes were made.
   /// Default is 'true' (writes any changes when the QLineEdit 'editingFinished' signal is emitted).
   ///
   Q_PROPERTY (bool writeOnFinish      READ getWriteOnFinish    WRITE setWriteOnFinish)

   /// If true the widget writes to the PV as value are changes.
   /// If false (default) a write only occurs when  as per writeOnLoseFocus,
   /// writeOnEnter and/or writeOnFinish values.
   /// Note: writeOnChange and confirmWrite are mutually exclusive.
   ///
   Q_PROPERTY (bool writeOnChange      READ getWriteOnChange    WRITE setWriteOnChange)

   /// Sets if this widget will ask for confirmation (using a dialog box) prior to writing data.
   /// Default is 'false' (will not ask for confirmation (using a dialog box) prior to writing data).
   /// Note: writeOnChange and confirmWrite are mutually exclusive.
   ///
   Q_PROPERTY (bool confirmWrite       READ getConfirmWrite     WRITE setConfirmWrite)

   /// Allow updated while widget has focus - defaults to false.
   ///
   Q_PROPERTY (bool allowFocusUpdate   READ getAllowFocusUpdate WRITE setAllowFocusUpdate)
   //
   // End of QENumericEdit specific properties =========================================

public:
   /// Create without a variable.
   /// Use setVariableNameProperty() and setSubstitutionsProperty() to define a
   /// variable and, optionally, macro substitutions later.
   ///
   explicit QENumericEdit (QWidget* parent = 0);

   /// Create with a variable.
   /// A connection is automatically established.
   /// If macro substitutions are required, create without a variable and set the
   /// variable and macro substitutions after creation.
   ///
   explicit QENumericEdit (const QString& variableName, QWidget* parent = 0);

   /// Destruction
   virtual ~QENumericEdit ();

   // Write the value (of the underlying QNumericEdit object) into the PV immediately.
   //
   void writeNow ();

   double getValue () const;
   void setValue (const double value, const bool isUserUpdate = false);  // as opposed to system update

   // Depricated versions of  getValue/setValue
   //
   double getNumericValue () const;
   void setNumericValue (const double value, const bool isUserUpdate = false);  // as opposed to system update

   // Property set and get functions.
   //
   void setAutoScale (const bool);
   bool getAutoScale () const;

   void setAddUnits (const bool);
   bool getAddUnits () const;

   void setWriteOnLoseFocus (const bool);
   bool getWriteOnLoseFocus () const;

   void setWriteOnEnter (const bool);
   bool getWriteOnEnter () const;

   void setWriteOnFinish (const bool);
   bool getWriteOnFinish () const;

   void setWriteOnChange (const bool);
   bool getWriteOnChange () const;

   void setConfirmWrite (const bool);
   bool getConfirmWrite () const;

   void setAllowFocusUpdate (const bool);
   bool getAllowFocusUpdate () const;

   void setLeadingZeros (const int value);
   int getLeadingZeros () const;

   void setPrecision (const int value);
   int getPrecision () const;

   void setMinimum (const double value);
   double getMinimum () const;

   void setMaximum (const double value);
   double getMaximum () const;

   // Expose access to the internal widget's set/get functions.
   //
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, bool, hasFrame, setFrame)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, Qt::Alignment, alignment, setAlignment)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, QNumericEdit::Notations, getNotation, setNotation)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, QEFixedPointRadix::Radicies, getRadix, setRadix)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, QEFixedPointRadix::Separators, getSeparator, setSeparator)
   QString getCleanText () const { return this->internalWidget->getCleanText (); }

signals:
   // Note, the following signals are common to many QE widgets,
   // if changing the doxygen comments, ensure relevent changes are migrated to all instances
   // These signals are emitted using the QEEmitter::emitDbValueChanged function.
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
   void fontChange (const QFont& font);
   void focusInEvent (QFocusEvent* event);

   // Return the Qt default context menu to add to the QE context menu.
   //
   QMenu* getDefaultContextMenu ();

   // override QEWidget fnctions.
   //
   void activated ();
   void establishConnection (unsigned int variableIndex);
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);

   // Drag and Drop
   void dragEnterEvent (QDragEnterEvent *event) { qcaDragEnterEvent( event ); }
   void dropEvent (QDropEvent *event)           { qcaDropEvent( event ); }
   // Don't drag from interactive widget
   // void mousePressEvent (QMouseEvent *event) { qcaMousePressEvent( event ); }
   // Use default getDrop/setDrop

   // Copy paste
   QString copyVariable ();
   QVariant copyData ();
   void paste (QVariant s);

private:
   void commonSetup ();

   // Calculates and applies auto values.
   //
   void calculateAutoValues ();

   enum WriteOptions { woReturnPressed = 0,
                       woEditingFinished };

   bool isOkayToWrite (const WriteOptions writeOption);

   QNumericEdit* internalWidget;
   QHBoxLayout* layout;         // holds the internal widget - any layout type will do
   QEFloatingFormatting floatingFormatting;

   bool isFirstUpdate;
   bool isConnected;
   bool messageDialogPresent;
   bool isModified;
   double lastValue;

   bool autoScale;
   bool addUnits;
   bool writeOnLoseFocus;
   bool writeOnEnter;
   bool writeOnFinish;
   bool writeOnChange;
   bool confirmWrite;
   bool allowFocusUpdate;

   // If when autoScale set true, we save these values so that when autoScale set false
   // we can reapply them.
   ///
   int designLeadingZeros;
   int designPrecision;
   double designMinimum;
   double designMaximum;

private slots:
   void connectionChanged (QCaConnectionInfo& connectionInfo,
                           const unsigned int &variableIndex);
   void externalValueUpdate (const double& value, QCaAlarmInfo&, QCaDateTime&, const unsigned int&);

   void useNewVariableNameProperty (QString variableNameIn,
                                    QString variableNameSubstitutionsIn,
                                    unsigned int variableIndex);

   void internalValueChanged (const double value);

   void returnPressed ();        // Act on the user pressing return in the widget
   void editingFinished ();      // Act on the user signaling text editing is complete (pressing return)
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
// None
#endif

#endif // QE_NUMERIC_EDIT_H
