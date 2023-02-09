/*  QEInteger.cpp
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

// Integer specific wrapper for QCaObject.

#include <QEInteger.h>
#include <QDebug>
#include <QEVectorVariants.h>

#define DEBUG  qDebug () << "QEInteger" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QEInteger::QEInteger( QString recordName, QObject *eventObject,
                      QEIntegerFormatting *integerFormattingIn,
                      unsigned int variableIndexIn ) :
   QCaObject( recordName, eventObject, variableIndexIn )
{
   this->initialise( integerFormattingIn );
}

//------------------------------------------------------------------------------
//
QEInteger::QEInteger( QString recordName, QObject *eventObject,
                      QEIntegerFormatting *integerFormattingIn,
                      unsigned int variableIndexIn, UserMessage* userMessageIn ) :
   QCaObject( recordName, eventObject, variableIndexIn, userMessageIn )
{
   this->initialise( integerFormattingIn );
}

//------------------------------------------------------------------------------
// Stream the QCaObject data through this class to generate integer data updates
//
void QEInteger::initialise( QEIntegerFormatting* integerFormattingIn )
{
   integerFormat = integerFormattingIn;

   QObject::connect( this, SIGNAL( connectionChanged(  QCaConnectionInfo&, const unsigned int&  ) ),
                     this, SLOT( forwardConnectionChanged( QCaConnectionInfo&, const unsigned int&  ) ) );

   QObject::connect( this, SIGNAL( dataChanged( const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&  ) ),
                     this, SLOT( convertVariant( const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&  ) ) );
}

//------------------------------------------------------------------------------
// Take a new integer value and write it to the database.
// The type of data formatted (text, floating, integer, etc) will be determined
// by the record data type,
// How the integer is parsed will be determined by the integer formatting.
//
void QEInteger::writeInteger( const long &data )
{
   this->writeData( integerFormat->formatValue( data ) );
}

//------------------------------------------------------------------------------
// Take a new integer value, insert into array data updating the arrayIndex slot,
// and write whole array to the database.
// Formatting as per writeInteger.
//
void QEInteger::writeIntegerElement( const long &data )
{
   QVariant elementValue = integerFormat->formatValue( data );
   this->writeDataElement( elementValue );
}

//------------------------------------------------------------------------------
// Take a new integer array and write it to the database.
// The type of data formatted (text, floating, integer, etc) will be determined
// by the record data type,
// How the integer is parsed will be determined by the integer formatting.
//
void QEInteger::writeInteger( const QVector<long> &data )
{
   this->writeData( integerFormat->formatValue( data ) );
}

//------------------------------------------------------------------------------
// Slot to recieve data updates from the base QCaObject and generate integer updates.
//
void QEInteger::convertVariant( const QVariant &value, QCaAlarmInfo& alarmInfo,
                                QCaDateTime& timeStamp, const unsigned int& variableIndex )
{
   if( ( value.type() == QVariant::List ) || QEVectorVariants::isVectorVariant( value ) )
   {
      emit integerArrayChanged( integerFormat->formatIntegerArray( value ),
                                alarmInfo, timeStamp, variableIndex );

      int ai = getArrayIndex();
      if( ai >= 0 && ai < value.toList().count() ) {
         // Convert this array element as a scalar update.
         const long item = integerFormat->formatInteger( value.toList().value( ai ));
         emit integerChanged( item, alarmInfo, timeStamp, variableIndex );
      }
   }
   else
   {
      emit integerChanged( integerFormat->formatInteger( value ),
                           alarmInfo, timeStamp, variableIndex );

      // A scalar is also an array with one element.
      //
      QVariantList array;
      array.append (value);
      emit integerArrayChanged( integerFormat->formatIntegerArray( array ),
                                alarmInfo, timeStamp, variableIndex );
   }
}

//------------------------------------------------------------------------------
// Re send connection change and with variableIndex - depricated.
//
void QEInteger::forwardConnectionChanged( QCaConnectionInfo& connectionInfo,
                                          const unsigned int& variableIndex)
{
   emit integerConnectionChanged( connectionInfo, variableIndex );
}

// end
