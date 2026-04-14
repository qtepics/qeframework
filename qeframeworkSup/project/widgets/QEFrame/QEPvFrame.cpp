/*  QEPvFrame.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2016-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include <QDebug>
#include <QECommon.h>
#include <QEPvFrame.h>

#define DEBUG qDebug () << "QEPvFrame" <<  __LINE__ << __FUNCTION__ << "  "

#define PV_VARIABLE_INDEX      0

//------------------------------------------------------------------------------
//
QEPvFrame::QEPvFrame (QWidget* parent) :
   QEFrame (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)
{
   this->commonSetup ();
}

//------------------------------------------------------------------------------
//
QEPvFrame::QEPvFrame (const QString& variableNameIn, QWidget* parent) :
   QEFrame (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)
{
   this->commonSetup ();
   this->setVariableName (variableNameIn, PV_VARIABLE_INDEX);
   this->activate ();
}

//------------------------------------------------------------------------------
//
QEPvFrame::~QEPvFrame ()
{
}

//------------------------------------------------------------------------------
//
void QEPvFrame::commonSetup ()
{
   this->setSelectPixmap (-1);  // none selected

   // Set default property values
   //
   this->setNumVariables (1);
   this->setVariableAsToolTip (false);

   // Set up default properties
   //
   this->setAllowDrop (false);

   // Use default context menu.
   //
   this->setupContextMenu ();

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   this->connectPvNameProperties (SLOT (usePvNameProperties (const QEPvNameProperties&)));
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type
// of QEChannel required. For a PV Frame widget a QEChannel that streams
// integers is required.
//
QEChannel* QEPvFrame::createQcaItem (unsigned int variableIndex)
{
   QEChannel* result = NULL;

   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return NULL;
   }

   const QString pvName = this->getSubstitutedVariableName (variableIndex);
   result = new QEInteger (pvName, this, &this->integerFormatting, variableIndex);

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
void QEPvFrame::establishConnection (unsigned int variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
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
      QObject::connect (qca,  SIGNAL (valueUpdated  (const QEIntegerValueUpdate&)),
                        this, SLOT   (pvValueUpdate (const QEIntegerValueUpdate&)));

      QObject::connect (qca,  SIGNAL (connectionUpdated (const QEConnectionUpdate&)),
                        this, SLOT   (connectionUpdated (const QEConnectionUpdate&)));
   }
}


//------------------------------------------------------------------------------
// This is the slot used to recieve new PV information.
//
void QEPvFrame::usePvNameProperties (const QEPvNameProperties& pvNameProperties)
{
   this->setVariableNameAndSubstitutions (pvNameProperties.pvName,
                                          pvNameProperties.substitutions,
                                          pvNameProperties.index);
}

//------------------------------------------------------------------------------
//
void QEPvFrame::connectionUpdated (const QEConnectionUpdate& update)
{
   const unsigned int vi = update.variableIndex;

   if (vi != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << vi;
      return;
   }

   // Note the connected state.
   //
   const bool isConnected = update.connectionInfo.isChannelConnected ();

   // Display the connected state
   //
   this->updateToolTipConnection (isConnected, vi);
   this->processConnectionInfo (isConnected, vi);

   // Signal channel connection change to any (Link) widgets.
   // using signal dbConnectionChanged.
   //
   this->emitDbConnectionChanged (vi);
}

//------------------------------------------------------------------------------
//
void QEPvFrame::pvValueUpdate (const QEIntegerValueUpdate& update)
{
   const unsigned int vi = update.variableIndex;

   if (vi != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << vi;
      return;
   }

   // Update QEFrame - select pixmap
   // Constrain modulo 16 ??? Set -1 if invalid ???
   //
   this->setSelectPixmap ((int) update.value);

   // Invoke common alarm handling processing.
   // Do we really want to do this ???
   //
   this->processAlarmInfo (update.alarmInfo, vi);

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged.
   //
   this->emitDbValueChanged (vi);
}

//==============================================================================
// Copy (no paste)
//
QString QEPvFrame::copyVariable ()
{
   return this->getSubstitutedVariableName (PV_VARIABLE_INDEX);
}

//------------------------------------------------------------------------------
//
QVariant QEPvFrame::copyData ()
{
   // or should we copy the pix map itself.
   return QVariant ( int (this->getSelectedPixmap ()));
}

// end
