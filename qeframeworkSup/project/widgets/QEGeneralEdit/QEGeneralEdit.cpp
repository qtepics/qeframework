/*  QEGeneralEdit.cpp
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
 *  Copyright (c) 2014,2016Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QDebug>
#include <QECommon.h>
#include <QEScaling.h>

#include "QEGeneralEdit.h"

#define DEBUG qDebug () << "QEGeneralEdit" << __LINE__ << __FUNCTION__ << "  "

#define PV_VARIABLE_INDEX      0

//-----------------------------------------------------------------------------
// Constructor with no initialisation
//
QEGeneralEdit::QEGeneralEdit (QWidget* parent) :
   QEFrame (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)

{
   this->commonSetup ();
}

//-----------------------------------------------------------------------------
// Constructor with known variable
//
QEGeneralEdit::QEGeneralEdit (const QString & variableNameIn,
                              QWidget* parent) :
   QEFrame (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)
{
   this->commonSetup ();
   this->setVariableName (variableNameIn, 0);
   this->activate ();
}

//---------------------------------------------------------------------------------
//
QSize QEGeneralEdit::sizeHint () const
{
   return QSize (430, 80);
}

//-----------------------------------------------------------------------------
// Setup common to all constructors
//
void QEGeneralEdit::commonSetup ()
{
   // Configure the panel.
   //
   this->setFrameShape (QFrame::Panel);
   this->setFrameShadow (QFrame::Plain);

   // QEFrame sets this to false (as it's not an EPICS aware widget).
   // But the QEGeneralEdit is EPICS aware, so set default to true.
   //
   this->setVariableAsToolTip (true);

   // Set up data
   // This control uses a single data source.
   //
   this->setNumVariables (1);

   // Set up default properties
   //
   this->setAllowDrop (true);
   this->setDisplayAlarmState (false);

   this->createInternalWidgets ();

   this->setMinimumWidth (400);
   this->setMinimumHeight (50);

   // Use default context menu.
   //
   this->setupContextMenu ();

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   this->connectNewVariableNameProperty (SLOT (useNewVariableNameProperty (QString, QString, unsigned int)));
}

//------------------------------------------------------------------------------
//
void QEGeneralEdit::createInternalWidgets ()
{
   this->verticalLayout = new QVBoxLayout (this);
   this->verticalLayout->setMargin (4);
   this->verticalLayout->setSpacing (4);

   this->pvNameLabel = new QLabel (this);
   this->pvNameLabel->setAlignment (Qt::AlignHCenter);
   this->pvNameLabel->setMinimumWidth (320);
   this->pvNameLabel->setMinimumHeight (17);
   this->pvNameLabel->setMaximumHeight (17);
   this->verticalLayout->addWidget (pvNameLabel);

   this->valueLabel = new QELabel (this);
   this->valueLabel->setArrayAction (QEStringFormatting::INDEX);
   this->valueLabel->setFrameShape (QFrame::Panel);
   this->valueLabel->setFrameShadow (QFrame::Plain);
   this->valueLabel->setMinimumHeight (19);
   this->valueLabel->setMaximumHeight (19);
   this->verticalLayout->addWidget (valueLabel);

   this->numericEditWidget = new QENumericEdit (this);
   this->numericEditWidget->setAddUnits (false);
   this->numericEditWidget->setMinimumSize (200, 23);
   this->verticalLayout->addWidget (numericEditWidget);

   this->radioGroupPanel = new QERadioGroup ("", "", this);
   this->radioGroupPanel->setMinimumSize (QSize (412, 23));
   this->radioGroupPanel->setColumns (3);
   this->verticalLayout->addWidget (this->radioGroupPanel);

   this->stringEditWidget = new QELineEdit (this);
   this->stringEditWidget->setMinimumSize (342, 23);
   this->verticalLayout->addWidget (this->stringEditWidget);

   this->numericEditWidget->setVisible (false);
   this->radioGroupPanel->setVisible (false);
   this->stringEditWidget->setVisible (true);  // allow one (at least at design time)
}

//------------------------------------------------------------------------------
//
void QEGeneralEdit::setArrayIndex (const int arrayIndex)
{
   // First call parent function.
   //
   QESingleVariableMethods::setArrayIndex (arrayIndex);

   // And then apply to each internal widget
   //
   this->valueLabel->setArrayIndex (arrayIndex);
   this->numericEditWidget->setArrayIndex (arrayIndex);
   this->radioGroupPanel->setArrayIndex (arrayIndex);
   this->stringEditWidget->setArrayIndex (arrayIndex);
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. A QCaObject that streams integers is required.
//
qcaobject::QCaObject*  QEGeneralEdit::createQcaItem (unsigned int variableIndex)
{
   qcaobject::QCaObject * result = NULL;
   QString pvName;

   if (variableIndex != 0) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return NULL;
   }

   pvName = this->getSubstitutedVariableName (0).trimmed ();
   this->pvNameLabel->setText (pvName);

   result = new qcaobject::QCaObject (pvName, this, variableIndex);

   // Apply currently defined array index.
   //
   this->setQCaArrayIndex (result);

   return result;
}


//------------------------------------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QEGeneralEdit::establishConnection (unsigned int variableIndex)
{
   if (variableIndex != 0) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject* qca = createConnection (variableIndex);

   // If a QCaObject object is now available to supply data update signals, connect it to the appropriate slots.
   //
   if (qca) {
      QObject::connect (qca,  SIGNAL (connectionChanged (QCaConnectionInfo &, const unsigned int& )),
                        this, SLOT  (connectionChanged (QCaConnectionInfo &, const unsigned int& )));

      QObject::connect (qca,SIGNAL (dataChanged (const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& )),
                        this, SLOT (dataChanged (const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& )));
   }
}

//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the s looks and change the tool tip
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QEGeneralEdit::connectionChanged (QCaConnectionInfo& connectionInfo, const unsigned int& )
{
   // Note the connected state
   //
   bool isConnected = connectionInfo.isChannelConnected ();

   // Display the connected state
   //
   updateToolTipConnection (isConnected);
   updateConnectionStyle (isConnected);

   this->isFirstUpdate = true;
}

//-----------------------------------------------------------------------------
//
void QEGeneralEdit::dataChanged (const QVariant& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& )

{
   qcaobject::QCaObject* qca = this->getQcaItem (0);

   if (qca && this->isFirstUpdate) {
      QWidget* useThisWidget = NULL;
      QString pvName;
      int numElements = 0;

      pvName = this->getSubstitutedVariableName (0).trimmed ();
      this->valueLabel->setVariableNameAndSubstitutions (pvName, "", 0);

      // Clear all three optional edit widgets.
      //
      // Can't use regular widget setVisible as this gets inter-twinggled with
      // the visibility set by the setting user level.
      //
      this->numericEditWidget->setRunVisible (false);
      this->radioGroupPanel->setRunVisible (false);
      this->stringEditWidget->setRunVisible (false);

      this->numericEditWidget->setVariableNameAndSubstitutions ("", "", 0);
      this->radioGroupPanel->setVariableNameAndSubstitutions ("", "", 0);
      this->stringEditWidget->setVariableNameAndSubstitutions ("", "", 0);

      QVariant workingValue = value;
      QVariant::Type type = workingValue.type ();

      if (type == QVariant::List) {
         int ai = this->getArrayIndex();
         if (ai >= 0 && ai < value.toList().count() ) {
            // Convert this array element as a scalar update.
            //
            workingValue =value.toList().value(ai);
            type = workingValue.type ();
         } else {
            DEBUG << " Array index out of bounds:" << ai;
            return; // do nothing
         }
      }

      // Use data type to figure out which type of editting widget is most
      // appropriate.
      //
      switch (type) {
         case QVariant::String:
            useThisWidget = this->stringEditWidget;
            break;

         case QVariant::Int:
         case QVariant::UInt:
         case QVariant::LongLong:
         case QVariant::ULongLong:
            numElements = qca->getEnumerations().count();
            if (numElements > 0) {
               int numRows;

               // represents an enumeration.
               //
               numRows = (numElements + 2) / 3;
               this->radioGroupPanel->setMinimumHeight ((numRows + 2) *  QEScaling::scale (20));

               useThisWidget = this->radioGroupPanel;
            } else {
               // basic integer
               //
               useThisWidget = this->numericEditWidget;

            }
            break;
\
         case QVariant::Double:
            useThisWidget = this->numericEditWidget;
            break;

         default:
            DEBUG << " Unexpected type:" << type;
            return; // do nothing
      }

      QEWidget* qeWidget = dynamic_cast <QEWidget*> (useThisWidget);
      if (useThisWidget && qeWidget) {

         qeWidget->setRunVisible (true);
         qeWidget->setVariableNameAndSubstitutions (pvName, "", 0);

         int newHeight =
               this->pvNameLabel->minimumHeight () +
               this->valueLabel->minimumHeight () +
               useThisWidget->minimumHeight () +
               QEScaling::scale (40);

         int newWidth =
               MAX (this->pvNameLabel->minimumWidth (),
                    useThisWidget->minimumWidth ()) +
               QEScaling::scale (20);

         this->setMinimumSize (newWidth, newHeight);
         this->setMaximumSize (QWIDGETSIZE_MAX, newHeight);
      }

      this->isFirstUpdate = false;
   }

   // Invoke common alarm handling processing.
   //
   this->processAlarmInfo (alarmInfo);
}


//==============================================================================
// Properties
// Update variable name etc.
//
void QEGeneralEdit::useNewVariableNameProperty (QString variableNameIn,
                                                QString variableNameSubstitutionsIn,
                                                unsigned int variableIndex)
{
   this->setVariableNameAndSubstitutions (variableNameIn,
                                          variableNameSubstitutionsIn,
                                          variableIndex);
}


//==============================================================================
// Drag drop
//
void QEGeneralEdit::setDrop (QVariant drop)
{
   this->setVariableName (drop.toString (), 0);
   this->establishConnection (0);
}

//------------------------------------------------------------------------------
//
QVariant QEGeneralEdit::getDrop ()
{
   QVariant result;

   if (this->isDraggingVariable ()) {
      result = QVariant (this->copyVariable ());
   } else {
      result = this->copyData ();
   }
   return result;
}

//==============================================================================
// Copy / Paste
//
QString QEGeneralEdit::copyVariable ()
{
   return getSubstitutedVariableName (0);
}

//------------------------------------------------------------------------------
//
QVariant QEGeneralEdit::copyData ()
{
   return QVariant ();
}

//------------------------------------------------------------------------------
//
void QEGeneralEdit::paste (QVariant v)
{
   if (this->getAllowDrop ()) {
      this->setDrop (v);
   }
}

// end
