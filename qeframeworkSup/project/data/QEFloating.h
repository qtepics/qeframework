/*  QEFloating.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2025 Australian Synchrotron
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, either version 3 of the License.
 *
 *  The EPICS QT Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

// Floating specific wrapper for QCaObject.

#ifndef QE_FLOATING_H
#define QE_FLOATING_H

#include <QMetaType>
#include <QVariant>
#include <QCaObject.h>
#include <QEFloatingFormatting.h>
#include <QEFrameworkLibraryGlobal.h>

// Structures used in signals to indicate connection and data updates.
//
struct QEFloatingValueUpdate {
   double value;
   QCaAlarmInfo alarmInfo;
   QCaDateTime timeStamp;
   unsigned int variableIndex;
};
Q_DECLARE_METATYPE (QEFloatingValueUpdate)

struct QEFloatingArrayUpdate {
   QVector<double> values;
   QCaAlarmInfo alarmInfo;
   QCaDateTime timeStamp;
   unsigned int variableIndex;
};
Q_DECLARE_METATYPE (QEFloatingArrayUpdate)


class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEFloating :
      public qcaobject::QCaObject
{
   Q_OBJECT

public:
   QEFloating (QString pvName, QObject* eventObject,
               QEFloatingFormatting* floatingFormattingIn,
               unsigned int variableIndexIn);

   QEFloating (QString pvName, QObject* eventObject,
               QEFloatingFormatting* floatingFormattingIn,
               unsigned int variableIndexIn,
               UserMessage* userMessageIn);

signals:
   // New style
   void valueUpdated (const QEFloatingValueUpdate& value);
   void arrayUpdated (const QEFloatingArrayUpdate& array);

   // Old style
   void floatingChanged (const double& value, QCaAlarmInfo& alarmInfo,
                         QCaDateTime& timeStamp, const unsigned int& variableIndex);
   void floatingArrayChanged (const QVector<double>& values, QCaAlarmInfo& alarmInfo,
                              QCaDateTime& timeStamp, const unsigned int& variableIndex);

public slots:
   void writeFloating (const double &data);
   void writeFloatingElement (const double &data);
   void writeFloating (const QVector<double> &data);

private:
   void initialise (QEFloatingFormatting* floatingFormattingIn);
   QEFloatingFormatting* floatingFormat;

private slots:
   void convertVariant (const QEVariantUpdate&);
};

#endif // QE_FLOATING_H
