/*  QEFloating.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2024 Australian Synchrotron
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

// Floating specific wrapper for QCaObject.

#ifndef QE_FLOATING_H
#define QE_FLOATING_H

#include <QtDebug>
#include <QVariant>
#include <QCaObject.h>
#include <QEFloatingFormatting.h>
#include <QEFrameworkLibraryGlobal.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEFloating : public qcaobject::QCaObject {
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
   void convertVariant (const QVariant &value, QCaAlarmInfo& alarmInfo,
                        QCaDateTime& timeStamp, const unsigned int& variableIndex);
};

#endif // QE_FLOATING_H
