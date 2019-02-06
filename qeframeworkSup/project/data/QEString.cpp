/*  QEString.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2018 Australian Synchrotron
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

// String wrapper for QCaObject variant data.

#include <QEString.h>
#include <QDebug>

#define DEBUG qDebug() << "QEString" << __LINE__ << __FUNCTION__ << "  "


/*

*/
QEString::QEString( QString recordName, QObject* eventObject,
                      QEStringFormatting* newStringFormat,
                      unsigned int variableIndexIn ) : QCaObject( recordName, eventObject, variableIndexIn ) {
    initialise( newStringFormat );
}

QEString::QEString( QString recordName, QObject* eventObject,
                      QEStringFormatting* newStringFormat,
                      unsigned int variableIndexIn, UserMessage* userMessageIn ) : QCaObject( recordName, eventObject, variableIndexIn, userMessageIn ) {
    initialise( newStringFormat );
}
/*
    Stream the QCaObject data through this class to generate textual data
    updates.
*/
void QEString::initialise( QEStringFormatting* newStringFormat ) {
    stringFormat = newStringFormat;

    QObject::connect( this, SIGNAL( connectionChanged(  QCaConnectionInfo&, const unsigned int& ) ),
                      this, SLOT( forwardConnectionChanged( QCaConnectionInfo&, const unsigned int& ) ) );

    QObject::connect( this, SIGNAL( dataChanged( const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                      this, SLOT( convertVariant( const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
}

/*
    Take a new text value and write it to the database.
    The type of data formatted (text, floating, integer, etc) will be determined by the record data type,
    How the text is parsed will be determined by the string formatting.
    For example, assume the record is a floating point record and the text is formatted as an integer.
    The string is parsed as in integer (123 is OK, 123.456 would fail), then converted to a floating point number.
    The above example is pedantic
    if the string formatting
*/
bool QEString::writeString( const QString &data, QString& message )
{
    bool ok = false;
    QVariant formattedData = stringFormat->formatValue( data, ok );
    if( ok )
    {
        writeData( formattedData );
    }
    else
    {
        message = QString( "Write failed, unabled to format: '" ).append( data ).append( "'." );
    }
    return ok;
}

void QEString::writeString( const QString &data )
{
    QString message;
    bool ok = writeString( data, message );
    if( !ok )
    {
        qDebug() << message;
    }
}

/*
    Take a new string value, insert into array data updating the arrayIndex slot, and write whole array to the database.
    Formatting as per writeString.
*/
bool QEString::writeStringElement( const QString &data, QString& message )
{
   bool ok = false;
   QVariant elementValue = stringFormat->formatValue( data, ok );
   if( ok )
   {
       writeDataElement( elementValue );
   }
   else
   {
       message = QString( "Write element failed, unabled to format:'" ).append( data ).append( "'." );
   }
   return ok;

}

void QEString::writeStringElement( const QString& data )
{
    QString message;
    bool ok = writeStringElement( data, message );
    if( !ok )
    {
        qDebug() << message;
    }
}

/*
    Take a new string array and write it to the database.
*/
bool QEString::writeString( const QVector<QString> &data, QString& message )
{
   bool ok = false;
   QVariant arrayValue = stringFormat->formatValue( data, ok );
   if( ok )
   {
       writeData( arrayValue );
   }
   else
   {
      message = QString( "Write element failed, unabled to format string array." );
   }

   return ok;
}

void QEString::writeString( const QVector<QString>& data )
{
    QString message;
    bool ok = writeString( data, message );
    if( !ok )
    {
        qDebug() << message;
    }
}

/*
    Take a new value from the database and emit a string,formatted
    as directed by the set of formatting information held by this class
*/
void QEString::convertVariant( const QVariant& value, QCaAlarmInfo& alarmInfo,
                               QCaDateTime& timeStamp, const unsigned int& variableIndex ) {

    // Set up variable details used by some formatting options
    stringFormat->setDbEgu( getEgu() );
    stringFormat->setDbEnumerations( getEnumerations() );
    stringFormat->setDbPrecision( getPrecision() );

    // Format the data and send it
    const QString formatted = stringFormat->formatString( value, getArrayIndex () );
    emit stringChanged( formatted, alarmInfo, timeStamp, variableIndex );
}

/*
    Re send connection change and with variableIndex - depricated.
*/
void QEString::forwardConnectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex) {
    emit stringConnectionChanged( connectionInfo, variableIndex );
}

// end
