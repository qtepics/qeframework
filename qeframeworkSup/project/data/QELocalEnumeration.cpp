/*  QELocalEnumeration.cpp
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

#include "QELocalEnumeration.h"

#include <QDebug>
#include <QMetaType>
#include <QTextStream>
#include <QEPlatform.h>

//-----------------------------------------------------------------------------
//
QELocalEnumeration::QELocalEnumeration ()
{
    setLocalEnumeration( "" );
}

//-----------------------------------------------------------------------------
//
QELocalEnumeration::QELocalEnumeration ( const QString &  localEnumerationIn )
{
    setLocalEnumeration( localEnumerationIn );
}

//-----------------------------------------------------------------------------
//
void QELocalEnumeration::setLocalEnumeration( const QString &  localEnumerationIn )
{
    // Save the original local enumeration string.
    // This is returned when the enumeration is requested as a property.
    localEnumerationString = localEnumerationIn;

    localEnumerationItem item;

    enum states {
        STATE_START,

        STATE_OPERATOR,

        STATE_START_VALUE_QUOTE,
        STATE_UNQUOTED_VALUE,
        STATE_QUOTED_VALUE,
        STATE_END_VALUE_QUOTE,

        STATE_DELIMITER,

        STATE_START_TEXT_QUOTE,
        STATE_UNQUOTED_TEXT,
        STATE_QUOTED_TEXT,
        STATE_END_TEXT_QUOTE,

        STATE_COMMA,

        STATE_END
    };

    int start = 0;                          // Index into enumeration text of current item of interest.
    int len = 0;                            // Length of current item of interest
    int state = STATE_OPERATOR;             // Current state of finite state table
    int size = localEnumerationIn.size();   // Length of local enumeration string to be processed

    // Start with no enumerations
    localEnumeration.clear();

    // Process the enumeration text using a finite state table.
    while( start+len <= size )
    {
        switch( state )
        {
            // Reading the optional operator before the value and it's enumeration string.  For example, the '=' in '0=on,1=off'
            case STATE_OPERATOR:
                // If nothing left, finish
                if( start >= size )
                {
                    state = STATE_END;
                    break;
                }

                // If haven't started yet, skip white space
                if( len == 0 && localEnumerationIn[start] == ' ' )
                {
                    start++;
                    break;
                }

                // If more operator characters, continue
                if((start+len < size) &&
                   (localEnumerationIn[start+len] == '>' ||
                    localEnumerationIn[start+len] == '<' ||
                    localEnumerationIn[start+len] == '=' ||
                    localEnumerationIn[start+len] == '!' ||
                    localEnumerationIn[start+len] == '*' ))
                {
                    len++;
                    break;
                }

                // If an operator is found, use it
                if( len )
                {
                    // Assume operation will not be valid
                    item.op = localEnumerationItem::UNKNOWN;

                    // Interpret operator
                    switch( len )
                    {
                        case 1: // single character operator <, >, =
                            switch( localEnumerationIn[start].toLatin1() )
                            {
                                case '<': item.op = localEnumerationItem::LESS;    break;
                                case '>': item.op = localEnumerationItem::GREATER; break;
                                case '=': item.op = localEnumerationItem::EQUAL;   break;
                                case '*': item.op = localEnumerationItem::ALWAYS;   break;
                            }
                            break;

                        case 2: // double character operator (always ending in '=') <=, >=, !=
                            if( localEnumerationIn[start+1] == '=' )
                            {
                                switch( localEnumerationIn[start].toLatin1() )
                                {
                                    case '<': item.op = localEnumerationItem::LESS_EQUAL;    break;
                                    case '>': item.op = localEnumerationItem::GREATER_EQUAL; break;
                                    case '!': item.op = localEnumerationItem::NOT_EQUAL;     break;
                                }
                            }
                            break;

                        default:
                            // Error do no more
                            state = STATE_END;
                            break;
                    }

                    // Step over operator onto the value if a comparison is to be made, or the text is there is no value expected
                    start += len;
                    len = 0;
                    if( item.op == localEnumerationItem::ALWAYS )
                        state = STATE_DELIMITER;
                    else
                        state = STATE_START_VALUE_QUOTE;
                    break;
                }

                // No operator - assume equality
                item.op = localEnumerationItem::EQUAL;
                state = STATE_START_VALUE_QUOTE;
                break;

            // Where an enumerations value is quoted, handle the opening quotation mark.
            // For example, the first quote in "Pump Off"=off,"Pump On"="on"
            case STATE_START_VALUE_QUOTE:
                // If nothing left, finish
                if( start >= size )
                {
                    state = STATE_END;
                    break;
                }

                // If haven't started yet, skip white space
                if( localEnumerationIn[start] == ' ' )
                {
                    start++;
                    break;
                }

                // If found '"' use it
                if( localEnumerationIn[start] == '"' )
                {
                    start++;
                    len = 0;
                    state = STATE_QUOTED_VALUE;
                    break;
                }

                // No quote found, assume unquoted text instead
                state = STATE_UNQUOTED_VALUE;
                break;

                // Reading a value. For example, the '0' in '0:on,1:off'
            case STATE_QUOTED_VALUE:
                // If nothing left, finish
                if( start+len >= size )
                {
                    state = STATE_END;
                    break;
                }

                // If have all value, note it
                if( localEnumerationIn[start+len] == '"' )
                {
                    item.sValue = localEnumerationIn.mid( start, len );
                    start += len;
                    len = 0;
                    item.dValue = item.sValue.toDouble();
                    state = STATE_END_VALUE_QUOTE;
                    break;
                }

                // Extend the text
                len++;
                break;

                // Reading a value. For example, the '0' in '0:on,1:off'
            case STATE_UNQUOTED_VALUE:
                // If nothing left, finish
                if( start+len >= size )
                {
                    state = STATE_END;
                    break;
                }

                // If haven't started yet, skip white space
                if( len == 0 && localEnumerationIn[start] == ' ' )
                {
                    start++;
                    break;
                }

                // If more characters, continue
                if( localEnumerationIn[start+len] != ' ' && localEnumerationIn[start+len] != ':' )
                {
                    len++;
                    break;
                }

                // If have a value, save it
                if( len )
                {
                    item.sValue = localEnumerationIn.mid( start, len );
                    item.dValue = item.sValue.toDouble();
                    start += len;
                    len = 0;
                    state = STATE_DELIMITER;
                    break;
                }

                // Error do no more
                state = STATE_END;
                break;

            // Where an enumerations value is quoted, handle the closing quotation mark.
            // For example, the second quote in "Pump Off"=off,"Pump On"="on"
            case STATE_END_VALUE_QUOTE:
                // If nothing left, finish
                if( start >= size )
                {
                    state = STATE_END;
                    break;
                }

                // If found '"' use it
                if( localEnumerationIn[start] == '"' )
                {
                    start++;
                    len = 0;
                    state = STATE_DELIMITER;
                    break;
                }

                // Error do no more
                state = STATE_END;
                break;

            // Reading the delimiter between value and text.
            case STATE_DELIMITER:
                // If nothing left, finish
                if( start+len >= size )
                {
                    state = STATE_END;
                    break;
                }

                // If haven't started yet, skip white space
                if( len == 0 && localEnumerationIn[start] == ' ' )
                {
                    start++;
                    break;
                }

                // If delimiter found, use it
                if( localEnumerationIn[start] == ':' )
                {
                    start++;
                    len = 0;
                    state = STATE_START_TEXT_QUOTE;
                    break;
                }

                // Error do no more
                state = STATE_END;
                break;

            // Where an enumerations string is quoted, handle the opening quotation mark.
            // For example, the first quote in 0=off,1="pump on"
            case STATE_START_TEXT_QUOTE:
                // If nothing left, finish
                if( start >= size )
                {
                    state = STATE_END;
                    break;
                }

                // If haven't started yet, skip white space
                if( localEnumerationIn[start] == ' ' )
                {
                    start++;
                    break;
                }

                // If found '"' use it
                if( localEnumerationIn[start] == '"' )
                {
                    start++;
                    len = 0;
                    state = STATE_QUOTED_TEXT;
                    break;
                }

                // No quote found, assume unquoted text instead
                state = STATE_UNQUOTED_TEXT;
                break;

            // Where an enumerations string is quoted, extract the string within quotation marks.
            // For example, the string 'pump on' in in 0:off,1:"pump on"
            case STATE_QUOTED_TEXT:
                // If nothing left, finish
                if( start+len >= size )
                {
                    state = STATE_END;
                    break;
                }

                // If have all text, save it
                if( localEnumerationIn[start+len] == '"' )
                {
                    item.text = localEnumerationIn.mid( start, len );
                    start += len;
                    len = 0;
                    localEnumeration.append( item );
                    item.dValue = 0.0;
                    item.sValue.clear();
                    item.op = localEnumerationItem::UNKNOWN;
                    item.text.clear();
                    state = STATE_END_TEXT_QUOTE;
                    break;
                }

                // Extend the text
                len++;
                break;

            // Where an enumerations string is not quoted, extract the string.
            // For example, the string 'off' in in 0=off,1="pump on"
            case STATE_UNQUOTED_TEXT:
                // If nothing left, finish
                if( start+len >= size )
                {
                    // if reached the end, use what ever we have
                    if( len )
                    {
                        item.text = localEnumerationIn.mid( start, len );
                        localEnumeration.append( item );
                    }
                    state = STATE_END;
                    break;
                }

                // If haven't started yet, skip white space
                if( len == 0 && localEnumerationIn[start] == ' ' )
                {
                    start++;
                    break;
                }

                // If have started, finish text when white space, comma
                if( len != 0 && ( localEnumerationIn[start+len] == ' ' ||  localEnumerationIn[start+len] == ',' ) )
                {
                    item.text = localEnumerationIn.mid( start, len );
                    start += len;
                    len = 0;
                    localEnumeration.append( item );
                    item.dValue = 0.0;
                    item.sValue.clear();
                    item.text.clear();
                    state = STATE_COMMA;
                    break;
                }

                // Extend the text
                len++;
                break;

            // Where an enumerations string is quoted, handle the closing quotation mark.
            // For example, the second quote in 0=off,1="pump on"
            case STATE_END_TEXT_QUOTE:
                // If nothing left, finish
                if( start >= size )
                {
                    state = STATE_END;
                    break;
                }

                // If found '"' use it
                if( localEnumerationIn[start] == '"' )
                {
                    start++;
                    len = 0;
                    state = STATE_COMMA;
                    break;
                }

                // Error do no more
                state = STATE_END;
                break;

            // Reading the ',' between each value and string pair.  For example, the ',' in '0:on,1:off'
            case STATE_COMMA:
                // If nothing left, finish
                if( start >= size )
                {
                    state = STATE_END;
                    break;
                }

                // If haven't started yet, skip white space
                if( localEnumerationIn[start] == ' ' )
                {
                    start++;
                    break;
                }

                // If found ',' use it
                if( localEnumerationIn[start] == ',' )
                {
                    start++;
                    len = 0;
                    state = STATE_OPERATOR;
                    break;
                }

                // Error do no more
                state = STATE_END;
                break;

            // finish. Re-initialise for safety
            case STATE_END:
                start = size+1;
                len = 0;
                break;
        }
    }

    // Replace any \n strings with a real new line character
    for( int i = 0; i < localEnumeration.size(); i++ )
    {
        localEnumeration[i].text.replace( "\\n", "\n" );
    }
}

//-----------------------------------------------------------------------------
//
QString QELocalEnumeration::getLocalEnumeration() const
{
   return localEnumerationString;
}

//-----------------------------------------------------------------------------
//
bool  QELocalEnumeration::isDefined() const
{
   return (localEnumeration.count() > 0);
}

//-----------------------------------------------------------------------------
//
QString QELocalEnumeration::valueToText( const QVariant & value , bool& match ) const
{
    QString result;
    QTextStream stream (&result);
    bool isDouble;
    double dValue;
    QString sValue;

    result = "";
    match = false;

    dValue = 0;

    // If it is a double, use it as a double.
    // If it is a string, use it as a string.
    // If it is anything else, try to convert it to a double, else a string.

    const QMetaType::Type vtype = QEPlatform::metaType (value);
    switch( vtype )
    {
        case QMetaType::Double:
            dValue = value.toDouble();
            isDouble = true;
            break;

        case QMetaType::QString:
            sValue = value.toString();
            isDouble = false;
            break;

        default:
            bool convertOk;
            dValue = value.toDouble( &convertOk );
            if( convertOk )
            {
                isDouble = true;
            }
            else
            {
                sValue = value.toString();
                isDouble = false;
            }
            break;
    }

    // Search for a matching value in the list of local enumerated strings
    int i;
    for( i = 0; i < localEnumeration.size(); i++ )
    {
        // Determine if the value matches an enumeration
        match = false;

#define LOCAL_ENUM_SEARCH( VALUE )             \
        switch( localEnumeration[i].op )       \
        {                                      \
            case localEnumerationItem::LESS:          if( VALUE <  localEnumeration[i].VALUE ) match = true;  break; \
            case localEnumerationItem::LESS_EQUAL:    if( VALUE <= localEnumeration[i].VALUE ) match = true;  break; \
            case localEnumerationItem::EQUAL:         if( VALUE == localEnumeration[i].VALUE ) match = true;  break; \
            case localEnumerationItem::NOT_EQUAL:     if( VALUE != localEnumeration[i].VALUE ) match = true;  break; \
            case localEnumerationItem::GREATER_EQUAL: if( VALUE >= localEnumeration[i].VALUE ) match = true;  break; \
            case localEnumerationItem::GREATER:       if( VALUE >  localEnumeration[i].VALUE ) match = true;  break; \
            case localEnumerationItem::ALWAYS:                                                 match = true;  break; \
            default:                                                                           match = false; break; \
        }


        if( isDouble )
        {
            LOCAL_ENUM_SEARCH( dValue );
        }
        else
        {
            LOCAL_ENUM_SEARCH( sValue );
        }

        // If the value does match, use the enumeration value
        if( match )
        {
            stream << localEnumeration[i].text;
            break;
        }
    }

    // If no match was found, generate the text directly from the value
    if( i >= localEnumeration.size() )
    {
        if( sValue.isEmpty() )
            stream << dValue;
        else
            stream << sValue;
    }

    return result;
}

//-----------------------------------------------------------------------------
//
QVariant QELocalEnumeration::textToValue( const QString & text, bool &ok ) const
{
    QVariant value = QVariant();

    // Init
    ok = false;

    // If value matched a local enumeration, use it
    //
    for( int i = 0; i < localEnumeration.size(); i++ )
    {
        if( text.compare( localEnumeration[i].text ) == 0 )
        {
            qulonglong ul = localEnumeration[i].dValue;
            value = QVariant( ul );
            ok = true;
            return value;
        }
    }

    // Value does not match an enumeration
    //
    value = QVariant( QString("Value does not match a local enumeration value.") );
    return value;
}

//-----------------------------------------------------------------------------
//
int QELocalEnumeration::textToInt( const QString & text, bool& ok ) const
{
   QVariant v;
   int result = 0;

   v = textToValue( text, ok );
   if( ok ) {
      result = v.toInt ( &ok );
   }
   return result;
}

//-----------------------------------------------------------------------------
//
double QELocalEnumeration::textToDouble( const QString & text, bool& ok ) const
{
   QVariant v;
   double result = 0.0;

   v = textToValue( text, ok );
   if( ok ) {
      result = v.toDouble ( &ok );
   }
   return result;
}

// end
