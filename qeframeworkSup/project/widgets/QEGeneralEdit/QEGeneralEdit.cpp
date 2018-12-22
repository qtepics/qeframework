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
 *  Copyright (c) 2014,2016,2017 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include "QEGeneralEdit.h"

// In this context, we are a user of the QEFramework library as we are using
// the moc generated ui_QEGeneralEdit.h file that needs access the meta
// types to be declared.
//
#define QE_DECLARE_METATYPE_IS_REQUIRED

#include <QDebug>
#include <QECommon.h>
#include <QEScaling.h>

#include <QELabel.h>
#include <QELineEdit.h>
#include <QENumericEdit.h>
#include <ui_QEGeneralEdit.h>

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
   this->setVariableName (variableNameIn, PV_VARIABLE_INDEX);
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
   this->ui = new Ui::General_Edit_Form ();
   this->ui->setupUi (this);

   // Clear design time styles.
   //
   this->ui->numericEditPanel->setStyleSheet("");
   this->ui->stringEditPanel->setStyleSheet("");

   // Set two of three edit modes invisible.
   //
   this->ui->numericEditPanel->setVisible (false);
   this->ui->enumerationEditPanel->setVisible (false);

   // No apply buttons by default.
   //
   this->setUseApplyButton (false);

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

   // Apply button connections (always connected even when not in use).
   //
   QObject::connect (this->ui->numericEditApplyButton, SIGNAL (clicked (bool)),
                     this, SLOT (onNumericEditApply (bool)));

   QObject::connect (this->ui->stringEditApplyButton, SIGNAL (clicked (bool)),
                     this, SLOT (onStringEditApply (bool)));

   // Updated for adjusting the leading zeros and precision of the numeric edit widget.
   //
   QObject::connect (this->ui->zerosEdit, SIGNAL (valueChanged (const int)),
                     this, SLOT (onZerosValueChanged (const int)));

   QObject::connect (this->ui->precisionEdit, SIGNAL (valueChanged (const int)),
                    this, SLOT (onPrecisionValueChanged (const int)));
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
   this->ui->valueLabel->setArrayIndex (arrayIndex);
   this->ui->numericEditWidget->setArrayIndex (arrayIndex);
   this->ui->radioGroupWidget->setArrayIndex (arrayIndex);
   this->ui->stringEditWidget->setArrayIndex (arrayIndex);
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. A QCaObject that streams integers is required.
//
qcaobject::QCaObject* QEGeneralEdit::createQcaItem (unsigned int variableIndex)
{
   qcaobject::QCaObject * result = NULL;
   QString pvName;

   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return NULL;
   }

   pvName = this->getSubstitutedVariableName (0).trimmed ();
   this->ui->pvNameLabel->setText (pvName);

   // We create a generic connection here as opposed to a QEInteger or QEFloating etc.
   //
   result = new qcaobject::QCaObject (pvName, this, variableIndex);

   // Apply currently defined array index/elements request values.
   //
   this->setSingleVariableQCaProperties (result);

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
   if (variableIndex != PV_VARIABLE_INDEX) {
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
                        this, SLOT   (connectionChanged (QCaConnectionInfo &, const unsigned int& )));

      QObject::connect (qca,SIGNAL (dataChanged (const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& )),
                        this, SLOT (dataChanged (const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& )));
   }
}

//------------------------------------------------------------------------------
// Act on a connection change - modify the tool tip
// We don't chage the style - the inner widgets can to that.
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QEGeneralEdit::connectionChanged (QCaConnectionInfo& connectionInfo,
                                       const unsigned int& variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Note the connected state
   //
   bool isConnected = connectionInfo.isChannelConnected ();

   // Display the connected state
   //
   this->updateToolTipConnection (isConnected);

   this->isFirstUpdate = true;

   // Signal channel connection change to any (Link) widgets.
   // using signal dbConnectionChanged.
   //
   this->emitDbConnectionChanged (PV_VARIABLE_INDEX);
}

//-----------------------------------------------------------------------------
//
void QEGeneralEdit::dataChanged (const QVariant& value, QCaAlarmInfo& alarmInfo,
                                 QCaDateTime&, const unsigned int& variableIndex)

{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   qcaobject::QCaObject* qca = this->getQcaItem (0);

   if (qca && this->isFirstUpdate) {
      const QString pvName = this->getSubstitutedVariableName (0).trimmed ();

      this->ui->valueLabel->setVariableNameAndSubstitutions (pvName, "", 0);
      this->ui->valueLabel->activate ();

      // Clear all three optional edit widgets.
      //
      this->ui->numericEditPanel->setVisible (false);
      this->ui->enumerationEditPanel->setVisible (false);
      this->ui->stringEditPanel->setVisible (false);

      this->ui->numericEditWidget->setVariableNameAndSubstitutions ("", "", 0);
      this->ui->radioGroupWidget->setVariableNameAndSubstitutions ("", "", 0);
      this->ui->stringEditWidget->setVariableNameAndSubstitutions ("", "", 0);

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

      // Use data type to figure out which type of editing widget is most
      // appropriate.
      //
      QWidget* useThisWidget = NULL;
      QWidget* inThisPanel = NULL;
      int numElements = 0;

      switch (type) {

         case QVariant::String:
            useThisWidget = this->ui->stringEditWidget;
            inThisPanel = this->ui->stringEditPanel;
            break;

         case QVariant::Int:
         case QVariant::UInt:
         case QVariant::LongLong:
         case QVariant::ULongLong:
            numElements = qca->getEnumerations().count();
            if (numElements > 0) {
               // represents an enumeration.
               //
               const int numRows = (numElements + 3) / 4;
               const int rowHgt = (this->getButtonStyle () == QRadioGroup::Radio) ? 24 : 28;
               const int minh = (numRows * QEScaling::scale (rowHgt)) + QEScaling::scale (24);
               this->ui->enumerationEditPanel->setMinimumHeight (minh);

               useThisWidget = this->ui->radioGroupWidget;
               inThisPanel = this->ui->enumerationEditPanel;
            } else {
               // basic integer
               //
               useThisWidget = this->ui->numericEditWidget;
               inThisPanel = this->ui->numericEditPanel;
            }
            break;

         case QVariant::Double:
            {
               int p = qca->getPrecision();

               this->ui->precisionEdit->setValue (p);

               double t = MAX (ABS (qca->getControlLimitLower ()),
                               ABS (qca->getControlLimitUpper ()));
               if (t == 0.0) {
                  t = MAX (ABS (qca->getDisplayLimitLower ()),
                           ABS (qca->getDisplayLimitUpper ()));
               }
               t = MAX (t, 1);
               int z = 1 + int (LOG10 (t));  // the (int) cast truncates to 0
               this->ui->zerosEdit->setValue (z);

               this->ui->numericEditWidget->setPrecision(p);
               this->ui->numericEditWidget->setLeadingZeros (z);

               useThisWidget = this->ui->numericEditWidget;
               inThisPanel = this->ui->numericEditPanel;
            }
            break;

         default:
            DEBUG << " Unexpected type:" << type;
            return; // do nothing
      }

      QEWidget* qeWidget = dynamic_cast <QEWidget*> (useThisWidget);
      if (inThisPanel && qeWidget) {

         inThisPanel->setVisible (true);
         qeWidget->setVariableNameAndSubstitutions (pvName, "", 0);
         qeWidget->activate ();

         int newHeight =
               this->ui->pvNameLabel->minimumHeight () +
               this->ui->valueLabel->minimumHeight () +
               inThisPanel->minimumHeight () +
               QEScaling::scale (20);

         int newWidth =
               MAX (this->ui->pvNameLabel->minimumWidth (),
                    inThisPanel->minimumWidth ()) +
               QEScaling::scale (20);

         this->setMinimumSize (newWidth, newHeight);
         this->setMaximumSize (QWIDGETSIZE_MAX, newHeight);
      }

      this->isFirstUpdate = false;
   }

   // Invoke common alarm handling processing.
   //
   this->processAlarmInfo (alarmInfo);

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged.
   //
   this->emitDbValueChanged (PV_VARIABLE_INDEX);
}

//------------------------------------------------------------------------------
//
void QEGeneralEdit::onStringEditApply (bool)
{
   this->ui->stringEditWidget->writeNow ();
}

//------------------------------------------------------------------------------
//
void QEGeneralEdit::onNumericEditApply (bool)
{
   this->ui->numericEditWidget->writeNow ();
}

//------------------------------------------------------------------------------
//
void QEGeneralEdit::onZerosValueChanged (const int value)
{
   this->ui->numericEditWidget->setAutoScale (false);
   this->ui->numericEditWidget->setLeadingZeros (value);
   this->ui->precisionEdit->setValue (MIN (15 - value, this->ui->precisionEdit->getValue ()));

   // The min and max will get limited by the current precision/leading zeros value.
   this->ui->numericEditWidget->setMinimum (-1.0E15);
   this->ui->numericEditWidget->setMaximum (+1.0E15);
}

//------------------------------------------------------------------------------
//
void QEGeneralEdit::onPrecisionValueChanged (const int value)
{
   this->ui->numericEditWidget->setAutoScale (false);
   this->ui->numericEditWidget->setPrecision (value);
   this->ui->zerosEdit->setValue (MIN (15 - value, this->ui->zerosEdit->getValue ()));

   // The min and max will get limited by the current precision/leading zeros value.
   this->ui->numericEditWidget->setMinimum (-1.0E15);
   this->ui->numericEditWidget->setMaximum (+1.0E15);
}

//==============================================================================
// Properties
// Update variable name etc.
//
void QEGeneralEdit::useNewVariableNameProperty (QString variableNameIn,
                                                QString substitutionsIn,
                                                unsigned int variableIndex)
{
   this->setVariableNameAndSubstitutions (variableNameIn, substitutionsIn, variableIndex);
}

//------------------------------------------------------------------------------
//
void QEGeneralEdit::setButtonStyle (const QRadioGroup::ButtonStyles style)
{
   this->ui->radioGroupWidget->setButtonStyle (style);
}

//------------------------------------------------------------------------------
//
QRadioGroup::ButtonStyles QEGeneralEdit::getButtonStyle () const
{
   return this->ui->radioGroupWidget->getButtonStyle ();
}

//------------------------------------------------------------------------------
//
void QEGeneralEdit::setButtonOrder (const QRadioGroup::ButtonOrders order)
{
   this->ui->radioGroupWidget->setButtonOrder (order);
}

//------------------------------------------------------------------------------
//
QRadioGroup::ButtonOrders QEGeneralEdit::getButtonOrder () const
{
   return this->ui->radioGroupWidget->getButtonOrder ();
}

//------------------------------------------------------------------------------
//
void QEGeneralEdit::setUseApplyButton (const bool useApplyButtonIn)
{
   this->useApplyButton = useApplyButtonIn;

   this->ui->numericEditApplyButton->setVisible (this->useApplyButton);
   this->ui->numericEditWidget->setWriteOnEnter (!this->useApplyButton);
   this->ui->numericEditWidget->setWriteOnFinish (!this->useApplyButton);

   this->ui->stringEditApplyButton->setVisible (this->useApplyButton);
   this->ui->stringEditWidget->setWriteOnEnter (!this->useApplyButton);
   this->ui->stringEditWidget->setWriteOnFinish (!this->useApplyButton);
}

//------------------------------------------------------------------------------
//
bool QEGeneralEdit::getUseApplyButton () const
{
   return this->useApplyButton;
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
   this->setVariableName (v.toString (), 0);
   this->establishConnection (0);
}

// end
