/*  QCaAlarmInfo.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2023 Australian Synchrotron
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

// Manage CA (and PVA) alarm and severity information

#ifndef QE_ALARM_INFO_H
#define QE_ALARM_INFO_H

#include <QObject>
#include <QList>
#include <QRegularExpression>
#include <QString>
#include <QStringList>
#include <QEPvNameUri.h>
#include <QEFrameworkLibraryGlobal.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QCaAlarmInfo
{
public:
   typedef unsigned short Status;    // Alarm onfo status type.
   typedef unsigned short Severity;  // Alarm onfo severity type.

   // For backward compatibility.
#  define QCAALARMINFO_SEVERITY  QCaAlarmInfo::Severity

   explicit QCaAlarmInfo();

   // General status and severity
   explicit QCaAlarmInfo( const Status status,
                          const Severity severity );

   // PV update status and severity
   explicit QCaAlarmInfo( const QEPvNameUri::Protocol protocol,
                          const QString& pvName,
                          const Status status,
                          const Severity severity,
                          const QString& message );

   virtual ~QCaAlarmInfo();

   bool operator==(const QCaAlarmInfo& other) const;   // Return true if equal
   bool operator!=(const QCaAlarmInfo& other) const;   // Return true if not equal

   QString statusName() const;       // Return the name of the current alarm state
   QString severityName() const;     // Return the name of the current alarm severity
   QString messageText() const;      // Return alarm message - empty string for CA
   bool isInAlarm() const;           // Return true if there is an alarm
   bool isMinor() const;             // Return true if there is a minor alarm
   bool isMajor() const;             // Return true if there is a major alarm
   bool isInvalid() const;           // Return true if there is an invalid alarm
   bool isOutOfService() const;      // Return true if the associated PV declared OOS.
   QString style() const;            // Return a style string to update the widget's look to reflect the current alarm state

   // getStyleColorName/getColorName return standard color for the alarm state.
   // The former is paler/less solid, suitable e.g. label backgrounds. The later
   // is more solid suitable for graphics. In both cases the returned format is
   // of the form of a 6 digit hex string, e.g.: "#0080ff"
   //
   QString getStyleColorName() const; // Return 'standard' style colour for the alarm state.
   QString getColorName() const;      // Return 'standard' colour for the alarm state.

   static Severity getInvalidSeverity();  // Return a severity that will not match any valid severity
   Severity getSeverity() const;      // Return the current severity
   Status   getStatus() const;        // Return the current status

private:
   QEPvNameUri::Protocol protocol;      // protocol - if known
   QString  pvName;      // pv/record name
   Status   status;      // Alarm state
   Severity severity;    // Alarm severity
   QString  message;     // Alarm message (PV Access only - otherwise empty string)
};


//------------------------------------------------------------------------------
// The main purpose of this class is to manage the color names, which in turn
// control the widget color dependent on the PV severtiy state.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QCaAlarmInfoColorNamesManager :
      public QObject
{
   Q_OBJECT
public:
   // Modes for programatically set color names.
   //
   enum ColorNameKinds {
      cnkPrimary,   // These color names can be userped by adaptation parameter.
      cnkOverride   // These color names supersede all other color name definitions.
   };

   static void setStyleColorNames (const ColorNameKinds kind,
                                   const QStringList& styleColorNames);
   static QStringList getStyleColorNames (const ColorNameKinds kind);

   static void setColorNames (const ColorNameKinds kind,
                              const QStringList& colorNames);
   static QStringList getColorNames (const ColorNameKinds kind);

   // Return default/standard style colour names and color names.
   //
   static QStringList getDefaultStyleColorNames();
   static QStringList getDefaultColorNames();

   // Set and get the out of service PV name list.
   // The matching includes/excludes .VAL when checking for a match, e.g
   // PV name SR11BCM01:CURRENT_MONITOR will match OOS name SR11BCM01:CURRENT_MONITOR.VAL
   // and vice-versa.
   //
   // This PV name list may contain regular expressions.
   //
   static void setOosPvNameList (const QStringList& pvNameList);
   static QStringList getOosPvNameList ();

   // clearOosPvNameList is a conveniance function functionally identical
   // to supplying an empty name list to setOosPvNameList.
   //
   static void clearOosPvNameList ();

private:
   explicit QCaAlarmInfoColorNamesManager ();
   ~QCaAlarmInfoColorNamesManager ();

   // Called after any of the source color names are updated.
   //
   static void determineColorNames ();

   // Used to extract adaptation parameters.
   // Uses the environment variables QE_STYLE_COLOR_NAMES and QE_COLOR_NAMES
   //
   static void extractAdaptationColors ();

   // Checks if the given name is flagged as out of service.
   //
   static bool isBasicNameMatch (const QString& pvName);
   static bool isSmartNameMatch (const QString& pvName);

   // Checks if the given name is flagged as out of service.
   //
   static bool isPvNameDeclaredOos (const QEPvNameUri::Protocol protocol,
                                    const QString& pvName);

   typedef QList<QRegularExpression> QRegularExpressionList;

   static QStringList oosPvNameList;             // textual regular expressions
   static QRegularExpressionList oosRegExpList;  // compiled regular expressions

   static bool elaborate ();
   static const bool callElaborate;

private slots:
   void applicationStartedHandler ();

   friend class QCaAlarmInfo;
};

#endif // QE_ALARM_INFO_H
