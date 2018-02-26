/*  QELink.h
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
 *  Copyright (c) 2009,2010,2015,2018 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef QE_LINK_H
#define QE_LINK_H

#include <QLabel>
#include <QEWidget.h>
#include <QVariant>
#include <QString>
#include <QEFrameworkLibraryGlobal.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QELink : public QLabel, QEWidget {
   Q_OBJECT

  public:
    QELink( QWidget *parent = 0 );

    enum conditions { CONDITION_EQ, CONDITION_NE, CONDITION_GT, CONDITION_GE, CONDITION_LT, CONDITION_LE };

    // Property convenience functions

    // condition
    void setCondition( conditions conditionIn );
    conditions getCondition();

    // comparisonValue Value to compare input signals to
    void    setComparisonValue( QString comparisonValue );
    QString getComparisonValue();

    // signalTrue (Signal if condition is met)
    void setSignalTrue( bool signalTrue );
    bool getSignalTrue();

    // signalFalse (Signal if condition not met)
    void setSignalFalse( bool signalFalse );
    bool getSignalFalse();

    // outTrueValue Value to emit if condition is met
    void    setOutTrueValue( QString outTrueValue );
    QString getOutTrueValue();

    // outFalseValue Value to emit if condition is not met
    void    setOutFalseValue( QString outFalseValue );
    QString getOutFalseValue();

  protected:
    conditions condition;       // Rule for comparing incoming signal
    QVariant comparisonValue;    // Value to compare incoming signal against

    bool signalTrue;            // Flag true if a signal should be emmited if the condition matches
    bool signalFalse;           // Flag true if a signal should be emmited if the condition does not match
    QVariant outTrueValue;      // Value to send in signal if condition matches
    QVariant outFalseValue;     // Value to send in signal if condition does not match

  public slots:
    void in( const bool& in );
    void in( const int& in );
    void in( const long& in );
    void in( const qlonglong& in );
    void in( const double& in );
    void in( const QString& in );

    void autoFillBackground( const bool& enable );

  signals:
    void out( const bool& out );
    void out( const int& out );
    void out( const long& out );
    void out( const qlonglong& out );
    void out( const double& out );
    void out( const QString& out );


  private:
    bool isProcessing;
    void sendValue( bool match );
    void emitValue( QVariant value );

  public:
    // Qt Designer Properties - condition
    Q_ENUMS(ConditionNames)
    Q_PROPERTY(ConditionNames condition READ getConditionProperty WRITE setConditionProperty)
    enum ConditionNames { Equal              = QELink::CONDITION_EQ,
                          NotEqual           = QELink::CONDITION_NE,
                          GreaterThan        = QELink::CONDITION_GT,
                          GreaterThanOrEqual = QELink::CONDITION_GE,
                          LessThan           = QELink::CONDITION_LT,
                          LessThanOrEqual    = QELink::CONDITION_LE };
    void setConditionProperty( ConditionNames condition ){ setCondition( (QELink::conditions)condition ); }
    ConditionNames getConditionProperty(){ return (ConditionNames)getCondition(); }

    Q_PROPERTY(QString comparisonValue READ getComparisonValue WRITE setComparisonValue)
    Q_PROPERTY(bool signalTrue READ getSignalTrue WRITE setSignalTrue)
    Q_PROPERTY(bool signalFalse READ getSignalFalse WRITE setSignalFalse)
    Q_PROPERTY(QString outTrueValue READ getOutTrueValue WRITE setOutTrueValue)
    Q_PROPERTY(QString outFalseValue READ getOutFalseValue WRITE setOutFalseValue)
    Q_PROPERTY(bool runVisible READ getRunVisible WRITE setRunVisible)

};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QELink::ConditionNames)
#endif

#endif   // QE_LINK_H
