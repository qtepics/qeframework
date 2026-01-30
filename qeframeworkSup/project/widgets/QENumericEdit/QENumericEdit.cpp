/*  QENumericEdit.cpp
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

#include "QENumericEdit.h"
#include <math.h>
#include <QColor>
#include <QDebug>
#include <QFontMetrics>
#include <QMessageBox>
#include <QECommon.h>
#include <QEScaling.h>

#define DEBUG qDebug () << "QENumericEdit" << __LINE__ << __FUNCTION__  << "  "

#define PV_VARIABLE_INDEX      0

static const QString applyStyleEnabled  = QEUtilities::colourToStyle (QColor (128, 232, 128));
static const QString applyStyleDisabled = QEUtilities::colourToStyle (QColor (216, 232, 216), QColor(128, 128, 128));


//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QENumericEdit::QENumericEdit (QWidget* parent) :
   QEAbstractWidget (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX),
   internalWidget (new QNumericEdit (this))
{
   this->commonSetup ();
}

//------------------------------------------------------------------------------
// Constructor with known variable
//
QENumericEdit::QENumericEdit (const QString& variableNameIn, QWidget* parent) :
   QEAbstractWidget (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX),
   internalWidget (new QNumericEdit (this))
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
   // Create internal widgets.
   //
   this->applyButtonText = "A";
   this->applyButton = new QPushButton (this->applyButtonText, this);
   this->applyButton->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);
   this->applyButton->setFocusPolicy (Qt::NoFocus);
   this->setApplyButtonWidth ();
   this->applyButton->setStyleSheet (applyStyleEnabled);

   // Copy actual widget size policy to the containing widget, then ensure
   // internal widget will expand to fill container widget.
   //
   this->setSizePolicy (this->internalWidget->sizePolicy ());
   this->internalWidget->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);

   this->layout = new QHBoxLayout (this);
   this->layout->setContentsMargins (0, 0, 0, 0);    // extact fit.
   this->layout->setSpacing (QEScaling::scale (4));

   // The layout takes ownership.
   //
   this->layout->addWidget (this->internalWidget);
   this->layout->addWidget (this->applyButton);

   // Button hidden until exposed.
   //
   this->applyButton->setVisible (false);
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

   // QPushButton signals
   //
   QObject::connect (this->applyButton,    SIGNAL (clicked      (bool)),
                     this,                 SLOT   (applyPressed (bool)));

   // Clear flags and ensure valid last value stored.
   //
   this->isConnected = false;
   this->messageDialogPresent = false;
   this->isModified = false;
   this->lastValue = 0.0;

   // Set default property values
   //
   this->addUnits = false;
   this->useApplyButton = false;
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
   this->setDisplayAlarmStateOption (QE::Always);

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

   // Some events must be applied to the internal widget
   //
   this->installEventFilter (this);
}

//------------------------------------------------------------------------------
//
QENumericEdit::~QENumericEdit ()
{
   // place holder
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setApplyButtonWidth ()
{
   QFontMetrics fm (this->applyButton->font());
   const int textWidth = fm.horizontalAdvance (applyButtonText + "   ");  // allow 1.5 spaces each side
   const int useWidth = MAX (textWidth, this->height());      // square or wider
   this->applyButton->setFixedWidth (useWidth);
}

//------------------------------------------------------------------------------
//
bool QENumericEdit::eventFilter (QObject* watched, QEvent* event)
{
   const QEvent::Type type = event->type ();
   bool result = false;

   switch (type) {
      case QEvent::FontChange:
         if (watched == this) {
            // Font must be mapped to the internal numeric edit and apply button
            //
            if (this->internalWidget) {
               this->internalWidget->setFont (this->font ());
            }

            if (this->applyButton) {
               this->applyButton->setFont (this->font ());
            }

            this->setApplyButtonWidth ();
         }
         break;

      case QEvent::EnabledChange:
         // The enabled state must be passed through to the apply button.
         // The internal QNumericEdit widget already gets this.
         //
         if (watched == this) {
            if (this->isEnabled()) {
               this->applyButton->setStyleSheet (applyStyleEnabled);
            } else {
               this->applyButton->setStyleSheet (applyStyleDisabled);
            }
         }
         break;

      default:
         result = false;
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::focusInEvent (QFocusEvent* event)
{
   this->internalWidget->setFocus ();        // pass to enclosed widget
   QEAbstractWidget::focusInEvent (event);   // pass to parent
}

//------------------------------------------------------------------------------
//
void QENumericEdit::resizeEvent (QResizeEvent* event)
{
   this->setApplyButtonWidth ();            // keep square or wider
   QEAbstractWidget::resizeEvent (event);   // pass to parent
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
   QEFloating* qca = qobject_cast <QEFloating*> (this->getQcaItem (PV_VARIABLE_INDEX));
   if (qca && qca->getChannelIsConnected ()) {
      // Check is value different ?? Compare with lastValue??
      //
      // Write the element value - honors array index.
      //
      qca->writeFloatingElement (this->getValue ());
      this->isModified = false;
   }
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setDefaultStyle (const QString& style)
{
   this->setStyleDefault (style);
}

//------------------------------------------------------------------------------
// slot
void QENumericEdit::setPvValue (const QString& text)
{
   bool okay;
   const double v = text.toDouble (&okay);
   if (okay) {
      this->setPvValue (v);
   } else {
      QString message = QString ("Cannot convert '%1' to a double").arg (text);

      message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STANDARD);
      this->sendMessage (message, mt);
      DEBUG << message;
   }
}

//------------------------------------------------------------------------------
// slot
void QENumericEdit::setPvValue (const int value)
{
   this->setPvValue (static_cast<double>(value));
}

//------------------------------------------------------------------------------
// slot
void QENumericEdit::setPvValue (const double value)
{
   this->internalWidget->setValue (value);
   this->writeNow ();
}

//------------------------------------------------------------------------------
// slot
void QENumericEdit::setPvValue (const bool value)
{
   this->setPvValue (value ? 1.0 : 0.0);
}

//==============================================================================
// Slots/Hook functions
//
void QENumericEdit::useNewVariableNameProperty (QString pvName,
                                                QString subs,
                                                unsigned int vi)
{
   this->setVariableNameAndSubstitutions (pvName, subs, vi);
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QEChannel required. A QEChannel that streams doubles is required.
//
QEChannel* QENumericEdit::createQcaItem (unsigned int variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variable index" << variableIndex;
      return NULL;
   }

   const QString pvName = this->getSubstitutedVariableName (variableIndex);
   QEChannel* result = new QEFloating (pvName, this, &this->floatingFormatting,
                                       variableIndex);

   // Apply currently defined array index/elements request values.
   //
   this->setSingleVariableQCaProperties (result);

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
   // If successfull, the QEChannel object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QEChannel.
   //
   QEChannel* qca = this->createConnection (variableIndex);

   // If a QEChannel object is now available to supply data update signals,
   // connect it to the appropriate slots.
   //
   if (qca) {
      QObject::connect (qca,  SIGNAL (connectionUpdated (const QEConnectionUpdate&)),
                        this, SLOT   (connectionUpdated (const QEConnectionUpdate&)));

      QObject::connect (qca,  SIGNAL (valueUpdated        (const QEFloatingValueUpdate&)),
                        this, SLOT   (externalValueUpdate (const QEFloatingValueUpdate&)));
   }
}

//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the widget looks and change the tool tip
// This is the slot used to recieve connection updates from a QEChannel based class.
//
void QENumericEdit::connectionUpdated (const QEConnectionUpdate& update)
{
   const unsigned int vi = update.variableIndex;

   if (vi != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variable index" << vi;
   }

   // Note the connected state
   //
   this->isConnected = update.connectionInfo.isChannelConnected ();

   // Enable internal widget iff connected.
   // Container widget remains enabled, so menues etc. still work.
   //
   this->internalWidget->setEnabled (this->isConnected);

   // Display the connected state
   //
   this->updateToolTipConnection (this->isConnected, vi);
   this->processConnectionInfo (this->isConnected, vi);

   // Set cursor to indicate access mode.
   //
   this->setAccessCursorStyle ();

   // Signal channel connection change to any (Link) widgets.
   // using signal dbConnectionChanged.
   //
   this->emitDbConnectionChanged (vi);
}

//------------------------------------------------------------------------------
//
void QENumericEdit::externalValueUpdate (const QEFloatingValueUpdate& update)
{
   const unsigned int vi = update.variableIndex;

   if (vi != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << vi;
      return;
   }

   // Check first/meta data update
   //
   if (update.isMetaUpdate) {

      // Check for auto scale and add units.
      //
      if (this->getAutoScale()) {
         this->calculateAutoValues ();
      }

      QEChannel* qca = this->getQcaItem (vi);
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
   this->lastValue = update.value;

   // Update the value if appropriate.
   // If the user is editing the object then updates will be
   // inapropriate, unless it is the first update and the
   // user has not started changing the text.
   // Update always allowed iff isAllowFocusUpdate has been set true.
   //
   bool allowUpdate;

   // The internal QNumericEdit's internal QLineEdit holds the focus.
   // Make hasFocus a QNumericEdit method
   //
   if (this->internalWidget->lineEdit->hasFocus ()) {
      allowUpdate = this->allowFocusUpdate || !this->isModified || update.isMetaUpdate;
   } else {
      // No focus - but maybe confirmation dialog present.
      allowUpdate = !this->messageDialogPresent;
   }

   if (allowUpdate) {
      // Update to refect current database value.
      // Note programatical updates like this, as opposed to user updates, do
      // not signal valueChanged (connected to internalValueChanged).
      //
      this->internalWidget->setValue (update.value);
   }

   // Invoke common alarm handling processing.
   //
   this->processAlarmInfo (update.alarmInfo, vi);

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged. Because the write underying QLineEdit may not have
   // occured (because we had focus), we cannot use the text () function.
   // We must reformat in order to always generate sensible signal text.
   //
   QString formattedText;
   formattedText = this->internalWidget->getFormattedText (update.value);
   this->emitDbValueChanged (formattedText, vi);
}

//------------------------------------------------------------------------------
//
void QENumericEdit::calculateAutoValues ()
{
   // If not connected, do nothing.
   //
   if (!this->isConnected) return;

   QEChannel* qca = this->getQcaItem (PV_VARIABLE_INDEX);
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

      if (this->getNotation() == QE::Scientific) {
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

   QEChannel* qca = this->getQcaItem (PV_VARIABLE_INDEX);
   if (!qca) return;   // sanity check
   const bool isMetaDataUpdate = qca->getIsMetaDataUpdate();

   // GUI-320
   if (this->writeOnChange && !isMetaDataUpdate) {
      this->writeNow ();
   }
}

//------------------------------------------------------------------------------
// The user has pressed the apply button.
//
void QENumericEdit::applyPressed (bool)
{
   this->writeNow ();
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
void QENumericEdit::setUseApplyButton (const bool useApplyButtonIn)
{
   this->useApplyButton = useApplyButtonIn;
   this->applyButton->setVisible (this->useApplyButton);
   if (this->applyButton) {
      this->setWriteOnEnter (false);
      this->setWriteOnChange (false);
      this->setWriteOnFinish (false);
      this->setWriteOnLoseFocus (false);
      this->setConfirmWrite (false);
   }
}

//------------------------------------------------------------------------------
//
bool QENumericEdit::getUseApplyButton () const
{
   return this->useApplyButton;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setApplyButtonText (const QString& text)
{
   this->applyButtonText = text;
   this->applyButton->setText (text);
   this->setApplyButtonWidth ();
}

//------------------------------------------------------------------------------
//
QString QENumericEdit::getApplyButtonText () const
{
   return this->applyButtonText;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setSubscribe (const bool subscribeIn)
{
   this->subscribe = subscribeIn;   // lives in VariableManager
}

//------------------------------------------------------------------------------
//
bool QENumericEdit::getSubscribe () const
{
   return this->subscribe;
}

//------------------------------------------------------------------------------
//
void QENumericEdit::setWriteOnLoseFocus (const bool value)
{
   this->writeOnLoseFocus = value && !this->useApplyButton;
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
   this->writeOnEnter = value && !this->useApplyButton;
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
   this->writeOnFinish = value && !this->useApplyButton;
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
   this->writeOnChange = value && !this->useApplyButton;

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
   this->confirmWrite = value && !this->useApplyButton;

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
