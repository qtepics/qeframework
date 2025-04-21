/*  QELink.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2025 Australian Synchrotron
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

#ifndef QE_LINK_H
#define QE_LINK_H

#include <QLabel>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QEWidget.h>
#include <QEFrameworkLibraryGlobal.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QELink : public QLabel, public QEWidget {
   Q_OBJECT
public:
   // Qt Designer Properties - condition
   //
   enum ConditionNames {
      Equal = 0,
      NotEqual,
      GreaterThan,
      GreaterThanOrEqual,
      LessThan,
      LessThanOrEqual,
      Lookup           // Not a test per se
   };
   Q_ENUM (ConditionNames)

   Q_PROPERTY (ConditionNames condition READ getCondition       WRITE setCondition)
   Q_PROPERTY (QString comparisonValue  READ getComparisonValue WRITE setComparisonValue)
   Q_PROPERTY (bool signalTrue          READ getSignalTrue      WRITE setSignalTrue)
   Q_PROPERTY (bool signalFalse         READ getSignalFalse     WRITE setSignalFalse)
   Q_PROPERTY (QString outTrueValue     READ getOutTrueValue    WRITE setOutTrueValue)
   Q_PROPERTY (QString outFalseValue    READ getOutFalseValue   WRITE setOutFalseValue)
   Q_PROPERTY (QStringList lookupValues READ getLookupValues    WRITE setLookupValues)
   Q_PROPERTY (bool runVisible          READ getRunVisible      WRITE setRunVisible)

public:
   explicit QELink (QWidget * parent = 0);
   ~QELink ();

   // Property convenience functions

   // condition
   void setCondition (const ConditionNames conditionIn);
   ConditionNames getCondition ();

   // comparisonValue Value to compare input signals to
   void setComparisonValue (const QString& comparisonValue);
   QString getComparisonValue () const;

   // signalTrue (Signal if condition is met)
   void setSignalTrue (const bool signalTrue);
   bool getSignalTrue () const;

   // signalFalse (Signal if condition not met)
   void setSignalFalse (const bool signalFalse);
   bool getSignalFalse () const;

   // outTrueValue Value to emit if condition is met
   void setOutTrueValue (const QString& outTrueValue);
   QString getOutTrueValue () const;

   // outFalseValue Value to emit if condition is not met
   void setOutFalseValue (const QString& outFalseValue);
   QString getOutFalseValue () const;

   // lookupValues
   void setLookupValues (const QStringList& lookupValues);
   QStringList getLookupValues () const;

signals:
   void out (const bool& out);
   void out (const int& out);
   void out (const long& out);
   void out (const qlonglong& out);
   void out (const double& out);
   void out (const QString& out);

public slots:
   void in (const bool& in);
   void in (const int& in);
   void in (const long& in);
   void in (const qlonglong& in);
   void in (const double& in);
   void in (const QString& in);

   void autoFillBackground (const bool &enable);

protected:
   ConditionNames condition;    // Rule for comparing incoming signal
   QVariant comparisonValue;    // Value to compare incoming signal against

   bool signalTrue;             // Flag true if a signal should be emmited if the condition matches
   bool signalFalse;            // Flag true if a signal should be emmited if the condition does not match
   QVariant outTrueValue;       // Value to send in signal if condition matches
   QVariant outFalseValue;      // Value to send in signal if condition does not match
   QStringList lookupValues;    // Value test the condtion name is Map

private:
   bool isProcessing;
   void sendValue (const bool match);
   void sendLookup (const qlonglong index);
   void emitValue (const QVariant& value);
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QELink::ConditionNames)
#endif

#endif                          // QE_LINK_H
