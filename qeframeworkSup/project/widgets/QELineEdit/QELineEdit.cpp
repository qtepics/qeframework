/*  QELineEdit.cpp
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

#include "QELineEdit.h"
#include <math.h>
#include <QColor>
#include <QDebug>
#include <QFontMetrics>
#include <QMessageBox>
#include <QECommon.h>
#include <QEScaling.h>

#define DEBUG qDebug () << "QELineEdit" << __LINE__ << __FUNCTION__  << "  "

#define PV_VARIABLE_INDEX      0

static const QString applyStyleEnabled  = QEUtilities::colourToStyle (QColor (128, 232, 128));
static const QString applyStyleDisabled = QEUtilities::colourToStyle (QColor (216, 232, 216), QColor(128, 128, 128));


//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QELineEdit::QELineEdit (QWidget* parent) :
   QEAbstractWidget (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX),
   QEStringFormattingMethods(),
   internalWidget (new QLineEdit (this))
{
   this->commonSetup ();
}

//------------------------------------------------------------------------------
// Constructor with known variable
//
QELineEdit::QELineEdit (const QString& variableNameIn, QWidget* parent) :
   QEAbstractWidget (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX),
   QEStringFormattingMethods(),
   internalWidget (new QLineEdit (this))
{
   this->commonSetup ();
   this->setVariableName (variableNameIn, PV_VARIABLE_INDEX);
   this->activate ();
}

//------------------------------------------------------------------------------
// Setup common to all constructors
//
void QELineEdit::commonSetup ()
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
   this->internalWidget->setAcceptDrops (false); // handled by outer widget

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

   // QLineEdit signals
   //
   QObject::connect (this->internalWidget, SIGNAL (textEdited           (const QString&)),
                     this,                 SLOT   (internalValueEdited (const QString&)));
   QObject::connect (this->internalWidget, SIGNAL (returnPressed ()),
                     this,                 SLOT   (returnPressed ()));
   QObject::connect (this->internalWidget, SIGNAL (editingFinished ()),
                     this,                 SLOT   (editingFinished ()));


   // QLineEdit signals to QELineEdit signals.
   //
   QObject::connect (this->internalWidget, SIGNAL (textChanged( const QString&)),
                     this,                 SIGNAL (textChanged( const QString&)));
   QObject::connect (this->internalWidget, SIGNAL (textEdited( const QString&)),
                     this,                 SIGNAL (textEdited( const QString&)));

   // QPushButton signals
   //
   QObject::connect (this->applyButton,    SIGNAL (clicked      (bool)),
                     this,                 SLOT   (applyPressed (bool)));

   // Clear flags and ensure valid last value stored.
   //
   this->isConnected = false;
   this->messageDialogPresent = false;
   this->isModified = false;
   this->lastValue = "";
   this->dropOption = QE::DropToVariable;

   // Set default property values
   //
   this->setAddUnits (false);
   this->useApplyButton = false;
   this->writeOnLoseFocus = false;
   this->writeOnEnter = true;
   this->writeOnFinish = true;
   this->confirmWrite = false;
   this->allowFocusUpdate = false;

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
   // Note: Turn off the menu for the internal QLineEdit and QLineEdit widgets.
   //
   this->internalWidget->setContextMenuPolicy (Qt::NoContextMenu);
   this->setupContextMenu ();

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   this->connectPvNameProperties (SLOT (usePvNameProperties (const QEPvNameProperties&)));

   // Some events must be applied to the internal widget
   //
   this->installEventFilter (this);
}

//------------------------------------------------------------------------------
//
QELineEdit::~QELineEdit ()
{
   // place holder
}

//------------------------------------------------------------------------------
//
void QELineEdit::setApplyButtonWidth ()
{
   QFontMetrics fm (this->applyButton->font());
   const int textWidth = fm.horizontalAdvance (applyButtonText + "   ");  // allow 1.5 spaces each side
   const int useWidth = MAX (textWidth, this->height());      // square or wider
   this->applyButton->setFixedWidth (useWidth);
}

//------------------------------------------------------------------------------
//
bool QELineEdit::eventFilter (QObject* watched, QEvent* event)
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
         // The internal QLineEdit widget already gets this.
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
void QELineEdit::focusInEvent (QFocusEvent* event)
{
   this->internalWidget->setFocus ();        // pass to enclosed widget
   QEAbstractWidget::focusInEvent (event);   // pass to parent
}

//------------------------------------------------------------------------------
//
void QELineEdit::resizeEvent (QResizeEvent* event)
{
   this->setApplyButtonWidth ();            // keep square or wider
   QEAbstractWidget::resizeEvent (event);   // pass to parent
}

//------------------------------------------------------------------------------
// Return the Qt default context menu. This is added to the QE context menu.
//
QMenu* QELineEdit::getDefaultContextMenu()
{
   QMenu* menu = this->internalWidget->createStandardContextMenu ();
   menu->setTitle ("Edit...");
   return menu;
}

//------------------------------------------------------------------------------
//
bool QELineEdit::isOkayToWrite (const WriteOptions writeOption)
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
         this->internalWidget->setText (this->lastValue);
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
QString QELineEdit::getValue () const
{
   return this->internalWidget->text ();
}

//------------------------------------------------------------------------------
//
void QELineEdit::setValue (const QString& value, const bool isUserUpdate)
{
   this->internalWidget->setText (value);
   if (isUserUpdate) {
      this->writeNow ();
   }
}

//------------------------------------------------------------------------------
//
void QELineEdit::writeNow ()
{
   // If not connected, do nothing.
   //
   if (!this->isConnected) {
      return;
   }

   // Get the variable to write to, and check it exists.
   //
   QEString* qca = qobject_cast <QEString*> (this->getQcaItem (PV_VARIABLE_INDEX));
   if (qca && qca->getChannelIsConnected ()) {
      // Check is value different ?? Compare with lastValue??
      //
      // Write the element value - honors array index.
      //
      QString message;
      const bool status = qca->writeStringElement (this->getValue (), message);
      if (!status) {
         QMessageBox::warning (this, QString("Write failed"), message, QMessageBox::Cancel);
         this->setFocus();
      }
      this->isModified = false;
   }
}

//------------------------------------------------------------------------------
//
void QELineEdit::setDefaultStyle (const QString& style)
{
   this->setStyleDefault (style);
}

//------------------------------------------------------------------------------
// slot
void QELineEdit::setPvValue (const QString& text)
{
   this->internalWidget->setText (text);
   this->writeNow ();
}

//------------------------------------------------------------------------------
// slot
void QELineEdit::setPvValue (const int value)
{
   this->setPvValue (QString::number (value));
}

//------------------------------------------------------------------------------
// slot
void QELineEdit::setPvValue (const double value)
{
   this->setPvValue (QString::number (value, 'e', 15));
}

//------------------------------------------------------------------------------
// slot
void QELineEdit::setPvValue (const bool value)
{
   const QString text = value ? "True" : "False";
   this->setPvValue (text);
}

//==============================================================================
// Slots/Hook functions
//
void QELineEdit::usePvNameProperties (const QEPvNameProperties& pvNameProperties)
{
   this->setVariableNameAndSubstitutions (pvNameProperties.pvName,
                                          pvNameProperties.substitutions,
                                          pvNameProperties.index);
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QEChannel required. A QEChannel that streams doubles is required.
//
QEChannel* QELineEdit::createQcaItem (unsigned int variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variable index" << variableIndex;
      return NULL;
   }

   const QString pvName = this->getSubstitutedVariableName (variableIndex);
   QEChannel* result = new QEString (pvName, this, &this->stringFormatting,
                                     variableIndex);

   // Apply currently defined array index/elements request values.
   //
   this->setSingleVariableQCaProperties (result);

   return result;
}

//------------------------------------------------------------------------------
//
void QELineEdit::activated ()
{
   // Ensure widget returns to default state when (re-)activated.
   //
   this->setStyleSheet ("");
   this->setValue ("", false);
   this->isModified = false;
}

//------------------------------------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QELineEdit::establishConnection (unsigned int variableIndex)
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

      QObject::connect (qca,  SIGNAL (valueUpdated        (const QEStringValueUpdate&)),
                        this, SLOT   (externalValueUpdate (const QEStringValueUpdate&)));

      QObject::connect (this, SIGNAL (requestResend()),
                        qca,  SLOT   (resendLastData()));
   }
}

//------------------------------------------------------------------------------
//
void QELineEdit::stringFormattingChange()
{
   emit requestResend();
}

//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the widget looks and change the tool tip
// This is the slot used to recieve connection updates from a QEChannel based class.
//
void QELineEdit::connectionUpdated (const QEConnectionUpdate& update)
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
void QELineEdit::externalValueUpdate (const QEStringValueUpdate& update)
{
   const unsigned int vi = update.variableIndex;

   if (vi != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << vi;
      return;
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

   // The internal QLineEdit's internal QLineEdit holds the focus.
   // Make hasFocus a QLineEdit method
   //
   if (this->internalWidget->hasFocus ()) {
      allowUpdate = this->allowFocusUpdate || !this->isModified || update.isMetaUpdate;
   } else {
      // No focus - but maybe confirmation dialog present.
      allowUpdate = !this->messageDialogPresent;
   }

   if (allowUpdate) {
      // Update to refect current database value.
      // Note programatical updates like this, as opposed to user updates, do
      // not signal valueChanged (connected to internalValueEdited).
      // Check alignment for pre/post space
      //
      QString value = update.value;

// Future enhancement - impacts get/setValue as well.
//    const Qt::Alignment a = this->internalWidget->alignment();
//    if ((a & Qt::AlignLeft) != 0) {
//       value = " " + value;
//    } else if ((a & Qt::AlignRight) != 0) {
//       value = value + " ";
//    }

      this->internalWidget->setText (value);
   }

   // Invoke common alarm handling processing.
   //
   this->processAlarmInfo (update.alarmInfo, vi);

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged. Because the write underying QLineEdit may not have
   // occured (because we had focus), we cannot use the text () function.
   //
   this->emitDbValueChanged (update.value, vi);
}

//------------------------------------------------------------------------------
//
void QELineEdit::internalValueEdited (const QString&)
{
   this->isModified = true;
}

//------------------------------------------------------------------------------
// The user has pressed the apply button.
//
void QELineEdit::applyPressed (bool)
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
void QELineEdit::returnPressed ()
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
void QELineEdit::editingFinished ()
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

//------------------------------------------------------------------------------
// Properties
//------------------------------------------------------------------------------
//
void QELineEdit::setUseApplyButton (const bool useApplyButtonIn)
{
   this->useApplyButton = useApplyButtonIn;
   this->applyButton->setVisible (this->useApplyButton);
   if (this->applyButton) {
      this->setWriteOnEnter (false);
      this->setWriteOnFinish (false);
      this->setWriteOnLoseFocus (false);
      this->setConfirmWrite (false);
   }
}

//------------------------------------------------------------------------------
//
bool QELineEdit::getUseApplyButton () const
{
   return this->useApplyButton;
}

//------------------------------------------------------------------------------
//
void QELineEdit::setApplyButtonText (const QString& text)
{
   this->applyButtonText = text;
   this->applyButton->setText (text);
   this->setApplyButtonWidth ();
}

//------------------------------------------------------------------------------
//
QString QELineEdit::getApplyButtonText () const
{
   return this->applyButtonText;
}

//------------------------------------------------------------------------------
//
void QELineEdit::setSubscribe (const bool subscribeIn)
{
   this->subscribe = subscribeIn;   // lives in VariableManager
}

//------------------------------------------------------------------------------
//
bool QELineEdit::getSubscribe () const
{
   return this->subscribe;
}

//------------------------------------------------------------------------------
//
void QELineEdit::setWriteOnLoseFocus (const bool value)
{
   this->writeOnLoseFocus = value && !this->useApplyButton;
}

//------------------------------------------------------------------------------
//
bool QELineEdit::getWriteOnLoseFocus () const
{
   return this->writeOnLoseFocus;
}

//------------------------------------------------------------------------------
//
void QELineEdit::setWriteOnEnter (const bool value)
{
   this->writeOnEnter = value && !this->useApplyButton;
}

//------------------------------------------------------------------------------
//
bool QELineEdit::getWriteOnEnter () const
{
   return this->writeOnEnter;
}

//------------------------------------------------------------------------------
//
void QELineEdit::setWriteOnFinish (const bool value)
{
   this->writeOnFinish = value && !this->useApplyButton;
}

//------------------------------------------------------------------------------
//
bool QELineEdit::getWriteOnFinish () const
{
   return this->writeOnFinish;
}

//------------------------------------------------------------------------------
//
void QELineEdit::setConfirmWrite (const bool value)
{
   this->confirmWrite = value && !this->useApplyButton;
}

//------------------------------------------------------------------------------
//
bool QELineEdit::getConfirmWrite () const
{
   return this->confirmWrite;
}

//------------------------------------------------------------------------------
//
void QELineEdit::setAllowFocusUpdate (const bool value)
{
   this->allowFocusUpdate = value;
}

//------------------------------------------------------------------------------
//
bool QELineEdit::getAllowFocusUpdate () const
{
   return this->allowFocusUpdate;
}

//------------------------------------------------------------------------------
//
void QELineEdit::setDropOption (QE::DropOption option)
{
   this->dropOption = option;
}

//------------------------------------------------------------------------------
//
QE::DropOption QELineEdit::getDropOption() const
{
   return this->dropOption;
}

//==============================================================================
// Copy / Paste
//
QString QELineEdit::copyVariable ()
{
   return this->getSubstitutedVariableName (PV_VARIABLE_INDEX);
}

//------------------------------------------------------------------------------
//
QVariant QELineEdit::copyData ()
{
   return QVariant (this->internalWidget->text ());
}

//------------------------------------------------------------------------------
// Also handles drop
void QELineEdit::paste (QVariant value)
{
   DEBUG;
   switch (this->getDropOption()) {
      case QE::DropToVariable:
         // Standard paste/drop updates the PV name.
         this->setVariableName (value.toString (), PV_VARIABLE_INDEX);
         this->establishConnection (PV_VARIABLE_INDEX);
         break;

      case QE::DropToText:
         DEBUG;
         this->setText (value.toString());
         break;

      case QE::DropToTextAndWrite:
         this->setText (value.toString());
         this->writeNow ();
         break;

      default:
         DEBUG << "bad drop option:" << static_cast<int> (this->getDropOption());
         break;
   }

}

// end
