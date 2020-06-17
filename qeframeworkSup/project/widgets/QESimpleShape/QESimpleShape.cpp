/*  QESimpleShape.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2020 Australian Synchrotron
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

#include "QESimpleShape.h"
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
//
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
   this->fillColour = this->getColor (invalid, 255);
   this->edgeAlarmState = DISPLAY_ALARM_STATE_ALWAYS;

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
   this->fillColour = QColor ("#ffffff");   // white
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
void QESimpleShape::setEdgeElementsRequired (const int elementsRequired)
{
   this->edge->setElementsRequired (elementsRequired);
}

//------------------------------------------------------------------------------
//
int QESimpleShape::getEdgeElementsRequired () const
{
   return this->edge->getElementsRequired ();
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

   // Note: essentially calls createQcaItem - provided the expanded PV name is not empty.
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

   if (variableIndex == MAIN_PV_INDEX) {
      QString pvName = this->getSubstitutedVariableName (variableIndex);

      // Note: we have dropped the interpretation of PV name as a literal integer.
      // Use a QSimpleShape widget instead.
      //
      result = new qcaobject::QCaObject (pvName, this, variableIndex);

      // Apply currently defined array index/elements request values.
      //
      this->setSingleVariableQCaProperties (result);

   } else if (variableIndex == EDGE_PV_INDEX) {
      QString pvName = this->getSubstitutedVariableName (variableIndex);
      result = new qcaobject::QCaObject (pvName, this, variableIndex);

      // Apply currently defined array index/elements request values.
      //
      this->edge->setSingleVariableQCaProperties (result);

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
      QObject::connect (qca,  SIGNAL (dataChanged   (const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int &)),
                        this, SLOT   (setShapeValue (const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int &)));

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
void QESimpleShape::setShapeValue (const QVariant& /* valueIn */, QCaAlarmInfo& alarmInfo,
                                   QCaDateTime&, const unsigned int& variableIndex)
{
   ASSERT_PV_INDEX (variableIndex, return);

   qcaobject::QCaObject* qca;
   QColor selectedEdgeColour;
   int channelValue;

   // Associated qca object - avoid the segmentation fault.
   //
   qca = this->getQcaItem (variableIndex);
   if (!qca) {
      return;
   }

   switch (variableIndex) {

      case MAIN_PV_INDEX:
         // Set up variable details used by some formatting options.
         //
         if (this->isFirstUpdate) {
            this->stringFormatting.setArrayAction (QEStringFormatting::INDEX);
            this->stringFormatting.setDbEgu (qca->getEgu ());
            this->stringFormatting.setDbEnumerations (qca->getEnumerations ());
            this->stringFormatting.setDbPrecision (qca->getPrecision ());
         }

         if (this->getTextFormat () == LocalEnumeration) {
            this->stringFormatting.setFormat (QEStringFormatting::FORMAT_LOCAL_ENUMERATE);
         } else {
            this->stringFormatting.setFormat (QEStringFormatting::FORMAT_DEFAULT);
         }

         // Update the shape value.
         // The value is independent of useAlarmColours.
         //
         // NOTE: If variant can't be converted to a number, this returns 0.
         //
         channelValue = int (qca->getIntegerValue ());

         if (this->useAlarmColours (this->getDisplayAlarmStateOption(), alarmInfo)) {
            // Save alarm colour.
            // Must do before we set value as getItemColour will get called.
            //
            this->fillColour = this->getColor (alarmInfo, 255);

         } else {
            // Save regular colour. This is essentally the same logic as in QSimpleShape
            // We want the modulo value to get the colour.
            // Note: % operator is remainder not modulo, so need to be smarter.
            //
            const int mod = this->getModulus();
            int val = channelValue % mod;
            if (val < 0) val += mod;
            this->fillColour = this->getColourProperty (val);
         }

         // Update the value in parent class.
         //
         this->setValue (channelValue);

         // This update is over, clear first update flag.
         //
         this->isFirstUpdate = false;
         break;

      case EDGE_PV_INDEX:
         // For now (at least) we treat everything not Never as Always.
         //
         if (this->useAlarmColours (this->edgeAlarmState, alarmInfo)) {
            selectedEdgeColour = this->getColor (alarmInfo, 255);
         } else {
            int ival = int (qca->getIntegerValue ());
            selectedEdgeColour = this->getColourProperty (ival & 15);
         }
         this->setEdgeColour (selectedEdgeColour);
         break;
   }

   // Invoke tool tip handling directly. We don't want to interfere with the style
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
// Determine if alarm colour to be used.
//
bool QESimpleShape::useAlarmColours (const standardProperties::displayAlarmStateOptions option,
                                     const QCaAlarmInfo& alarmInfo) const
{
   bool result = true;
   switch (option) {
     case standardProperties::DISPLAY_ALARM_STATE_NEVER:
         result = false;
         break;

     case standardProperties::DISPLAY_ALARM_STATE_ALWAYS:
         result = true;
         break;

     case standardProperties::DISPLAY_ALARM_STATE_WHEN_IN_ALARM:
         result = alarmInfo.isInAlarm();
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
//
QString QESimpleShape::getItemText ()
{
   QString result;

   switch (this->getTextFormat ()) {
      case QSimpleShape::PvText:
      case QSimpleShape::LocalEnumeration:
         {
            qcaobject::QCaObject* qca = this->getQcaItem (MAIN_PV_INDEX);
            if (!qca) break;  // sanity check
            if (!qca->getChannelIsConnected ()) break;
            bool isDefined;
            QVariant value;
            QCaAlarmInfo alarmInfo;
            QCaDateTime timeStamp;
            qca->getLastData (isDefined, value, alarmInfo, timeStamp);
            if (!isDefined) break;
            result = this->stringFormatting.formatString (value, this->getArrayIndex ());
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
   return this->fillColour;
}

//------------------------------------------------------------------------------
//
void QESimpleShape::stringFormattingChange()
{
   this->update ();
}

//------------------------------------------------------------------------------
//
QESimpleShape::DisplayAlarmStateOptions QESimpleShape::getEdgeAlarmStateOptionProperty () const
{
   return DisplayAlarmStateOptions (this->edgeAlarmState);
}

//------------------------------------------------------------------------------
//
void QESimpleShape::setEdgeAlarmStateOptionProperty (DisplayAlarmStateOptions option)
{
   this->edgeAlarmState = standardProperties::displayAlarmStateOptions (option);

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
   QString result;
   result = this->getSubstitutedVariableName (MAIN_PV_INDEX);
   if (!result.isEmpty()) {
      result.append (" ");
   }
   result.append (this->getSubstitutedVariableName (EDGE_PV_INDEX));
   return result;
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
   QStringList pvNameList;

   // v.toSring is a bit limiting when v is a StringList or a List of Strings, so
   // use common variantToStringList function which handles these options.
   //
   pvNameList = QEUtilities::variantToStringList (v);

   if (pvNameList.count() >= 1) {
      this->setVariableName (pvNameList.value (0, ""), MAIN_PV_INDEX);
      this->establishConnection (MAIN_PV_INDEX);
   }

   if (pvNameList.count() >= 2) {
      this->setVariableName (pvNameList.value (1, ""), EDGE_PV_INDEX);
      this->establishConnection (EDGE_PV_INDEX);
   }
}

// end
