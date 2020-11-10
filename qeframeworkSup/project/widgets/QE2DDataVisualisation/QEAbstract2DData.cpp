/*  QEAbstract2DData.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2020 Australian Synchrotron.
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
 *    andrews@ansto.gov.au
 */

#include "QEAbstract2DData.h"
#include <QDebug>
#include <QECommon.h>
#include <UserMessage.h>

#define DEBUG qDebug () << "QEAbstract2DData" << __LINE__ << __FUNCTION__ << "  "

static const double minSpan = 1.0e-3;   /// duplicate

//------------------------------------------------------------------------=-----
// Constructor with no initialisation
//
QEAbstract2DData::QEAbstract2DData (QWidget* parent) :
   QEFrame (parent)
{
   this->commonSetup ();
}

//---------------------------------------------------------------------------=--
// Constructor with known variable(s)
//
QEAbstract2DData::QEAbstract2DData (const QString& dataVariableName,
                                    QWidget* parent) :
   QEFrame (parent)
{
   this->commonSetup ();
   this->setVariableName (dataVariableName, DATA_PV_INDEX);
   this->activate ();
}

//---------------------------------------------------------------------------=--
//
QEAbstract2DData::QEAbstract2DData (const QString& dataVariableName,
                                    const QString& widthVariableName,
                                    QWidget* parent) :
   QEFrame (parent)
{
   this->commonSetup ();
   this->setVariableName (dataVariableName, DATA_PV_INDEX);
   this->setVariableName (widthVariableName, WIDTH_PV_INDEX);
   this->activate ();
}

//---------------------------------------------------------------------------=--
//
QEAbstract2DData::~QEAbstract2DData () { }

//--------------------------------------------------------------------------=---
// Setup common to all constructors
//
void QEAbstract2DData::commonSetup ()
{
   // Configure the panel.
   //
   this->setFrameShape (QFrame::NoFrame);
   this->setFrameShadow (QFrame::Plain);

   // Set default property values.
   //
   // Mainly so that we can see it in designer.
   //
   this->setStyleDefault ("QWidget { background-color: #e0dcda; }");

   this->setAllowDrop (true);
   this->setDisplayAlarmStateOption (DISPLAY_ALARM_STATE_ALWAYS);

   this->mDataWidth = 100;
   this->mAutoScale = false;
   this->mMinimum = 0.0;
   this->mMaximum = 255.0;
   this->mDataFormat = array2D;
   this->mNumberOfSets = 40;

   this->pvDataWidthAvailable = false;
   this->pvDataWidth = 100;
   this->updateCount = 0;

   // QEFrame sets this to false (as it's not an EPICS aware widget).
   // But the QEAbstract2DData is EPICS aware, so set default to true.
   //
   this->setVariableAsToolTip (true);

   // Set up data
   //
   this->setNumVariables (2);

   // Use default context menu.
   //
   this->setupContextMenu ();

   this->dnpm.setVariableIndex (DATA_PV_INDEX);
   this->wnpm.setVariableIndex (WIDTH_PV_INDEX);

   QObject::connect(&this->dnpm, SIGNAL (newVariableNameProperty (QString, QString, unsigned int)),
                    this,        SLOT   (setVariableNameProperty (QString, QString, unsigned int)));

   QObject::connect(&this->wnpm, SIGNAL (newVariableNameProperty (QString, QString, unsigned int)),
                    this,        SLOT   (setVariableNameProperty (QString, QString, unsigned int)));
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject's required.
//
qcaobject::QCaObject* QEAbstract2DData::createQcaItem (unsigned int vi)
{
   qcaobject::QCaObject* result = NULL;
   QString pvName;

   switch (vi) {
      case DATA_PV_INDEX:
         pvName = this->getSubstitutedVariableName (vi).trimmed ();
         result = new QEFloating (pvName, this, &this->floatingFormatting, vi);
         break;

      case WIDTH_PV_INDEX:
         pvName = this->getSubstitutedVariableName (vi).trimmed ();
         result = new QEInteger (pvName, this, &this->integerFormatting, vi);
         break;

      default:
         DEBUG << "unexpected variableIndex" << vi;
         return NULL;
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
void QEAbstract2DData::establishConnection (unsigned int vi)
{
   qcaobject::QCaObject* qca = NULL;

   switch (vi) {
      case DATA_PV_INDEX:
         qca = this->createConnection (vi);
         if (!qca) break;

         QObject::connect (qca,  SIGNAL (connectionChanged (QCaConnectionInfo &, const unsigned int& )),
                           this, SLOT   (connectionChanged (QCaConnectionInfo &, const unsigned int& )));

         QObject::connect (qca, SIGNAL (floatingArrayChanged (const QVector <double>&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                           this,  SLOT (onDataArrayUpdate    (const QVector <double>&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));
         break;

      case WIDTH_PV_INDEX:
         qca = this->createConnection (vi);
         if (!qca) break;

         QObject::connect (qca,  SIGNAL (connectionChanged (QCaConnectionInfo &, const unsigned int& )),
                           this, SLOT   (connectionChanged (QCaConnectionInfo &, const unsigned int& )));

         QObject::connect (qca,  SIGNAL (integerChanged (const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                           this, SLOT   (onWidthUpdate  (const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));
         break;

      default:
         DEBUG << "unexpected variableIndex" << vi;
         break;
   }
}

//------------------------------------------------------------------------------
//
int QEAbstract2DData::getEffectiveDataWidth () const
{
   // If width PV data is connected and a value available, use it,
   // otherwise just use design time property value.
   //
   return this->pvDataWidthAvailable ? this->pvDataWidth : this->mDataWidth;
}

//------------------------------------------------------------------------------
//
QEAbstract2DData::TwoDimensionalData
QEAbstract2DData::getData () const
{
   return this->data;
}

//------------------------------------------------------------------------------
//
double QEAbstract2DData::getValue (const int srcRow, const int srcCol,
                                   const double defaultValue) const
{
   const QEFloatingArray emptyArray;
   const int effectiveDataWidth = this->getEffectiveDataWidth();

   double result = defaultValue;

   if (this->getDataFormat() == array1D) {
      QEFloatingArray dataSet = data.value (srcRow, emptyArray);
      result = dataSet.value (srcCol, defaultValue);
   } else {
      // array2D
      const QEFloatingArray dataSet = data.value (0, emptyArray);
      const int index = effectiveDataWidth * srcRow + srcCol;
      result = dataSet.value (index, defaultValue);
   }

   return result;
}

//------------------------------------------------------------------------------
//
int QEAbstract2DData::getUpdateCount () const
{
   return this->updateCount;
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::getDataMinMaxValues (double& min, double& max)
{
   const int number = this->data.count();

   double tempMin = +1.0e+80;
   double tempMax = -1.0e+80;

   // Process each data set "row" in turn.
   //
   for (int j = 0; j < number; j++) {
      QEFloatingArray dataSet = this->data.value (j);
      const int m = dataSet.count ();
      if (m == 0) continue;
      const double dataSetMin = dataSet.minimumValue (tempMin, false);
      const double dataSetMax = dataSet.maximumValue (tempMax, false);
      tempMin = MIN (tempMin, dataSetMin);
      tempMax = MAX (tempMax, dataSetMax);
   }

   if (tempMax >= tempMin) {
      // At least 1 data point - assign values.
      //
      min = tempMin;
      max = tempMax;
   }
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::getNumberRowsAndCols (const bool potential,
                                             int& numberRows, int& numberCols)
{
   const int number = this->data.count();

   if (this->getDataFormat() == array1D) {
      numberCols = this->data.value (0).count();   // assume all the same size.

      // Some rows may be blank/missing when number < mNumberOfSets.
      //
      numberRows = potential ? this->mNumberOfSets : number;
   } else {
      // array2D
      const int total = data.value (0).count();   // there is only one.
      numberCols = this->getEffectiveDataWidth ();
      numberRows = (total + numberCols - 1) / numberCols;
   }
}

//------------------------------------------------------------------------------
//
QString QEAbstract2DData::getUnits() const
{
   QString result = "";
   qcaobject::QCaObject* qca = this->getQcaItem (DATA_PV_INDEX);
   if (qca) {
      result = qca->getEgu();
   }
   return result;
}

//------------------------------------------------------------------------------
//
int QEAbstract2DData::getPrecision() const
{
   int result = 0;
   qcaobject::QCaObject* qca = this->getQcaItem (DATA_PV_INDEX);
   if (qca) {
      result = qca->getPrecision();
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setReadOut (const QString& text)
{
   message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STATUS);
   this->sendMessage (text, mt);
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::updateDataVisulation ()
{
   DEBUG << "function not implemented error";
}

//==============================================================================
// Slots
void QEAbstract2DData::setVariableNameProperty (QString pvName,
                                             QString subs,
                                             unsigned int vi)
{
   if ((vi != DATA_PV_INDEX) && (vi != WIDTH_PV_INDEX)) {
      DEBUG << "unexpected variableIndex" << vi;
      return;
   }

   this->setVariableNameAndSubstitutions (pvName, subs, vi);
}

//------------------------------------------------------------------------------
// Act on a connection change - modify the tool tip
// We don't chage the style - the inner widgets can to that.
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QEAbstract2DData::connectionChanged (QCaConnectionInfo& connectionInfo,
                                          const unsigned int& vi)
{
   if ((vi != DATA_PV_INDEX) && (vi != WIDTH_PV_INDEX)) {
      DEBUG << "unexpected variableIndex" << vi;
      return;
   }

   // Note the connected state.
   //
   bool isConnected = connectionInfo.isChannelConnected ();

   // Display the connected state.
   //
   this->updateToolTipConnection (isConnected, vi);

   // Signal channel connection change to any (Link) widgets.
   // using signal dbConnectionChanged.
   //
   switch (vi) {
      case DATA_PV_INDEX:
         if (isConnected) {
            // Reset on connect, on disconnect leave last image in place.
            //
            this->data.clear ();
            this->updateDataVisulation ();
         }
         break;

      case WIDTH_PV_INDEX:
         this->pvDataWidthAvailable = false;
         this->updateCount = 0;
         break;
   }

   this->emitDbConnectionChanged (vi);
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::onDataArrayUpdate (const QVector<double>& values,
                                          QCaAlarmInfo& alarmInfo,
                                          QCaDateTime&, const unsigned int& vi)
{
   if (vi != DATA_PV_INDEX) {
      DEBUG << "unexpected variableIndex" << vi;
      return;
   }

   QEFloatingArray data (values);
   this->data.append (data);

   int number = (this->mDataFormat == array2D) ? 1 : this->mNumberOfSets;
   while (this->data.count () > number) {
      this->data.removeFirst ();
   }

   this->updateCount++;

   this->updateDataVisulation ();

   // Invoke common alarm handling processing.
   //
   this->processAlarmInfo (alarmInfo);

   // Signal a database value change to any Link (or other) widgets using
   // the dbValueChanged signal.
   //
   this->emitDbValueChanged (vi);
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::onWidthUpdate (const long value, QCaAlarmInfo& alarmInfo,
                                      QCaDateTime&, const unsigned int& vi)
{
   if (vi != WIDTH_PV_INDEX) {
      DEBUG << "unexpected variableIndex" << vi;
      return;
   }

   int temp = MAX (1, value);  // Ensure strictly positive.

   // Update image iff value has changes.
   //
   this->pvDataWidthAvailable = true;
   if (this->pvDataWidth != temp) {
      this->pvDataWidth = temp;
      this->updateDataVisulation ();
   }

   // Update the tool tip for this PV, but not general widget alarm state.
   //
   this->updateToolTipAlarm (alarmInfo, vi);

   // Signal a database value change to any Link (or other) widgets using
   // the dbValueChanged signal.
   //
   this->emitDbValueChanged (vi);
}

//==============================================================================
// Properties
// Update variable name etc.
//
void QEAbstract2DData::setDataPvName (const QString dataVariable)
{
   this->dnpm.setVariableNameProperty (dataVariable);
}

//------------------------------------------------------------------------------
//
QString QEAbstract2DData::getDataPvName () const
{
   return this->dnpm.getVariableNameProperty ();
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setWidthPvName (const QString widthVariable)
{
   this->wnpm.setVariableNameProperty (widthVariable);
}

//------------------------------------------------------------------------------
//
QString QEAbstract2DData::getWidthPvName () const
{
   return this->wnpm.getVariableNameProperty ();
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setVariableNameSubstitutions (const QString variableSubstitutions)
{
   // Any substitutions apply to all variables.
   //
   this->dnpm.setSubstitutionsProperty (variableSubstitutions);
   this->wnpm.setSubstitutionsProperty (variableSubstitutions);
}

//------------------------------------------------------------------------------
//
QString QEAbstract2DData::getVariableNameSubstitutions () const
{
   // The substitutions from either property name manager will do.
   //
   return this->dnpm.getSubstitutionsProperty ();
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setDataWidth (const int dataWidth)
{
   this->mDataWidth = MAX (1, dataWidth);
   this->updateDataVisulation();
}

//------------------------------------------------------------------------------
//
int QEAbstract2DData::getDataWidth () const
{
   return this->mDataWidth;
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setDataFormat (const DataFormats dataFormat)
{
   this->mDataFormat = dataFormat;
   this->updateDataVisulation();
}

//------------------------------------------------------------------------------
//
QEAbstract2DData::DataFormats
QEAbstract2DData::getDataFormat () const
{
   return this->mDataFormat;
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setNumberOfSets (const int numberOfSets)
{
   this->mNumberOfSets = LIMIT (numberOfSets, 1, 1024);
   this->updateDataVisulation();
}

//------------------------------------------------------------------------------
//
int QEAbstract2DData::getNumberOfSets () const
{
   return this->mNumberOfSets;
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setAutoScale (const bool autoScale)
{
   this->mAutoScale = autoScale;
   this->updateDataVisulation();
}

//------------------------------------------------------------------------------
//
bool QEAbstract2DData::getAutoScale () const
{
   return this->mAutoScale;
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setMinimum (const double minimum)
{
   this->mMinimum = minimum;
   this->mMaximum = MAX (this->mMaximum, this->mMinimum + minSpan);
   this->updateDataVisulation();
}

//------------------------------------------------------------------------------
//
double QEAbstract2DData::getMinimum () const
{
   return this->mMinimum;
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setMaximum (const double maximum)
{
   this->mMaximum = maximum;
   this->mMinimum = MIN (this->mMinimum, this->mMaximum - minSpan);
   this->updateDataVisulation();
}

//------------------------------------------------------------------------------
//
double QEAbstract2DData::getMaximum () const
{
   return this->mMaximum;
}

//==============================================================================
// Copy / Paste
//
QString QEAbstract2DData::copyVariable ()
{
   QString result;

   result = this->getSubstitutedVariableName (DATA_PV_INDEX);
   if (!result.isEmpty()) {
      result.append (" ");
   }
   result.append (this->getSubstitutedVariableName (WIDTH_PV_INDEX));

   return result;
}

//------------------------------------------------------------------------------
//
QVariant QEAbstract2DData::copyData ()
{
   return QVariant ();
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::paste (QVariant v)
{
   QStringList pvNameList;

   // v.toSring is a bit limiting when v is a StringList or a List of Strings,
   // so use common variantToStringList function which handles these options.
   //
   pvNameList = QEUtilities::variantToStringList (v);

   if (pvNameList.count() >= 1) {
      this->setVariableName (pvNameList.value (0, ""), DATA_PV_INDEX);
      this->establishConnection (DATA_PV_INDEX);
   }

   if (pvNameList.count() >= 2) {
      this->setVariableName (pvNameList.value (1, ""), WIDTH_PV_INDEX);
      this->establishConnection (WIDTH_PV_INDEX);
   }
}

// end
