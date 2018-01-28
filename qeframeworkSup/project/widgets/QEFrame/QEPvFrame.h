/*  QEPvFrame.h
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
 *  Copyright (c) 2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_PV_FRAME_H
#define QE_PV_FRAME_H

#include <QString>
#include <QWidget>
#include <QVariant>

#include <QEInteger.h>
#include <QEIntegerFormatting.h>
#include <QESingleVariableMethods.h>

#include <QEFrame.h>
#include <QEFrameworkLibraryGlobal.h>

/// The QEPvFrame class provides an extension to the QEFrame class in that it allows
/// the value of a nominated PV to select one of 8 pixmaps to use as frame background.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEPvFrame :
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

public:
   explicit QEPvFrame (QWidget * parent = 0);
   explicit QEPvFrame (const QString & variableName, QWidget * parent = 0);
   virtual ~QEPvFrame ();

signals:
   // Note, the following signals are common to many QE widgets,
   // if changing the doxygen comments, ensure relevent changes are migrated to all instances
   // These signals are emitted using the QEEmitter::emitDbValueChanged function.
   // This signal emiited as a result of changes to the primary PV variable only,
   // i.e. excludes the edgeVariable.
   /// Sent when the widget is updated following a data change
   /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
   /// For example a QList widget could log updates from this widget.
   void dbValueChanged ();                      // signal event
   void dbValueChanged (const QString& out);    // signal as formatted text
   void dbValueChanged (const int& out);        // signal as int if applicable
   void dbValueChanged (const long& out);       // signal as long if applicable
   void dbValueChanged (const qlonglong& out);  // signal as qlonglong if applicable
   void dbValueChanged (const double& out);     // signal as floating if applicable
   void dbValueChanged (const bool& out);       // signal as bool: value != 0 if applicable

   // This signal is emitted using the QEEmitter::emitDbConnectionChanged function.
   /// Sent when the widget state updated following a channel connection change
   /// Applied to provary varible.
   void dbConnectionChanged (const bool & isConnected);

protected:
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);
   void establishConnection (unsigned int variableIndex);

   // No drag/drop

   // Copy / no paste
   QString copyVariable ();
   QVariant copyData ();

private:
   void commonSetup ();

   QEIntegerFormatting integerFormatting;

private slots:
   void useNewVariableNameProperty (QString variableName,
                                    QString variableNameSubstitutions,
                                    unsigned int variableIndex);

   void connectionChanged (QCaConnectionInfo & connectionInfo,
                           const unsigned int &variableIndex);

   void pvValueUpdate (const long &value, QCaAlarmInfo & alarmInfo,
                       QCaDateTime & dateTime, const unsigned int &variableIndex);
};

#endif                          // QE_PV_FRAME_H
