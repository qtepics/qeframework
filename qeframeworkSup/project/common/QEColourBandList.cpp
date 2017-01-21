/*  QEColourBandList.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
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
 *  Copyright (c) 2015 Australian Synchrotron
 *
 */

#include <alarm.h>

#include <QEPlatform.h>
#include <QCaAlarmInfo.h>
#include <QCaObject.h>
#include <QEWidget.h>
#include <QEColourBandList.h>

//------------------------------------------------------------------------------
//
QEColourBandList::QEColourBandList ()
{
   this->clear ();
}

//------------------------------------------------------------------------------
//
QEColourBandList::~QEColourBandList ()
{
   this->clear ();
}

//------------------------------------------------------------------------------
//
void QEColourBandList::clear ()
{
   this->list.clear ();
}

//------------------------------------------------------------------------------
//
void QEColourBandList::append (const QEColourBand& item)
{
   this->list.append (item);
}

//------------------------------------------------------------------------------
//
int QEColourBandList::count () const
{
   return this->list.count ();
}

//------------------------------------------------------------------------------
//
QEColourBand QEColourBandList::value (int j) const
{
   return this->list.value (j);
}

//------------------------------------------------------------------------------
// static
QEColourBand QEColourBandList::createColourBand
      (const double lower, const double upper, const QColor& colour)
{
   QEColourBand result;
   result.lower = lower;
   result.upper = upper;
   result.colour = colour;
   return result;
}

//------------------------------------------------------------------------------
// static
QEColourBand QEColourBandList::createColourBand (const double lower, const double upper,
                                                  const unsigned short severity)
{
   QCaAlarmInfo alarmInfo (0, severity);
   int saturation;
   QColor colour;

   saturation = (severity == NO_ALARM) ? 32 : 128;
   colour = QEWidget::getColor (alarmInfo, saturation);
   return QEColourBandList::createColourBand (lower, upper, colour);
}

//------------------------------------------------------------------------------
//
void QEColourBandList::setAlarmColours (const double dispLower,
                                        const double dispUpper,
                                        qcaobject::QCaObject* qca)
{
   this->list.clear ();
   if (!qca) return;  // sanity check

   double alarmLower = qca->getAlarmLimitLower ();
   double alarmUpper = qca->getAlarmLimitUpper ();
   double warnLower = qca->getWarningLimitLower ();
   double warnUpper = qca->getWarningLimitUpper ();
   bool alarmIsDefined;
   bool warnIsDefined;

   // Unfortunately, the Channel Access protocol only provides the alarm/warning
   // values and not the associated severities. We assume major severity for alarms,
   // and minor severity for warnings.
   // If the HIGH/LOW/HIHI/LOLO values are not defined in the database, the alarm
   // warning levels are returnd as NaN (at least for some record types), so we
   // must check for this as well.
   //
   if (QEPlatform::isNaN (alarmLower)) alarmLower = 0.0;
   if (QEPlatform::isNaN (alarmLower)) alarmLower = 0.0;
   alarmIsDefined = (alarmLower != alarmUpper);

   if (QEPlatform::isNaN (warnLower)) warnLower = 0.0;
   if (QEPlatform::isNaN (warnUpper)) warnUpper = 0.0;
   warnIsDefined = (warnLower != warnUpper);

   if (alarmIsDefined) {
      if (warnIsDefined) {
         // All alarms defined.
         //
         this->list << QEColourBandList::createColourBand (dispLower,  alarmLower, MAJOR_ALARM);
         this->list << QEColourBandList::createColourBand (alarmLower, warnLower,  MINOR_ALARM);
         this->list << QEColourBandList::createColourBand (warnLower,  warnUpper,  NO_ALARM);
         this->list << QEColourBandList::createColourBand (warnUpper,  alarmUpper, MINOR_ALARM);
         this->list << QEColourBandList::createColourBand (alarmUpper, dispUpper,  MAJOR_ALARM);
      } else {
         // Major alarms defined.
         //
         this->list << QEColourBandList::createColourBand (dispLower,  alarmLower, MAJOR_ALARM);
         this->list << QEColourBandList::createColourBand (alarmLower, alarmUpper, NO_ALARM);
         this->list << QEColourBandList::createColourBand (alarmUpper, dispUpper,  MAJOR_ALARM);
      }
   } else {
      if (warnIsDefined) {
         // Minor alarms defined.
         //
         this->list << QEColourBandList::createColourBand (dispLower, warnLower, MINOR_ALARM);
         this->list << QEColourBandList::createColourBand (warnLower, warnUpper, NO_ALARM);
         this->list << QEColourBandList::createColourBand (warnUpper, dispUpper, MINOR_ALARM);
      } else {
         // No alarms defined at all.
         //
         this->list << QEColourBandList::createColourBand (dispLower, dispUpper, NO_ALARM);
      }
   }
}

// end
