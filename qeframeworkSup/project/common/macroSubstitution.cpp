/*  macroSubstitutions.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2014-2019 Australian Synchrotron
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

#include "macroSubstitution.h"
#include <QDebug>
#include <QtGlobal>

#define DEBUG qDebug () << "macroSubstitution" << __LINE__  << __FUNCTION__ << "  "


//==============================================================================
// macroSubstitutionfunctions
//==============================================================================
// Constructor
macroSubstitution::macroSubstitution () { }

//------------------------------------------------------------------------------
//
macroSubstitution::macroSubstitution (const QString& keyIn, const QString& valueIn)
{
   this->key = keyIn;
   this->value = valueIn;
}

//------------------------------------------------------------------------------
// Destructor
//
macroSubstitution::~macroSubstitution () { }

//------------------------------------------------------------------------------
//
const QString macroSubstitution::getKey () const
{
   return this->key;
}

//------------------------------------------------------------------------------
//
const QString macroSubstitution::getValue () const
{
   return this->value;
}


//==============================================================================
// macroSubstitutionList functions
//==============================================================================
// Constructor - parse string
macroSubstitutionList::macroSubstitutionList () { }

//------------------------------------------------------------------------------
//
macroSubstitutionList::macroSubstitutionList (const QString& string)
{
   this->addMacroSubstitutions (string);
}

//------------------------------------------------------------------------------
// Destructor
//
macroSubstitutionList::~macroSubstitutionList ()
{
   this->parts.clear();
}

//------------------------------------------------------------------------------
// Parse a string of macro substitutions and return a list of keys and values
//
// This is implemented using a finite state table.
// the states are defined in the enumeration list 'states'.
// The case statement for each state is prefixed with a comment showing which
// parts of the format that case deals with.
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
void macroSubstitutionList::addMacroSubstitutions (const QString& substitutions)
{
   // Loop variables
   int index;

   int keyStart = 0;            // Index to first key character
   int keyEnd = 0;              // Index to character PAST last key character (key length = keyEnd - keyStart)
   int valueStart = 0;          // Index to first value character
   int valueEnd = 0;            // Index to character PAST last value character (value length = valueEnd - valueStart)

   QString key;                 // Key extracted from macro substitutions
   QString value;               // Value extracted from macro substitutions

   bool processingSpaces = false;       // True if working through spaces that may be in the middle of a value

   enum states {
      PRE_KEY, KEY, POST_KEY, EQUATE, VALUE_START_QUOTE,
      VALUE, VALUE_QUOTED, POST_VALUE, ERROR
   };
   states state = PRE_KEY;

   // Process the substitutions.
   //
   const int count = substitutions.length ();
   for (index = 0; index < count; index++) {
      // Get next character from the macro substitutions
      //
      const char nextChar = substitutions.at (index).toLatin1 ();

      // Finite state switch
      switch (state) {

         // Error parsing. Ignore rest of string.
         case ERROR:
            index = substitutions.length ();
            break;

         // [___]KEY[___]=[___][[']VALUE[']][___][,...]
         //  ^^^ ^
         case PRE_KEY:
            switch (nextChar) {
               case '\t':
               case ' ':
               case ',': // Handles case where macros were blindly added on to an
                         // empty macro string with a ',' in between (,KEY=VALUE_)
                  break;

               case '=':
               case '\'':
                  state = ERROR;
                  break;

               default:
                  keyStart = index;
                  state = KEY;
                  break;
            }
            break;

         // [___]KEY[___]=[___][[']VALUE[']][___][,...]
         //       ^^ ^   ^
         case KEY:
            switch (nextChar) {
               case '\t':
               case ' ':
                  keyEnd = index;
                  key = substitutions.mid (keyStart, keyEnd - keyStart);
                  state = POST_KEY;
                  break;

               case '=':
                  keyEnd = index;
                  key = substitutions.mid (keyStart, keyEnd - keyStart);
                  state = EQUATE;
                  break;

               default:
                  break;
            }
            break;

         // [___]KEY[___]=[___][[']VALUE[']][___][,...]
         //           ^^ ^
         case POST_KEY:
            switch (nextChar) {
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
            switch (nextChar) {
               case '\t':
               case ' ':
                  break;

               case '\'':
                  state = VALUE_START_QUOTE;
                  break;

               case ',':
                  value = QString ();
                  this->append (key, value);

                  state = PRE_KEY;
                  break;

               default:
                  valueStart = index;
                  processingSpaces = false;

                  state = VALUE;
                  break;
            }
            break;

         // [___]KEY[___]=[___]'VALUE'[___][,...]
         //                     ^    ^
         case VALUE_START_QUOTE:
            switch (nextChar) {
               case '\'':
                  value = QString ();
                  this->append (key, value);

                  state = POST_VALUE;
                  break;

               default:
                  valueStart = index;

                  state = VALUE_QUOTED;
                  break;
            }
            break;

         // [___]KEY[___]=[___]VALUE[___][,...]
         //                     ^^^^ ^^^  ^
         case VALUE:
            switch (nextChar) {
               case '\t':
               case ' ':
                  if (!processingSpaces) {
                     // Working through spaces that may be in the middle of the value
                     // This will mark the value end if there are no more non white space value characters
                     processingSpaces = true;
                     valueEnd = index;
                  }
                  break;

               case ',':
                  if (!processingSpaces) {
                     valueEnd = index;
                  }
                  value = substitutions.mid (valueStart, valueEnd - valueStart);
                  this->append (key, value);

                  processingSpaces = false;
                  state = PRE_KEY;
                  break;

               default:
                  // No longer working through spaces that may be in the middle of the value
                  processingSpaces = false;
                  break;
            }
            break;

         // [___]KEY[___]=[___]'VALUE'[___][,...]
         //                      ^^^^^
         case VALUE_QUOTED:
            switch (nextChar) {
               case '\'':
                  valueEnd = index;

                  value = substitutions.mid (valueStart, valueEnd - valueStart);
                  this->append (key, value);

                  state = POST_VALUE;
                  break;

               default:
                  break;
            }
            break;

         // [___]KEY[___]=[___][[']VALUE[']][___][,...]
         //                                  ^^^  ^
         case POST_VALUE:
            switch (nextChar) {
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
   switch (state) {
      case VALUE:
         if (!processingSpaces) {
            valueEnd = index;
         }
         value = substitutions.mid (valueStart, valueEnd - valueStart);
         this->append (key, value);
         break;

      case EQUATE:
         value = QString ();
         this->append (key, value);
         break;

      default:
         break;
   }
}

//------------------------------------------------------------------------------
// Add a key/value pair.
// When a key is included more than once, the first takes precidence,
// so don't bother adding successive identical keys.
//
void macroSubstitutionList::append (const QString& key, const QString& value)
{
   // Don't bother appending if the key is already present.
   //
   const int count = parts.count ();
   for (int i = 0; i < count; i++) {
      if (key == this->parts.at (i).key) {
         return;
      }
   }

   // Add the key/value pair
   //
   this->parts.append (macroSubstitution (key, value));
}

//------------------------------------------------------------------------------
// Replace occurances of a single key with a value [static]
//
void macroSubstitutionList::substituteKey (QString& string, QString key, const QString value)
{
   key.prepend ("$(");
   key.append (")");
   string.replace (key, value);
}

//------------------------------------------------------------------------------
// Perform a set of macro substitutions on a string
//
QString macroSubstitutionList::substitute (const QString& string) const
{
   static const int numberOfPasses = 10;

   QString result = string;
   const int count = this->parts.count ();

   // Anything to do? Skip if input is empty or does noy even contain
   // a '$' character or the number of substitution elements is zero.
   // Avoids a lot debug clutter (when debugging) and may speed things
   // up a bit.
   // 
   if (result.isEmpty() ||
       !result.contains('$') ||
       (count == 0)) return result;

   // Apply the substitutions
   // We apply multiple times to allow for dereferencing,
   // i.e. supposed  AA='$(BB)' and BB='CC'
   // On pass 1 $(AA) becones $(BB), on pass 2 $(BB) becomes CC
   // We limit the number of passes to ten to avoid infinite loops
   //
   for (int j = 1; j <= numberOfPasses; j++) {
      const QString preSubstitutionResult = result;
      for (int i = 0; i < count; i++) {
         macroSubstitution part = this->parts.at (i);
         substituteKey (result, part.key, part.value);
      }

      // If no change on this pass - all done.
      //
      if (result == preSubstitutionResult) break;

      // There was a change - go again unless limit exceeded.
      //
      if (j == numberOfPasses) {
         DEBUG << string << "expansion requires more than"
               << numberOfPasses << " passes.";
      }
   }

   return result;
}

//------------------------------------------------------------------------------
// Return the substitution as a clean comma delimited string
//
QString macroSubstitutionList::getString () const
{
   QString result;
   const int count = this->parts.count ();
   for (int i = 0; i < count; i++) {
      // Add delimiter if required
      if (i != 0) {
         result.append (",");
      }
      // Get next key/value pair
      macroSubstitution part = this->parts.at (i);

      // Quote value if required
      QString value;
      if (part.value.contains (' ')) {
         value = QString ("'%1'").arg (part.value);
      } else {
         value = part.value;
      }

      // Add next key/value pair to the string
      result.append (QString ("%1=%2").arg (part.key).arg (value));
   }

   // Return the macro substitution string
   return result;

}

//------------------------------------------------------------------------------
// Return the number of substitutions
//
int macroSubstitutionList::getCount () const
{
   return this->parts.count ();
}

//------------------------------------------------------------------------------
// Return a key
//
const QString macroSubstitutionList::getKey (const int i) const
{
   if ((i >= 0) && (i < this->parts.count ())) {
      return this->parts.at (i).key;
   } else {
      return QString ();
   }
}

//------------------------------------------------------------------------------
// Return a value (given a position index inthe macro substitution list)
// Return an empty string if index is out of range
//
const QString macroSubstitutionList::getValue (const int i) const
{
   if ((i >= 0) && (i < this->parts.count ())) {
      return this->parts.at (i).value;
   } else {
      return QString ();
   }
}

//------------------------------------------------------------------------------
// Return a value (given a key)
// Return an empty string if the key is not found
//
const QString macroSubstitutionList::getValue (const QString& keyIn) const
{
   const int count = this->parts.count ();

   for (int i = 0; i < count; i++) {
      if (this->parts.at (i).key == keyIn) {
         return this->parts.at (i).value;
      }
   }
   return QString ();
}

//------------------------------------------------------------------------------
// Extract substitution item (given an index)
//
const macroSubstitution macroSubstitutionList::getItem (const int i) const
{
   macroSubstitution defaultItem;
   return this->parts.value (i, defaultItem);
}


//==============================================================================
// Debug functions
//==============================================================================
//
QDebug operator<< (QDebug dbg, const macroSubstitution& item)
{
#if QT_VERSION >= 0x050400
   dbg.noquote();
#endif
   dbg << QString ("macroSubstitution('%1' => '%2')")
          .arg (item.getKey ())
          .arg (item.getValue ());
#if QT_VERSION >= 0x050400
   dbg.quote();
#endif
   dbg.maybeSpace ();
   return dbg;
}

//------------------------------------------------------------------------------
//
QDebug operator<< (QDebug dbg, const macroSubstitutionList& list)
{
   const int count = list.getCount ();

   for (int i = 0; i < count; i++) {
      dbg  << "\n  " << i << list.getItem(i);
   }
   dbg.maybeSpace ();
   return dbg;
}

// end
