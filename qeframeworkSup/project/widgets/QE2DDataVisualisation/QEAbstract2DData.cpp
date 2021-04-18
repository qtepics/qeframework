/*  QEAbstract2DData.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2020-2021 Australian Synchrotron.
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
#include <QAction>
#include <QDebug>
#include <QECommon.h>
#include <UserMessage.h>
#include <QCaObject.h>
#include <QEStripChartRangeDialog.h>

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
   this->rangeDialog = new QEStripChartRangeDialog (this);
   this->rangeDialog->setWindowTitle ("Data Minimum/Maximum");

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

   this->addContextMenuScaling = true;
   this->rawNumberOfRows = 0;
   this->rawNumberOfCols = 0;
   this->sliceRowOffset = 0;
   this->sliceColOffset = 0;
   this->slicedNumberOfRows = 0;
   this->slicedNumberOfCols = 0;
   this->displayedNumberOfRows = 0;
   this->displayedNumberOfCols = 0;

   // Set default porpety values.
   //
   this->mDataWidth = 100;
   this->mRotation = NoRotation;
   this->mVerticalFlip = false;
   this->mHorizontalFlip = false;
   this->mVerticalSliceFirst = 0;
   this->mVerticalSliceLast = -1;
   this->mHorizontalSliceFirst = 0;
   this->mHorizontalSliceLast = -1;

   this->mVerticalBin = 1;
   this->mHorizontalBin = 1;
   this->mDataBinning = decimate;

   this->mScaleMode = manual;
   this->mMinimum = 0.0;
   this->mMaximum = 255.0;
   this->mDataFormat = array2D;
   this->mNumberOfSets = 40;
   this->mMouseMoveSignals = signalStatus;

   this->pvDataWidthAvailable = false;
   this->pvDataWidth = 100;
   this->updateCount = 0;

   // QEFrame sets this to false (as it's not an EPICS aware widget).
   // But the QEAbstract2DData is EPICS aware, so set default back to true.
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
void QEAbstract2DData::findDataRowCol (const int displayRow, const int displayCol,
                                       int& sourceRow, int& sourceCol) const
{
   int temp;

   // Start with no rotation and no flipping.
   //
   sourceRow = displayRow;
   sourceCol = displayCol;

   // Processes slicing, rotation and flips.
   // As we apply slice, rotate, then flip, we must first un-flip, un-rotate
   // and then un-slice.
   //
   if (this->mVerticalFlip)
      sourceRow = this->displayedNumberOfRows - 1 - sourceRow;

   if (this->mHorizontalFlip)
      sourceCol = this->displayedNumberOfCols - 1 - sourceCol;

   switch (this->mRotation) {
      case NoRotation:
         // pass
         break;

      case Rotate90Right:
         // Do the rotation
         //
         temp = sourceCol;
         sourceCol = sourceRow;
         sourceRow = this->slicedNumberOfRows - 1 - temp;
         break;

      case Rotate90Left:
         // Do the rotation
         //
         temp = sourceRow;
         sourceRow = sourceCol;
         sourceCol = this->slicedNumberOfCols - 1 - temp;
         break;

      case Rotate180:
         // Do the rotation
         //
         sourceRow = this->slicedNumberOfRows - 1 - sourceRow;
         sourceCol = this->slicedNumberOfCols - 1 - sourceCol;
         break;
   }

   // Lastly, de-slice the data, but first check out of range indices.
   //
   sourceRow += this->sliceRowOffset;
   sourceCol += this->sliceColOffset;
}

//------------------------------------------------------------------------------
//
double QEAbstract2DData::getDataValue (const int sourceRow, const int sourceCol,
                                       const double defaultValue) const
{
   const QEFloatingArray emptyArray;
   double result = defaultValue;

   if (this->getDataFormat() == array1D) {
      QEFloatingArray dataSet = data.value (sourceRow, emptyArray);
      result = dataSet.value (sourceCol, defaultValue);
   } else {
      // array2D
      const QEFloatingArray dataSet = data.value (0, emptyArray);
      const int effectiveDataWidth = this->getEffectiveDataWidth();
      const int index = effectiveDataWidth * sourceRow + sourceCol;
      result = dataSet.value (index, defaultValue);
   }

   return result;
}

//------------------------------------------------------------------------------
//
double QEAbstract2DData::getValue (const int displayRow, const int displayCol,
                                   const double defaultValue) const
{
   int sourceRow;
   int sourceCol;

   // Map/traslate the displayed row and column into the data/source row and column.
   //
   this->findDataRowCol (displayRow, displayCol, sourceRow, sourceCol);

   // Now that we have the source row and sorce col, extract the data value.
   //
   double result;
   result = getDataValue (sourceRow, sourceCol, defaultValue);
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
void QEAbstract2DData::getDataMinMaxValues (double& min, double& max) const
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
void QEAbstract2DData::getScaleModeMinMaxValues (double& min, double& max) const
{
   qcaobject::QCaObject* qca = NULL;
   double lopr;
   double hopr;

   // Ensure not erroneous - and use manual scaling as the defaults.
   //
   min = this->getMinimum();
   max = this->getMaximum();

   switch (this->getScaleMode()) {
      case manual:
         min = this->getMinimum();
         max = this->getMaximum();
         break;

      case operatingRange:
         qca = this->getQcaItem (DATA_PV_INDEX);
         if (qca) {  // sanity check
            // Get the defined operating range values.
            lopr = qca->getDisplayLimitLower();
            hopr = qca->getDisplayLimitUpper();
         } else {
            lopr = hopr = 0.0;
         }

         // Check that sensible limits have been defined and not just left
         // at the default (i.e. zero) values by a lazy database creator.
         //
         if ((lopr != 0.0) || (hopr != 0.0)) {
            min = lopr;
            max = hopr;
         }
         break;

      case dynamic:
         // Use the current values for the scaling.
         //
         this->getDataMinMaxValues (min, max);
         break;

      case displayed:
         // Transient state - go with default
      default:
         break;
   }

}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::getNumberRowsAndCols (int& numberRows, int& numberCols) const
{
   numberRows = this->displayedNumberOfRows;
   numberCols = this->displayedNumberOfCols;
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
void QEAbstract2DData::setMouseOverElement (const int displayRow, const int displayCol)
{
   // Special value used to indicate a no value.
   // This number is chosen as -(2.0 ** 48) because it:
   // a) can be exactly represented as a double, and
   // b) is a value that is not ever expected to 'turn up' as an actual value.
   //
   static const double noValue = -281474976710656.0;

   int sourceRow;
   int sourceCol;

   this->findDataRowCol (displayRow, displayCol, sourceRow, sourceCol);
   const double value = this->getDataValue (sourceRow, sourceCol, noValue);

   // Do we send a status message ?
   //
   if (this->mMouseMoveSignals & (signalStatus|signalText)) {
      // Prepare the text
      //
      QString message;
      if (value != noValue) {
         qcaobject::QCaObject* qca;
         QString egu = "";

         qca = this->getQcaItem (DATA_PV_INDEX);
         if (qca) {
            egu = qca->getEgu();
         }

         message = QString ("row:%1  col:%2  value: %3 %4")
               .arg (sourceRow + 1, 3)   // This message is for a real person,  ...
               .arg (sourceCol + 1, 3)   // not a C/C++ compiler, hence +1.
               .arg (value)
               .arg (egu);

      } else {
         message = "";
      }

      // Do we send a status message?
      // Appears on the status bar of containing form.
      //
      if (this->mMouseMoveSignals & signalStatus) {
         this->setReadOut (message);
      }

      // Do we emit time/value as string signal ?
      // Can go to QLabel or any other widget that accepts a string.
      //
      if (this->mMouseMoveSignals & signalText) {
         emit mouseElementChanged (message);
      }
   }

   // Do we send a data signal message ?
   //
   if (this->mMouseMoveSignals & signalData) {
      if (value != noValue) {
         emit this->mouseElementChanged (sourceRow, sourceCol, value);
      } else {
         emit this->mouseElementChanged (-1, -1, 0.0);
      }
   }
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::updateDataVisulation ()
{
   DEBUG << "abstract function not implemented error";
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::calculateDataVisulationValues()
{
   if (this->getDataFormat() == array1D) {
      this->rawNumberOfRows = this->mNumberOfSets;
      this->rawNumberOfCols = this->data.value (0).count();   // assume all rows the same size.
   } else {
      // array2D
      const int total = this->data.value (0).count();
      this->rawNumberOfCols = this->getEffectiveDataWidth ();
      this->rawNumberOfRows =  // avoid the divide by zero
            (total + this->rawNumberOfCols - 1) /
            MAX (this->rawNumberOfCols, 1);
   }

   int first;
   int last;

   // Calc first and last.
   // If specified first and/or end are negative, when we count from end,
   // i.e. an index of -1 meand last index, i.e. number - 1
   //
   // Do the rows and vertical slices.
   //
   #define UNSIGN_INDEX(index, number) LIMIT((index >= 0 ? index : number + index), 0, number)

   first = UNSIGN_INDEX (this->mVerticalSliceFirst, this->rawNumberOfRows);
   last  = UNSIGN_INDEX (this->mVerticalSliceLast,  this->rawNumberOfRows);

   this->sliceRowOffset = first;
   this->slicedNumberOfRows = MAX (last - first + 1, 0);   // allow zero, but not negative

   // Ditto the columns and horizontal slices
   //
   first = UNSIGN_INDEX (this->mHorizontalSliceFirst, this->rawNumberOfCols);
   last  = UNSIGN_INDEX (this->mHorizontalSliceLast,  this->rawNumberOfCols);

   this->sliceColOffset = first;
   this->slicedNumberOfCols = MAX (last - first + 1, 0);

   #undef UNSIGN_INDEX

   // Now calc rotation info
   //
   switch (this->mRotation) {
      case NoRotation:
      case Rotate180:
         this->displayedNumberOfRows = this->slicedNumberOfRows;
         this->displayedNumberOfCols = this->slicedNumberOfCols;
         break;

      case Rotate90Left:
      case Rotate90Right:
         this->displayedNumberOfRows = this->slicedNumberOfCols;
         this->displayedNumberOfCols = this->slicedNumberOfRows;
         break;
   }

   // Lastly call hook function.
   //
   this->updateDataVisulation ();
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
            this->calculateDataVisulationValues ();
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

   this->calculateDataVisulationValues ();

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
      this->calculateDataVisulationValues ();
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
void QEAbstract2DData::setMouseMoveSignals (const MouseMoveSignalFlags flags)
{
   this->mMouseMoveSignals = flags;
}

//------------------------------------------------------------------------------
//
QEAbstract2DData::MouseMoveSignalFlags QEAbstract2DData::getMouseMoveSignals () const
{
   return this->mMouseMoveSignals;
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setDataWidth (const int dataWidth)
{
   this->mDataWidth = MAX (1, dataWidth);
   this->calculateDataVisulationValues();
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
   this->calculateDataVisulationValues();
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
   this->calculateDataVisulationValues();
}

//------------------------------------------------------------------------------
//
int QEAbstract2DData::getNumberOfSets () const
{
   return this->mNumberOfSets;
}


//------------------------------------------------------------------------------
//
void QEAbstract2DData::setRotation (const RotationOptions rotation)
{
   this->mRotation = rotation;
   this->calculateDataVisulationValues();
}

//------------------------------------------------------------------------------
//
QEAbstract2DData::RotationOptions
QEAbstract2DData::getRotation() const
{
   return this->mRotation;
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setVerticalFlip (const bool verticalFlip)
{
   this->mVerticalFlip = verticalFlip;
   this->calculateDataVisulationValues();
}

//------------------------------------------------------------------------------
//
bool QEAbstract2DData::getVerticalFlip () const
{
   return this->mVerticalFlip;
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setHorizontalFlip (const bool horizontalFlip)
{
   this->mHorizontalFlip = horizontalFlip;
   this->calculateDataVisulationValues();
}

//------------------------------------------------------------------------------
//
bool QEAbstract2DData::getHorizontalFlip () const
{
   return this->mHorizontalFlip;
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setVerticalSliceFirst (const int first)
{
   this->mVerticalSliceFirst = first;
   this->calculateDataVisulationValues();
}

//------------------------------------------------------------------------------
//
int QEAbstract2DData::getVerticalSliceFirst () const
{
   return this->mVerticalSliceFirst;
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setVerticalSliceLast (const int last)
{
   this->mVerticalSliceLast = last;
   this->calculateDataVisulationValues();
}

//------------------------------------------------------------------------------
//
int QEAbstract2DData::getVerticalSliceLast () const
{
   return this->mVerticalSliceLast;
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setHorizontalSliceFirst (const int first)
{
   this->mHorizontalSliceFirst = first;
   this->calculateDataVisulationValues();
}

//------------------------------------------------------------------------------
//
int QEAbstract2DData::getHorizontalSliceFirst () const
{
   return this->mHorizontalSliceFirst;
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setHorizontalSliceLast (const int last)
{
   this->mHorizontalSliceLast = last;
   this->calculateDataVisulationValues();
}

//------------------------------------------------------------------------------
//
int QEAbstract2DData::getHorizontalSliceLast () const
{
   return this->mHorizontalSliceLast;
}

//------------------------------------------------------------------------------
//
void  QEAbstract2DData::setVerticalBin  (const int bin)
{
    this->mVerticalBin = LIMIT(bin, 1, 100);
}

//------------------------------------------------------------------------------
//
int  QEAbstract2DData::getVerticalBin () const
{
   return this->mVerticalBin;
}

//------------------------------------------------------------------------------
//
void  QEAbstract2DData::setHorizontalBin  (const int bin)
{
   this->mHorizontalBin = LIMIT(bin, 1, 100);
}

//------------------------------------------------------------------------------
//
int  QEAbstract2DData::getHorizontalBin () const
{
   return this->mHorizontalBin;
}

//------------------------------------------------------------------------------
//
void  QEAbstract2DData::setDataBinning  (const DataBinning option)
{
    this->mDataBinning = option;
}

//------------------------------------------------------------------------------
//
QEAbstract2DData::DataBinning  QEAbstract2DData::getDataBinning () const
{
    return mDataBinning;
}


//------------------------------------------------------------------------------
//
void QEAbstract2DData::setScaleMode (const ScaleModes scaleMode)
{
   this->mScaleMode = scaleMode;
   if (this->mScaleMode == displayed) {
      // The displayed scale mode is transient.
      // It provides a snap-shot the current dynamic scale.
      //
      double min = this->getMinimum();
      double max = this->getMaximum();
      this->getDataMinMaxValues (min, max);
      this->setMinimum (min);
      this->setMaximum (max);
      this->mScaleMode = manual;
   }
   this->calculateDataVisulationValues();
}

//------------------------------------------------------------------------------
//
QEAbstract2DData::ScaleModes QEAbstract2DData::getScaleMode () const
{
   return this->mScaleMode;
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setMinimum (const double minimum)
{
   this->mMinimum = minimum;
   this->mMaximum = MAX (this->mMaximum, this->mMinimum + minSpan);
   this->calculateDataVisulationValues();
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setMinimum (const int minimum)
{
   this->setMinimum (double (minimum));
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
   this->calculateDataVisulationValues();
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::setMaximum (const int maximum)
{
   this->setMaximum (double (maximum));
}

//------------------------------------------------------------------------------
//
double QEAbstract2DData::getMaximum () const
{
   return this->mMaximum;
}

//==============================================================================
// Context menu - place holder.
//
QMenu* QEAbstract2DData::buildContextMenu ()
{
   QMenu* menu = ParentWidgetClass::buildContextMenu ();
   QAction* action;

   // TODO: Consider rotation/flipping and scaling sub-menus

   menu->addSeparator ();

   action = new QAction ("No Rotation", menu);
   action->setCheckable (true);
   action->setChecked (this->getRotation() == NoRotation);
   action->setData (A2DDCM_NO_ROTATION);
   menu->addAction (action);

   action = new QAction ("Rotate 90 Right", menu);
   action->setCheckable (true);
   action->setChecked (this->getRotation() == Rotate90Right);
   action->setData (A2DDCM_ROTATE_90_RIGHT);
   menu->addAction (action);

   action = new QAction ("Rotate 180", menu);
   action->setCheckable (true);
   action->setChecked (this->getRotation() == Rotate180);
   action->setData (A2DDCM_ROTATE_180);
   menu->addAction (action);

   action = new QAction ("Rotate 90 Left", menu);
   action->setCheckable (true);
   action->setChecked (this->getRotation() == Rotate90Left);
   action->setData (A2DDCM_ROTATE_90_LEFT);
   menu->addAction (action);

   menu->addSeparator ();

   action = new QAction ("Vertical Flip (about horizontal axis)", menu);
   action->setCheckable (true);
   action->setChecked (this->getVerticalFlip());
   action->setData (A2DDCM_VERTICAL_FLIP);
   menu->addAction (action);

   action = new QAction ("Horizontal Flip (about vertical axis)", menu);
   action->setCheckable (true);
   action->setChecked (this->getHorizontalFlip());
   action->setData (A2DDCM_HORIZONTAL_FLIP);
   menu->addAction (action);

   // Not applicable to QESurface.
   //
   if (this->addContextMenuScaling) {
      menu->addSeparator ();

      action = new QAction ("Manual Scale", menu);
      action->setCheckable (true);
      action->setChecked (this->getScaleMode() == manual);
      action->setData (A2DDCM_MANUAL_SCALE);
      menu->addAction (action);

      action = new QAction ("Operating Range Scale", menu);
      action->setCheckable (true);
      action->setChecked (this->getScaleMode() == operatingRange);
      action->setData (A2DDCM_OPERATING_RANGE_SCALE);
      menu->addAction (action);

      action = new QAction ("Dynamic Range Scale", menu);
      action->setCheckable (true);
      action->setChecked (this->getScaleMode() == dynamic);
      action->setData (A2DDCM_DYNAMIC_SCALE);
      menu->addAction (action);

      action = new QAction ("Use Displayed Range", menu);
      action->setData (A2DDCM_DISPLAYED_SCALE);
      menu->addAction (action);

      action = new QAction ("Min/Max Selection...", menu);
      action->setData (A2DDCM_MIN_MAX_DIALOG);
      menu->addAction (action);
   }

   return  menu;
}

//------------------------------------------------------------------------------
//
void QEAbstract2DData::contextMenuTriggered (int selectedItemNum)
{
   int n;

   switch (selectedItemNum) {

      case A2DDCM_NO_ROTATION:
         this->setRotation (NoRotation);
         break;

      case A2DDCM_ROTATE_90_RIGHT:
         this->setRotation (Rotate90Right);
         break;

      case A2DDCM_ROTATE_180:
         this->setRotation (Rotate180);
         break;

      case A2DDCM_ROTATE_90_LEFT:
         this->setRotation (Rotate90Left);
         break;

      case A2DDCM_VERTICAL_FLIP:
         this->setVerticalFlip (!this->getVerticalFlip());      // flip the vert. flip state
         break;

      case A2DDCM_HORIZONTAL_FLIP:
         this->setHorizontalFlip (!this->getHorizontalFlip());  // flip the hor. flip state
         break;

      case A2DDCM_MANUAL_SCALE:
         this->setScaleMode (manual);
         break;

      case A2DDCM_OPERATING_RANGE_SCALE:
         this->setScaleMode (operatingRange);
         break;

      case A2DDCM_DYNAMIC_SCALE:
         this->setScaleMode (dynamic);
         break;

      case A2DDCM_DISPLAYED_SCALE:
         this->setScaleMode (displayed);
         break;

      case A2DDCM_MIN_MAX_DIALOG:
         this->rangeDialog->setRange (this->mMinimum, this->mMaximum);
         n = this->rangeDialog->exec (this);
         if (n == 1) {
            // User has selected okay.
            //
            this->setMinimum (this->rangeDialog->getMinimum ());
            this->setMaximum (this->rangeDialog->getMaximum ());
         }
         break;

      default:
         // Call parent class function.
         //
         ParentWidgetClass::contextMenuTriggered (selectedItemNum);
         break;
   }
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
