/*  QERadioGroup.cpp
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
 *  Copyright (c) 2013,2014,2016,2017 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include "QERadioGroup.h"

#include <QDebug>
#include <QRadioButton>
#include <QPushButton>
#include <QECommon.h>

#define DEBUG qDebug () << "QERadioGroup" << __LINE__ << __FUNCTION__ << "  "

#define PV_VARIABLE_INDEX      0
#define TITLE_VARIABLE_INDEX   1

//-----------------------------------------------------------------------------
// Constructor with no initialisation
//
QERadioGroup::QERadioGroup (QWidget* parent) :
   QEAbstractWidget (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)
{
   this->commonSetup (" QERadioGroup ");
}

//-----------------------------------------------------------------------------
// Constructor with known variable
//
QERadioGroup::QERadioGroup (const QString& variableNameIn, QWidget* parent) :
   QEAbstractWidget (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)
{
   this->commonSetup (" QERadioGroup ");
   this->setVariableName (variableNameIn, PV_VARIABLE_INDEX);
   activate();
}

//-----------------------------------------------------------------------------
// Constructor with title and known variable
//
QERadioGroup::QERadioGroup (const QString& title, const QString& variableNameIn,
                            QWidget* parent) :
   QEAbstractWidget (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)
{
   this->commonSetup (title);
   this->setVariableName (variableNameIn, PV_VARIABLE_INDEX);
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
   // Create internal widget.
   //
   this->internalWidget = new QRadioGroup (title, this);

   // Copy actual widget size policy to the containing widget, then ensure
   // internal widget will expand to fill container widget.
   //
   this->setSizePolicy (this->internalWidget->sizePolicy ());
   this->internalWidget->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);

   this->layout = new QHBoxLayout (this);
   this->layout->setMargin (0);    // extact fit.
   this->layout->addWidget (this->internalWidget);

   QObject::connect (this->internalWidget, SIGNAL (valueChanged         (const int)),
                     this,                 SLOT   (internalValueChanged (const int)));

   // Set default property values
   //
   // Set up data
   // This control uses a single data source
   // We use 2nd "variable" for the title.
   //
   this->setNumVariables (2);

   // Set variable index used to select write access cursor style.
   //
   this->setControlPV (PV_VARIABLE_INDEX);

   // Title managed as second variable.
   //
   this->setVariableName (title, TITLE_VARIABLE_INDEX);

   this->setMinimumSize (120, 40);

   // Set up default properties
   //
   this->setVariableAsToolTip (true);
   this->setAllowDrop (false);
   this->setDisplayAlarmState (true);
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
   this->titleVnpm.setVariableIndex (TITLE_VARIABLE_INDEX);
   QObject::connect
       (&this->titleVnpm, SIGNAL (newVariableNameProperty  (QString, QString, unsigned int)),
        this,             SLOT (useNewVariableNameProperty (QString, QString, unsigned int)));
}

//---------------------------------------------------------------------------------
//
QSize QERadioGroup::sizeHint () const
{
   return QSize (200, 80);
}

//---------------------------------------------------------------------------------
//
void QERadioGroup::fontChange (const QFont&)
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
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. A QCaObject that streams integers is required.
//
qcaobject::QCaObject* QERadioGroup::createQcaItem (unsigned int variableIndex)
{
   qcaobject::QCaObject* result = NULL;

   switch (variableIndex) {

      case PV_VARIABLE_INDEX:
         result = new QEInteger (this->getSubstitutedVariableName (variableIndex),
                                 this, &this->integerFormatting, variableIndex);

         // Apply currently defined array index/elements request values.
         //
         this->setSingleVariableQCaProperties (result);
         break;

      case TITLE_VARIABLE_INDEX:
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

      case PV_VARIABLE_INDEX:
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

      case TITLE_VARIABLE_INDEX:
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
   this->isFirstUpdate = true;
   this->currentIndex = -1;

   // Set cursor to indicate access mode.
   //
   this->setAccessCursorStyle ();

   if (variableIndex == PV_VARIABLE_INDEX) {
      // Signal channel connection change to any (Link) widgets.
      // using signal dbConnectionChanged.
      //
      this->emitDbConnectionChanged (PV_VARIABLE_INDEX);
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

   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // If and only iff first update (for this connection) then use enumeration
   // values to re-populate the radio group.
   //
   if (this->isFirstUpdate) {
      this->isFirstUpdate = false;
      this->setRadioGroupText ();
   }

   // Set and save the selected index value.
   //
   this->currentIndex = value;

   if (this->valueToIndex.containsF (value)) {
      selectedIndex = this->valueToIndex.valueF (value, -1);
   } else {
      // We haven't mapped this value - use hidden selection.
      // This will uncheck all the "real" buttons
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
   this->emitDbValueChanged (formattedText, PV_VARIABLE_INDEX);
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

   // Build forward and revserse EPICS value to button index/position maps.
   // We do this even when using db enuberations and the mapping is trivial.
   //
   // Clear maps.
   //
   this->valueToIndex.clear ();

   if (this->useDbEnumerations) {
      qca = getQcaItem (PV_VARIABLE_INDEX);
      if (qca) {
         enumerations = qca->getEnumerations ();

         // Create indentity map.
         //
         for (j = 0; j < enumerations.count (); j++) {
            this->valueToIndex.insertF (j, j);
         }
      }

   } else {

      // Build up enumeration list using the local enumerations.  This may be
      // sparce    e.g.: 1 => Red, 5 => Blue, 63 => Green.  We create a reverse
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

         this->valueToIndex.insertF (n, j);
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
   QEInteger *qca = (QEInteger *) getQcaItem (PV_VARIABLE_INDEX);

   // If a QCa object is present (if there is a variable to write to)
   // then write the value
   if (qca) {
      int value;

      // Validate using inverse mapping.
      //
      if (!this->valueToIndex.containsI (selectedIndex)) {
         return;
      }

      // Get the value associated with this button.
      //
      value = this->valueToIndex.valueI (selectedIndex);

      // Don't write current value.
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
   if (variableIndex == TITLE_VARIABLE_INDEX) {
      QString title = this->getSubstitutedVariableName (variableIndex);
      this->internalWidget->setTitle (title);
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
// Copy / Paste
//
QString QERadioGroup::copyVariable ()
{
   return this->getSubstitutedVariableName (PV_VARIABLE_INDEX);
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
   this->setVariableName (v.toString (), PV_VARIABLE_INDEX);
   this->establishConnection (PV_VARIABLE_INDEX);
}

// end
