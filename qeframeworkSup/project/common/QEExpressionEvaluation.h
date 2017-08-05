/*  QEExpressionEvaluation.h
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
 *  Copyright (c) 2013 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QEEXPRESSIONEVALUATION_H
#define QEEXPRESSIONEVALUATION_H

#include <QHash>
#include <QString>
#include <QEFrameworkLibraryGlobal.h>

//---------------------------------------------------------------------------------
/// Description:
/// The QEExpressionEvaluation object allows the evaluation of expressions similar
/// to that available in calc/calcout records; and in fact "under the covers" it
/// uses the calcRecord's postfix functions out of the Com shared library.
///
/// The user initialises the QEExpressionEvaluation object with an expression as
/// per the CALC field of a calc (or calcout) record, e.g. "A + LN (B/C)", and
/// then may evaluate the expression by calling the Evaluate method with an
/// array of Doubles, each element supplying the value for A, B, C etc.
/// The standard CALC functions (SIN, LOG, ABS etc.) are all available.
///
/// The QEExpressionEvaluation object provides an extended input name space for
/// the user supplied expressions, i.e. 52 names (A .. Z, A' .. Z') instead of
/// the native 12 inputs (A .. L). However, the MAXIMUM number of inputs may
/// that may be used in any one expression is still LIMITED to 12.
///
/// That is:  "B + L' + M + X + Y' "  is an allowed expression, but
/// "A + B + C + D + E + F + G + H + I + J + K + L' + M" is invalid because
/// more than 12 inputs specified.
///
/// This input name space extension is to allow sensible use with the Strip
/// Chart widget that has 12 available inputs and the Plotter widget that has
/// 16 avialable inputs plus an X input.
///
/// Also, the length of the input string is not limited to the 40 characters
/// of the CALC field, but may use the full 100 characters allowed by the
/// underlying postfix function.
///
/// Acknowledgements:
/// QEExpressionEvaluation is a direct crib of TCalculate out of the Delphi OPI framework.
/// The postfix and calcPerform functions were written by Bob Dalesio (12-12-86).
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEExpressionEvaluation {
public:
   QEExpressionEvaluation ();
   virtual ~QEExpressionEvaluation ();

   // User arguments A .. Z and A' .. Z'.
   //
   static const int NumberUserArguments = 26;
   static const int NumberInputKinds = 2;
   enum InputKinds { Normal, Primed };

   typedef double CalculateArguments [NumberInputKinds][NumberUserArguments];

   bool initialise (const QString& expression);
   QString getCalcError () { return this-> calcError; }

   double evaluate (const CalculateArguments& userArgs, bool* okay = 0);

   static void clear (CalculateArguments& userArgs);
   static int indexOf (const char c);

private:
   bool buildMaps (const QString& expression, QString& translated);

   static const int MaxInfixSize = 100;

   // This is the value from the INFIX_TO_POSTFIX_SIZE macro from postfix.h
   // plus a 20 just in case.
   //
   static const int MaxPostfixSize = 524 + 20;
   char postFix [MaxPostfixSize];

   typedef QHash<int, int> ArgumentMaps;

   QString calcError;
   ArgumentMaps userInputMap;   // maps 0 .. 51 => 0 .. 11
   ArgumentMaps argumentMap;    // maps 0 .. 11 => 0 .. 51
};

#endif  // QEEXPRESSIONEVALUATION_H
