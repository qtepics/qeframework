/*  QELCDNumber.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2018-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_LCD_NUMBER_H
#define QE_LCD_NUMBER_H

#include <QHBoxLayout>
#include <QString>
#include <QVector>
#include <QLCDNumber>

#include <QECommon.h>
#include <QEEnums.h>
#include <QEAbstractWidget.h>
#include <QEWidget.h>
#include <QEFloating.h>
#include <QEFloatingFormatting.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QESingleVariableMethods.h>
#include <QEStringFormattingMethods.h>

/// This class provides an EPICS aware extention to the QLCDNumber widget.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QELCDNumber :
      public QEAbstractWidget,
      public QESingleVariableMethods,
      public QEStringFormattingMethods
{
   Q_OBJECT

   typedef QEAbstractWidget ParentWidgetClass;

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


   // Begin QE LCD Number specific properties ===========================================
   //
   Q_PROPERTY (bool smallDecimalPoint                READ smallDecimalPoint WRITE setSmallDecimalPoint)
   Q_PROPERTY (QLCDNumber::SegmentStyle segmentStyle READ segmentStyle      WRITE setSegmentStyle)

   // A sub set of the standard string formatting.
   //
   /// Precision used when formatting floating point numbers. The default is 4.
   /// This is only used if useDbPrecision is false.
   Q_PROPERTY (int  precision READ getPrecision WRITE setPrecision)

   /// If true (default), format floating point numbers using the precision supplied with the data.
   /// If false, the precision property is used.
   Q_PROPERTY (bool useDbPrecision READ getUseDbPrecision WRITE setUseDbPrecision)

   /// Notation used for numerical formatting. Default is fixed.
   ///
   Q_PROPERTY (QE::Notations notation READ getNotationProperty WRITE setNotationProperty)
   //
   // End of QE LCD Number specific properties ==========================================

public:
   /// Create without a variable.
   /// Use setVariableNameProperty() and setSubstitutionsProperty() to define a
   /// variable and, optionally, macro substitutions later.
   ///
   explicit QELCDNumber (QWidget* parent = 0);

   /// Create with a variable.
   /// A connection is automatically established.
   /// If macro substitutions are required, create without a variable and set
   /// the variable and macro substitutions after creation.
   ///
   explicit QELCDNumber (const QString& variableName, QWidget *parent = 0);

   /// Destruction
   virtual ~QELCDNumber();

   QSize sizeHint () const;                             ///< Size hint for designer.

   void setNotationProperty (QE::Notations notation);   ///< Access function for #notation property - refer to #notation property for details
   QE::Notations getNotationProperty() const;           ///< Access function for #notation property - refer to #notation property for details

   // Expose access to the internal widget's set/get functions.
   //
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, bool,                     smallDecimalPoint, setSmallDecimalPoint)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, QLCDNumber::SegmentStyle, segmentStyle,      setSegmentStyle)

signals:
   // Note, the following signals are common to many QE widgets,
   // if changing the doxygen comments, ensure relevent changes are migrated to all instances
   // These signals are emitted using the QEEmitter::emitDbValueChanged function.
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

   /// Internal use only. Used when changing a property value to force a
   /// re-display to reflect the new property value.
   ///
   void requestResend();

protected:
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);
   void establishConnection (unsigned int variableIndex);
   void stringFormattingChange() { emit this->requestResend(); }

   // Drag and Drop
   void dragEnterEvent (QDragEnterEvent *event) { this->qcaDragEnterEvent (event); }
   void dropEvent (QDropEvent *event)           { this->qcaDropEvent (event); }
   void mousePressEvent (QMouseEvent *event)    { this->qcaMousePressEvent (event); }

   // Use default setDrop and getDrop

   // Copy paste
   QString copyVariable();
   QVariant copyData();
   void paste (QVariant v);

private:
   void setup();

   QLCDNumber* internalWidget;
   QHBoxLayout* layout;         // holds the internal widget - any layout type will do

   QEFloatingFormatting floatingFormatting;
   bool useDbPrecison;
   QVariant lastValue;

private slots:
   void connectionChanged (QCaConnectionInfo& connectionInfo,
                           const unsigned int& variableIndex);

   void setPvValue (const double& value, QCaAlarmInfo&,
                    QCaDateTime&, const unsigned int&);

   void useNewVariableNameProperty (QString variableNameIn,
                                    QString variableNameSubstitutionsIn,
                                    unsigned int variableIndex);
};

#endif // QE_LCD_NUMBER_H
