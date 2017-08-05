/*  QESimpleShape.cpp
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
 *  Copyright (c) 2013,2014,2016,2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QESimpleShape.h>
#include <alarm.h>
#include <QDebug>
#include <QECommon.h>

#define DEBUG qDebug () << "QESimpleShape" << __LINE__ << __FUNCTION__ << "  "

#define MAIN_PV_INDEX   0
#define EDGE_PV_INDEX   1

//-----------------------------------------------------------------------------
// Macro fuction to enure varable index has an expected value.
//
#define ASSERT_PV_INDEX(vi, action)   {                                \
   if ((vi != MAIN_PV_INDEX) && (vi != EDGE_PV_INDEX)) {               \
      DEBUG << "unexpected variableIndex" << vi;                       \
      action;                                                          \
   }                                                                   \
}


//-----------------------------------------------------------------------------
// Constructor with no initialisation
//
QESimpleShape::QESimpleShape (QWidget* parent) :
   QSimpleShape (parent),
   QEWidget (this),
   QESingleVariableMethods (this, MAIN_PV_INDEX)
{
   this->setup ();
}

//-----------------------------------------------------------------------------
// Constructor with known variable
//
QESimpleShape::QESimpleShape (const QString& variableNameIn, QWidget* parent) :
   QSimpleShape (parent),
   QEWidget (this),
   QESingleVariableMethods (this, MAIN_PV_INDEX)
{
   this->setup ();
   this->setVariableName (variableNameIn, 0);
   this->activate ();
}

//-----------------------------------------------------------------------------
QESimpleShape::~QESimpleShape ()
{
    if (this->edge) delete this->edge;
}

//-----------------------------------------------------------------------------
// Setup common to all constructors
//
void QESimpleShape::setup ()
{
   QCaAlarmInfo invalid (NO_ALARM, INVALID_ALARM);

   this->edge = new QESingleVariableMethods (this, EDGE_PV_INDEX);

   // Set up data
   //
   // This control uses two data sources
   //
   this->setNumVariables (2);
   this->setVariableAsToolTip (true);
   this->setDisplayAlarmStateOption (DISPLAY_ALARM_STATE_ALWAYS);
   this->setAllowDrop (false);
   this->setIsActive (false);

   // Set the initial state
   // Widget is inactive until connected.
   //
   this->isStaticValue = false;
   this->channelValue = 0;
   this->channelAlarmColour = this->getColor (invalid, 255);
   this->edgeAlarmState = Always;

   // Use default context menu.
   //
   this->setupContextMenu ();

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   this->connectNewVariableNameProperty
         (SLOT (useNewVariableNameProperty (QString, QString, unsigned int)));
   this->edge->connectNewVariableNameProperty
         (SLOT (useNewVariableNameProperty (QString, QString, unsigned int)));
}

//------------------------------------------------------------------------------
//
void QESimpleShape::activated ()
{
   // Ensure widget returns to default state when (re-)activated.
   //
   this->setIsActive (false);
   this->channelValue = 0;
   this->channelAlarmColour = QColor ("#ffffff");   // white
   this->setValue (0);
}

//------------------------------------------------------------------------------
//
void QESimpleShape::setVariableNameSubstitutionsProperty (const QString& substitutions)
{
   // Apply to both parent/inherited instabce and edge instance.
   //
   QESingleVariableMethods::setVariableNameSubstitutionsProperty (substitutions);
   this->edge->setVariableNameSubstitutionsProperty (substitutions);
}

//------------------------------------------------------------------------------
//
void QESimpleShape::setEdgeArrayIndex (const int arrayIndex)
{
   this->edge->setArrayIndex (arrayIndex);
}

//------------------------------------------------------------------------------
//
int QESimpleShape::getEdgeArrayIndex () const
{
   return this->edge->getArrayIndex ();
}

//------------------------------------------------------------------------------
//
void QESimpleShape::setEdgeVariableNameProperty (const QString& variableName)
{
   this->edge->setVariableNameProperty (variableName);
}

//------------------------------------------------------------------------------
//
QString QESimpleShape::getEdgeVariableNameProperty () const
{
   return this->edge->getVariableNameProperty ();
}

//------------------------------------------------------------------------------
// Update variable name etc.
//
void QESimpleShape::useNewVariableNameProperty (QString variableNameIn,
                                                QString variableNameSubstitutionsIn,
                                                unsigned int variableIndex)
{
   ASSERT_PV_INDEX (variableIndex, return);

   if (variableIndex == MAIN_PV_INDEX) this->isStaticValue = false;

   // Note: essentially calls createQcaItem - provided expanded pv name is not empty.
   //
   this->setVariableNameAndSubstitutions (variableNameIn,
                                          variableNameSubstitutionsIn,
                                          variableIndex);
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of QCaObject required.
// For shape, a QCaObject that streams integers is required.
//
qcaobject::QCaObject* QESimpleShape::createQcaItem (unsigned int variableIndex)
{
   ASSERT_PV_INDEX (variableIndex, return NULL);

   qcaobject::QCaObject* result = NULL;
   QString pvName;
   int number;
   bool okay;

   if (variableIndex == MAIN_PV_INDEX) {
      pvName = this->getSubstitutedVariableName (variableIndex);
      number = pvName.toInt (&okay);

      // Has designer/user just set an integer (as opposed to a PV name)?.
      // Note: no sensible PV names are just integers.
      //
      if (okay) {
         this->isStaticValue = true;
         this->channelValue = number;
         this->setValue (number);
         qDebug () << "QESimpleShape: PV name interpreted as integer depreciated - use a QSimpleShape widget instead";
      } else {
         // Assume it is a PV.
         //
         result = new QEInteger (pvName, this, &this->integerFormatting, variableIndex);

         // Apply currently defined array index.
         //
         this->setQCaArrayIndex (result);
      }

   } else if (variableIndex == EDGE_PV_INDEX) {
      pvName = this->getSubstitutedVariableName (variableIndex);
      result = new QEInteger (pvName, this, &this->integerFormatting, variableIndex);

      // Apply currently defined array index.
      //
      this->edge->setQCaArrayIndex (result);

   } else {
      result = NULL;         // Unexpected
   }

   return result;
}


//------------------------------------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QESimpleShape::establishConnection (unsigned int variableIndex)
{
   ASSERT_PV_INDEX (variableIndex, return);

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject* qca = this->createConnection (variableIndex);

   // If a QCaObject object is now available to supply data update signals,
   // connect it to the appropriate slots.
   //
   if (qca) {
      QObject::connect (qca,  SIGNAL (integerChanged  (const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int &)),
                        this, SLOT   (setShapeValue   (const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int &)));

      QObject::connect (qca,  SIGNAL (connectionChanged (QCaConnectionInfo&, const unsigned int &)),
                        this, SLOT   (connectionChanged (QCaConnectionInfo&, const unsigned int &)));
   }
}

//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the s looks and change the tool tip
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QESimpleShape::connectionChanged (QCaConnectionInfo & connectionInfo,
                                       const unsigned int& variableIndex)
{
   ASSERT_PV_INDEX (variableIndex, return);

   bool isConnected;

   // Note the connected state
   //
   isConnected = connectionInfo.isChannelConnected ();

   // Display the connected state
   //
   this->updateToolTipConnection (isConnected, variableIndex);

   if (variableIndex == MAIN_PV_INDEX) {
      // Widget draws itself - a styleSheet not applicable per se.
      // However we stillcall processConnectionInfo (isConnected)
      //
      this->processConnectionInfo (isConnected, variableIndex);
      this->setIsActive (isConnected);

      this->isFirstUpdate = true;  // more trob. than it's worth to check if connect or disconnect.

      // Signal channel connection change to any (Link) widgets.
      // using signal dbConnectionChanged.
      //
      this->emitDbConnectionChanged (MAIN_PV_INDEX);
   }
}

//------------------------------------------------------------------------------
// Update the shape value
// This is the slot used to recieve data updates from a QCaObject based class.
//
void QESimpleShape::setShapeValue (const long &valueIn, QCaAlarmInfo & alarmInfo,
                                   QCaDateTime &, const unsigned int& variableIndex)
{
   ASSERT_PV_INDEX (variableIndex, return);

   qcaobject::QCaObject* qca;
   QColor selectedEdgeColour;

   // Associated qca object - avoid the segmentation fault.
   //
   qca = getQcaItem (variableIndex);
   if (!qca) {
      return;
   }

   switch (variableIndex) {

      case MAIN_PV_INDEX:
         // Set up variable details used by some formatting options.
         //
         if (this->isFirstUpdate) {
            this->stringFormatting.setDbEgu (qca->getEgu ());
            this->stringFormatting.setDbEnumerations (qca->getEnumerations ());
            this->stringFormatting.setDbPrecision (qca->getPrecision ());
         }

         if (this->getTextFormat () == LocalEnumeration) {
            this->stringFormatting.setFormat (QEStringFormatting::FORMAT_LOCAL_ENUMERATE);
         } else {
            this->stringFormatting.setFormat (QEStringFormatting::FORMAT_DEFAULT);
         }

         // Save alarm colour.
         // Must do before we set value as getItemColour will get called.
         //
         this->channelAlarmColour = this->getColor (alarmInfo, 255);

         // Save value and update the shape value.
         // This essentially stores data twice, but the QSimpleShape stores the
         // modulo value, but we want to keep actual value (for getItemText).
         //
         this->channelValue = valueIn;
         this->setValue ((int) valueIn);

         // This update is over, clear first update flag.
         //
         this->isFirstUpdate = false;
         break;

      case EDGE_PV_INDEX:
         // For now (at least) we treat everything not Never as Always.
         //
         if (this->edgeAlarmState != Never) {
            selectedEdgeColour = this->getColor (alarmInfo, 255);
         } else {
            selectedEdgeColour = this->getColourProperty (valueIn & 15);
         }
         this->setEdgeColour (selectedEdgeColour);
         break;
   }

   // Invoke tool tip handling directly. We don;t want to interfer with the style
   // as widget draws it's own stuff with own, possibly clear, colours.
   //
   this->updateToolTipAlarm (alarmInfo.severityName (), variableIndex);

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged (for main variable only).
   //
   if (variableIndex == MAIN_PV_INDEX) {
      this->emitDbValueChanged (MAIN_PV_INDEX);
   }
}

//------------------------------------------------------------------------------
//
QString QESimpleShape::getItemText ()
{
   QString result;

   switch (this->getTextFormat ()) {
      case QSimpleShape::PvText:
      case QSimpleShape::LocalEnumeration:

         if (this->isStaticValue) {
            // There is no channel - just use a plain number.
            //
            result.setNum (this->channelValue);
         } else {
            result = this->stringFormatting.formatString (this->channelValue, this->getArrayIndex ());
         }
         break;

      default:
         // Just use base class function as is.
         //
         result = QSimpleShape::getItemText ();
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
//
QColor QESimpleShape::getItemColour ()
{
   QColor result;

   if (this->getDisplayAlarmState ()) {
      result = this->channelAlarmColour;
   } else {
      // Just use base class function as is.
      result = QSimpleShape::getItemColour ();
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QESimpleShape::stringFormattingChange()
{
   this->update ();
}

//------------------------------------------------------------------------------
//
QESimpleShape::DisplayAlarmStateOptions QESimpleShape::getEdgeAlarmStateOptionProperty ()
{
   return this->edgeAlarmState;
}

//------------------------------------------------------------------------------
//
void QESimpleShape::setEdgeAlarmStateOptionProperty (DisplayAlarmStateOptions option)
{
   this->edgeAlarmState = option;
   // Force update (if we can).
   //
   qcaobject::QCaObject* qca = this->getQcaItem (EDGE_PV_INDEX);
   if (qca) qca->resendLastData ();
}

//==============================================================================
// Copy / paste
//
QString QESimpleShape::copyVariable ()
{
   return this->getSubstitutedVariableName (MAIN_PV_INDEX);
}

//------------------------------------------------------------------------------
//
QVariant QESimpleShape::copyData ()
{
   return QVariant (this->getValue ());
}

//------------------------------------------------------------------------------
//
void QESimpleShape::paste (QVariant v)
{
   this->setVariableName (v.toString (), MAIN_PV_INDEX);
   this->establishConnection (MAIN_PV_INDEX);
}

// end
