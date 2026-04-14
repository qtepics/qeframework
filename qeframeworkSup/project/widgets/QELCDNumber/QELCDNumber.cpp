/*  QELCDNumber.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2018-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QELCDNumber.h"
#include <alarm.h>
#include <QDebug>
#include <QECommon.h>
#include <QEStringFormatting.h>

#define DEBUG qDebug () << "QELCDNumber" << __LINE__ << __FUNCTION__ << "  "

#define PV_VARIABLE_INDEX      0

//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QELCDNumber::QELCDNumber (QWidget * parent):
   ParentWidgetClass (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX),
   QEStringFormattingMethods ()
{
   this->setup ();
}

//------------------------------------------------------------------------------
// Constructor with known variable
//
QELCDNumber::QELCDNumber (const QString & variableNameIn,
                          QWidget * parent) :
   ParentWidgetClass (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX),
   QEStringFormattingMethods ()
{
   this->setup ();
   this->setVariableName (variableNameIn, PV_VARIABLE_INDEX);
   this->activate ();
}

//------------------------------------------------------------------------------
//
QELCDNumber::~QELCDNumber() { }   // place holder

//------------------------------------------------------------------------------
//
QSize QELCDNumber::sizeHint () const
{
   return QSize (64, 24);
}

//------------------------------------------------------------------------------
// Setup common to all constructors
//
void QELCDNumber::setup ()
{
   // Override parant default properties.
   //
   this->setVariableAsToolTip (true);

   // Create internal widget.
   //
   this->internalWidget = new QLCDNumber (this);

   // Copy actual widget size policy to the containing widget, then ensure
   // internal widget will expand to fill container widget.
   //
   this->setSizePolicy (this->internalWidget->sizePolicy ());
   this->internalWidget->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);

   this->layout = new QHBoxLayout (this);
   this->layout->setContentsMargins (0, 0, 0, 0);    // extact fit.
   this->layout->addWidget (this->internalWidget);

   this->setMinimumSize (this->internalWidget->minimumSize ());

   // Copy default framing to container, and clear from internal widget.
   //
   this->setFrameShape (this->internalWidget->frameShape());
   this->setFrameShadow (this->internalWidget->frameShadow());

   this->internalWidget->setFrameShape (QFrame::NoFrame);
   this->internalWidget->setFrameShadow (QFrame::Plain);

   // Set up data
   // This control used a single data source
   //
   this->setNumVariables (1);

   // Set up default properties
   //
   this->setArrayAction (QE::Index);
   this->setAllowDrop (false);

   this->lastValue = QVariant ();

   // Use default context menu.
   //
   this->setupContextMenu ();

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   const char* slot = SLOT (usePvNameProperties (const QEPvNameProperties&));
   this->connectPvNameProperties (slot);
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of QEChannel required.
// For a progress bar a QEChannel that streams integers is required.
//
QEChannel* QELCDNumber::createQcaItem (unsigned int variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) return NULL;  // sanity check

   QEChannel* result = NULL;

   const QString pvName = this->getSubstitutedVariableName (variableIndex);
   result = new QEFloating (pvName, this, &this->floatingFormatting, variableIndex);

   // Apply currently defined array index and elements request values.
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
void QELCDNumber::establishConnection (unsigned int variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) return;  // sanity check

   // Create a connection.
   // If successfull, the QEChannel object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QEChannel.
   //
   QEChannel* qca = this->createConnection (variableIndex);

   // If a QEChannel object is now available to supply data update signals, connect it to the appropriate slots.
   //
   if ((qca) && (variableIndex == PV_VARIABLE_INDEX)) {
      QObject::connect (qca,  SIGNAL (valueUpdated (const QEFloatingValueUpdate&)),
                        this, SLOT   (setPvValue   (const QEFloatingValueUpdate&)));

      QObject::connect (qca, SIGNAL (connectionUpdated (const QEConnectionUpdate&)),
                        this, SLOT  (connectionUpdated (const QEConnectionUpdate&)));
   }
}

//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the widget looks and change the tool tip
// This is the slot used to recieve connection updates from a QEChannel based class.
//
void QELCDNumber::connectionUpdated (const QEConnectionUpdate& update)
{
   const unsigned int vi = update.variableIndex;

   if (vi != PV_VARIABLE_INDEX) return;  // sanity check

   // Note the connected state
   //
   const bool isConnected = update.connectionInfo.isChannelConnected ();

   // We can do this on connect as well as disconnect.
   //
   this->lastValue = QVariant ();

   // Display the connected state
   this->updateToolTipConnection (isConnected, vi);

   // Change style to reflect being connected/disconnected.
   //
   this->processConnectionInfo (isConnected, vi);

   // Signal channel connection change to any (Link) widgets.
   // using signal dbConnectionChanged.
   //
   this->emitDbConnectionChanged (vi);
}

//------------------------------------------------------------------------------
// Update the LCD Number value
// This is the slot used to recieve data updates from a QEChannel based class.
//
void QELCDNumber::setPvValue (const QEFloatingValueUpdate& update)
{
   const unsigned int vi = update.variableIndex;

   if (vi != PV_VARIABLE_INDEX) return;  // sanity check

   // Associated qca object - avoid the segmentation fault.
   //
   QEChannel* qca = getQcaItem (vi);
   if (!qca) return;   // sanity check

   if (update.isMetaUpdate) {
      // Set up variable details used by some formatting options
      //
      this->stringFormatting.setAddUnits (false);   // strictly numeric
      this->stringFormatting.setDbPrecision (qca->getPrecision ());
   }

   // Form and save the image - must do before call to setValue.
   //
   QString theImage = this->stringFormatting.formatString (update.value, this->getArrayIndex());

   int digitCount = theImage.length ();   // minimum required

   double lower = qca->getDisplayLimitLower ();
   double upper = qca->getDisplayLimitUpper ();

   // Check that sensible limits have been defined and not just left
   // at the default (i.e. zero) values by a lazy database creator.
   //
   if ((lower != 0.0) || (upper != 0.0)) {
      // +1 for round up, +1 for decimal point, +1 for sign plus the precision.
      //
      int n = LOG10 (MAX (ABS (lower), ABS(upper))) + 3 + qca->getPrecision ();
      digitCount = MAX (n, digitCount);
   }

   this->internalWidget->setDigitCount (digitCount);
   this->internalWidget->display (theImage);
   this->lastValue = QVariant (double (update.value));

   // Invoke common alarm handling processing.
   //
   this->processAlarmInfo (update.alarmInfo, vi);

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged signals declared in header file.
   //
   this->emitDbValueChanged (vi);
}

//------------------------------------------------------------------------------
// Update variable name etc.
//
void QELCDNumber::usePvNameProperties (const QEPvNameProperties& pvNameProperties)
{
   this->setVariableNameAndSubstitutions (pvNameProperties.pvName,
                                          pvNameProperties.substitutions,
                                          pvNameProperties.index);
}

//==============================================================================
// Copy (no paste)
//
QString QELCDNumber::copyVariable ()
{
   return this->getSubstitutedVariableName (PV_VARIABLE_INDEX);
}

//------------------------------------------------------------------------------
//
QVariant QELCDNumber::copyData ()
{
   return QVariant (this->lastValue);
}

//------------------------------------------------------------------------------
//
void QELCDNumber::paste (QVariant drop)
{
   this->setVariableName (drop.toString (), PV_VARIABLE_INDEX);
   this->establishConnection (0);
}

//------------------------------------------------------------------------------
//
void QELCDNumber::setNotationProperty (QE::Notations notationIn)
{
   this->setNotation (notationIn);
}

//------------------------------------------------------------------------------
//
QE::Notations QELCDNumber::getNotationProperty() const
{
   return this->getNotation();
}

// end
