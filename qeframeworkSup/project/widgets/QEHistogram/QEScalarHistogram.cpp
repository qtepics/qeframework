/*  QEScalarHistogram.cpp
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

#include "QEScalarHistogram.h"
#include <QDebug>
#include <QECommon.h>
#include <QCaObject.h>

#define DEBUG  qDebug () << "QEScalarHistogram" << __LINE__ << __FUNCTION__ << "  "

static const QColor disconnectedColour = QColor (0xe8e8e8);

//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QEScalarHistogram::QEScalarHistogram (QWidget * parent) :
   QEHistogram (parent),
   QEWidget (this),
   QEStringFormattingMethods ()
{
   this->histogram = this;  // alias

   this->setVariableAsToolTip (true);

   // Set histogram properties.
   //
   this->histogram->setAutoScale (true);
   this->setReadoutPrecision (6);
   this->setFormat (QE::Default);
   this->setNotation (QE::Automatic);
   this->setUseDbPrecision (false);
   this->setAddUnits (true);

   this->mScaleMode = Manual;

   // Set up data
   //
   this->setNumVariables (ARRAY_LENGTH (this->vnpm));

   // Set up default properties
   //
   this->setAllowDrop (false);

   // Use default context menu.
   //
   this->setupContextMenu ();
   this->selectedChannel = -1;

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   for (int j = 0; j < ARRAY_LENGTH (this->vnpm); j++) {
      this->vnpm [j].setVariableIndex (j);
      QObject::connect (&this->vnpm [j], SIGNAL (newVariableNameProperty  (QString, QString, unsigned int)),
                        this,            SLOT   (newVariableNameProperty  (QString, QString, unsigned int)));
   }
}


//------------------------------------------------------------------------------
// This is the slot used to recieve new PV information.
//
void QEScalarHistogram::newVariableNameProperty (QString pvName, QString subs, unsigned int pvi)
{
   this->histogram->clearValue ((int) pvi);
   this->setVariableNameAndSubstitutions (pvName, subs, pvi);
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. For a histogram floating point numbers are required.
//
qcaobject::QCaObject* QEScalarHistogram::createQcaItem (unsigned int pvi)
{
   qcaobject::QCaObject* result = NULL;

   if (pvi < ARRAY_LENGTH (this->vnpm)) {
      QString pvName = this->getSubstitutedVariableName (pvi);
      result = new QEFloating (pvName, this, &this->floatingFormatting, pvi);
   }

   return result;
}

//------------------------------------------------------------------------------
// Start updating.
//
void QEScalarHistogram::establishConnection (unsigned int variableIndex)
{
   if (variableIndex >= ARRAY_LENGTH (this->vnpm)) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject* qca = this->createConnection (variableIndex);

   // If a QCaObject object is now available to supply data update signals,
   // connect it to the appropriate slots.
   //
   if (qca) {
      QObject::connect (qca, SIGNAL (floatingChanged (const double &, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)),
                        this, SLOT  (setChannelValue (const double &, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)));

      QObject::connect (qca,  SIGNAL (connectionChanged (QCaConnectionInfo &, const unsigned int &)),
                        this, SLOT   (connectionChanged (QCaConnectionInfo &, const unsigned int &)));

      // Also set/reset value. This mimics a disconnection.
      // Note: this also creates the underlying entry with the histogram widget.
      // Usefull for indexOfPosition calls before first PV update or missing PVs.
      //
      this->histogram->setColour ((int) variableIndex, disconnectedColour);
      this->histogram->setValue ((int) variableIndex, this->getMaximum());
   }
}


//------------------------------------------------------------------------------
// Act on a connection change.
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QEScalarHistogram::connectionChanged (QCaConnectionInfo & connectionInfo,
                                           const unsigned int & variableIndex)
{
   bool pvConnected;

   if (variableIndex >= ARRAY_LENGTH (this->vnpm)) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Note the connected state
   //
   pvConnected = connectionInfo.isChannelConnected ();

   // Display the connected state
   //
   this->updateToolTipConnection (pvConnected, variableIndex);

   // This is a multi PV widget.
   // Do not use processConnectionInfo.
   //
   // If this is a disconnect - set gray.
   // If this is a connect, we will soon change from gray to required colour.
   //
   this->histogram->setColour ((int) variableIndex, disconnectedColour);
   this->histogram->setValue ((int) variableIndex, this->getMaximum());
}

//------------------------------------------------------------------------------
//
void QEScalarHistogram::updateHistogramScale ()
{
   qcaobject::QCaObject* qca = NULL;
   double lopr;
   double hopr;

   switch (this->mScaleMode) {

      case Manual:
         this->histogram->setAutoScale (false);
         break;

      case Auto:
         this->histogram->setAutoScale (true);
         break;

      case OperationalRange:
         lopr = 0.0;
         hopr = 0.0;
         for (int j = 0; j < QE_HISTOGRAM_NUMBER_VARIABLES; j++) {
            qca = this->getQcaItem (j);
            if (qca) {
               double lowLim = qca->getDisplayLimitLower ();
               double upLim = qca->getDisplayLimitUpper ();

               // Has the operating range been defined??
               //
               if ((lowLim != 0.0) || (upLim != 0.0)) {
                  // Yes - incorporate into overall operating range.
                  //
                  lopr = MIN (lopr, lowLim);
                  hopr = MAX (hopr, upLim);
               }
            }
         }

         // Has at least one PV specified a valid range?
         //
         if ((lopr != 0.0) || (hopr != 0.0)) {
            // Yes - use the range.
            //
            this->histogram->setMinimum (lopr);
            this->histogram->setMaximum (hopr);
            this->histogram->setAutoScale (false);
         }
         // else just leave as is.

         break;
   }
}

//------------------------------------------------------------------------------
// Update the histogram bar value
// This is the slot used to recieve data updates from a QCaObject based class.
//
void QEScalarHistogram::setChannelValue (const double& value,
                                         QCaAlarmInfo& alarmInfo,
                                         QCaDateTime&,
                                         const unsigned int &variableIndex)
{
   double displayValue = value;
   if (variableIndex >= ARRAY_LENGTH (this->vnpm)) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Associated qca object - avoid any segmentation fault.
   //
   qcaobject::QCaObject* qca = this->getQcaItem (variableIndex);
   if (!qca) return;   // sanity check

   const bool isMetaDataUpdate = qca->getIsMetaDataUpdate();

   QColor colour;

   if (this->getUseAlarmState (alarmInfo)) {
      colour = this->getColor (alarmInfo, 255);
      if (alarmInfo.isInvalid()) {
         // When invalid, set the height (or width) of the bar to maximum,
         // so that the user can actually see it. Invalid value are often zero
         // and not readily visible to the user.
         //
         displayValue = this->getMaximum();
      }
   } else {
      colour = this->histogram->getBarColour ();
   }

   this->histogram->setColour ((int) variableIndex, colour);
   this->histogram->setValue ((int) variableIndex, displayValue);

   // First/meta update (for this connection).
   //
   if (isMetaDataUpdate) {
      this->updateHistogramScale ();
   }

   // Don't invoke common alarm handling processing.
   // Invoke for tool tip processing directly.
   //
   this->updateToolTipAlarm (alarmInfo, variableIndex);
}

//------------------------------------------------------------------------------
//
void QEScalarHistogram::onMouseIndexChanged (const int index)
{
   this->genReadOut (index);
}

//------------------------------------------------------------------------------
//
void QEScalarHistogram::onMouseIndexPressed (const int index,
                                             const Qt::MouseButton)
{
   // Used by context menu as well as drag-and-drop processing.
   //
   this->selectedChannel = index;
}

//------------------------------------------------------------------------------
//
void QEScalarHistogram::addPvName (const QString& pvName)
{
   for (int slot = 0; slot < ARRAY_LENGTH (this->vnpm); slot++) {
      QString currentPvName = this->getSubstitutedVariableName (slot);
      if (currentPvName.isEmpty ()) {
         // Found an empty slot.
         //
         this->setVariableName (pvName, slot);
         this->establishConnection (slot);
         break;
      }
   }
}

//------------------------------------------------------------------------------
//
void QEScalarHistogram::setPvNameSubstitutions (const QString& pvNameSubstitutions)
{
   // Set all (but read 1).
   //
   for (int j = 0; j < ARRAY_LENGTH (this->vnpm); j++) {
      this->vnpm [j].setSubstitutionsProperty (pvNameSubstitutions);
   }
}

//------------------------------------------------------------------------------
//
QString QEScalarHistogram::getPvNameSubstitutions () const
{
   // All the same - any will do.
   //
   return this->vnpm [0].getSubstitutionsProperty ();
}

//------------------------------------------------------------------------------
//
void QEScalarHistogram::setScaleMode (const ScaleModes scaleModeIn)
{
   this->mScaleMode = scaleModeIn;
   this->updateHistogramScale ();
}

//------------------------------------------------------------------------------
//
QEScalarHistogram::ScaleModes QEScalarHistogram::getScaleMode () const
{
   return this->mScaleMode;
}

//------------------------------------------------------------------------------
//
void QEScalarHistogram::setReadoutPrecision (const int readoutPrecisionIn)
{
   this->setPrecision (readoutPrecisionIn);
}

//------------------------------------------------------------------------------
//
int QEScalarHistogram::getReadoutPrecision () const
{
   return this->getPrecision ();
}

//------------------------------------------------------------------------------
//
void QEScalarHistogram::setReadoutFormat (const QE::Formats formatIn)
{
   this->setFormat (formatIn);
}

//------------------------------------------------------------------------------
//
QE::Formats QEScalarHistogram::getReadoutFormat() const
{
   return this->getFormat();
}

//------------------------------------------------------------------------------
//
void QEScalarHistogram::setReadoutNotation (const QE::Notations notationIn)
{
   this->setNotation (notationIn);
}

//------------------------------------------------------------------------------
//
QE::Notations QEScalarHistogram::getReadoutNotation () const
{
   return this->getNotation();
}

//------------------------------------------------------------------------------
//
void QEScalarHistogram::setReadOut (const QString& text)
{
   message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STATUS);
   this->sendMessage (text, mt);
}

//------------------------------------------------------------------------------
// Index is channel number.
//
void QEScalarHistogram::genReadOut (const int index)
{
   qcaobject::QCaObject* qca = NULL;
   QString text;

   if (index >= 0) {
      qca = this->getQcaItem ((unsigned int) index);
   }

   if (qca) {
      bool isDefined;
      QVariant value;
      QCaAlarmInfo alarmInfo;
      QCaDateTime timeStamp;

      qca->getLastData (isDefined, value, alarmInfo, timeStamp);

      text = qca->getRecordName ();
      if (isDefined) {
         this->stringFormatting.setDbEgu (qca->getEgu ());
         text.append (" ").append (this->stringFormatting.formatString (value, 0));
      } else {
         text.append ("  undefined.");
      }
   } else {
      text = "";
   }
   this->setReadOut (text);
}

//==============================================================================
// Copy (no paste)
//
QString QEScalarHistogram::copyVariable ()
{
   if (this->selectedChannel >= 0) {
      return this->getSubstitutedVariableName ((unsigned int) this->selectedChannel);
   } else {
      return "";
   }
}

//------------------------------------------------------------------------------
//
QVariant QEScalarHistogram::copyData ()
{
   QVariant result;
   qcaobject::QCaObject* qca = NULL;

   if (this->selectedChannel >= 0) {
      qca = this->getQcaItem ((unsigned int) this->selectedChannel);
   }

   if (qca) {
      bool isDefined;
      QVariant value;
      QCaAlarmInfo alarmInfo;
      QCaDateTime timeStamp;

      qca->getLastData (isDefined, value, alarmInfo, timeStamp);
      if (isDefined) {
         result = value;
      }
   } else {
      result = QVariant ();
   }

   return result;
}

//---------------------------------------------------------------------------------
//
void QEScalarHistogram::paste (QVariant s)
{
   QStringList pvNameList = QEUtilities::variantToStringList (s);
   for (int j = 0; j < pvNameList.count (); j++) {
      this->addPvName (pvNameList.value (j));
   }
}

// end
