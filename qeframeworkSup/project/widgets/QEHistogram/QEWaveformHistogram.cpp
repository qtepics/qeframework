/*  QEWaveformHistogram.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2014-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEWaveformHistogram.h"
#include <QDebug>
#include <QCaObject.h>
#include <QEPVNameSelectDialog.h>

#define DEBUG  qDebug () << "QEWaveformHistogram"  << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QEWaveformHistogram::QEWaveformHistogram (QWidget* parent) :
   QEHistogram (parent),
   QEWidget (this),
   QEStringFormattingMethods ()
{
   this->histogram = this;

   // Create dialog.
   //
   this->pvNameSelectDialog = new QEPVNameSelectDialog (this);

   this->setVariableAsToolTip (true);

   // Set histogram properties.
   //
   this->histogram->setAutoScale (true);

   this->mScaleMode = Manual;
   this->setReadoutPrecision (6);
   this->setFormat (QE::Default);
   this->setNotation (QE::Automatic);
   this->setUseDbPrecision (false);
   this->setAddUnits (true);
   this->useFullLengthArraySubscriptions = false;  // go with modern behaviour by default.

   // Set up data
   //
   this->setNumVariables (1);

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
   this->vnpm.setVariableIndex (0);
      QObject::connect (&this->vnpm, SIGNAL (newVariableNameProperty  (QString, QString, unsigned int)),
                        this,        SLOT   (newVariableNameProperty  (QString, QString, unsigned int)));
}


//------------------------------------------------------------------------------
// This is the slot used to recieve new PV information.
//
void QEWaveformHistogram::newVariableNameProperty (QString pvName, QString subs, unsigned int pvi)
{
   this->histogram->clear ();
   this->setVariableNameAndSubstitutions (pvName, subs, pvi);
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. For a histogram floating point numbers are required.
//
qcaobject::QCaObject* QEWaveformHistogram::createQcaItem (unsigned int pvi)
{
   qcaobject::QCaObject* result = NULL;

   if (pvi == 0) {
      QString pvName = this->getSubstitutedVariableName (pvi);
      result = new QEFloating (pvName, this, &this->floatingFormatting, pvi);

      if (result && !this->useFullLengthArraySubscriptions) {
         // Only read effective number, e.g. as defied by .NORD for a waveform record.
         //
         result->setRequestedElementCount (0);
      }
   }

   return result;
}

//------------------------------------------------------------------------------
// Start updating.
//
void QEWaveformHistogram::establishConnection (unsigned int variableIndex)
{

   if (variableIndex != 0) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
    }

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject* qca = createConnection (variableIndex);

   // If a QCaObject object is now available to supply data update signals,
   // connect it to the appropriate slots.
   //
   if (qca) {
      QObject::connect (qca, SIGNAL (floatingArrayChanged (const QVector<double>&, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)),
                        this, SLOT  (setChannelArrayValue (const QVector<double>&, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)));

      QObject::connect (qca,  SIGNAL (connectionChanged (QCaConnectionInfo &, const unsigned int &)),
                        this, SLOT   (connectionChanged (QCaConnectionInfo &, const unsigned int &)));
   }
}


//------------------------------------------------------------------------------
// Act on a connection change.
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QEWaveformHistogram::connectionChanged (QCaConnectionInfo & connectionInfo,
                                             const unsigned int & variableIndex)
{
   bool pvConnected;

   if (variableIndex != 0) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Note the connected state
   //
   pvConnected = connectionInfo.isChannelConnected ();

   // Display the connected state
   //
   this->updateToolTipConnection (pvConnected, variableIndex);

   // Do not use processConnectionInfo.
   //
   // If this is a disconnect - set gray.
   // If this is a connect, we will soon change from gray to required colour.
   //
   const int n =this->histogram->count ();
   for (int j = 0; j < n; j++ ) {
      this->histogram->setColour (j, QColor (0xe8e8e8));
   }

   // Signal a channel connection change to any widgets using the
   // dbConnectionChanged signal.
   //
   this->emitDbConnectionChanged (variableIndex);
}

//------------------------------------------------------------------------------
//
void QEWaveformHistogram::updateHistogramScale ()
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
         qca = this->getQcaItem (0);
         if (qca) {
            lopr = qca->getDisplayLimitLower ();
            hopr = qca->getDisplayLimitUpper ();
         }

         // Has the PV specified a valid range?
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
void QEWaveformHistogram::setChannelArrayValue (const QVector<double>& value,
                                                QCaAlarmInfo& alarmInfo,
                                                QCaDateTime&,
                                                const unsigned int& variableIndex)
{
   if (variableIndex != 0) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Associated qca object - avoid any segmentation fault.
   //
   qcaobject::QCaObject* qca = this->getQcaItem (variableIndex);
   if (!qca) return;   // sanity check

   const bool isMetaDataUpdate = qca->getIsMetaDataUpdate();

   this->histogram->setValues (value);

   const int n =this->histogram->count ();
   if (this->getUseAlarmState (alarmInfo)) {
      QColor colour = this->getColor (alarmInfo, 255);
      for (int j = 0; j < n; j++ ) {
         this->histogram->setColour (j, colour);
      }
   } else {
      for (int j = 0; j < n; j++ ) {
         this->histogram->clearColour (j);
      }
   }

   // First/meta update (for this connection).
   //
   if (isMetaDataUpdate) {
      this->updateHistogramScale ();
   }

   // Don't invoke common alarm handling processing.
   // Invoke for tool tip processing directly.
   //
   this->updateToolTipAlarm (alarmInfo, variableIndex);

   // Signal a database value change to any widgets using dbValueChanged.
   //
   this->emitDbValueChanged (variableIndex);
}

//------------------------------------------------------------------------------
//
void QEWaveformHistogram::onMouseIndexChanged (const int index)
{
   this->genReadOut (index);
}

//------------------------------------------------------------------------------
//
void QEWaveformHistogram::onMouseIndexPressed (const int index,
                                               const Qt::MouseButton)
{
   // Used by context menu as well as drag-and-drop processing.
   //
   this->selectedChannel = index;
}

//------------------------------------------------------------------------------
//
QMenu* QEWaveformHistogram::buildContextMenu ()
{
   QMenu* menu = QEWidget::buildContextMenu ();
   QAction* action;

   menu->addSeparator ();

   action = new QAction ("Add/Edit PV Name...", menu);
   action->setCheckable (false);
   action->setData (QEWH_PV_NAME_SELECT_DIALOG);
   menu->addAction (action);

   return  menu;
}

//------------------------------------------------------------------------------
//
void QEWaveformHistogram::contextMenuTriggered (int selectedItemNum)
{
   QString oldPvName;
   int n;

   switch (selectedItemNum) {

      case QEWH_PV_NAME_SELECT_DIALOG:
         oldPvName = this->copyVariable ();
         this->pvNameSelectDialog->setPvName (oldPvName);
         n = this->pvNameSelectDialog->exec (this);
         if (n == 1) {
            // User has selected okay.
            //
            const QString newPvName = this->pvNameSelectDialog->getPvName ();
            if (newPvName != oldPvName) {
               this->setPvName (newPvName);
            }
         }
         break;

      default:
         // Call parent class function.
         //
         QEWidget::contextMenuTriggered (selectedItemNum);
         break;
   }
}

//------------------------------------------------------------------------------
//
void QEWaveformHistogram::setPvName (const QString& pvNameIn)
{
   this->setVariableName (pvNameIn, 0);
   this->establishConnection (0);
}

//------------------------------------------------------------------------------
//
void QEWaveformHistogram::setVariableNameProperty (const QString& variableName)
{
   this->vnpm.setVariableNameProperty (variableName);
}

//------------------------------------------------------------------------------
//
QString QEWaveformHistogram::getVariableNameProperty ()
{
   return this->vnpm.getVariableNameProperty ();
}

//------------------------------------------------------------------------------
//
void QEWaveformHistogram::setPvNameSubstitutions (const QString& pvNameSubstitutions)
{
   this->vnpm.setSubstitutionsProperty (pvNameSubstitutions);
}

//------------------------------------------------------------------------------
//
QString QEWaveformHistogram::getPvNameSubstitutions () const
{
   return this->vnpm.getSubstitutionsProperty ();
}

//------------------------------------------------------------------------------
//
void QEWaveformHistogram::setScaleMode (const ScaleModes scaleModeIn)
{
   this->mScaleMode = scaleModeIn;
   this->updateHistogramScale ();
}

//------------------------------------------------------------------------------
//
QEWaveformHistogram::ScaleModes QEWaveformHistogram::getScaleMode () const
{
   return this->mScaleMode;
}

//------------------------------------------------------------------------------
//
void QEWaveformHistogram::setReadoutPrecision (const int readoutPrecisionIn)
{
   this->setPrecision (readoutPrecisionIn);
}

//------------------------------------------------------------------------------
//
int QEWaveformHistogram::getReadoutPrecision () const
{
   return this->getPrecision ();
}

//------------------------------------------------------------------------------
//
void QEWaveformHistogram::setReadoutFormat (const QE::Formats formatIn)
{
   this->setFormat (formatIn);
}

//------------------------------------------------------------------------------
//
QE::Formats QEWaveformHistogram::getReadoutFormat() const
{
   return this->getFormat();
}

//------------------------------------------------------------------------------
//
void QEWaveformHistogram::setReadoutNotation (const QE::Notations notationIn)
{
   this->setNotation (notationIn);
}

//------------------------------------------------------------------------------
//
QE::Notations QEWaveformHistogram::getReadoutNotation () const
{
   return this->getNotation();
}

//------------------------------------------------------------------------------
//
void QEWaveformHistogram::setFullLengthArraySubscriptions (const bool useFullLengthArraySubscriptionsIn)
{
   this->useFullLengthArraySubscriptions = useFullLengthArraySubscriptionsIn;
}

//------------------------------------------------------------------------------
//
bool QEWaveformHistogram::getFullLengthArraySubscriptions() const
{
   return this->useFullLengthArraySubscriptions;
}

//------------------------------------------------------------------------------
//
void QEWaveformHistogram::setReadOut (const QString& text)
{
   message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STATUS);
   this->sendMessage (text, mt);
}

//------------------------------------------------------------------------------
// Index is channel element index.
//
void QEWaveformHistogram::genReadOut (const int index)
{
   qcaobject::QCaObject* qca = NULL;
   QString text;

   if (index >= 0) {
      qca = this->getQcaItem (0);
   }

   if (qca) {
      bool isDefined;
      QVariant valueList;
      QVariant value;
      QCaAlarmInfo alarmInfo;
      QCaDateTime timeStamp;

      qca->getLastData (isDefined, valueList, alarmInfo, timeStamp);

      // This is for the user - not a C/C++ compiler - number elements from 1.
      //
      text = qca->getPvName ().append (QString (" [%1]").arg (index + 1));

      if (isDefined) {
         value = valueList.toList ().value (index);
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
QString QEWaveformHistogram::copyVariable ()
{
   return this->getSubstitutedVariableName (0);
}

//------------------------------------------------------------------------------
//
QVariant QEWaveformHistogram::copyData ()
{
   QVariant result;
   qcaobject::QCaObject* qca = NULL;

   qca = this->getQcaItem (0);

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

//------------------------------------------------------------------------------
//
void QEWaveformHistogram::paste (QVariant v)
{
   QStringList pvNameList = QEUtilities::variantToStringList (v);

   // Use the first suppled pvName - any others are ignored.
   //
   if (pvNameList.count () > 0) {
      QString pvName = pvNameList.value (0, "");
      if (!pvName.isEmpty ()) {
         this->setPvName (pvName);
      }
   }
}

// end
