/*  QEAnalogProgressBar.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2011-2025 Australian Synchrotron
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

/*
  This class is a CA aware analog progress bar widget based on the analog progress bar widget.
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details
 */

#include "QEAnalogProgressBar.h"
#include <alarm.h>
#include <QDebug>
#include <QECommon.h>
#include <QCaObject.h>
#include <QEStringFormatting.h>

#define DEBUG qDebug () << "QEAnalogProgressBar" << __LINE__ << __FUNCTION__ << "  "

#define ALARM_SATURATION      128
#define NO_ALARM_SATURATION    32

#define PV_VARIABLE_INDEX      0

//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QEAnalogProgressBar::QEAnalogProgressBar (QWidget * parent):
   QEAnalogIndicator (parent),
   QEWidget (this),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX),
   QEStringFormattingMethods ()
{
   this->setup ();
}


//------------------------------------------------------------------------------
// Constructor with known variable
//
QEAnalogProgressBar::QEAnalogProgressBar (const QString & variableNameIn,
                                          QWidget * parent) :
   QEAnalogIndicator (parent),
   QEWidget (this),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX),
   QEStringFormattingMethods ()
{
   this->setup ();
   this->setVariableName (variableNameIn, PV_VARIABLE_INDEX);
   this->activate ();
}


//------------------------------------------------------------------------------
// Setup common to all constructors
//
void QEAnalogProgressBar::setup ()
{
   // Set up data
   // This control used a single data source
   this->setNumVariables (1);

   // Set up default properties
   this->setArrayAction (QE::Index);
   this->useDbDisplayLimits = false;
   this->alarmSeverityDisplayMode = background;
   this->setAllowDrop (false);

   // Set the initial state
   this->setIsActive (false);  // Essentially as if disabled, until we connect.

   // Use progress bar signals
   // --Currently none--

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
// Implementation of QEWidget's virtual funtion to create the specific type of QCaObject required.
// For a progress bar a QCaObject that streams integers is required.
//
qcaobject::QCaObject* QEAnalogProgressBar::createQcaItem (unsigned int variableIndex)
{
   qcaobject::QCaObject* result = NULL;

   if (variableIndex == PV_VARIABLE_INDEX) {
      result = new QEFloating (getSubstitutedVariableName (variableIndex), this,
                               &this->floatingFormatting, variableIndex);

      // Apply currently defined array index/elements request values.
      //
      this->setSingleVariableQCaProperties (result);

   } else {
      result = NULL;            // Unexpected
   }

   return result;
}

//------------------------------------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QEAnalogProgressBar::establishConnection (unsigned int variableIndex)
{
   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject* qca = this->createConnection (variableIndex);

   // If a QCaObject object is now available to supply data update signals, connect it to the appropriate slots.
   //
   if ((qca) && (variableIndex == PV_VARIABLE_INDEX)) {
      QObject::connect (qca,  SIGNAL (floatingChanged   (const double &, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)),
                        this, SLOT (setProgressBarValue (const double &, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)));

      QObject::connect (qca, SIGNAL (connectionChanged (QCaConnectionInfo &,const unsigned int&)),
                        this, SLOT  (connectionChanged (QCaConnectionInfo &,const unsigned int&)));

      QObject::connect (this, SIGNAL (requestResend ()),
                        qca,  SLOT  (resendLastData ()));
   }
}

//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the progress bar looks and change the tool tip
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QEAnalogProgressBar::connectionChanged (QCaConnectionInfo& connectionInfo,
                                             const unsigned int& variableIndex)
{
   // Note the connected state
   const bool isConnected = connectionInfo.isChannelConnected ();

   // Display the connected state
   this->updateToolTipConnection (isConnected, variableIndex);

   // Change style to reflect being connected/disconnected.
   //
   this->processConnectionInfo (isConnected, variableIndex);
   this->setIsActive (isConnected);

   // Signal channel connection change to any (Link) widgets.
   // using signal dbConnectionChanged.
   //
   this->emitDbConnectionChanged (variableIndex);
}

//------------------------------------------------------------------------------
// Determine if alarm colour to be used.
//
bool QEAnalogProgressBar::useAlarmColours (QCaAlarmInfo& alarmInfo) const
{
   qcaobject::QCaObject* qca = this->getQcaItem (PV_VARIABLE_INDEX);
   if (!qca) return false;
   if (!qca->getDataIsAvailable()) return false;

   bool isDefined;
   QVariant value;
   QCaDateTime timeStamp;
   qca->getLastData (isDefined, value, alarmInfo, timeStamp);
   if (!isDefined) return false;

   return this->getUseAlarmState (alarmInfo);
}

//------------------------------------------------------------------------------
// virtual override
//
QColor QEAnalogProgressBar::getBackgroundPaintColour () const
{
   QColor result;
   QCaAlarmInfo alarmInfo;

   // Is alarm colour in use and appicable to the background colour?
   //
   if (this->useAlarmColours (alarmInfo) &&
       this->getAlarmSeverityDisplayMode () == background) {

      // Use low saturation when no alarm, otherwise set a medium saturation level.
      //
      int saturation = (alarmInfo.getSeverity () == NO_ALARM) ? NO_ALARM_SATURATION : ALARM_SATURATION;
      result = this->getColor (alarmInfo, saturation);

   } else {
      // No connection or no data or not in use - just use parent function.
      //
      result = QEAnalogIndicator::getBackgroundPaintColour ();
   }
   return result;
}

//------------------------------------------------------------------------------
// virtual override
//
QColor QEAnalogProgressBar::getForegroundPaintColour () const
{
   QColor result;
   QCaAlarmInfo alarmInfo;

   // Is alarm colour in use and appicable to the foreground colour?
   //
   if (this->useAlarmColours (alarmInfo) &&
       this->getAlarmSeverityDisplayMode () == foreground) {
      // Use low saturation when no alarm, otherwise set a medium saturation level.
      //
      int saturation = ALARM_SATURATION;
      result = this->getColor (alarmInfo, saturation);

   } else {
      // No connection or no data or not in use - just use parent function.
      //
      result = QEAnalogIndicator::getForegroundPaintColour ();
   }
   return result;
}

//------------------------------------------------------------------------------
// Provide image, e.g. with EGU if appropriate
// virtual override
//
QString QEAnalogProgressBar::getTextImage () const
{
   return this->theImage;
}

//------------------------------------------------------------------------------
// Create a single thresholds and colour band item.
//
QEAnalogIndicator::Band QEAnalogProgressBar::createBand (const double lower,
                                                         const double upper,
                                                         unsigned short severity) const
{
   Band result;
   QCaAlarmInfo alarmInfo (0, severity);
   int saturation;

   result.lower = lower;
   result.upper = upper;

   saturation = (severity == NO_ALARM) ? NO_ALARM_SATURATION : ALARM_SATURATION;
   result.colour = this->getColor (alarmInfo, saturation);

   return result;
}

//------------------------------------------------------------------------------
// Create a list of alarm thresholds and colours.
// virtual override
//
QEAnalogIndicator::BandList QEAnalogProgressBar::getBandList () const
{
   BandList result;

   result.clear ();

   // Associated qca object - avoid the segmentation fault.
   //
   qcaobject::QCaObject* qca = this->getQcaItem (PV_VARIABLE_INDEX);
   if (qca) {
      const double dispLower = this->getMinimum ();
      const double dispUpper = this->getMaximum ();
      const double alarmLower = qca->getAlarmLimitLower ();
      const double alarmUpper = qca->getAlarmLimitUpper ();
      const double warnLower = qca->getWarningLimitLower ();
      const double warnUpper = qca->getWarningLimitUpper ();
      bool alarmIsDefined;
      bool warnIsDefined;

      // Unfortunately, the Channel Access protocol only provides the
      // alarm/warning values, and not the associated severities.
      // We assume major for alarms, and minor for warnings.
      //
      alarmIsDefined = ((alarmLower != 0.0) || (alarmUpper != 0.0));
      warnIsDefined = ((warnLower != 0.0) || (warnUpper != 0.0));

      if (alarmIsDefined) {
         if (warnIsDefined) {
            // All alarms defined.
            //
            result << createBand (dispLower,  alarmLower, MAJOR_ALARM);
            result << createBand (alarmLower, warnLower,  MINOR_ALARM);
            result << createBand (warnLower,  warnUpper,  NO_ALARM);
            result << createBand (warnUpper,  alarmUpper, MINOR_ALARM);
            result << createBand (alarmUpper, dispUpper,  MAJOR_ALARM);
         } else {
            // Major alarms defined.
            //
            result << createBand (dispLower,  alarmLower, MAJOR_ALARM);
            result << createBand (alarmLower, alarmUpper, NO_ALARM);
            result << createBand (alarmUpper, dispUpper,  MAJOR_ALARM);
         }
      } else {
         if (warnIsDefined) {
            // Minor alarms defined.
            //
            result << createBand (dispLower, warnLower, MINOR_ALARM);
            result << createBand (warnLower, warnUpper, NO_ALARM);
            result << createBand (warnUpper, dispUpper, MINOR_ALARM);
         } else {
            // No alarms defined at all.
            //
            result << createBand (dispLower, dispUpper, NO_ALARM);
         }
      }
   }
   return result;
}

//------------------------------------------------------------------------------
// Update the progress bar value
// This is the slot used to recieve data updates from a QCaObject based class.
//
void QEAnalogProgressBar::setProgressBarValue (const double& value,
                                               QCaAlarmInfo& alarmInfo,
                                               QCaDateTime&,
                                               const unsigned int& variableIndex)
{
   // If not enabled then do nothing.
   // NOTE: the regular isEnabled is hidden by function in the standard properties
   //
   if (!QWidget::isEnabled ())
      return;

   // Associated qca object - avoid the segmentation fault.
   //
   qcaobject::QCaObject* qca = this->getQcaItem (variableIndex);
   if (!qca) return;   // sanity check

   const bool isMetaDataUpdate = qca->getIsMetaDataUpdate();

   if (isMetaDataUpdate) {

      // Set up variable details used by some formatting options
      //
      this->stringFormatting.setDbEgu (qca->getEgu ());
      this->stringFormatting.setDbEnumerations (qca->getEnumerations ());
      this->stringFormatting.setDbPrecision (qca->getPrecision ());

      // Update display limits if requested and defined.
      //
      if (this->getUseDbDisplayLimits ()) {

         double lower;
         double upper;

         lower = qca->getDisplayLimitLower ();
         upper = qca->getDisplayLimitUpper ();

         // Check that sensible limits have been defined and not just left
         // at the default (i.e. zero) values by a lazy database creator.
         // Otherwise, leave as design time limits.
         //
         if ((lower != 0.0) || (upper != 0.0)) {
            this->setRange (lower, upper);
         }
      }
   }

   // Form and save the image - must do before call to setValue.
   //
   this->theImage = this->stringFormatting.formatString (value, this->getArrayIndex());

   // Update the progress bar
   //
   this->setValue (value);
   this->update ();   // always update and redraw

   // Invoke common alarm handling processing.
   // Although this sets widget style, we invoke for tool tip processing only.
   //
   this->processAlarmInfo (alarmInfo, variableIndex);

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged signals declared in header file.
   //
   this->emitDbValueChanged (variableIndex);
}

//------------------------------------------------------------------------------
// Update variable name etc.
//
void QEAnalogProgressBar::useNewVariableNameProperty (QString variableNameIn,
                                                      QString variableNameSubstitutionsIn,
                                                      unsigned int variableIndex)
{
   this->setVariableNameAndSubstitutions (variableNameIn, variableNameSubstitutionsIn, variableIndex);
}

//==============================================================================
// Copy/paste
//
QString QEAnalogProgressBar::copyVariable ()
{
   return this->getSubstitutedVariableName (PV_VARIABLE_INDEX);
}

//------------------------------------------------------------------------------
//
QVariant QEAnalogProgressBar::copyData ()
{
   return QVariant (this->getValue ());
}

//------------------------------------------------------------------------------
//
void QEAnalogProgressBar::paste (QVariant s)
{
   this->setVariableName (s.toString (), PV_VARIABLE_INDEX);
   this->establishConnection (PV_VARIABLE_INDEX);
}

//------------------------------------------------------------------------------
// useDbDisplayLimits
//
void QEAnalogProgressBar::setUseDbDisplayLimits (bool useDbDisplayLimitsIn)
{
   this->useDbDisplayLimits = useDbDisplayLimitsIn;
}

//------------------------------------------------------------------------------
//
bool QEAnalogProgressBar::getUseDbDisplayLimits () const
{
   return this->useDbDisplayLimits;
}

//------------------------------------------------------------------------------
//
void QEAnalogProgressBar::setAlarmSeverityDisplayMode (const AlarmSeverityDisplayModes mode)
{
   if (this->alarmSeverityDisplayMode != mode) {
      this->alarmSeverityDisplayMode = mode;
      this->update ();
   }
}

//------------------------------------------------------------------------------
//
QEAnalogProgressBar::AlarmSeverityDisplayModes
QEAnalogProgressBar::getAlarmSeverityDisplayMode () const
{
   return this->alarmSeverityDisplayMode;
}

// end
