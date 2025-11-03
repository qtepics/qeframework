/*  QENumericEdit.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2025 Australian Synchrotron
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
 *    andrews@ansto.gov.au
 */

#ifndef QE_NUMERIC_EDIT_H
#define QE_NUMERIC_EDIT_H

#include <QHBoxLayout>
#include <QPushButton>
#include <QString>
#include <QSize>

#include <QECommon.h>
#include <QEEnums.h>
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
   /// The widget does not allow Automatic
   ///
   Q_PROPERTY (QE::Notations notation     READ getNotation       WRITE setNotation)

   /// Specify radix, default is Decimal.
   ///
   Q_PROPERTY (QE::Radicies radix        READ getRadix       WRITE setRadix)

   /// Specify digit 'thousands' separator character, default is NoSeparator.
   ///
   Q_PROPERTY (QE::Separators separator  READ getSeparator   WRITE setSeparator)

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

   /// When true, there is always a sign charactere "+" or "-".
   /// When false (default), no "+" sign when range of valued non-negative.
   ///
   Q_PROPERTY (bool forceSign        READ getForceSign      WRITE setForceSign)

   /// When set true (default is false) the numeric edit widget will wrap around
   /// from the max/min to min/max value.
   ///
   Q_PROPERTY (bool wrapValue        READ getWrapValue      WRITE setWrapValue)

   /// Speficies the mimimum allowed value.
   /// This is only used if autoScale is false.
   ///
   Q_PROPERTY (double minimum          READ getMinimum          WRITE setMinimum)

   /// Speficies the maximum allowed value.
   /// This is only used if autoScale is false.
   ///
   Q_PROPERTY (double maximum          READ getMaximum          WRITE setMaximum)

   /// If true (default is false), add engineering units supplied with the data.
   ///
   Q_PROPERTY (bool addUnits           READ getAddUnits         WRITE setAddUnits)

   /// Sets if this widget subscribes for data updates and displays current data.
   /// Default is 'true' (subscribes for and displays data updates)
   ///
   Q_PROPERTY (bool subscribe          READ getSubscribe        WRITE setSubscribe)

   /// When set true, internal apply button is exposed and used to trigger write.
   /// All other write are options set false and inhibited from being set true.
   ///
   Q_PROPERTY (bool useApplyButton     READ getUseApplyButton   WRITE setUseApplyButton)

   /// The text displayed on the apply button. This default text is "A".
   ///
   Q_PROPERTY (QString applyButtonText READ getApplyButtonText  WRITE setApplyButtonText)

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

   double getValue () const;
   void setValue (const double value, const bool isUserUpdate = false);  // as opposed to system update

   // Property set and get functions.
   //
   void setAutoScale (const bool);
   bool getAutoScale () const;

   void setAddUnits (const bool);
   bool getAddUnits () const;

   void setUseApplyButton (const bool);
   bool getUseApplyButton () const;

   void setApplyButtonText (const QString& text);
   QString getApplyButtonText () const;

   void setSubscribe (const bool);
   bool getSubscribe () const;

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
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, bool, getWrapValue, setWrapValue)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, bool, getForceSign, setForceSign)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, Qt::Alignment, alignment, setAlignment)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, QE::Notations, getNotation, setNotation)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, QE::Radicies, getRadix, setRadix)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, QE::Separators, getSeparator, setSeparator)

   QString getCleanText () const { return this->internalWidget->getCleanText (); }

signals:
   // Note, the following signals are common to many QE widgets,
   // if changing the doxygen comments, ensure relevent changes are migrated to all instances
   // These signals are emitted using the QEEmitter::emitDbValueChanged function.
   /// Sent when the widget is updated following a data change
   /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
   /// For example a QList widget could log updates from this widget.
   //
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
   //
   void dbConnectionChanged (const bool& isConnected);

public slots:
   // Write the value (of the underlying QNumericEdit object) to the PV immediately.
   //
   void writeNow ();

   /// Update the default style applied to this widget.
   //
   void setDefaultStyle (const QString& style);

   /// Update the widget and write to the main associated control process variable.
   /// Supplied values are converted to the approiate widget value type.
   /// If conversion not possible or resultant value out of range, no widget/PV update occurs.
   //
   void setPvValue (const QString& text);
   void setPvValue (const int value);
   void setPvValue (const double value);
   void setPvValue (const bool value);

protected:
   bool eventFilter (QObject* watched, QEvent* event);
   void focusInEvent (QFocusEvent* event);
   void resizeEvent (QResizeEvent* event);

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
   void setApplyButtonWidth ();

   // Calculates and applies auto values.
   //
   void calculateAutoValues ();

   enum WriteOptions { woReturnPressed = 0,
                       woEditingFinished };

   bool isOkayToWrite (const WriteOptions writeOption);

   QNumericEdit* const internalWidget;
   QPushButton* applyButton;
   QHBoxLayout* layout;         // holds the internal numeric edit and apply button.

   QEFloatingFormatting floatingFormatting;

   bool isConnected;
   bool messageDialogPresent;
   bool isModified;
   double lastValue;

   bool autoScale;
   bool addUnits;
   bool useApplyButton;
   QString applyButtonText;
   bool writeOnLoseFocus;
   bool writeOnEnter;
   bool writeOnFinish;
   bool writeOnChange;
   bool confirmWrite;
   bool allowFocusUpdate;

   // If when autoScale set true, we save these values so that when autoScale
   // is set falsewe can reapply them.
   //
   int designLeadingZeros;
   int designPrecision;
   double designMinimum;
   double designMaximum;

private slots:
   void connectionChanged (QCaConnectionInfo& connectionInfo,
                           const unsigned int &variableIndex);

   void externalValueUpdate (const double& value, QCaAlarmInfo&, 
                             QCaDateTime&, const unsigned int&);

   void useNewVariableNameProperty (QString variableName,
                                    QString variableNameSubstitutions,
                                    unsigned int variableIndex);

   void internalValueChanged (const double value);

   void applyPressed (bool);     // Act on the user pressing the apply button
   void returnPressed ();        // Act on the user pressing return in the widget
   void editingFinished ();      // Act on the user signaling text editing is complete (pressing return)
};

#endif // QE_NUMERIC_EDIT_H
