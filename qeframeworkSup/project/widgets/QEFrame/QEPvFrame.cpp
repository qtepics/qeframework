/*  QEPvFrame.cpp
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
 *  Copyright (c) 2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
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
   this->connectNewVariableNameProperty (SLOT (useNewVariableNameProperty (QString, QString, unsigned int)));
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type
// of QCaObject required. For a PV Frame widget a QCaObject that streams
// integers is required.
//
qcaobject::QCaObject * QEPvFrame::createQcaItem (unsigned int variableIndex)
{
   qcaobject::QCaObject * result = NULL;

   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return NULL;
   }

   result = new QEInteger (getSubstitutedVariableName (variableIndex),
                           this, &this->integerFormatting, variableIndex);

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
void QEPvFrame::establishConnection (unsigned int variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject * qca = createConnection (variableIndex);

   // If a QCaObject object is now available to supply data update signals,
   // connect it to the appropriate slots.
   //
   if (qca) {
      QObject::connect (qca, SIGNAL (integerChanged (const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                        this, SLOT  (pvValueUpdate  (const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));

      QObject::connect (qca,  SIGNAL (connectionChanged (QCaConnectionInfo&, const unsigned int&)),
                        this, SLOT   (connectionChanged (QCaConnectionInfo&, const unsigned int&)));
   }
}


//------------------------------------------------------------------------------
// This is the slot used to recieve new PV information.
//
void QEPvFrame::useNewVariableNameProperty (QString pvName,
                                            QString pvNameSubstitutions,
                                            unsigned int pvIndex)
{
   this->setVariableNameAndSubstitutions (pvName, pvNameSubstitutions, pvIndex);
}

//------------------------------------------------------------------------------
//
void QEPvFrame::connectionChanged (QCaConnectionInfo & connectionInfo,
                                   const unsigned int &variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Note the connected state.
   //
   const bool isConnected = connectionInfo.isChannelConnected ();

   // Display the connected state
   //
   this->updateToolTipConnection (isConnected, variableIndex);
   this->processConnectionInfo (isConnected, variableIndex);

   // Signal channel connection change to any (Link) widgets.
   // using signal dbConnectionChanged.
   //
   this->emitDbConnectionChanged (variableIndex);
}

//------------------------------------------------------------------------------
//
void QEPvFrame::pvValueUpdate (const long &value, QCaAlarmInfo & alarmInfo,
                               QCaDateTime &, const unsigned int &variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Update QEFrame - select pixmap
   // Constrain modulo 8 ??? Set -1 if invalid ???
   //
   this->setSelectPixmap ((int) value);

   // Invoke common alarm handling processing.
   // Do we really want to do this ???
   //
   this->processAlarmInfo (alarmInfo, variableIndex);

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged.
   //
   this->emitDbValueChanged (variableIndex);
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
