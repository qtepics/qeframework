/*  QESelector.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2021 Australian Synchrotron
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

#ifndef QE_SELECTOR_H
#define QE_SELECTOR_H

#include <QHBoxLayout>
#include <QComboBox>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QWidget>

#include <QECommon.h>
#include <QEString.h>
#include <QEStringFormatting.h>

#include <QEAbstractWidget.h>
#include <QESingleVariableMethods.h>
#include <QEFrameworkLibraryGlobal.h>

/// This widget allows one on a number of string values to be selected from a
/// pre-dertermined list of strings and the selected valyue written to the
/// associated PV.  The underlying widgert used for this is a QComboBox widget.
/// A typical use case would be selecting a positioner PV name or detector PV
/// name for use with the sscan record.
///
/// A delimiter may also be specified. Only text before the delimiter is written
/// to the PV. In this way a user friendly comment can be added to the cmbo box
/// drop down list. The typlical delimtere would be a space or comma.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QESelector :
      public QEAbstractWidget,
      public QESingleVariableMethods
{
   Q_OBJECT
public:
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

   // Start of QESelector specific properties.
   //
   enum SourceOptions {
      stringListSource,     // use the string list - default
      textFileSource        // use a configuration text file
   };
   Q_ENUMS(SourceOptions)

   Q_PROPERTY (SourceOptions source    READ getSource          WRITE setSource)

   // StringList mode
   Q_PROPERTY (QStringList stringList  READ getStringList      WRITE setStringList)

   // Text File mode
   Q_PROPERTY (QString sourceFilename   READ getSourceFilename  WRITE setSourceFilename)

   enum Delimiters {
      NoDelimiter = 0,
      SpaceDelimiter,
      CommaDelimiter
   };
   Q_ENUMS(Delimiters)

   Q_PROPERTY (int maxVisibleItems   READ maxVisibleItems  WRITE setMaxVisibleItems)

   Q_PROPERTY (Delimiters delimiter  READ  getDelimiter    WRITE setDelimiter)

   /// Sets if this widget subscribes for data updates and displays current data.
   /// Default is 'true' (subscribes for and displays data updates)
   ///
   Q_PROPERTY (bool subscribe        READ getSubscribe     WRITE setSubscribe)

   /// Sets if this widget writes any changes as the user selects values (the
   /// QComboBox 'activated' signal is emitted).  Default is 'true' (writes any
   /// changes when the QComboBox 'activated' signal is emitted).
   ///
   Q_PROPERTY (bool writeOnChange READ getWriteOnChange WRITE setWriteOnChange)

   /// Allow updated while widget has focus - defaults to false.
   ///
   Q_PROPERTY (bool allowFocusUpdate READ getAllowFocusUpdate WRITE setAllowFocusUpdate)
   //
   // end QESelector specific properties.

public:
   explicit QESelector (QWidget* parent = 0);
   explicit QESelector (const QString& variableName, QWidget* parent = 0);
   virtual ~QESelector ();

   void setSource (const SourceOptions infoSource);
   SourceOptions getSource() const;

   void setStringList (const QStringList& userInfoList);
   QStringList getStringList () const;

   void setSourceFilename (const QString& userInfoFile);
   QString getSourceFilename () const;

   void setDelimiter(const Delimiters delimiter);
   Delimiters getDelimiter() const;

   void setSubscribe (const bool subscribe);
   bool getSubscribe () const;

   void setWriteOnChange (const  bool writeOnChange);
   bool getWriteOnChange () const;

   void setAllowFocusUpdate (const bool allowFocusUpdate);
   bool getAllowFocusUpdate () const;

   // Expose access to the internal widget's set/get functions.
   //
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (internalWidget, int, maxVisibleItems, setMaxVisibleItems)

public slots:
   // write the value (of the underlying QComboBox object) into the PV immediately.
   //
   void writeNow ();

signals:
   // Note, the following signals are common to many QE widgets, if changing the
   // doxygen comments, ensure relevent changes are migrated to all instances
   // These signals are emitted using the QEEmitter::emitDbValueChanged function.
   /// Sent when the widget is updated following a data change
   /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
   /// For example a QList widget could log updates from this widget.
   ///
   void dbValueChanged ();                       // signal event
   void dbValueChanged (const QString& out);   // signal as enumeration text
   void dbValueChanged (const int& out);       // signal as int if applicable
   void dbValueChanged (const long& out);      // signal as long if applicable
   void dbValueChanged (const qlonglong& out); // signal as qlonglong if applicable
   void dbValueChanged (const double& out);    // signal as floating if applicable
   void dbValueChanged (const bool& out);      // signal as bool: value != 0 if applicable

   // This signal is emitted using the QEEmitter::emitDbConnectionChanged function.
   /// Sent when the widget state updated following a channel connection change.
   ///
   void dbConnectionChanged (const bool& isConnected);

protected:
   QSize sizeHint () const;
   void focusInEvent (QFocusEvent* event);
   bool eventFilter (QObject* watched, QEvent* event);

   // override QEWidget fnctions.
   //
   void establishConnection (unsigned int variableIndex);
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);

   // Drag and Drop
   void dragEnterEvent (QDragEnterEvent *event) { qcaDragEnterEvent (event); }
   void dropEvent (QDropEvent *event)           { qcaDropEvent (event); }
   void mousePressEvent (QMouseEvent *event)    { qcaMousePressEvent (event); }

   // Use default getDrop/setDrop

   // Copy paste
   QString copyVariable ();
   QVariant copyData ();
   void paste (QVariant v);

private:
   void commonSetup ();
   void updateDropDownList ();
   QString extractValue (const QString& item) const; // uses delimiter to extract value
   static QStringList readList (const QString& filename);

   QComboBox* internalWidget;    // internal combo box widget
   QHBoxLayout* layout;          // holds the interbnal widget - any layout type will do
   QEStringFormatting formatting;

   SourceOptions infoSource;
   QStringList userInfoList;
   QStringList fileInfoList;
   QStringList* infoList;        // reference none of the above
   QString userInfoFile;         // info file name
   Delimiters delimiter;
   bool writeOnChange;           // write value to database when user changes value
   bool isAllowFocusUpdate;
   bool isFirstUpdate;

private slots:
   void connectionChanged (QCaConnectionInfo& connectionInfo,
                           const unsigned int& variableIndex);

   void valueUpdate (const QString& text, QCaAlarmInfo& alarmInfo,
                     QCaDateTime& dateTime, const unsigned int& variableIndex);

   void useNewVariableNameProperty (QString variableName,
                                    QString substitutions,
                                    unsigned int variableIndex);

   void selected (const QString& value);
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QESelector::SourceOptions)
Q_DECLARE_METATYPE (QESelector::Delimiters)
#endif

#endif           // QE_SELECTOR_H
