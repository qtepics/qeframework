/*  QCaAlarmInfo.h
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

// Manage CA alarm and severity information

#ifndef QCA_ALARM_INFO_H
#define QCA_ALARM_INFO_H

#include <QString>
#include <QStringList>
#include <QEFrameworkLibraryGlobal.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QCaAlarmInfo
{
public:
    typedef unsigned short Status;  // Alarm onfo status type.
    typedef unsigned short Severity;  // Alarm onfo severity type.

    // For backward compatibility.
#   define QCAALARMINFO_SEVERITY  QCaAlarmInfo::Severity

    explicit QCaAlarmInfo();
    explicit QCaAlarmInfo( const Status statusIn,
                           const Severity severityIn,
                           const QString& message = "" );
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
    QString style() const;            // Return a style string to update the widget's look to reflect the current alarm state

    // getStyleColorName/getColorName return standard color for the alarm state.
    // The former is paler/less solid, suitable e.g. label backgrounds. The later
    // is more solid suitable for graphics. In both cases the returned format is
    // of the form of a 6 digit hex string, e.g.: "#0080ff"
    //
    QString getStyleColorName() const; // Return 'standard' style colour for the alarm state.
    QString getColorName() const;      // Return 'standard' colour for the alarm state.

    static Severity getInvalidSeverity(); // Return a severity that will not match any valid severity
    Severity getSeverity() const;      // Return the current severity
    Status   getStatus() const;        // Return the current status

    // The following fuunctions take or return a QStringList with four elements indexed
    // by alarm severity, i.e. no alarm, minor, major and invalid.
    //
    static void setStyleColorNames( const QStringList& styleColorNames ); // Set prefered style colour names
    static QStringList getStyleColorNames();                              // Return prefered style colour names

    static void setColorNames( const QStringList& colorNames );           // Set prefered colour names
    static QStringList getColorNames();                                   // Return prefered colour names

    static QStringList getDefaultStyleColorNames();                       // Return default/standard style colour names
    static QStringList getDefaultColorNames();                            // Return default/standard colour names

private:
    Status   status;      // Alarm state
    Severity severity;    // Alarm severity
    QString message;      // Alarm message (pva)

    // default constant colour names
    static const QStringList defaultStyleColorNames;
    static const QStringList defaultColorNames;

    // current dynamic colour names
    static QStringList styleColorNames;
    static QStringList colorNames;

    // used to extract adaptation parameters - idempotent
    static void extractAdaptationColors ();
};

#endif // QCA_ALARM_INFO_H
