/*  QENumericEdit.cpp
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
 *  Copyright (c) 2013,2014,2016 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <math.h>
#include <QDebug>
#include <QMessageBox>

#include <QECommon.h>
#include "QENumericEdit.h"

#define DEBUG qDebug () << "QENumericEdit" << __LINE__ << __FUNCTION__  << "  "

#define PV_VARIABLE_INDEX      0

//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QENumericEdit::QENumericEdit (QWidget* parent) :
   QEAbstractWidget (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)
{
   this->commonSetup ();
}

//------------------------------------------------------------------------------
// Constructor with known variable
//
QENumericEdit::QENumericEdit (const QString& variableNameIn, QWidget* parent) :
   QEAbstractWidget (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)
{
   this->commonSetup ();
   this->setVariableName (variableNameIn, PV_VARIABLE_INDEX);
   this->activate ();
}

//------------------------------------------------------------------------------
// Setup common to all constructors
//
void QENumericEdit::commonSetup ()
{
   // Create internal widget.
   //
   this->internalWidget = new QNumericEdit (this);

   // Copy actual widget size policy to the containing widget, then ensure
   // internal widget will expand to fill container widget.
   //
   this->setSizePolicy (this->internalWidget->sizePolicy ());
   this->internalWidget->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);

   this->layout = new QHBoxLayout (this);
   this->layout->setMargin (0);    // extact fit.
   this->layout->addWidget (this->internalWidget);

   this->setMinimumSize (this->internalWidget->minimumSize ());
   this->internalWidget->setPrecision (2);
   this->internalWidget->setLeadingZeros (3);

   // QNumericEdit signals
   //
   QObject::connect (this->internalWidget, SIGNAL (valueChanged         (const double)),
                     this,                 SLOT   (internalValueChanged (const double)));
   QObject::connect (this->internalWidget, SIGNAL (returnPressed ()),
                     this,                 SLOT   (returnPressed ()));
   QObject::connect (this->internalWidget, SIGNAL (editingFinished ()),
                     this,                 SLOT   (editingFinished ()));

   // Clear flags and ensure valid last value stored.
   //
   this->isFirstUpdate = false;
   this->isConnected = false;
   this->messageDialogPresent = false;
   this->isModified = false;
   this->lastValue = 0.0;

   // Set default property values
   //
   this->addUnits = true;
   this->writeOnLoseFocus = false;
   this->writeOnEnter = true;
   this->writeOnFinish = true;
   this->writeOnChange = false;
   this->confirmWrite = false;
   this->allowFocusUpdate = false;

   // Initialise design values based on internal widget values.
   // These are cached separetly from the internal QNumeric widget to
   // support autoScale on/off transitions.
   //
   this->designLeadingZeros = this->internalWidget->getLeadingZeros ();
   this->designPrecision = this->internalWidget->getPrecision ();
   this->designMinimum = this->internalWidget->getMinimum ();
   this->designMaximum = this->internalWidget->getMaximum ();

   this->autoScale = false;    // force set true functionality
   this->setAutoScale (true);

   // Set up data
   // This control uses a single data source
   //
   this->setNumVariables (1);

   // Set variable index used to select write access cursor style.
   //
   this->setControlPV (PV_VARIABLE_INDEX);

   // Set up default properties
   //
   this->setVariableAsToolTip (true);
   this->setAllowDrop (false);
   this->setDisplayAlarmState (true);

   // Use default context menu.
   // Note: Turn off the menu for the internal QNumericEdit and QLineEdit widgets.
   //
   this->internalWidget->lineEdit->setContextMenuPolicy (Qt::NoContextMenu);
   this->internalWidget->setContextMenuPolicy (Qt::NoContextMenu);
   this->setupContextMenu ();

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   this->connectNewVariableNameProperty
         (SLOT (useNewVariableNameProperty (QString, QString, unsigned int)));
}

//------------------------------------------------------------------------------
//
QENumericEdit::~QENumericEdit ()
{
   // place holder
}

//------------------------------------------------------------------------------
//
void QENumericEdit::fontChange (const QFont&)
{
   // We use this overridden function as a trigger to update the internal
   // widget's font. The given parameter (which we don't use)  lags by one change,
   // but this->font () is up to date, so we use that.
   //
   if (this->internalWidget) {
      this->internalWidget->setFont (this->font ());
   }
}

//------------------------------------------------------------------------------
//
void QENumericEdit::focusInEvent (QFocusEvent* event)
{
   this->internalWidget->setFocus ();        // pass to enclosed widget
   QEAbstractWidget::focusInEvent (event);   // pass to parent
}

//------------------------------------------------------------------------------
// Return the Qt default context menu. This is added to the QE context menu.
//
QMenu* QENumericEdit::getDefaultContextMenu()
{
   QMenu* menu = this->internalWidget->lineEdit->createStandardContextMenu ();
   menu->setTitle ("Edit...");
   return menu;
}

//------------------------------------------------------------------------------
//
bool QENumericEdit::isOkayToWrite (const WriteOptions writeOption)
{
   // Already being handled.
   //
   if (this->messageDialogPresent) return false;

   bool result = true;
   int confirm = QMessageBox::Yes;

   switch (writeOption) {

      case woReturnPressed:
         if (!this->confirmWrite) {
            confirm = QMessageBox::Yes;
            break;
         }
         this->messageDialogPresent = true;
         confirm = QMessageBox::question (this, "Confirm write",
                                          "Do you want to write this value?",
                                          QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                          QMessageBox::Yes);
         this->messageDialogPresent = false;
         this->setFocus ();   // ensure focus returns to source widget
         break;

      case woEditingFinished:
         this->messageDialogPresent = true;
         confirm = QMessageBox::warning  (this, "Value changed",
                                          "You altered a value but didn't write it.\n"
                                          "Do you want to write this value?",
                                          QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
                                          QMessageBox::No);
         this->messageDialogPresent = false;
         break;

      default:
         DEBUG << "unexpected option" << (int) writeOption;
         confirm = QMessageBox::Yes;
         break;
   }

   switch (confirm) {

      case QMessageBox::Yes:
         // Write the value
         result = true;
         break;

      case QMessageBox::No:
         // Abort the write, revert to latest value
         result = false;
         this->internalWidget->setValue (this->lastValue);
         this->isModified = false;
         break;

      case QMessageBox::Cancel:
         // Don't write the value, keep editing/move back to the field being edited.
         //
         result = false;
         this->setFocus ();
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
//
double QENumericEdit::getValue () const
{
   return this->internalWidget->getValue ();
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setValue (const double value, const bool isUserUpdate)
{
    this->internalWidget->setValue (value);
    if (isUserUpdate) {
       this->writeNow ();
    }
}

//------------------------------------------------------------------------------
//
void QENumericEdit::writeNow ()
{
   // If not connected, do nothing.
   //
   if (!this->isConnected) {
      return;
   }

   // Get the variable to write to, and check it exists.
   //
   QEFloating* qca = dynamic_cast <QEFloating*> (getQcaItem (PV_VARIABLE_INDEX));
   if (qca) {
      // Check is value different ?? Compare with lastValue??
      //
      // Write the element value - honors array index.
      //
      qca->writeFloatingElement (this->getValue ());
      this->isModified = false;
   }
}

//------------------------------------------------------------------------------
// Depricated
double QENumericEdit::getNumericValue () const
{
   return this->getValue();
}

//------------------------------------------------------------------------------
// Depricated
void QENumericEdit::setNumericValue (const double value, const bool isUserUpdate)
{
   this->setValue (value, isUserUpdate);
}

//==============================================================================
// Slots/Hook functions
//
void QENumericEdit::useNewVariableNameProperty (QString variableName,
                                                QString substitutions,
                                                unsigned int variableIndex)
{
   this->setVariableNameAndSubstitutions (variableName, substitutions, variableIndex);
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. A QCaObject that streams doubles is required.
//
qcaobject::QCaObject* QENumericEdit::createQcaItem (unsigned int variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variable index" << variableIndex;
      return NULL;
   }

   qcaobject::QCaObject* result = NULL;

   result = new QEFloating (this->getSubstitutedVariableName (variableIndex),
                            this, &this->floatingFormatting, variableIndex);

   // Apply current array index to new QCaObject
   //
   this->setQCaArrayIndex (result);

   return result;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::activated ()
{
   // Ensure widget returns to default state when (re-)activated.
   //
   this->setStyleSheet ("");
   this->setValue (0.0, false);
   this->isModified = false;
}

//------------------------------------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QENumericEdit::establishConnection (unsigned int variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variable index" << variableIndex;
   }

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject* qca = this->createConnection (variableIndex);

   // If a QCaObject object is now available to supply data update signals,
   // connect it to the appropriate slots.
   //
   if (qca) {
      QObject::connect (qca,  SIGNAL (connectionChanged (QCaConnectionInfo&, const unsigned int&)),
                        this, SLOT   (connectionChanged (QCaConnectionInfo&, const unsigned int&)));

      QObject::connect (qca,  SIGNAL (floatingChanged     (const double&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                        this, SLOT   (externalValueUpdate (const double&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));
   }
}

//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the widget looks and change the tool tip
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QENumericEdit::connectionChanged (QCaConnectionInfo& connectionInfo,
                                       const unsigned int& variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variable index" << variableIndex;
   }

   // Note the connected state
   //
   this->isConnected = connectionInfo.isChannelConnected ();

   // More trob. than it's worth to check if this is a connect or disconnect.
   //
   this->isFirstUpdate = true;

   // Enable internal widget iff connected.
   // Container widget remains enabled, so menues etc. still work.
   //
   this->internalWidget->setEnabled (this->isConnected);

   // Display the connected state
   //
   this->updateToolTipConnection (this->isConnected, variableIndex);
   this->processConnectionInfo (this->isConnected, variableIndex);

   // Set cursor to indicate access mode.
   //
   this->setAccessCursorStyle ();

   // Signal channel connection change to any (Link) widgets.
   // using signal dbConnectionChanged.
   //
   this->emitDbConnectionChanged (PV_VARIABLE_INDEX);
}

//------------------------------------------------------------------------------
//
void QENumericEdit::externalValueUpdate (const double& value,
                                         QCaAlarmInfo& alarmInfo,
                                         QCaDateTime&,
                                         const unsigned int& variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Check first update (per connection)
   //
   if (this->isFirstUpdate) {

      // Check for auto scale and add units.
      //
      if (this->getAutoScale()) {
         this->calculateAutoValues ();
      }

      qcaobject::QCaObject* qca = this->getQcaItem (PV_VARIABLE_INDEX);
      QString suffix;
      if (this->getAddUnits () && qca) {
         suffix = qca->getEgu ();
         if (!suffix.isEmpty()) {
            suffix.prepend (" ");
         }
      }
      this->internalWidget->setSuffix (suffix);
   }

   // Save the most recent value. If the user is editing the value updates are
   // not applied except in circumstances outlined below. If the user cancels the
   // write, the value the widget should revert to the latest external (EPICS
   // database) value. This last value is also used to manage notifying user
   // changes (save what the user will be changing from).
   //
   this->lastValue = value;

   // Update the value if appropriate.
   // If the user is editing the object then updates will be
   // inapropriate, unless it is the first update and the
   // user has not started changing the text.
   // Update always allowed iff isAllowFocusUpdate has been set true.
   //
   bool allowUpdate;

   if (this->hasFocus ()) {
      allowUpdate = this->allowFocusUpdate || !this->isModified || this->isFirstUpdate;
   } else {
      // No focus - but maybe confirmation dialog present.
      allowUpdate = !this->messageDialogPresent;
   }

   if (allowUpdate) {
      // Update to refect current database value.
      // Note programatical updates like this, as opposed to user updates, do
      // not signal valueChanged (connected to internalValueChanged).
      //
      this->internalWidget->setValue (value);
   }

   // Invoke common alarm handling processing.
   //
   this->processAlarmInfo (alarmInfo, variableIndex);

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged. Because the write underying QLineEdit may not have
   // occured (because we had focus), we cannot use the text () function.
   // We must reformat in order to always generate sensible signal text.
   //
   QString formattedText;
   formattedText = this->internalWidget->getFormattedText (value);
   this->emitDbValueChanged (formattedText, PV_VARIABLE_INDEX);

   // First update is now over
   //
   this->isFirstUpdate = false;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::calculateAutoValues ()
{
   // If not connected, do nothing.
   //
   if (!this->isConnected) return;

   qcaobject::QCaObject* qca = this->getQcaItem (PV_VARIABLE_INDEX);
   if (!qca) return;        // sainity check

   // Do the auto scale calculations.
   //
   double autoMinimum;
   double autoMaximum;
   int autoLeadingZeros;
   int autoPrecision;

   // Check that sensible limits have been defined and not just left
   // at the default (i.e. zero) values by a lazy database creator.
   // Otherwise, use the design time limits.
   //
   double ctrlLow = qca->getControlLimitLower ();
   double ctrlUpp = qca->getControlLimitUpper ();

   // If control limits are undefined - try display limits.
   //
   if ((ctrlLow == 0.0) && (ctrlUpp == 0.0)) {
      ctrlLow = qca->getDisplayLimitLower ();
      ctrlUpp = qca->getDisplayLimitUpper ();
   }

   if ((ctrlLow != 0.0) || (ctrlUpp != 0.0)) {
      // At least one of these limits is non-zero - assume database creator
      // has thought about this.
      //
      autoMinimum = ctrlLow;
      autoMaximum = ctrlUpp;

      if (this->getNotation() == QNumericEdit::Scientific) {
         autoLeadingZeros = 1;

      } else {
         double temp;
         temp = MAX (1.0,  ABS (autoMinimum));
         temp = MAX (temp, ABS (autoMaximum));

         // the (int) cast truncates to 0
         //
         autoLeadingZeros = 1 + (int) (log10 (temp));
      }

   } else {
      // Just go with design property values.
      //
      autoMinimum = this->designMinimum;
      autoMaximum = this->designMaximum;
      autoLeadingZeros = this->designLeadingZeros;
   }
   autoPrecision = qca->getPrecision ();

   // Now apply auto values to the internal QNumericEdit object.
   // Note the order of application.
   //
   this->internalWidget->setLeadingZeros (autoLeadingZeros);
   this->internalWidget->setPrecision (autoPrecision);
   this->internalWidget->setMinimum (autoMinimum);
   this->internalWidget->setMaximum (autoMaximum);
}

//------------------------------------------------------------------------------
//
void QENumericEdit::internalValueChanged (const double)
{
   this->isModified = true;
   if (this->writeOnChange) {
      this->writeNow ();
   }
}

//------------------------------------------------------------------------------
// The user has pressed return/enter. (Not write when user enters the widget)
// Note, it doesn't matter if the user presses return and both this function
// AND editingFinished() is called since setText is called in each to clear
// the 'isModified' flag. So, the first called will perform the write, the
// second (if any) will do nothing.
//
void QENumericEdit::returnPressed ()
{
   // If not connected, do nothing.
   //
   if (!this->isConnected) {
      return;
   }

   if (this->writeOnEnter && this->isModified) {
      if (this->isOkayToWrite (woReturnPressed)) {
         this->writeNow ();
      }
   }
}

//------------------------------------------------------------------------------
// The user has 'finished editing' such as pressed return/enter or moved
// focus from the object.
// Note, it doesn't matter if the user presses return and both this function
// AND returnPressed() is called since setText is called in each to clear
// the 'isModified' flag. So, the first called will perform the write, the
// second (if any) will do nothing.
//
void QENumericEdit::editingFinished ()
{
   // If not connected, do nothing.
   //
   if (!this->isConnected) {
      return;
   }

   if (this->writeOnFinish && this->isModified) {
      if (this->isOkayToWrite (woEditingFinished)) {
         this->writeNow ();
      }
   }
}

//==============================================================================
// Properties
//
void QENumericEdit::setAutoScale (const bool value)
{
   if (this->autoScale != value) {
      // Value has changed.
      //
      this->autoScale = value;

      if (this->autoScale) {
         // Auto scale turned on - recalculate and re-apply auto values (if we can).
         //
         this->calculateAutoValues ();

      } else {
         // Auto scale turned off - restore the design time property values.
         //
         this->internalWidget->setLeadingZeros (this->designLeadingZeros);
         this->internalWidget->setPrecision (this->designPrecision);
         this->internalWidget->setMinimum (this->designMinimum);
         this->internalWidget->setMaximum (this->designMaximum);
      }
   }
}

//------------------------------------------------------------------------------
//
bool QENumericEdit::getAutoScale () const
{
   return this->autoScale;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setAddUnits (const bool value)
{
   this->addUnits = value;
}

//------------------------------------------------------------------------------
//
bool QENumericEdit::getAddUnits () const
{
   return this->addUnits;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setWriteOnLoseFocus (const bool value)
{
   this->writeOnLoseFocus = value;
}

//------------------------------------------------------------------------------
//
bool QENumericEdit::getWriteOnLoseFocus () const
{
   return this->writeOnLoseFocus;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setWriteOnEnter (const bool value)
{
   this->writeOnEnter = value;
}

//------------------------------------------------------------------------------
//
bool QENumericEdit::getWriteOnEnter () const
{
   return this->writeOnEnter;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setWriteOnFinish (const bool value)
{
   this->writeOnFinish = value;
}

//------------------------------------------------------------------------------
//
bool QENumericEdit::getWriteOnFinish () const
{
   return this->writeOnFinish;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setWriteOnChange (const bool value)
{
   this->writeOnChange = value;

   // Confirm write and write on change are deemed mutually exclusive.
   //
   if (this->writeOnChange) this->confirmWrite = false;
}

//------------------------------------------------------------------------------
//
bool QENumericEdit::getWriteOnChange () const
{
   return this->writeOnChange;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setConfirmWrite (const bool value)
{
   this->confirmWrite = value;

   // Confirm write and write on change are deemed mutually exclusive.
   //
   if (this->confirmWrite) this->writeOnChange = false;
}

//------------------------------------------------------------------------------
//
bool QENumericEdit::getConfirmWrite () const
{
   return this->confirmWrite;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setAllowFocusUpdate (const bool value)
{
   this->allowFocusUpdate = value;
}

//------------------------------------------------------------------------------
//
bool QENumericEdit::getAllowFocusUpdate () const
{
   return this->allowFocusUpdate;
}

//------------------------------------------------------------------------------
// Note: For leading zeros, precison, minimum and maximum, we cache the design time
// values here in case that at design time or programatically during run time
// we flip between auto scale and design time scale settings.  When autoScale off or
// the channel not connected then we apply the value immediately.
//
void QENumericEdit::setLeadingZeros (const int value)
{
   this->designLeadingZeros = value;
   if (!this->autoScale || !this->isConnected) {
      this->internalWidget->setLeadingZeros (value);
   }
}

//------------------------------------------------------------------------------
//
int QENumericEdit::getLeadingZeros () const
{
   return this->designLeadingZeros;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setPrecision (const int value)
{
   this->designPrecision = value;
   if (!this->autoScale || !this->isConnected) {
      this->internalWidget->setPrecision (value);
   }
}

//------------------------------------------------------------------------------
//
int QENumericEdit::getPrecision () const
{
   return this->designPrecision;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setMinimum (const double value)
{
   this->designMinimum = value;
   if (!this->autoScale || !this->isConnected) {
      this->internalWidget->setMinimum (value);
   }
}

//------------------------------------------------------------------------------
//
double QENumericEdit::getMinimum () const
{
   return this->designMinimum;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setMaximum (const double value)
{
   this->designMaximum = value;
   if (!this->autoScale || !this->isConnected) {
      this->internalWidget->setMaximum (value);
   }
}

//------------------------------------------------------------------------------
//
double QENumericEdit::getMaximum () const
{
   return this->designMaximum;
}

//==============================================================================
// Copy / Paste
//
QString QENumericEdit::copyVariable ()
{
   return this->getSubstitutedVariableName (PV_VARIABLE_INDEX);
}

//------------------------------------------------------------------------------
//
QVariant QENumericEdit::copyData ()
{
   return QVariant (this->internalWidget->getValue ());
}

//------------------------------------------------------------------------------
//
void QENumericEdit::paste (QVariant v)
{
   this->setVariableName (v.toString (), PV_VARIABLE_INDEX);
   this->establishConnection (PV_VARIABLE_INDEX);
}

// end
