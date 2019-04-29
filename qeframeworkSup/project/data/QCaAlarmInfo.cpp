/*  QCaAlarmInfo.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2019 Australian Synchrotron
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

// CA alarm info manager

#include <QCaAlarmInfo.h>
#include <alarm.h>
#include <acai_client_types.h>
#include <QEArchiveInterface.h>

// Default standard colors.
// These string lists are index by alarm severity
//
const QStringList QCaAlarmInfo::defaultStyleColorNames = QStringList ()
        << "#e0eee0"    // pale green
        << "#ffff00"    // yellow
        << "#ff8080"    // pale red
        << "#ffffff";   // white

const QStringList QCaAlarmInfo::defaultColorNames = QStringList ()
        << "#00ff00"    // green
        << "#ffff00"    // yellow
        << "#ff0000"    // red
        << "#ffffff";   // white


// Current standard colors.
// These string lists are index by alarm severity
//
QStringList QCaAlarmInfo::styleColorNames = QCaAlarmInfo::defaultStyleColorNames;
QStringList QCaAlarmInfo::colorNames = QCaAlarmInfo::defaultColorNames;

/*
   Update/extract current style names.
 */
void QCaAlarmInfo::setStyleColorNames( const QStringList& styleColorNamesIn )
{
    styleColorNames = styleColorNamesIn;
}

QStringList QCaAlarmInfo::getStyleColorNames()
{
   return styleColorNames;
}

/*
   Update/extract current color names.
 */
void QCaAlarmInfo::setColorNames( const QStringList& colorNamesIn )
{
    colorNames = colorNamesIn;
}

QStringList QCaAlarmInfo::getColorNames()
{
   return colorNames;
}

/*
   Extract constant default color style names.
 */
QStringList QCaAlarmInfo::getDefaultStyleColorNames()
{
   return defaultStyleColorNames;
}

/*
   Extract constant default color names.
 */
QStringList QCaAlarmInfo::getDefaultColorNames()
{
   return defaultColorNames;
}

/*
  Construct an empty instance.
  By default there is no alarm present.
 */
QCaAlarmInfo::QCaAlarmInfo()
{
    status = NO_ALARM;
    severity = NO_ALARM;
    message = "";
}

/*
  Construct an instance given an alarm state and severity
 */
QCaAlarmInfo::QCaAlarmInfo( const Status statusIn,
                            const Severity severityIn,
                            const QString& messageIn )
{
    status = statusIn;
    severity = severityIn;
    message = messageIn;
}

/*
  Deconstruct - place holder
 */
QCaAlarmInfo::~QCaAlarmInfo() { }

/*
  Equality function.
 */
bool QCaAlarmInfo::operator==(const QCaAlarmInfo& other) const
{
    return( ( this->status == other.status ) && ( this->severity == other.severity ) );
}

/*
  InEquality function - defined in terms of == to ensure consistancy.
 */
bool QCaAlarmInfo::operator!=(const QCaAlarmInfo& other) const
{
    return !(*this == other);
}


/*
  Return a string identifying the alarm state
 */
QString QCaAlarmInfo::statusName() const
{
    // TODO - PVA status strings are differnt.
    ACAI::ClientAlarmCondition condition = ACAI::ClientAlarmCondition( status );
    return QString::fromStdString( ACAI::alarmStatusImage( condition ) );
}

/*
  Return a string identifying the alarm severity
 */
QString QCaAlarmInfo::severityName() const
{
    QString result;

    if ((severity & 0x0f00) == 0x0f00) {
        // Do CA archiver severity specials.
        //
        QEArchiveInterface::archiveAlarmSeverity sevr = QEArchiveInterface::archiveAlarmSeverity (severity);
        result = QEArchiveInterface::alarmSeverityName (sevr);
    } else {
        ACAI::ClientAlarmSeverity sevr = ACAI::ClientAlarmSeverity( severity );
        result = QString::fromStdString( ACAI::alarmSeverityImage( sevr ) );
    }
    return result;
}

/*
  Return alarm message - empty string for CA
 */
QString QCaAlarmInfo::messageText() const
{
    return message;
}

/*
  Return true if there is an alarm
 */
bool QCaAlarmInfo::isInAlarm() const {
    return( severity != NO_ALARM );
}

/*
  Return true if there is a minor alarm
 */
bool QCaAlarmInfo::isMinor() const {
    return( severity == MINOR_ALARM );
}

/*
  Return true if there is a major alarm
 */
bool QCaAlarmInfo::isMajor() const {
    return( severity == MAJOR_ALARM );
}

/*
  Return true if there is an invalid alarm
 */
bool QCaAlarmInfo::isInvalid() const {
    return !ACAI::alarmSeverityIsValid( ACAI::ClientAlarmSeverity( severity ) );
    return( severity == INVALID_ALARM );
}

/*
  Return a style string to update the widget's look to reflect the current alarm state
 */
QString QCaAlarmInfo::style() const
{
    QString styleColor = getStyleColorName();
    QString result;

    switch( severity )
    {
        case NO_ALARM:
        case MINOR_ALARM:
        case MAJOR_ALARM:
        case INVALID_ALARM:
            result = QString( "QWidget { background-color: %1; }" ).arg( styleColor );
            break;

        default:
            result = "";
    }

    return result;
}

/*
  Return the style color name for the alarm state
 */
QString QCaAlarmInfo::getStyleColorName() const
{
    return styleColorNames.value( int(severity), "#ffffff" );
}


/*
  Return the color name for the alarm state
 */
QString QCaAlarmInfo::getColorName() const
{
    return colorNames.value( int(severity), "#ffffff" );
}

/*
  Return a severity that will not match any valid severity (static)
  Not to be confused with the invalid state.
 */
QCaAlarmInfo::Severity QCaAlarmInfo::getInvalidSeverity()
{
    return ACAI::CLIENT_ALARM_NSEV;
}

/*
  Return the severity
  The caller is only expected to compare this to
 */
QCaAlarmInfo::Severity QCaAlarmInfo::getSeverity() const
{
    return severity;
}

// end
