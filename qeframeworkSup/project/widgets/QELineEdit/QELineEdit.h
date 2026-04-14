/*  QElineEdit.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_LINE_EDIT_H
#define QE_LINE_EDIT_H

#include <QHBoxLayout>
#include <QPushButton>
#include <QString>
#include <QSize>

#include <QECommon.h>
#include <QEEnums.h>
#include <QEAbstractWidget.h>
#include <QLineEdit>
#include <QEChannel.h>
#include <QEString.h>
#include <QEStringFormatting.h>
#include <QEStringFormattingMethods.h>
#include <QESingleVariableMethods.h>
#include <QEFrameworkLibraryGlobal.h>

// QEAbstractWidget provides all standard QEWidget properties
// QElineEdit holds a single QLineEdit widget
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QELineEdit :
      public QEAbstractWidget,
      public QESingleVariableMethods,
      public QEStringFormattingMethods
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

   // Start of internal QLineEdit properties =========================================
   /// Refer to QLineEdit documentation
   //
   ///
   Q_PROPERTY (QString inputMask        READ inputMask             WRITE setInputMask)

   /// Defines the maximum input text length.
   ///
   Q_PROPERTY (int maxLength            READ maxLength             WRITE setMaxLength)

   /// This property holds whether the numeric edit draws itself with a frame.
   /// If enabled (the default) the numeric edit draws itself inside a frame,
   /// otherwise the line edit draws itself without any frame.
   ///
   Q_PROPERTY (bool frame               READ hasFrame              WRITE setFrame)

   Q_PROPERTY (QLineEdit::EchoMode echoMode READ echoMode          WRITE setEchoMode)

   /// This property holds the alignment of the line edit.
   /// Both horizontal and vertical alignment is allowed here, Qt::AlignJustify will map to Qt::AlignLeft.
   /// By default, this property contains a combination of Qt::AlignLeft and Qt::AlignVCenter.
   ///
   Q_PROPERTY (Qt::Alignment alignment  READ alignment             WRITE setAlignment)

   Q_PROPERTY (bool readOnly            READ isReadOnly            WRITE setReadOnly)

   Q_PROPERTY (QString placeholderText  READ placeholderText       WRITE setPlaceholderText)

   Q_PROPERTY (Qt::CursorMoveStyle cursorMoveStyle READ cursorMoveStyle WRITE setCursorMoveStyle)

   Q_PROPERTY (bool clearButtonEnabled  READ isClearButtonEnabled  WRITE setClearButtonEnabled)
   //
   // End of internal QLineEdit properties ===========================================

   // BEGIN-STRING-FORMATTING-PROPERTIES =============================================
   // String formatting properties
   // These properties should be identical for every widget managing strings.
   // WHEN MAKING CHANGES: Use the update_widget_properties script in the
   // resources directory.
public:
   /// Precision used when formatting floating point numbers. The default is 4.
   /// This is only used if useDbPrecision is false.
   Q_PROPERTY(int  precision READ getPrecision WRITE setPrecision)

   /// If true (default), format floating point numbers using the precision supplied with the data.
   /// If false, the precision property is used.
   Q_PROPERTY(bool useDbPrecision READ getUseDbPrecision WRITE setUseDbPrecision)

   /// The minimum number of leading digits before any decimal/radix point. The defult is 1.
   ///
   Q_PROPERTY(int leadingZeros READ getLeadingZeros WRITE setLeadingZeros)

   /// If true (default), add engineering units supplied with the data.
   ///
   Q_PROPERTY(bool addUnits READ getAddUnits WRITE setAddUnits)

   /// If false (default), no "+" sign, when true always add a sign.
   ///
   Q_PROPERTY(bool forceSign READ getForceSign WRITE setForceSign)

   // NOTE, keep in sync. The documentation below is repeated in
   // QEStringFormatting::setLocalEnumeration() (in QEStringformatting.cpp)
   //
   /// An enumeration list used to data values. Used only when the formatting option is 'local enumeration'.
   /// Value is converted to an integer and used to select a string from this list.
   ///
   /// Format is:
   ///
   ///   [[<|<=|=|!=|>=|>]value1|*] : string1 , [[<|<=|=|!=|>=|>]value2|*] : string2 , [[<|<=|=|!=|>=|>]value3|*] : string3 , ...
   ///
   /// Where:
   ///   <  Less than
   ///   <= Less than or equal
   ///   =  Equal (default if no operator specified)
   ///   >= Greather than or equal
   ///   >  Greater than
   ///   *  Always match (used to specify default text)
   ///
   /// Values may be numeric or textual
   /// Values do not have to be in any order, but first match wins
   /// Values may be quoted
   /// Strings may be quoted
   /// Consecutive values do not have to be present.
   /// Operator is assumed to be equality if not present.
   /// White space is ignored except within quoted strings.
   /// \n may be included in a string to indicate a line break
   ///
   /// Examples are:
   ///
   /// 0:Off,1:On
   /// 0 : "Pump Running", 1 : "Pump not running"
   /// 0:"", 1:"Warning!\nAlarm"
   /// <2:"Value is less than two", =2:"Value is equal to two", >2:"Value is grater than 2"
   /// 3:"Beamline Available", *:""
   /// "Pump Off":"OH NO!, the pump is OFF!","Pump On":"It's OK, the pump is on"
   ///
   /// The data value is converted to a string if no enumeration for that value is available.
   /// For example, if the local enumeration is '0:off,1:on', and a value of 10 is processed, the text generated is '10'.
   /// If a blank string is required, this should be explicit. for example, '0:off,1:on,10:""'
   ///
   /// A range of numbers can be covered by a pair of values as in the following example: >=4:"Between 4 and 8",<=8:"Between 4 and 8"
   Q_PROPERTY(QString/*localEnumerationList*/ localEnumeration READ getLocalEnumeration WRITE setLocalEnumeration)

   /// Format to apply to data. Default is 'Default' in which case the data type supplied with the data determines how the data is formatted.
   /// For all other options, an attempt is made to format the data as requested (whatever its native form).
   Q_PROPERTY(QE::Formats format READ getFormatProperty WRITE setFormatProperty)

   void setFormatProperty (QE::Formats format) { this->setFormat(format); }  ///< Access function for #format property - refer to #format property for details
   QE::Formats getFormatProperty() const { return this->getFormat(); }       ///< Access function for #format property - refer to #format property for details

   /// Base used for when formatting integers. Default is 10 (duh!)
   ///
   Q_PROPERTY(int radix READ getRadix WRITE setRadix)

   /// Seperators used for interger and fixed point formatting. Default is None.
   ///
   Q_PROPERTY(QE::Separators separator READ getSeparator WRITE setSeparator)

   /// Notation used for numerical formatting. Default is fixed.
   ///
   Q_PROPERTY(QE::Notations notation READ getNotation WRITE setNotation)

   /// Text formatting option for array data. Default is ASCII. Options are:
   /// \li ASCII - treat array as a single text string. For example an array of three characters 'a' 'b' 'c' will be formatted as 'abc'.
   /// \li APPEND - treat array as an array of numbers and format a string containing them all with a space between each.
   ///              For example, an array of three numbers 10, 11 and 12 will be formatted as '10 11 12'.
   /// \li INDEX - Extract a single item from the array. The item is then formatted as any other non array data would be.
   ///             The item selected is determined by the arrayIndex property. For example, if arrayIndex property is 1,
   ///             an array of three numbers 10, 11 and 12 will be formatted as '11'.
   ///
   Q_PROPERTY(QE::ArrayActions arrayAction READ getArrayAction WRITE setArrayAction)

public:
   // END-STRING-FORMATTING-PROPERTIES ===============================================

   // QElineEdit specific properties ==================================================
   //
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

   /// Sets if this widget will ask for confirmation (using a dialog box) prior to writing data.
   /// Default is 'false' (will not ask for confirmation (using a dialog box) prior to writing data).
   /// Note: writeOnChange and confirmWrite are mutually exclusive.
   ///
   Q_PROPERTY (bool confirmWrite       READ getConfirmWrite     WRITE setConfirmWrite)

   /// Allow updated while widget has focus - defaults to false.
   ///
   Q_PROPERTY (bool allowFocusUpdate   READ getAllowFocusUpdate WRITE setAllowFocusUpdate)

   /// Allow drag/drops mode to this widget.
   //  Option: DropToVariable [default] - the dropped text will be used as a new variable name.
   /// Option: DropToText - just set text on the widget
   /// Option: DropToTextAndWrite - write the text to the PV if the PV has been connected otherwise equivalent to DropToText
   ///
   Q_PROPERTY (QE::DropOption dropOption READ getDropOption WRITE setDropOption)

   //
   // End of QElineEdit specific properties ===========================================

public:
   /// Create without a variable.
   /// Use setVariableNameProperty() and setSubstitutionsProperty() to define a
   /// variable and, optionally, macro substitutions later.
   ///
   explicit QELineEdit (QWidget* parent = 0);

   /// Create with a variable.
   /// A connection is automatically established.
   /// If macro substitutions are required, create without a variable and set the
   /// variable and macro substitutions after creation.
   ///
   explicit QELineEdit (const QString& variableName, QWidget* parent = 0);

   /// Destruction
   virtual ~QELineEdit ();

   QString getValue () const;
   void setValue (const QString& value, const bool isUserUpdate = false);  // as opposed to system update

   // Property set and get functions.
   //
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

   void setConfirmWrite (const bool);
   bool getConfirmWrite () const;

   void setAllowFocusUpdate (const bool);
   bool getAllowFocusUpdate () const;

   void setDropOption (QE::DropOption option);
   QE::DropOption getDropOption () const;

   // Expose access to the internal widget's property set/get functions.
   //
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, QString, text, setText)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, QString, inputMask, setInputMask)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, int, maxLength, setMaxLength)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, bool, hasFrame, setFrame)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, QLineEdit::EchoMode, echoMode, setEchoMode)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, Qt::Alignment, alignment, setAlignment)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, bool, isReadOnly, setReadOnly)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, QString, placeholderText, setPlaceholderText)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, Qt::CursorMoveStyle, cursorMoveStyle, setCursorMoveStyle)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, bool, isClearButtonEnabled, setClearButtonEnabled)

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

   /// Internal use only. Used by QEConfiguredLayout to be notified when one
   /// of its widgets has written something.
   // Signal a user attempt to change a value.
   // Values are strings as the user sees them.
   //
   void userChange( const QString& oldValue, const QString& newValue, const QString& lastValue );

   /// Internal use only. Used when changing a property value to force a
   /// re-display to reflect the new property value.
   //
   void requestResend();

   /// From interbal widget
   //
   void textChanged( const QString& );
   void textEdited( const QString& );

public slots:
   // Write the value (of the underlying QLineEdit object) to the PV immediately.
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

   void stringFormattingChange();

   // override QEWidget fnctions.
   //
   void activated ();
   void establishConnection (unsigned int variableIndex);
   QEChannel* createQcaItem (unsigned int variableIndex);

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

   enum WriteOptions {
       woReturnPressed = 0,
       woEditingFinished
   };

   bool isOkayToWrite (const WriteOptions writeOption);

   QLineEdit* const internalWidget;
   QPushButton* applyButton;
   QHBoxLayout* layout;         // holds the internal numeric edit and apply button.

   bool isConnected;
   bool messageDialogPresent;
   bool isModified;
   QString lastValue;

   bool useApplyButton;
   QString applyButtonText;
   bool writeOnLoseFocus;
   bool writeOnEnter;
   bool writeOnFinish;
   bool confirmWrite;
   bool allowFocusUpdate;
   QE::DropOption dropOption;

private slots:
   void connectionUpdated   (const QEConnectionUpdate& update);
   void externalValueUpdate (const QEStringValueUpdate& update);
   void usePvNameProperties (const QEPvNameProperties& properties);
   void internalValueEdited (const QString& text);

   void applyPressed (bool);     // Act on the user pressing the apply button
   void returnPressed ();        // Act on the user pressing return in the widget
   void editingFinished ();      // Act on the user signaling text editing is complete (pressing return)
};

#endif   // QE_LINE_EDIT_H
