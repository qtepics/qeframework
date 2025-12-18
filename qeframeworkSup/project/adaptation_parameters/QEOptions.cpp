/*  QEOptions.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEOptions.h"
#include <QCoreApplication>
#include <QDebug>
#include <QEStringFormatting.h>

static const QChar nullLetter = QChar ((ushort) 0xDEAD);    // A bit arbitary

static const QString nullString =                           // Also a bit arbitary
      QString (nullLetter).append (nullLetter).append (nullLetter).append (nullLetter);

#define NOT_A_NUMBER  "__not_a_number__"


//--------------------------------------------------------------------------------------
//
QEOptions::QEOptions ()
{
   this->args = QCoreApplication::arguments ();

   // Remove the program name from the set of arguments.
   //
   if (this->args.count() > 0) {
      // Only remove program name if there is one - this voids the segment fault.
      // Note: if core application not started then this will be the case and no
      // options will be available; also QCoreApplication::arguments generate an
      // error message for this, so we don't..
      //
      this->args.removeFirst ();
   }

   this->setUpCommon ();
}

//--------------------------------------------------------------------------------------
//
QEOptions::QEOptions (const QStringList& argsIn)
{
   this->args = argsIn;
   this->setUpCommon ();
}

//--------------------------------------------------------------------------------------
//
void QEOptions::setUpCommon ()
{
   // Find first parameter, i.e. non option argument.
   //
   // Example: Condider options  "--tom"  "dick"  "harry"
   //
   // "--tom" is an option, and "dick" is deemed the first true parameter,
   // so parameterOffset is set to 1.
   //
   // However if user calls getString ("tom"), then we know "dick" is the value
   // associated with "tom", so parameterOffset as set to 2, and "harry" is the
   // first parameter.
   //
   this->parameterOffset = 0;
   for (int j = this->args.count() - 1; j >= 0; j--) {
      QString arg = this->args.value (j);
      if (arg.startsWith("-")) {
         // We have found the last option.
         //
         this->parameterOffset = j + 1;
         break;
      }
   }
}

//--------------------------------------------------------------------------------------
//
QEOptions::~QEOptions ()
{
   this->args.clear ();
}

//--------------------------------------------------------------------------------------
//
void QEOptions::registerOptionArgument (int p)
{
   if (this->parameterOffset < (p + 1)) {
      this->parameterOffset = (p + 1);
   }
}

//--------------------------------------------------------------------------------------
//
int QEOptions::getParameterCount ()
{
   return this->args.count () - this->parameterOffset;
}

//--------------------------------------------------------------------------------------
//
QString QEOptions::getParameter (const int i)
{
   if (i >= 0) {
      return this->args.value (this->parameterOffset + i, "");
   }
   return "";
}


//======================================================================================
//
bool QEOptions::isSpecified (const QString& option, const QChar letter)
{
   QString stringVal;
   stringVal = this->getString (option, letter, nullString);
   return (stringVal != nullString);
}

//--------------------------------------------------------------------------------------
//
bool QEOptions::isSpecified (const QString& option)
{
   return this->isSpecified (option, nullLetter);
}

//--------------------------------------------------------------------------------------
//
bool QEOptions::isSpecified (const QChar letter)
{
   return this->isSpecified (nullString, letter);
}


//======================================================================================
//
bool QEOptions::getBool (const QString& option, const QChar letter)
{
   const QString lookForA = QString ("--") + option;
   const QString lookForB = QString ("-") + letter;
   bool result = false;
   int j;

   for (j = 0; j < this->args.count (); j++ ) {
      QString arg = this->args.value (j);

      // Is arg == --option ?
      //
      if (arg == lookForA || arg == lookForB) {
         result = true;
         break;
      }
   }

   return result;
}

//--------------------------------------------------------------------------------------
//
bool QEOptions::getBool (const QString& option)
{
   return this->getBool (option, nullLetter);
}

//--------------------------------------------------------------------------------------
//
bool QEOptions::getBool  (const QChar letter)
{
   return this->getBool (nullString, letter);
}


//======================================================================================
//
QString QEOptions::getString (const QString& option, const QChar letter, const QString& defaultValue)
{
   const QString lookForA = QString ("--") + option;
   const QString lookForB = QString ("-") + letter;
   const QString lookForEqA = lookForA + QString ("=");
   const QString lookForEqB = lookForB + QString ("=");
   QString result = defaultValue;
   int j;

   for (j = 0; j < this->args.count (); j++ ) {
      QString arg = this->args.value (j);

      // Is arg == --option=something ?
      //
      if (arg.startsWith (lookForEqA)) {
         // return something
         //
         result = arg.remove(0, lookForEqA.length ());
         break;
      }

      // Is arg == -l=something ?
      //
      if (arg.startsWith (lookForEqB)) {
         // return something
         //
         result = arg.remove(0, lookForEqB.length ());
         break;
      }

      // Is arg == --option  or -l?
      //
      if (arg == lookForA || arg == lookForB) {
         // return next arg.
         //
         this->registerOptionArgument (j + 1);
         result = this->args.value (j + 1, defaultValue);
         break;
      }
   }
   return result;
}

//--------------------------------------------------------------------------------------
//
QString QEOptions::getString (const QString& option, const QString& defaultValue)
{
   return this->getString (option, nullLetter, defaultValue);
}

//--------------------------------------------------------------------------------------
//
QString QEOptions::getString (const QChar letter, const QString& defaultValue)
{
   return this->getString (nullString, letter, defaultValue);
}


//======================================================================================
//
int QEOptions::getInt (const QString& option, const QChar letter, const int defaultValue)
{
   int result = defaultValue;
   QString sval;
   bool okay;

   sval = this->getString (option, letter, NOT_A_NUMBER);
   if (sval != NOT_A_NUMBER) {
      QEStringFormatting fmt;  // go with defaults
      result = fmt.toInt (sval, okay);
      if (!okay) result = defaultValue;
   }
   return result;
}

//--------------------------------------------------------------------------------------
//
int QEOptions::getInt (const QString& option, const int defaultValue)
{
   return this->getInt (option, nullLetter, defaultValue);
}

//--------------------------------------------------------------------------------------
//
int QEOptions::getInt (const QChar letter, const int defaultValue)
{
   return this->getInt (nullString, letter, defaultValue);
}


//======================================================================================
//
double QEOptions::getFloat  (const QString& option, const QChar letter, const double  defaultValue)
{
   double result = defaultValue;
   QString sval;
   bool okay;

   sval = this->getString (option, letter, NOT_A_NUMBER);

   if (sval != NOT_A_NUMBER) {
      result = sval.toDouble (&okay);
      if (!okay) result = defaultValue;
   }
   return result;
}

//--------------------------------------------------------------------------------------
//
double QEOptions::getFloat (const QString& option, const double defaultValue)
{
   return this->getFloat (option, nullLetter, defaultValue);
}

//--------------------------------------------------------------------------------------
//
double QEOptions::getFloat (const QChar letter, const double defaultValue)
{
   return this->getFloat (nullString, letter, defaultValue);
}

// end
