/*  QEAnalogSlider.cpp
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

#include "QEAnalogSlider.h"
#include <alarm.h>
#include <QDebug>
#include <QRadioButton>
#include <QPushButton>
#include <QECommon.h>
#include <QEDisplayRanges.h>
#include <QEPvPropertiesUtilities.h>

#define DEBUG qDebug () << "QEAnalogSlider" << __LINE__ << __FUNCTION__ << " "

#define SET_POINT_VARIABLE_INDEX      0
#define READ_BACK_VARIABLE_INDEX      1

// Marker 0 used by parent class for save/revert value.
//
#define SET_POINT_MARKER              1
#define READ_BACK_MARKER              2

//-----------------------------------------------------------------------------
// Constructor with no initialisation
//
QEAnalogSlider::QEAnalogSlider (QWidget* parent) :
   QAnalogSlider (parent),
   QESingleVariableMethods (this, SET_POINT_VARIABLE_INDEX),
   QEWidget (this)
{
   this->commonSetup ();
}

//-----------------------------------------------------------------------------
// Constructor with known variable
//
QEAnalogSlider::QEAnalogSlider (const QString& variableNameIn,
                                const QString& readbackNameIn,
                                QWidget* parent) :
   QAnalogSlider (parent),
   QESingleVariableMethods (this, SET_POINT_VARIABLE_INDEX),
   QEWidget (this)
{
   this->commonSetup ();
   this->setVariableName (variableNameIn, SET_POINT_VARIABLE_INDEX);

   this->setVariableName (readbackNameIn, READ_BACK_VARIABLE_INDEX);
   this->activate();
}

//-----------------------------------------------------------------------------
// Setup common to all constructors
//
void QEAnalogSlider::commonSetup ()
{
   // Create second single variable methods object for the readback PV.
   //
   this->readback = new QESingleVariableMethods (this, READ_BACK_VARIABLE_INDEX);

   // Connect inherited valueChanged signal to own valueChanged slot.
   // Use virtual function in stread?
   //
   QObject::connect (this, SIGNAL (valueChanged (const double)),
                     this, SLOT   (valueChanged (const double)));

   // Set default property values
   //
   this->setNumVariables (2);

   // Set variable index used to select write access cursor style.
   //
   this->setControlPV (SET_POINT_VARIABLE_INDEX);

   // Set up default properties
   //
   this->setVariableAsToolTip (true);
   this->setAllowDrop (false);
   this->setDisplayAlarmStateOption (QE::Never);
   this->setIsActive (false);

   this->setShowSaveRevert (true);
   this->setShowApply (true);

   this->mAutoScale = true;
   this->mContinuousWrite = false;
   this->mAxisAlarmColours = false;
   this->isConnected = false;

   this->stringFormatting.setArrayAction (QE::Index);

   // Use default context menu.
   //
   this->setupContextMenu ();

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   this->connectNewVariableNameProperty
         (SLOT (useNewVariableNameProperty (QString, QString, unsigned int)));

   this->readback->connectNewVariableNameProperty
         (SLOT (useNewVariableNameProperty (QString, QString, unsigned int)));
}

//------------------------------------------------------------------------------
//
QEAnalogSlider::~QEAnalogSlider()
{
   if (this->readback) delete this->readback;
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. A QCaObject that streams integers is required.
//
qcaobject::QCaObject* QEAnalogSlider::createQcaItem (unsigned int variableIndex)
{
   qcaobject::QCaObject* result = NULL;

   switch (variableIndex) {
      case SET_POINT_VARIABLE_INDEX:
         result = new QEFloating (this->getSubstitutedVariableName (variableIndex),
                                  this, &this->floatingFormatting, variableIndex);

         // Apply currently defined array index/elements request values.
         //
         this->setSingleVariableQCaProperties (result);
         break;

      case READ_BACK_VARIABLE_INDEX:
         result = new QEString (this->getSubstitutedVariableName (variableIndex),
                                this, &this->stringFormatting, variableIndex);

         // Apply currently defined array index/elements request values.
         //
         this->readback->setSingleVariableQCaProperties (result);
         break;

      default:
         DEBUG << "unexpected variableIndex" << variableIndex;
         result = NULL;
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QEAnalogSlider::establishConnection (unsigned int variableIndex)
{
   qcaobject::QCaObject* qca = NULL;

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   switch (variableIndex) {

      case SET_POINT_VARIABLE_INDEX:
         qca = this->createConnection (variableIndex);
         if (!qca) break;

         QObject::connect (qca,  SIGNAL (connectionChanged     (QCaConnectionInfo&, const unsigned int&)),
                           this, SLOT   (mainConnectionChanged (QCaConnectionInfo&, const unsigned int&)));

         QObject::connect (qca,  SIGNAL (floatingChanged (const double&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                           this, SLOT   (floatingChanged (const double&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));
         break;

      case READ_BACK_VARIABLE_INDEX:
         qca = this->createConnection (variableIndex);
         if (!qca) break;

         QObject::connect (qca,  SIGNAL (connectionChanged          (QCaConnectionInfo&, const unsigned int&)),
                           this, SLOT   (secondaryConnectionChanged (QCaConnectionInfo&, const unsigned int&)));

         QObject::connect (qca,  SIGNAL (stringChanged (const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                           this, SLOT   (stringChanged (const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));
         break;

      default:
         DEBUG << "unexpected variableIndex" << variableIndex;
         break;
   }
}

//------------------------------------------------------------------------------
// Need to do some smarts if/when not radix 10.
//
void QEAnalogSlider::calculateAutoValues (qcaobject::QCaObject* qca)
{
   if (!qca) return;   // sainity check.
   if (!this->getAutoScale()) return;   // no auto scaling

   // Do the auto scale calculations.
   //
   double ctrlLow;
   double ctrlUpp;

   // Check that sensible limits have been defined and not just left
   // at the default (i.e. zero) values by a lazy database creator.
   // Otherwise, leave as design time limits.
   //
   ctrlLow = qca->getControlLimitLower ();
   ctrlUpp = qca->getControlLimitUpper ();

   // If control limits are undefined - try display limits.
   //
   if ((ctrlLow == 0.0) && (ctrlUpp == 0.0)) {
      ctrlLow = qca->getDisplayLimitLower ();
      ctrlUpp = qca->getDisplayLimitUpper ();
   }

   // If control and display limits are undefined - forget it.
   //
   if ((ctrlLow == 0.0) && (ctrlUpp == 0.0)) return;
   if (ctrlUpp < ctrlLow) {
      DEBUG << "PV" << qca->getPvName() << " poorly defined limits ignored";
      return;
   }

   double dummy;  // we are not interested in modified limits here.
   double major;

   const QEDisplayRanges displayRange (ctrlLow, ctrlUpp);
   displayRange.adjustMinMax (10, false, dummy, dummy, major);

   const double minor = major / 5.0;
   const int precision = qca->getPrecision ();

   // Now we can update the parent widget.
   //
   this->setMinimum (ctrlLow);
   this->setMaximum (ctrlUpp);
   this->setMinorInterval (minor);
   this->setMajorInterval (major);
   this->setPrecision (precision);
}

//------------------------------------------------------------------------------
//
void QEAnalogSlider::calcColourBandList ()
{
   qcaobject::QCaObject* qca = this->getQcaItem (SET_POINT_VARIABLE_INDEX);

   QEColourBandList bandList;

   if (qca && this->mAxisAlarmColours) {
      bandList.setAlarmColours (this->getMinimum (), this->getMaximum(), qca);
   }

   this->setColourBandList (bandList);
}


//------------------------------------------------------------------------------
// On activation, treat as if invalid until we know better.
//
void QEAnalogSlider::activated ()
{
   QCaAlarmInfo alarmInfo (0, INVALID_ALARM);
   this->processAlarmInfo (alarmInfo, SET_POINT_VARIABLE_INDEX);
   this->setLeftText ("");
   this->setCentreText ("");
}

//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the s looks and change the tool tip
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QEAnalogSlider::mainConnectionChanged (QCaConnectionInfo& connectionInfo,
                                            const unsigned int& variableIndex)
{
   // Note the connected state
   //
   this->isConnected = connectionInfo.isChannelConnected ();

   // Display the connected state
   //
   this->updateToolTipConnection (this->isConnected, variableIndex);

   // Is this the main control PV.
   //
   this->setIsActive (this->isConnected);

   if (this->isConnected) {
      QEAxisPainter* ap = this->getAxisPainter ();
      ap->setMarkerVisible (SET_POINT_MARKER, false);
   }

   // Set cursor to indicate access mode.
   //
   this->setAccessCursorStyle ();

   // Signal a channel connection change to any Link (or other) widgets using one
   // of the dbValueChanged signals declared in header file.
   //
   this->emitDbConnectionChanged (variableIndex);
}

//-----------------------------------------------------------------------------
//
void QEAnalogSlider::secondaryConnectionChanged (QCaConnectionInfo& connectionInfo,
                                                 const unsigned int& variableIndex)
{
   // Display the connected state
   //
   this->updateToolTipConnection (connectionInfo.isChannelConnected (), variableIndex);

   if (connectionInfo.isChannelConnected ()) {
      QEAxisPainter* ap = this->getAxisPainter ();
      ap->setMarkerVisible (READ_BACK_MARKER, false);
   }

   // Set cursor to indicate access mode.
   //
   this->setAccessCursorStyle ();
}

//-----------------------------------------------------------------------------
//
void QEAnalogSlider::floatingChanged (const double& value,
                                      QCaAlarmInfo& alarmInfo,
                                      QCaDateTime&,
                                      const unsigned int& variableIndex)
{
   if (variableIndex != SET_POINT_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Associated qca object - avoid the segmentation fault.
   //
   qcaobject::QCaObject* qca = this->getQcaItem (variableIndex);
   if (!qca) return;   // sanity check

   const bool isMetaDataUpdate = qca->getIsMetaDataUpdate();

   if (isMetaDataUpdate && (variableIndex == SET_POINT_VARIABLE_INDEX)) {
      // Determine auto scaling values based on the PV's meta data.
      //
      this->calculateAutoValues (qca);
      this->updateAxisAndSlider ();
      this->calcColourBandList ();
   }

   // Repositon the slider to refect current database value.
   //
   this->setValue (value);

   QEAxisPainter* ap = this->getAxisPainter ();

   ap->setMarkerValue (SET_POINT_MARKER, value);
   ap->setMarkerColour (SET_POINT_MARKER, QColor (255, 155, 55));
   ap->setMarkerVisible (SET_POINT_MARKER, true);

   // Invoke common alarm handling processing.
   //
   this->processAlarmInfo (alarmInfo, variableIndex);

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged signals declared in header file.
   //
   this->emitDbValueChanged (variableIndex);
}

//-----------------------------------------------------------------------------
//
void QEAnalogSlider::stringChanged (const QString& value,
                                    QCaAlarmInfo& alarmInfo,
                                    QCaDateTime&,
                                    const unsigned int& variableIndex)
{
   qcaobject::QCaObject* qca = NULL;

   // Only the main control PV sets alarm related style changes.
   //
   switch (variableIndex) {

      case READ_BACK_VARIABLE_INDEX:
         this->setCentreText (value);
         this->updateToolTipAlarm (alarmInfo, variableIndex);

         qca = this->getQcaItem (variableIndex);
         if (qca) {
            QEAxisPainter* ap = this->getAxisPainter ();
            ap->setMarkerValue (READ_BACK_MARKER, qca->getFloatingValue ());
            ap->setMarkerColour (READ_BACK_MARKER, QColor (55, 255, 55));
            ap->setMarkerVisible (READ_BACK_MARKER, true);
         }

         break;

      default:
         DEBUG << "unexpected variableIndex" << variableIndex;
         return;
         break;
   }
}

//---------------------------------------------------------------------------------
//
void QEAnalogSlider::valueChanged (const double)
{
   if (this->getContinuousWrite()) {
      this->writeNow ();
   }
}

//---------------------------------------------------------------------------------
//
void QEAnalogSlider::applyButtonClicked (bool x)
{
    QAnalogSlider::applyButtonClicked (x);  // call parent class first
    this->writeNow ();
}


//------------------------------------------------------------------------------
// slot
void QEAnalogSlider::setManagedVisible (bool v)
{
   this->setRunVisible(v);
}

//------------------------------------------------------------------------------
// slot
void QEAnalogSlider::writeNow ()
{
   QEFloating *qca = (QEFloating *) getQcaItem (SET_POINT_VARIABLE_INDEX);
   if (qca && qca->getChannelIsConnected ()) { // sanity check
      // Write the value: update database to reflect current slider position.
      //
      qca->writeFloatingElement (this->getValue ());
   }
}

//------------------------------------------------------------------------------
// slot
void QEAnalogSlider::setPvValue (const QString& text)
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
void QEAnalogSlider::setPvValue (const int value)
{
   this->setPvValue (static_cast<double>(value));
}

//------------------------------------------------------------------------------
// slot
void QEAnalogSlider::setPvValue (const double value)
{
   this->setValue (value);
   this->writeNow ();
}

//------------------------------------------------------------------------------
// slot
void QEAnalogSlider::setPvValue (const bool value)
{
   this->setPvValue (value ? 1.0 : 0.0);
}

//------------------------------------------------------------------------------
//
void QEAnalogSlider::useNewVariableNameProperty (QString variableName,
                                                 QString substitutions,
                                                 unsigned int variableIndex)
{
   this->setVariableNameAndSubstitutions (variableName, substitutions, variableIndex);
}


//==============================================================================
// Properties
// Update variable name etc.
//
//------------------------------------------------------------------------------
//
void QEAnalogSlider::setReadbackNameProperty (const QString& variableName)
{
   this->readback->setVariableNameProperty (variableName);
}

//------------------------------------------------------------------------------
//
QString QEAnalogSlider::getReadbackNameProperty () const
{
   return this->readback->getVariableNameProperty ();
}

//------------------------------------------------------------------------------
//
void QEAnalogSlider::setReadbackElementsRequired (const int elementsRequired)
{
   this->readback->setElementsRequired (elementsRequired);
}

//------------------------------------------------------------------------------
//
int QEAnalogSlider::getReadbackElementsRequired () const
{
   return this->readback->getElementsRequired ();
}

//------------------------------------------------------------------------------
//
void QEAnalogSlider::setReadbackArrayIndex (const int arrayIndex)
{
   this->readback->setArrayIndex (arrayIndex);
}

//------------------------------------------------------------------------------
//
int QEAnalogSlider::getReadbackArrayIndex () const
{
   return this->readback->getArrayIndex ();
}

//------------------------------------------------------------------------------
//
void QEAnalogSlider::setVariableNameSubstitutionsProperty (const QString& substitutions)
{
   // Must set both - as each variable name proprty manager needs it's own copy.
   //
   QESingleVariableMethods::setVariableNameSubstitutionsProperty (substitutions);
   this->readback->setVariableNameSubstitutionsProperty (substitutions);
}

//------------------------------------------------------------------------------
//
void QEAnalogSlider::setContinuousWrite (const bool value)
{
   this->mContinuousWrite = value;
   if (value) this->writeNow ();
}

//------------------------------------------------------------------------------
//
bool QEAnalogSlider::getContinuousWrite () const
{
   return this->mContinuousWrite;
}

//------------------------------------------------------------------------------
//
void QEAnalogSlider::setAutoScale (const bool value)
{
   this->mAutoScale = value;
   this->updateAxisAndSlider ();
}

//------------------------------------------------------------------------------
//
bool QEAnalogSlider::getAutoScale () const
{
   return this->mAutoScale;
}

//------------------------------------------------------------------------------
//
void QEAnalogSlider::setAlarmColours (const bool value)
{
   this->mAxisAlarmColours = value;
//   this->updateAxisAndSlider ();
   this->calcColourBandList ();
}

//------------------------------------------------------------------------------
//
bool QEAnalogSlider::getAlarmColours () const
{
   return this->mAxisAlarmColours;
}


//==============================================================================
// Copy / Paste
//
QString QEAnalogSlider::copyVariable ()
{
   QStringList list;

   list << this->getSubstitutedVariableName (SET_POINT_VARIABLE_INDEX);
   list << this->getSubstitutedVariableName (READ_BACK_VARIABLE_INDEX);

   return list.join (" ");
}

//------------------------------------------------------------------------------
//
QVariant QEAnalogSlider::copyData ()
{
   return QVariant (this->getValue ());
}

//------------------------------------------------------------------------------
//
void QEAnalogSlider::paste (QVariant s)
{
   QStringList pvNameList;

   pvNameList = QEUtilities::variantToStringList (s);

   this->setVariableName (pvNameList.value (0, ""), SET_POINT_VARIABLE_INDEX);
   this->establishConnection (SET_POINT_VARIABLE_INDEX);

   this->setVariableName (pvNameList.value (1, ""), READ_BACK_VARIABLE_INDEX);
   this->establishConnection (READ_BACK_VARIABLE_INDEX);

}

// end
