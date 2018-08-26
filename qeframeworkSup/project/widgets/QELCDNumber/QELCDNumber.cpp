/*  QELCDNumber.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2018 Australian Synchrotron
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

#include "QELCDNumber.h"
#include <alarm.h>
#include <QDebug>
#include <QECommon.h>
#include <QCaObject.h>
#include <QEStringFormatting.h>

#define DEBUG qDebug () << "QELCDNumber" << __LINE__ << __FUNCTION__ << "  "

#define PV_VARIABLE_INDEX      0

//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QELCDNumber::QELCDNumber (QWidget * parent):
   QEFrame (parent),
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
   QEFrame (parent),
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
   this->layout->setMargin (0);    // extact fit.
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
   this->setArrayAction (QEStringFormatting::INDEX);
   this->setAllowDrop (false);

   this->lastValue = QVariant (QVariant::Invalid);

   // Use default context menu.
   //
   this->setupContextMenu ();

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   const char* slot = SLOT (useNewVariableNameProperty (QString, QString, unsigned int));
   this->connectNewVariableNameProperty (slot);
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of QCaObject required.
// For a progress bar a QCaObject that streams integers is required.
//
qcaobject::QCaObject* QELCDNumber::createQcaItem (unsigned int variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) return NULL;  // sanity check

   qcaobject::QCaObject* result = NULL;

   result = new QEFloating (getSubstitutedVariableName (variableIndex), this,
                            &this->floatingFormatting, variableIndex);

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
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject* qca = this->createConnection (variableIndex);

   // If a QCaObject object is now available to supply data update signals, connect it to the appropriate slots.
   //
   if ((qca) && (variableIndex == PV_VARIABLE_INDEX)) {
      QObject::connect (qca,  SIGNAL (floatingChanged (const double &, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)),
                        this, SLOT   (setPvValue      (const double &, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)));

      QObject::connect (qca, SIGNAL (connectionChanged (QCaConnectionInfo &,const unsigned int&)),
                        this, SLOT  (connectionChanged (QCaConnectionInfo &,const unsigned int&)));
   }
}


//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the widget looks and change the tool tip
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QELCDNumber::connectionChanged (QCaConnectionInfo& connectionInfo,
                                     const unsigned int& variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) return;  // sanity check

   // Note the connected state
   //
   const bool isConnected = connectionInfo.isChannelConnected ();

   // We can do this on connect as well as disconnect.
   //
   this->lastValue = QVariant (QVariant::Invalid);
   this->isFirstUpdate = true;

   // Display the connected state
   this->updateToolTipConnection (isConnected, variableIndex);

   // Change style to reflect being connected/disconnected.
   //
   this->processConnectionInfo (isConnected, variableIndex);

   // Signal channel connection change to any (Link) widgets.
   // using signal dbConnectionChanged.
   //
   this->emitDbConnectionChanged (variableIndex);
}

//------------------------------------------------------------------------------
// Update the progress bar value
// This is the slot used to recieve data updates from a QCaObject based class.
//
void QELCDNumber::setPvValue (const double& value,
                              QCaAlarmInfo& alarmInfo,
                              QCaDateTime&,
                              const unsigned int& variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) return;  // sanity check

   // Associated qca object - avoid the segmentation fault.
   //
   qcaobject::QCaObject* qca = getQcaItem (PV_VARIABLE_INDEX);
   if (!qca)
      return;

   if (this->isFirstUpdate) {

      // Set up variable details used by some formatting options
      //
      this->stringFormatting.setAddUnits (false);   // strictly numeric
      this->stringFormatting.setDbPrecision (qca->getPrecision ());
   }

   // Form and save the image - must do before call to setValue.
   //
   QString theImage = this->stringFormatting.formatString (value, this->getArrayIndex());

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
   this->lastValue = QVariant (double (value));

   // Invoke common alarm handling processing.
   //
   this->processAlarmInfo (alarmInfo, variableIndex);

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged signals declared in header file.
   //
   this->emitDbValueChanged (variableIndex);

   // This update is over, clear first update flag.
   //
   this->isFirstUpdate = false;
}

//------------------------------------------------------------------------------
// Update variable name etc.
//
void QELCDNumber::useNewVariableNameProperty (QString pvName,
                                              QString subs,
                                              unsigned int pvi)
{
   this->setVariableNameAndSubstitutions (pvName, subs, pvi);
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
void QELCDNumber::setNotationProperty (Notations notationIn)
{
   this->setNotation (QEStringFormatting::notations (notationIn));
}

//------------------------------------------------------------------------------
//
QELCDNumber::Notations QELCDNumber::getNotationProperty() const
{
   return Notations (this->getNotation());
}

// end
