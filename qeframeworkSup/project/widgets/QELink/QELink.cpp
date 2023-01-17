/*  QELink.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2022 Australian Synchrotron
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

#include <QELink.h>
#include <ContainerProfile.h>
#include <QVariant>
#include <QString>
#include <QDebug>
#include <QECommon.h>

/*
    Constructor with no initialisation
*/
QELink::QELink( QWidget *parent ) : QLabel( parent ), QEWidget( this )
{
    // Don't display this widget, by default (will always display in 'Designer'
    setRunVisible( false );

    // Set default properties
    setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    setText( "Link" );
    setIndent( 6 );

    setStyleSheet ( QEUtilities::offBackgroundStyle() );

    signalFalse = true;
    signalTrue = true;
    isProcessing = false;

    condition = CONDITION_EQ;
}

// Common comparison. Macro to evaluate the 'in' signal value.
// Determine if the 'in' signal value matches the condition
// If match and signaling on a match, then send a signal
// If not a match and signaling on no match, then send a signal
#define EVAL_CONDITION                                              \
                                                                    \
    bool match = false;                                             \
    switch( condition )                                             \
    {                                                               \
        case CONDITION_EQ: if( inVal == val ) match = true; break;  \
        case CONDITION_NE: if( inVal != val ) match = true; break;  \
        case CONDITION_GT: if( inVal >  val ) match = true; break;  \
        case CONDITION_GE: if( inVal >= val ) match = true; break;  \
        case CONDITION_LT: if( inVal <  val ) match = true; break;  \
        case CONDITION_LE: if( inVal <= val ) match = true; break;  \
    }                                                               \
                                                                    \
    sendValue( match );

// Slot to perform a comparison on a bool
void QELink::in( const bool& inVal )
{
    bool val = comparisonValue.toBool();
    EVAL_CONDITION;
}

// Slot to perform a comparison on an integer (int)
void QELink::in( const int& inVal )
{
    qlonglong val = comparisonValue.toLongLong();
    EVAL_CONDITION;
}

// Slot to perform a comparison on an integer (long)
void QELink::in( const long& inVal )
{
    qlonglong val = comparisonValue.toLongLong();
    EVAL_CONDITION;
}

// Slot to perform a comparison on an integer (qLongLong)
void QELink::in( const qlonglong& inVal )
{
    qlonglong val = comparisonValue.toLongLong();
    EVAL_CONDITION;
}

// Slot to perform a comparison on a floating point number
void QELink::in( const double& inVal )
{
    double val = comparisonValue.toDouble();
    EVAL_CONDITION;
}

// Slot to perform a comparison on a string
void QELink::in( const QString& inVal )
{
    bool stringIsNum = false;

    // If the string is a valid number, compare it as a number

    QStringList inList = QEUtilities::split( inVal );
    if( inList.size() )
    {
        double inDouble = inList[0].toDouble( &stringIsNum );
        if( stringIsNum )
        {
            in( inDouble );
        }
    }

    // If the string is not a valid number, do a string comparison
    if( !stringIsNum )
    {
        QString val = comparisonValue.toString();
        EVAL_CONDITION;
    }
}

// Generate appropriate signals following a comparison of an input value
void QELink::sendValue( bool match )
{
    // Avoid infinite signal-slot loops.
    if( !isProcessing ){
        isProcessing = true;

        // If input comparison matched, emit the appropriate value if required
        if( match )
        {
            if( signalTrue )
                emitValue( outTrueValue );
        }

        // If input comparison did not match, emit the appropriate value if required
        else
        {
            if( signalFalse )
                emitValue( outFalseValue );
        }

        isProcessing = false;
    }
}

// Emit signals required when input value matches or fails to match
void QELink::emitValue( QVariant value )
{
    emit out( value.toBool() );
    emit out( value.toInt() );
    emit out( long (value.toLongLong()) );
    emit out( value.toLongLong() );
    emit out( value.toDouble() );
    emit out( value.toString() );
}

// Slot to allow signal/slot manipulation of the auto fill background
// attribute of the base label class
void QELink::autoFillBackground( const bool& enable )
{
    setAutoFillBackground( enable );
}

//==============================================================================
// Property convenience functions

// condition
void QELink::setCondition( conditions conditionIn )
{
    condition = conditionIn;
}
QELink::conditions QELink::getCondition()
{
    return condition;
}

// comparisonValue Value to compare input signals to
void    QELink::setComparisonValue( QString comparisonValueIn )
{
    comparisonValue = QVariant(comparisonValueIn);
}
QString QELink::getComparisonValue()
{
    return comparisonValue.toString();
}

// signalTrue (Signal if condition is met)
void QELink::setSignalTrue( bool signalTrueIn )
{
    signalTrue = signalTrueIn;
}
bool QELink::getSignalTrue()
{
    return signalTrue;
}

// signalFalse (Signal if condition not met)
void QELink::setSignalFalse( bool signalFalseIn )
{
    signalFalse = signalFalseIn;
}
bool QELink::getSignalFalse()
{
    return signalFalse;
}

// outTrueValue Value to emit if condition is met
void    QELink::setOutTrueValue( QString outTrueValueIn )
{
    outTrueValue = QVariant(outTrueValueIn);
}
QString QELink::getOutTrueValue()
{
    return outTrueValue.toString();
}

// outFalseValue Value to emit if condition is not met
void    QELink::setOutFalseValue( QString outFalseValueIn )
{
    outFalseValue = QVariant(outFalseValueIn);
}
QString QELink::getOutFalseValue()
{
    return outFalseValue.toString();
}

// end
