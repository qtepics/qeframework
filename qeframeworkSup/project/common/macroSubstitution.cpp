/*  macroSubstitutions.cpp
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
 *  Copyright (c) 2014 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
 * Description:
 *
 * A class to manage macro substitutions.
 * Macro substitutions are often provioded as a string of keys and values.
 *
 * This class parses such strings, and manages macro substitutions using a list of keys and values.
 */

#include <macroSubstitution.h>

// Constructor - parse string
macroSubstitutionList::macroSubstitutionList( const QString string )
{
    addMacroSubstitutions( string );
}


// Parse a string of macro substitutions and return a list of keys and values
//
// This is implemented using a finite state table.
// the states are defined in the enumeration list 'states'.
// The case statement for each state is prefixed with a comment showing which parts of the format that case deals with.
//
// The substitutions format is:
// [___]KEY[___]=[___][[']VALUE[']][___][,...]
//
// where:
//    ___ = whitespace
//    If optional ' is present before VALUE, a ' must be present after VALUE
//    VALUE may include any character (including white space) except '
//    If VALUE is not present, key is replaced with an empty string
//
// Example:   AAA=123, BBB = 456, CCC = xx xx   ,  DDD= 'xx xx'  EEE=
// Note, in the above example, the values for CCC and DDD are both 'xx xx'
//
void macroSubstitutionList::addMacroSubstitutions( const QString subs )
{
    // Loop variables
    int i;
    int count;

    int keyStart = 0;   // Index to first key character
    int keyEnd = 0;     // Index to character PAST last key character (key length = keyEnd - keyStart)
    int valueStart = 0; // Index to first value character
    int valueEnd = 0;   // Index to character PAST last value character (value length = valueEnd - valueStart)

    QString key;        // Key extracted from macro substitutions
    QString value;      // Value extracted from macro substitutions

    bool processingSpaces = false; // True if working through spaces that may be in the middle of a value

    enum states{ PRE_KEY, KEY, POST_KEY, EQUATE, VALUE_START_QUOTE, VALUE, VALUE_QUOTED, POST_VALUE, ERROR };
    states state = PRE_KEY;

    // Process the substitutions.
    count = subs.length();
    for( i = 0; i < count; i++ )
    {
        // Get next character from the macro substitutions
        char nextChar = subs.at(i).toLatin1();

        // Finite state switch
        switch( state )
        {

        // Error parsing. Ignore rest of string.
        case ERROR:
            i = subs.length();
            break;

        // [___]KEY[___]=[___][[']VALUE[']][___][,...]
        //  ^^^ ^
        case PRE_KEY:
            switch( nextChar )
            {
            case '\t':
            case ' ':
            case ',':  // Handles case where macros were blindly added on to an empty macro string with a ',' in between (,KEY=VALUE_)
                break;

            case '=':
            case '\'':
                state = ERROR;
                break;

            default:
                keyStart = i;
                state = KEY;
                break;
            }
            break;

        // [___]KEY[___]=[___][[']VALUE[']][___][,...]
        //       ^^ ^   ^
        case KEY:
            switch( nextChar )
            {
            case '\t':
            case ' ':
                keyEnd = i;
                key = subs.mid( keyStart, keyEnd-keyStart );
                state = POST_KEY;
                break;

            case '=':
                keyEnd = i;
                key = subs.mid( keyStart, keyEnd-keyStart );
                state = EQUATE;
                break;

            default:
                break;
            }
            break;

        // [___]KEY[___]=[___][[']VALUE[']][___][,...]
        //           ^^ ^
        case POST_KEY:
            switch( nextChar )
            {
            case '\t':
            case ' ':
                break;

            case '=':
                state = EQUATE;
                break;

            default:
                state = ERROR;
                break;
            }
            break;

        // [___]KEY[___]=[___][[']VALUE[']][___][,...]
        //                ^^^   ^ ^              ^
        case EQUATE:
            switch( nextChar )
            {
            case '\t':
            case ' ':
                break;

            case '\'':
                state = VALUE_START_QUOTE;
                break;

            case ',':
                value = QString();
                append( key, value );

                state = PRE_KEY;
                break;

            default:
                valueStart = i;
                processingSpaces = false;

                state = VALUE;
                break;
            }
            break;

        // [___]KEY[___]=[___]'VALUE'[___][,...]
        //                     ^    ^
        case VALUE_START_QUOTE:
            switch( nextChar )
            {
            case '\'':
                value = QString();
                append( key, value );

                state = POST_VALUE;
                break;

            default:
                valueStart = i;

                state = VALUE_QUOTED;
                break;
            }
            break;

        // [___]KEY[___]=[___]VALUE[___][,...]
        //                     ^^^^ ^^^  ^
        case VALUE:
            switch( nextChar )
            {
            case '\t':
            case ' ':
                if( !processingSpaces )
                {
                    processingSpaces = true;  // Working through spaces that may be in the middle of the value
                    valueEnd = i;             // This will mark the value end if there are no more non white space value characters
                }
                break;

            case ',':
                if( !processingSpaces )
                {
                    valueEnd = i;
                }
                value = subs.mid( valueStart, valueEnd-valueStart );
                append( key, value );

                processingSpaces = false;
                state = PRE_KEY;
                break;

            default:
                processingSpaces = false;   // No longer working through spaces that may be in the middle of the value
                break;
            }
            break;

        // [___]KEY[___]=[___]'VALUE'[___][,...]
        //                      ^^^^^
        case VALUE_QUOTED:
            switch( nextChar )
            {
            case '\'':
                valueEnd = i;

                value = subs.mid( valueStart, valueEnd-valueStart );
                append( key, value );

                state = POST_VALUE;
                break;

            default:
                break;
            }
            break;

        // [___]KEY[___]=[___][[']VALUE[']][___][,...]
        //                                  ^^^  ^
        case POST_VALUE:
            switch( nextChar )
            {
            case '\t':
            case ' ':
                break;

            case ',':
                state = PRE_KEY;
                break;

            default:
                state = ERROR;
                break;
            }
            break;
        }
    }

    // Use last value
    switch( state )
    {
    case VALUE:
        if( !processingSpaces )
        {
            valueEnd = i;
        }
        value = subs.mid( valueStart, valueEnd-valueStart );
        append( key, value );
        break;

    case EQUATE:
        value = QString();
        append( key, value );
        break;

    default:
        break;
    }
}

// Add a key/value pair.
// When a key is included more than once, the first takes precidence,
// so don't bother adding successive identical keys.
void macroSubstitutionList::append( QString key, QString value )
{
    // Don't bother appending if the key is already present.
    int count = parts.count();
    for( int i = 0; i < count; i++ )
    {
        if( key == parts.at(i).key)
        {
            return;
        }
    }

    // Add the key
    parts.append( macroSubstitution( key, value ) );
}

// Replace occurances of a single key with a value
void macroSubstitutionList::substituteKey( QString& string, QString key, const QString value )
{
    key.prepend( "$(" );
    key.append( ")" );
    string.replace( key, value );
}

// Perform a set of macro substitutions on a string
QString macroSubstitutionList::substitute( const QString& string ) const
{
    QString result = string;

    // Apply the substitutions
    int count = parts.count();
    for( int i = 0; i < count; i++ )
    {
        macroSubstitution part = parts.at(i);
        substituteKey( result, part.key, part.value );
    }
    return result;
}

// Return the substitution as a clean comma delimited string
QString macroSubstitutionList::getString() const
{
    QString result;
    int count = parts.count();
    for( int i = 0; i < count; i++ )
    {
        // Add delimiter if required
        if( i != 0 )
        {
            result.append( "," );
        }

        // Get next key/value pair
        macroSubstitution part = parts.at(i);

        // Quote value if required
        QString value;
        if( part.value.contains( ' ' ) )
        {
            value = QString( "'%1'" ).arg( part.value );
        }
        else
        {
            value = part.value;
        }

        // Add next key/value pair to the string
        result.append( QString( "%1=%2" ).arg( part.key ).arg( value ) );
    }

    // Return the macro substitution string
    return result;

}

// Return the number of substitutions
int macroSubstitutionList::getCount()
{
    return parts.count();
}

// Return a key
const QString macroSubstitutionList::getKey( const unsigned int i ) const
{
    if( (int)i < parts.count() )
    {
        return parts.at( i ).key;
    }
    else
    {
        return QString();
    }
}

// Return a value (given a position index inthe macro substitution list)
// Return an empty string if index is out of range
const QString macroSubstitutionList::getValue( const unsigned int i ) const
{
    if( (int)i < parts.count() )
    {
        return parts.at( i ).value;
    }
    else
    {
        return QString();
    }
}

// Return a value (given a key)
// Return an empty string if the key is not found
const QString macroSubstitutionList::getValue( const QString keyIn ) const
{
    for( int i = 0; i < parts.count(); i++ )
    {
        if( parts.at(i).key == keyIn )
        {
            return parts.at(i).value;
        }
    }
    return QString();
}
