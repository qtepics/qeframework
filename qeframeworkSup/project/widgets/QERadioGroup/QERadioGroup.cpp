/*  QERadioGroup.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2014-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QERadioGroup.h"

#include <limits>
#include <QDebug>
#include <QRadioButton>
#include <QPushButton>
#include <QECommon.h>

#define DEBUG qDebug () << "QERadioGroup" << __LINE__ << __FUNCTION__ << "  "

//-----------------------------------------------------------------------------
// Constructor with no initialisation
//
QERadioGroup::QERadioGroup (QWidget* parent) :
   QEAbstractWidget (parent),
   QESingleVariableMethods (this, pvVariableIndex)
{
   this->commonSetup (" QERadioGroup ");
}

//-----------------------------------------------------------------------------
// Constructor with known variable
//
QERadioGroup::QERadioGroup (const QString& variableNameIn, QWidget* parent) :
   QEAbstractWidget (parent),
   QESingleVariableMethods (this, pvVariableIndex)
{
   this->commonSetup (" QERadioGroup ");
   this->setVariableName (variableNameIn, pvVariableIndex);
   this->activate();
}

//-----------------------------------------------------------------------------
// Constructor with title and known variable
//
QERadioGroup::QERadioGroup (const QString& title, const QString& variableNameIn,
                            QWidget* parent) :
   QEAbstractWidget (parent),
   QESingleVariableMethods (this, pvVariableIndex)
{
   this->commonSetup (title);
   this->setVariableName (variableNameIn, pvVariableIndex);
   this->activate();
}

//-----------------------------------------------------------------------------
// Place holder
//
QERadioGroup::~QERadioGroup() { }

//-----------------------------------------------------------------------------
// Setup common to all constructors
//
void QERadioGroup::commonSetup (const QString& title)
{
   this->setMinimumSize (16, 16);   // start small - let designer set larger.

   // Create internal widget.
   //
   this->internalWidget = new QRadioGroup (this);

   // Overall minumum size set by this widget, not the internal widget
   //
   this->internalWidget->setMinimumSize (0, 0);

   // Copy actual widget size policy to the containing widget, then ensure
   // internal widget will expand to fill container widget.
   //
   this->setSizePolicy (this->internalWidget->sizePolicy ());
   this->internalWidget->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);

   this->layout = new QHBoxLayout (this);
   this->layout->setContentsMargins (0, 0, 0, 0);    // extact fit.
   this->layout->addWidget (this->internalWidget);

   QObject::connect (this->internalWidget, SIGNAL (valueChanged         (const int)),
                     this,                 SLOT   (internalValueChanged (const int)));

   // Set default property values.
   //
   // Set up data
   // This control uses a single data source
   // We use the 2nd "variable" for the title.
   //
   this->setNumVariables (2);

   // Set variable index used to select write access cursor style.
   //
   this->setControlPV (pvVariableIndex);

   // Title managed as second variable.
   //
   this->setVariableName (title, titleVariableIndex);

   // Set up default properties
   //
   this->setVariableAsToolTip (true);
   this->setAllowDrop (false);
   this->setDisplayAlarmStateOption (QE::Always);
   this->useDbEnumerations = true;      // as opposed to local enumeations.

   // Set the initial state
   //
   this->currentIndex = -1;

   // Use default context menu.
   //
   this->setupContextMenu ();

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   this->connectNewVariableNameProperty
         (SLOT (useNewVariableNameProperty (QString, QString, unsigned int)));

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   this->titleVnpm.setVariableIndex (titleVariableIndex);
   QObject::connect
       (&this->titleVnpm, SIGNAL (newVariableNameProperty  (QString, QString, unsigned int)),
        this,             SLOT (useNewVariableNameProperty (QString, QString, unsigned int)));

   this->setSubstitutedTitleProperty (title);

   // Some events must be applied to the internal widgets
   //
   this->installEventFilter (this);
}

//---------------------------------------------------------------------------------
//
QSize QERadioGroup::sizeHint () const
{
   return QSize (140, 40);
}

//---------------------------------------------------------------------------------
// QEAbstractWidget (parent class) captures some of these events and does not
// call appropriate virtual function.  So must intercept there events here.
//
bool QERadioGroup::eventFilter (QObject* watched, QEvent* event)
{
   const QEvent::Type type = event->type ();
   bool result = false;

   switch (type) {
      case QEvent::FontChange:
         if (watched == this) {
            // Propagate font change to embedded buttons.
            //
            this->internalWidget->setFont (this->font());
            result = QEAbstractWidget::event (event);   // call parent fuction;
         }
         break;

      default:
         result = false;
   }

   return result;
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. A QCaObject that streams integers is required.
//
qcaobject::QCaObject* QERadioGroup::createQcaItem (unsigned int variableIndex)
{
   qcaobject::QCaObject* result = NULL;

   switch (variableIndex) {

      case pvVariableIndex:
         result = new QEInteger (this->getSubstitutedVariableName (variableIndex),
                                 this, &this->integerFormatting, variableIndex);

         // Apply currently defined array index/elements request values.
         //
         this->setSingleVariableQCaProperties (result);
         break;

      case titleVariableIndex:
         // do nothing
         result = NULL;
         break;

      default:
         DEBUG << "unexpected variableIndex" << variableIndex;
         result = NULL;
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QERadioGroup::activated ()
{
   // Ensure widget returns to default state when (re-)activated.
   //
   this->setStyleSheet ("");
   this->internalWidget->setValue (-1);
}

//------------------------------------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QERadioGroup::establishConnection (unsigned int variableIndex)
{
   qcaobject::QCaObject* qca = NULL;

   switch (variableIndex) {

      case pvVariableIndex:
         // Create a connection.
         // If successfull, the QCaObject object that will supply data update signals will be returned
         // Note createConnection creates the connection and returns reference to existing QCaObject.
         //
         qca = this->createConnection (variableIndex);

         // If a QCaObject object is now available to supply data update signals, connect it to the appropriate slots.
         //
         if (qca) {
            QObject::connect (qca,  SIGNAL (connectionChanged (QCaConnectionInfo&, const unsigned int&)),
                              this, SLOT   (connectionChanged (QCaConnectionInfo&, const unsigned int&)));

            QObject::connect (qca,  SIGNAL (integerChanged (const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                              this, SLOT   (valueUpdate    (const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));
         }
         break;

      case titleVariableIndex:
         // do nothing
         break;

      default:
         DEBUG << "unexpected variableIndex" << variableIndex;
         break;
   }
}

//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the s looks and change the tool tip
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QERadioGroup::connectionChanged (QCaConnectionInfo& connectionInfo,
                                      const unsigned int& variableIndex)
{
   bool isConnected;

   // Note the connected state
   //
   isConnected = connectionInfo.isChannelConnected ();

   // Display the connected state
   // Note: only one/first "variable" is a PV. Modify the tool tip class object
   //       to only display actual PV name and connection status.
   //
   this->setNumberToolTipVariables (1);
   this->updateToolTipConnection (isConnected, variableIndex);
   this->processConnectionInfo (isConnected, variableIndex);

   this->internalWidget->setEnabled (isConnected);

   // more trob. than it's worth to check if this is a connect or disconnect.
   //
   this->currentIndex = -1;

   // Set cursor to indicate access mode.
   //
   this->setAccessCursorStyle ();

   if (variableIndex == pvVariableIndex) {
      // Signal channel connection change to any (Link) widgets.
      // using signal dbConnectionChanged.
      //
      this->emitDbConnectionChanged (pvVariableIndex);
   }
}

//-----------------------------------------------------------------------------
//
void QERadioGroup::valueUpdate (const long &value,
                                QCaAlarmInfo & alarmInfo,
                                QCaDateTime &,
                                const unsigned int &variableIndex)
{
   int selectedIndex = -1;

   if (variableIndex != pvVariableIndex) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Get the associate channel object.
   //
   qcaobject::QCaObject* qca = this->getQcaItem (variableIndex);
   if (!qca) return;   // sanity check

   const bool isMetaDataUpdate = qca->getIsMetaDataUpdate();

   // If and only iff first/meta update (for this connection) then use enumeration
   // values to re-populate the radio group.
   //
   if (isMetaDataUpdate) {
      this->setRadioGroupText ();
   }

   // Set and save the selected index value.
   //
   this->currentIndex = value;

   if (this->valueIndexMap.containsF (value)) {
      selectedIndex = this->valueIndexMap.valueF (value, -1);
   } else {
      // We haven't mapped this value - use hidden selection.
      // This will uncheck all the "real" buttons.
      //
      selectedIndex = -1;
   }
   this->internalWidget->setValue (selectedIndex);

   // Invoke common alarm handling processing.
   // Only first "variable" is a PV.
   //
   this->setNumberToolTipVariables (1);
   this->processAlarmInfo (alarmInfo, variableIndex);

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged.
   //
   QString formattedText;
   formattedText = this->internalWidget->getStrings ().value (selectedIndex, "unknown");
   this->emitDbValueChanged (formattedText, pvVariableIndex);
}

//---------------------------------------------------------------------------------
//
void QERadioGroup::setRadioGroupText ()
{
   qcaobject::QCaObject* qca = NULL;
   QStringList enumerations;
   QString text;
   bool isMatch;
   int n;
   int j;

   // Build forward and reverse EPICS value to button index/position maps.
   // We do this even when using db enuberations and the mapping is trivial.
   //
   // Clear maps.
   //
   this->valueIndexMap.clear ();

   if (this->useDbEnumerations) {
      qca = this->getQcaItem (pvVariableIndex);
      if (qca) {
         enumerations = qca->getEnumerations ();

         // Create indentity map.
         //
         for (j = 0; j < enumerations.count (); j++) {
            this->valueIndexMap.insertF (j, j);
         }
      }

   } else {

      // Build up enumeration list using the local enumerations.  This may be
      // sparce: e.g.: 1 => Red, 5 => Blue, 63 => Green.  We create a reverse
      // map 0 => 1; 1 => 5; 2 => 63 so that when user selects the an element,
      // say Blue, we can map this directly to integer value of 5.
      //
      // Search upto values range -128 .. 128    NOTE: this is arbitary.
      // Maybe localEnumeration can be modified to provide a min/max value
      // or a list of values.
      //
      enumerations.clear ();
      for (n = -128; n <= 128; n++) {
         text = this->localEnumerations.valueToText (n, isMatch);

         // Unless exact match, do not use.
         //
         if (!isMatch) continue;
         if (text.isEmpty ()) continue;

         j = enumerations.count ();
         if (j >=this->internalWidget-> getMaximumButtons ()) {
            // We are full - ignore the rest.
            break;
         }
         enumerations.append (text);

         this->valueIndexMap.insertF (n, j);
      }
   }

   this->internalWidget->setStrings (enumerations);
}

//---------------------------------------------------------------------------------
//
void QERadioGroup::internalValueChanged (const int selectedIndex)
{
   // Get the variable to write to
   //
   QEInteger* qca = qobject_cast<QEInteger*> (this->getQcaItem (pvVariableIndex));

   // If a QCa object is present (if there is a variable to write to)
   // then write the value
   if (qca) {
      // Validate using inverse mapping.
      //
      if (!this->valueIndexMap.containsI (selectedIndex)) {
         return;
      }

      // Get the value associated with this button.
      //
      const int value = this->valueIndexMap.valueI (selectedIndex);

      // Don't write current value.
      // Do we actually need this check?
      //
      if (value == this->getCurrentIndex ()) {
         return;
      }

      // Write the value
      //
      qca->writeIntegerElement (value);
   }
}

//------------------------------------------------------------------------------
// Write a value immedietly. Keep in line with QEComboBox.
//
void QERadioGroup::writeNow()
{
   // Get the variable to write to
   //
   QEInteger* qca = qobject_cast<QEInteger*> (this->getQcaItem (pvVariableIndex));

   // If a QCa object is present (if there is a variable to write to)
   // then write the value.
   //
   if (qca) {
      const int selectedIndex = this->getCurrentIndex();

      // Validate using inverse mapping.
      //
      if (!this->valueIndexMap.containsI (selectedIndex)) {
         return;
      }

      // Get the value associated with this button.
      //
      const int value = this->valueIndexMap.valueI (selectedIndex);

      // Write the value
      //
      qca->writeIntegerElement (value);
   }
}

//------------------------------------------------------------------------------
// slot
void QERadioGroup::setDefaultStyle (const QString& style)
{
   this->setStyleDefault (style);
}

//------------------------------------------------------------------------------
// Note: keep aligned with QEComboBox::setPvValue
// slot
void QERadioGroup::setPvValue (const QString& text)
{
   // First check it text is one of the enumeration values.
   // If not, then check is a valid integer.
   //
   int value = this->internalWidget->findText (text);
   if (value == -1) {
      // Repeat with a trimmed string.
      value = this->internalWidget->findText (text.trimmed());
   }

   if (value >= 0 && !this->useDbEnumerations) {
      // We have a match and local enumeration is in use.
      // We must to the reverse map.
      //
      int temp;
      if (this->valueIndexMap.containsI(value)) {
         temp = this->valueIndexMap.valueI (value);
      } else {
         temp = -1;
      }
      value = temp;
   }

   if (value == -1) {
      // Try interpretting as an integer number.
      //
      bool okay;
      const int temp = text.toInt (&okay);
      if (okay) {
         value = temp;
      }
   }

   if (value >= 0) {
      this->setPvValue (value);
   } else {
      QString message = QString ("Cannot convert '%1' to an integer").arg (text);

      message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STANDARD);
      this->sendMessage (message, mt);
      DEBUG << message;
   }
}

//------------------------------------------------------------------------------
// slot
void QERadioGroup::setPvValue (const int value)
{
   this->internalWidget->setValue (value);
   this->currentIndex = value;   // QERadioGroup caches this locally - duplicate source of truth ;-(
   this->writeNow ();
}

//------------------------------------------------------------------------------
// slot
void QERadioGroup::setPvValue (const double value)
{
   static const int imin = std::numeric_limits<int>::min();
   static const int imax = std::numeric_limits<int>::max();

   if ((value >= imin) && (value <= imax)) {
      this->setPvValue (static_cast<int>(value));
   } else {
      QString message = QString ("Cannot convert '%1' to an integer").arg (value);

      message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STANDARD);
      this->sendMessage (message, mt);
      DEBUG << message;
   }
}

//------------------------------------------------------------------------------
// slot
void QERadioGroup::setPvValue (const bool value)
{
   this->setPvValue (value ? 1 : 0);
}


//------------------------------------------------------------------------------
//
int QERadioGroup::getCurrentIndex () const
{
   return this->currentIndex;
}

//------------------------------------------------------------------------------
//
void QERadioGroup::useNewVariableNameProperty (QString variableName,
                                               QString substitutions,
                                               unsigned int variableIndex)
{
   this->setVariableNameAndSubstitutions (variableName, substitutions, variableIndex);

   // Both the variable name and the title use the same useNewVariableNameProperty slot.
   //
   if (variableIndex == titleVariableIndex) {
      QString title = this->getSubstitutedVariableName (variableIndex);
      this->internalWidget->setOwnTitle (title);
   }
}

//==============================================================================
// Properties
// Update variable name etc.
//
//------------------------------------------------------------------------------
//
void QERadioGroup::setVariableNameSubstitutionsProperty (const QString& substitutions)
{
   // Call parent function
   //
   QESingleVariableMethods::setVariableNameSubstitutionsProperty (substitutions);

   // Also update title substitutions.
   //
   this->titleVnpm.setSubstitutionsProperty (substitutions);
}

//------------------------------------------------------------------------------
//
void QERadioGroup::setSubstitutedTitleProperty (const QString& substitutedTitle)
{
   this->titleVnpm.setVariableNameProperty (substitutedTitle);
}

//------------------------------------------------------------------------------
//
QString QERadioGroup::getSubstitutedTitleProperty () const
{
   return this->titleVnpm.getVariableNameProperty();
}

//------------------------------------------------------------------------------
//
void QERadioGroup::setUseDbEnumerations (bool useDbEnumerationsIn)
{
   if (this->useDbEnumerations != useDbEnumerationsIn) {
      this->useDbEnumerations = useDbEnumerationsIn;
      this->setRadioGroupText ();
   }
}

//------------------------------------------------------------------------------
//
bool QERadioGroup::getUseDbEnumerations () const
{
   return this->useDbEnumerations;
}

//------------------------------------------------------------------------------
//
void QERadioGroup::setLocalEnumerations (const QString& localEnumerationsIn)
{
   this->localEnumerations.setLocalEnumeration (localEnumerationsIn);
   if (!this->useDbEnumerations) {
      this->setRadioGroupText ();
   }
}

//------------------------------------------------------------------------------
//
QString QERadioGroup::getLocalEnumerations () const
{
   return this->localEnumerations.getLocalEnumeration ();
}

//==============================================================================
// Custom context menu
//
QMenu* QERadioGroup::buildContextMenu ()
{
   // Start with the standard QE Widget menu
   //
   QMenu* menu = QEWidget::buildContextMenu ();

   QAction* action;
   action = new QAction ("Apply current selection", menu);
   action->setCheckable (false);
   action->setData (QERG_APPLY_CURRENT_SELECTION);

   contextMenu::insertBefore (menu, action, contextMenu::CM_SHOW_PV_PROPERTIES);
   contextMenu::insertSeparatorBefore (menu, contextMenu::CM_SHOW_PV_PROPERTIES);

   return menu;
}

//------------------------------------------------------------------------------
//
void QERadioGroup::contextMenuTriggered (int selectedItemNum)
{
   switch (selectedItemNum) {

      case QERG_APPLY_CURRENT_SELECTION:
         this->writeNow();
         break;

      default:
         // Call parent class function.
         //
         QEWidget::contextMenuTriggered (selectedItemNum);
         break;
   }
}

//==============================================================================
// Copy / Paste
//
QString QERadioGroup::copyVariable ()
{
   return this->getSubstitutedVariableName (pvVariableIndex);
}

//------------------------------------------------------------------------------
//
QVariant QERadioGroup::copyData ()
{
   return QVariant (this->currentIndex);
}

//------------------------------------------------------------------------------
//
void QERadioGroup::paste (QVariant v)
{
   this->setVariableName (v.toString (), pvVariableIndex);
   this->establishConnection (pvVariableIndex);
}

// end
