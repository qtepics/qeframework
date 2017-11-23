/*  QEExpressionEvaluation.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2013,2017 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include "QEExpressionEvaluation.h"
#include <QDebug>
#include <QECommon.h>
#include <postfix.h>   // out of EPICS


#define DEBUG  qDebug () << "QEExpressionEvaluation" << __LINE__ << __FUNCTION__ << "  "

//---------------------------------------------------------------------------------
//
QEExpressionEvaluation::QEExpressionEvaluation (const bool allowPrimedInputIn) :
   allowPrimedInput (allowPrimedInputIn)
{
   // We are erroneous until Postfix called.
   //
   this->initialise ("");
}

//---------------------------------------------------------------------------------
//
QEExpressionEvaluation::~QEExpressionEvaluation ()
{
   // place holder
}

//---------------------------------------------------------------------------------
//
bool QEExpressionEvaluation::initialise (const QString& expression)
{
   bool okay;
   QString translated;

   // Tooo big ?
   //
   if (expression.length() > MaxInfixSize) {
      this->calcError = "Expression too long";
      return false;
   }

   okay = this->buildMaps (expression, translated);
   if (!okay) {
      this->calcError = "Build Map failed - too many args";
      return false;
   }

   // Now apply map
   //
   const char* pinfix;
   long status;
   short error;

   pinfix = translated.toLatin1 ().data ();

   status = postfix (pinfix, this->postFix, &error);
   this->calcError = QString (calcErrorStr (error));

   return (status == 0);
}

//---------------------------------------------------------------------------------
//
void QEExpressionEvaluation::clear (CalculateArguments& userArgs)
{
   int i, j;
   for (i = 0; i < ARRAY_LENGTH (userArgs); i++) {
      for (j = 0; j < ARRAY_LENGTH (userArgs [i]); j++) {
         userArgs [i][j] = 0.0;
      }
   }
}

//---------------------------------------------------------------------------------
//
int QEExpressionEvaluation::indexOf (const char c)
{
   int result;

   result = (int) c - (int) 'A';
   if (result >= 0 && result < NumberUserArguments) return result;

   result = (int) c - (int) 'a';
   if (result >= 0 && result < NumberUserArguments) return result;

   return -1;
}

//---------------------------------------------------------------------------------
//
double QEExpressionEvaluation::evaluate (const CalculateArguments& userArgs, bool* okayOut) const
{
   double result = 0.0;
   long status;
   double args [CALCPERFORM_NARGS];
   int j;
   int u;
   int kind;
   int letter;

   // convert user arguments into post fix arguments.
   //
   for (j = 0; j < ARRAY_LENGTH (args); j++) {
      args [j] = 0.0;
      if (this->argumentMap.contains (j)) {
         u = this->argumentMap.value (j);
         if (u >= 0 && u < 2*NumberUserArguments) {
            kind   = u / NumberUserArguments;
            letter = u % NumberUserArguments;
            args [j] = userArgs [kind] [letter];
         }
      }
   }

   status = calcPerform (args, &result, this->postFix);
   if (okayOut) {
      *okayOut = (status == 0);
   }

   return result;
}

//---------------------------------------------------------------------------------
// Looks for and collates single input letters A .. Z and A' .. Z'  and maps
// these onto A .. L.
//
bool QEExpressionEvaluation::buildMaps (const QString& expression, QString& translated)
{
   static const QChar primeChar = '\'';

   int len;
   int j;
   QChar x;
   QChar y;
   int kind;
   char c;
   int letter;
   int argIndex;
   int userArg;

   this->argumentMap.clear();
   this->userInputMap.clear();

   argIndex = -1;
   translated = "";

   len = expression.length ();
   for (j = 0; j < len; j++) {
      x = expression.at (j);
      // Skip prime.
      // TODO: Only skip ' following A..Z
      //
      if (x == primeChar) {
         if (this->allowPrimedInput) {
            continue;
         } else {
            return false;
         }
      }

      if (!x.isLetter ()) {
         translated.append (x);
         continue;
      }

      // Look at previous/next chars
      // We want, e.g., a stand alone 'I', but not an I in "SIN"
      //
      if (j > 0) {
         y = expression.at (j - 1);
         if (y.isLetterOrNumber ()) {
            translated.append (x);
            continue;
         }
      }
      if (j + 1 <  len) {
         y = expression.at (j + 1);
         if (y.isLetterOrNumber ()) {
            translated.append (x);
            continue;
         }
      }

      kind = Normal;
      if (this->allowPrimedInput &&
          (j + 1 <  len) &&
          (expression.at (j + 1) == primeChar)) {
         kind = Primed;
      }

      c = x.toUpper().toLatin1 ();
      letter = (int) c - (int) 'A';
      userArg = (NumberUserArguments * kind) + letter;

      if (!this->userInputMap.contains (userArg)) {
         // Add new map int item.
         //
         argIndex = argIndex + 1;   // allocate next postfix argument number.

         if (argIndex >= CALCPERFORM_NARGS) {
            qDebug () << "Too many inputs: " << expression;
            return false;
         }

         this->userInputMap.insert (userArg, argIndex);
         this->argumentMap.insert (argIndex, userArg);
      }

      letter = this->userInputMap.value (userArg);
      c = (char) ((int) 'A' + letter);
      translated.append (c);

   }
   return true;
}

// end
