/*  QEWaveformHistogram.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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
 *  Copyright (c) 2014,2016 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */


#include <QDebug>
#include <QCaObject.h>
#include <QEWaveformHistogram.h>

#define DEBUG  qDebug () << "QEWaveformHistogram" << __FUNCTION__ << __LINE__

//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QEWaveformHistogram::QEWaveformHistogram (QWidget * parent) : QEFrame (parent)
{
   // Set default property values
   // Super class....
   //
   this->setFrameShape (QFrame::Panel);
   this->setFrameShadow (QFrame::Plain);
   this->setVariableAsToolTip (true);

   // Create internal widget.
   //
   this->layout = new QHBoxLayout (this);
   this->layout->setMargin (0);    // extact fit.
   this->histogram = new QEHistogram (this);
   this->layout->addWidget (this->histogram);

   // Set histogram properties.
   //
   this->histogram->setFrameShape (QFrame::NoFrame);
   this->histogram->setFrameShadow (QFrame::Plain);
   this->histogram->setAutoScale (true);

   QObject::connect (this->histogram, SIGNAL (mouseIndexChanged    (const int)),
                     this,            SLOT   (mouseIndexChangedSlot (const int)));

   QObject::connect (this->histogram, SIGNAL (mouseIndexPressed    (const int, const Qt::MouseButton)),
                     this,            SLOT   (mouseIndexPressedSlot (const int, const Qt::MouseButton)));

   this->mScaleMode = Manual;
   this->isFirstUpdate = true;

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

   // Do not use updateConnectionStyle.
   //
   // If this is a disconnect - set gray.
   // If this is a connect, we will soon change from gray to required colour.
   //
   const int n =this->histogram->count ();
   for (int j = 0; j < n; j++ ) {
      this->histogram->setColour (j, QColor (0xe8e8e8));
   }

   // More trob. than it's worth to check if this is a connect or disconnect.
   //
   this->isFirstUpdate = true;
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

   this->histogram->setValues (value);

   const int n =this->histogram->count ();
   if (this->getDisplayAlarmState ()) {
      QColor colour = this->getColor (alarmInfo, 255);
      for (int j = 0; j < n; j++ ) {
         this->histogram->setColour (j, colour);
      }
   } else {
      for (int j = 0; j < n; j++ ) {
         this->histogram->clearColour (j);
      }
   }

   // First update (for this connection).
   //
   if (this->isFirstUpdate) {
      this->updateHistogramScale ();
   }

   // Don't invoke common alarm handling processing.
   // Invoke for tool tip processing directly.
   //
   this->updateToolTipAlarm (alarmInfo.severityName (), variableIndex);
}

//------------------------------------------------------------------------------
//
void QEWaveformHistogram::mouseIndexChangedSlot (const int index)
{
   this->genReadOut (index);
   emit this->mouseIndexChanged (index);
}

//------------------------------------------------------------------------------
//
void QEWaveformHistogram::mouseIndexPressedSlot (const int index,
                                                const Qt::MouseButton button)
{
   // Used by context menu as well as drag-and-drop processing.
   //
   this->selectedChannel = index;
   emit this->mouseIndexPressed (index, button);
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
      text = qca->getRecordName ().append (QString (" [%1]").arg (index + 1));

      if (isDefined) {
         value = valueList.toList ().value (index);
         text.append (" ").append (value.toString ());
         text.append(" ").append (qca->getEgu ());
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
   QVariant result (QVariant::Invalid);
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
