/*  QCaAlarmInfo.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2020 Australian Synchrotron
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
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

// Alarm info manager

#include "QCaAlarmInfo.h"
#include <QColor>
#include <QDebug>
#include <QTimer>
#include <alarm.h>
#include <acai_client_types.h>
#include <QECommon.h>
#include <QEAdaptationParameters.h>
#include <QEArchiveInterface.h>

#define DEBUG  qDebug () << "QCaAlarmInfo" << __LINE__ << __FUNCTION__ << "  "

// Quazi severity (set to more more than 3, i.e. INVALID)
//
static const QCaAlarmInfo::Severity OOS_ALARM = 4;
static const int NUMBER_SEVERITIES = 5;

//------------------------------------------------------------------------------
// Default standard color names.
// These string lists are index by alarm severity and/or out of service.
//
const QStringList defaultStyleColorNames = QStringList ()
      << "#e0eee0"                // pale green
      << "#ffff00"                // yellow
      << "#ff8080"                // pale red
      << "#ffffff"                // white
      << "#80c0ff";               // pale blue

const QStringList defaultColorNames = QStringList ()
      << "#00ff00"                // green
      << "#ffff00"                // yellow
      << "#ff0000"                // red
      << "#ffffff"                // white
      << "#0080ff";               // blue


// Adaptation (environment variable) defined color names.
//
QStringList adaptationStyleColorNames = QStringList () << "" << "" << "" << "" << "";
QStringList adaptationColorNames      = QStringList () << "" << "" << "" << "" << "";

// Programatically defined color names.
// Over-rideable [0] and Regular [1]
//
QStringList programStyleColorNames [2] = {
   QStringList () << "" << "" << "" << "" << "",
   QStringList () << "" << "" << "" << "" << ""
};

QStringList programColorNames [2] = {
   QStringList () << "" << "" << "" << "" << "",
   QStringList () << "" << "" << "" << "" << ""
};

// Priority (low to high):
// Default, program-cnkPrimary, adaptation, program-cnkOverride.
// Only non empty color names can override a lower priority name.

// The standard color names currently in use - by default the default color names.
// These string lists are indexed by alarm severity
//
QStringList styleColorNames = defaultStyleColorNames;
QStringList colorNames = defaultColorNames;


//------------------------------------------------------------------------------
// Update/extract current style names.
// deprecated
void QCaAlarmInfo::setStyleColorNames (const QStringList& styleColorNamesIn)
{
   QCaAlarmInfoColorNamesManager::setStyleColorNames (QCaAlarmInfoColorNamesManager::cnkOverride, styleColorNamesIn);
}

//------------------------------------------------------------------------------
// deprecated
QStringList QCaAlarmInfo::getStyleColorNames ()
{
   return QCaAlarmInfoColorNamesManager::getStyleColorNames (QCaAlarmInfoColorNamesManager::cnkOverride);
}

//------------------------------------------------------------------------------
// Update/extract current color names.
// deprecated
void QCaAlarmInfo::setColorNames (const QStringList& colorNamesIn)
{
   QCaAlarmInfoColorNamesManager::setColorNames (QCaAlarmInfoColorNamesManager::cnkOverride, colorNamesIn);
}

//------------------------------------------------------------------------------
// deprecated
QStringList QCaAlarmInfo::getColorNames ()
{
   return QCaAlarmInfoColorNamesManager::getColorNames (QCaAlarmInfoColorNamesManager::cnkOverride);
}

//------------------------------------------------------------------------------
// Extract constant default color style names.
// deprecated
QStringList QCaAlarmInfo::getDefaultStyleColorNames ()
{
   return QCaAlarmInfoColorNamesManager::getDefaultStyleColorNames ();
}

//------------------------------------------------------------------------------
// Extract constant default color names.
// deprecated
QStringList QCaAlarmInfo::getDefaultColorNames ()
{
   return QCaAlarmInfoColorNamesManager::getDefaultColorNames ();
}

//------------------------------------------------------------------------------
// Construct an empty instance.
// By default there is no alarm present.
//
QCaAlarmInfo::QCaAlarmInfo ()
{
   this->pvName = "";
   this->status = NO_ALARM;
   this->severity = NO_ALARM;
   this->message = "";
}

//------------------------------------------------------------------------------
// Construct an instance given an alarm state and severity
//
QCaAlarmInfo::QCaAlarmInfo (const Status statusIn,
                            const Severity severityIn)
{
   this->pvName = "";
   this->status = statusIn;
   this->severity = severityIn;
   this->message = "";
}

//------------------------------------------------------------------------------
// Construct an instance given an alarm state and severity together with
// pvName and mesage (PVA only).
//
QCaAlarmInfo::QCaAlarmInfo (const QString& pvNameIn,
                            const Status statusIn,
                            const Severity severityIn,
                            const QString & messageIn)
{
   this->pvName = pvNameIn;
   this->status = statusIn;
   this->severity = severityIn;
   this->message = messageIn;
}

//------------------------------------------------------------------------------
// Deconstruct - place holder
//
QCaAlarmInfo::~QCaAlarmInfo () { }

//------------------------------------------------------------------------------
// Equality function.
// We only check severity and status, we exclude message and name.
//
bool QCaAlarmInfo::operator== (const QCaAlarmInfo& other) const
{
   return ((this->status == other.status) && (this->severity == other.severity));
}

//------------------------------------------------------------------------------
// InEquality function - defined in terms of == to ensure consistancy.
//
bool QCaAlarmInfo::operator!= (const QCaAlarmInfo& other) const
{
   return !(*this == other);
}

//------------------------------------------------------------------------------
// Return a string identifying the alarm state
//
QString QCaAlarmInfo::statusName () const
{
   // TODO - PVA status strings are different.
   //
   ACAI::ClientAlarmCondition condition = ACAI::ClientAlarmCondition (this->status);
   return QString::fromStdString (ACAI::alarmStatusImage (condition));
}

//------------------------------------------------------------------------------
// Return a string identifying the alarm severity
//
QString QCaAlarmInfo::severityName () const
{
   QString result;

   if ((this->severity & 0x0f00) == 0x0f00) {
      // Do CA archiver severity specials.
      //
      QEArchiveInterface::archiveAlarmSeverity sevr =
            QEArchiveInterface::archiveAlarmSeverity (this->severity);
      result = QEArchiveInterface::alarmSeverityName (sevr);
   } else {
      ACAI::ClientAlarmSeverity sevr = ACAI::ClientAlarmSeverity (this->severity);
      result = QString::fromStdString (ACAI::alarmSeverityImage (sevr));
   }

   return result;
}

//------------------------------------------------------------------------------
// Return alarm message - empty string for CA
//
QString QCaAlarmInfo::messageText () const
{
   return this->message;
}

//------------------------------------------------------------------------------
// Return true if there is an alarm
//
bool QCaAlarmInfo::isInAlarm () const
{
   return (this->severity != NO_ALARM);
}

//------------------------------------------------------------------------------
// Return true if there is a minor alarm
//
bool QCaAlarmInfo::isMinor () const
{
   return (this->severity == MINOR_ALARM);
}

//------------------------------------------------------------------------------
// Return true if there is a major alarm
//
bool QCaAlarmInfo::isMajor () const
{
   return (this->severity == MAJOR_ALARM);
}

//------------------------------------------------------------------------------
// Return true if there is an invalid alarm
//
bool QCaAlarmInfo::isInvalid () const
{
   return (this->severity == INVALID_ALARM);
}

//------------------------------------------------------------------------------
// Return true if the associated PV declared OOS.
//
bool QCaAlarmInfo::isOutOfService() const
{
   return QCaAlarmInfoColorNamesManager::isPvNameDeclaredOos (this->pvName);
}

//------------------------------------------------------------------------------
// Return a style string to update the widget's look to reflect the current alarm state
//
QString QCaAlarmInfo::style () const
{
   QString styleColor = this->getStyleColorName ();
   QColor bgColor (styleColor);

   QString result;

   switch (this->severity) {
      case NO_ALARM:
      case MINOR_ALARM:
      case MAJOR_ALARM:
      case INVALID_ALARM:
      case OOS_ALARM:
         // colourToStyle sets the font color to white or black as appropriate.
         //
         result = QEUtilities::colourToStyle (bgColor);
         break;

      default:
         result = "";
   }

   return result;
}

//------------------------------------------------------------------------------
// Return the style color name for the alarm state
//
QString QCaAlarmInfo::getStyleColorName () const
{
   Severity pvSeverity = this->severity;
   if (QCaAlarmInfoColorNamesManager::isPvNameDeclaredOos(this->pvName)) {
      pvSeverity = OOS_ALARM;
   }
   return styleColorNames.value (int (pvSeverity), "#ffffff");
}

//------------------------------------------------------------------------------
// Return the color name for the alarm state
//
QString QCaAlarmInfo::getColorName () const
{
   Severity pvSeverity = this->severity;
   if (QCaAlarmInfoColorNamesManager::isPvNameDeclaredOos(this->pvName)) {
      pvSeverity = OOS_ALARM;
   }
   return colorNames.value (int (pvSeverity), "#ffffff");
}

//------------------------------------------------------------------------------
// Return a severity that will not match any valid severity (static)
// Not to be confused with the invalid state.
//
QCaAlarmInfo::Severity QCaAlarmInfo::getInvalidSeverity ()
{
   return Severity (ACAI::CLIENT_ALARM_NSEV);
}

//------------------------------------------------------------------------------
// Return the severity
//
QCaAlarmInfo::Severity QCaAlarmInfo::getSeverity () const
{
   return this->severity;
}

//------------------------------------------------------------------------------
// Return the status
//
QCaAlarmInfo::Status QCaAlarmInfo::getStatus () const
{
   return this->status;
}

//==============================================================================
// QCaAlarmInfoColorNamesManager
//==============================================================================
//
QStringList QCaAlarmInfoColorNamesManager::OosPvNameList;

//------------------------------------------------------------------------------
//
QCaAlarmInfoColorNamesManager::QCaAlarmInfoColorNamesManager () : QObject (NULL)
{
   // Request call back after 0 ms after the main event loop has started, and
   // hence after the QApplication has started.
   // We need to do this because QEAdaptationParameters relies on the application
   // already running.
   //
   QTimer::singleShot (0, this, SLOT (applicationStartedHandler ()));
}

//------------------------------------------------------------------------------
//
QCaAlarmInfoColorNamesManager::~QCaAlarmInfoColorNamesManager () { }

//------------------------------------------------------------------------------
// Updates the currently in use color name lists: styleColorNames and colorNames
// static
void QCaAlarmInfoColorNamesManager::determineColorNames ()
{
   QStringList workList;

   // Merge alternatives into the as used style color names.
   //
   workList = defaultStyleColorNames;
   for (int j = 0; j < NUMBER_SEVERITIES; j++) {
      QString v1 = programStyleColorNames[cnkPrimary].value (j, "");
      QString v2 = adaptationStyleColorNames.value (j, "");
      QString v3 = programStyleColorNames[cnkOverride].value (j, "");

      QString cn = "";
      if (!v1.isEmpty()) cn = v1;
      if (!v2.isEmpty()) cn = v2;
      if (!v3.isEmpty()) cn = v3;
      if (!cn.isEmpty()) workList.replace (j, cn);
   }
   styleColorNames = workList;

   // Ditto the color names
   //
   workList = defaultColorNames;
   for (int j = 0; j < NUMBER_SEVERITIES; j++) {
      QString v1 = programColorNames[cnkPrimary].value (j, "");
      QString v2 = adaptationColorNames.value (j, "");
      QString v3 = programColorNames[cnkOverride].value (j, "");

      QString cn = "";
      if (!v1.isEmpty()) cn = v1;
      if (!v2.isEmpty()) cn = v2;
      if (!v3.isEmpty()) cn = v3;
      if (!cn.isEmpty()) workList.replace (j, cn);
   }
   colorNames = workList;
}


//------------------------------------------------------------------------------
// static
void QCaAlarmInfoColorNamesManager::setStyleColorNames (const ColorNameKinds kind,
                                                        const QStringList& styleColorNames)
{
   programStyleColorNames [kind] = styleColorNames;
   QCaAlarmInfoColorNamesManager::determineColorNames ();
}

//------------------------------------------------------------------------------
// static
QStringList QCaAlarmInfoColorNamesManager::getStyleColorNames (const ColorNameKinds kind)
{
   return programStyleColorNames [kind];
}

//------------------------------------------------------------------------------
// static
void QCaAlarmInfoColorNamesManager::setColorNames (const ColorNameKinds kind,
                                                   const QStringList& colorNames)
{
   programColorNames [kind] = colorNames;
   QCaAlarmInfoColorNamesManager::determineColorNames ();
}

//------------------------------------------------------------------------------
// static
QStringList QCaAlarmInfoColorNamesManager::getColorNames (const ColorNameKinds kind)
{
   return programColorNames [kind];
}

//------------------------------------------------------------------------------
// static
//
QStringList QCaAlarmInfoColorNamesManager::getDefaultStyleColorNames()
{
   return defaultStyleColorNames;
}

//------------------------------------------------------------------------------
// static
QStringList QCaAlarmInfoColorNamesManager::getDefaultColorNames()
{
   return defaultColorNames;
}

//------------------------------------------------------------------------------
// static
void QCaAlarmInfoColorNamesManager::setOosPvNameList (const QStringList& pvNameListIn)
{
   QCaAlarmInfoColorNamesManager::OosPvNameList = pvNameListIn;
}

//------------------------------------------------------------------------------
// static
QStringList QCaAlarmInfoColorNamesManager::getOosPvNameList ()
{
   return QCaAlarmInfoColorNamesManager::OosPvNameList;
}

//------------------------------------------------------------------------------
// static
//
void QCaAlarmInfoColorNamesManager::clearOosPvNameList ()
{
   QCaAlarmInfoColorNamesManager::OosPvNameList.clear();
}

//------------------------------------------------------------------------------
// static
bool QCaAlarmInfoColorNamesManager::isPvNameDeclaredOos (const QString& pvName)
{
   bool result;
   // TODO: Think about the protocol qualifier - currently absent
   //       Think about .VAL vs. no field specified
   //       What about allowing regular expressions.
   //
   result = !pvName.isEmpty() &&
            QCaAlarmInfoColorNamesManager::OosPvNameList.contains(pvName);
   return result;
}

//------------------------------------------------------------------------------
// Uses the environment variables QE_STYLE_COLOR_NAMES and QE_COLOR_NAMES
// to overdie the style colors.
// static
void QCaAlarmInfoColorNamesManager::extractAdaptationColors ()
{
   QEAdaptationParameters ap ("QE_");
   QString namesSet;
   QStringList nameList;
   int number;

   // Do styleColorNames
   namesSet = ap.getString ("style_color_names", "");
   if (!namesSet.isEmpty ()) {
      nameList = namesSet.split (":", QString::KeepEmptyParts);

      // We can't do a simple assign - user may have specified less or more than
      // the expected number of severity colour names.
      //
      number = MIN (nameList.count(), NUMBER_SEVERITIES);
      for (int j = 0; j < number; j++) {
         const QString v = nameList.value (j, "");
         adaptationStyleColorNames.replace (j, v);
      }
   }

   // Ditto colorNames
   namesSet = ap.getString ("color_names", "");
   if (!namesSet.isEmpty ()) {
      nameList = namesSet.split(":", QString::KeepEmptyParts);
      number = MIN (nameList.count(), NUMBER_SEVERITIES);
      for (int j = 0; j < number; j++) {
         const QString v = nameList.value (j, "");
         adaptationColorNames.replace (j, v);
      }
   }

   QCaAlarmInfoColorNamesManager::determineColorNames ();
}

//------------------------------------------------------------------------------
// slot
void QCaAlarmInfoColorNamesManager::applicationStartedHandler ()
{
   QCaAlarmInfoColorNamesManager::extractAdaptationColors ();
}

//------------------------------------------------------------------------------
// Create an instance of QCaAlarmInfoExtra - static
//
bool QCaAlarmInfoColorNamesManager::elaborate ()
{
   // We don't worry about deleting this object.
   //
   new QCaAlarmInfoColorNamesManager ();

   return true;
}

//------------------------------------------------------------------------------
// static - just used as a means to call elaborate ()
//
const bool QCaAlarmInfoColorNamesManager::callElaborate =
           QCaAlarmInfoColorNamesManager::elaborate ();

// end
