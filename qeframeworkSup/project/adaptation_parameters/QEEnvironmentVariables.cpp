/*  QEAdaptationParameters.cpp $
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
 *  Copyright (c) 2013 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QDebug>
#include <QByteArray>
#include <QtGlobal>

#include "QEEnvironmentVariables.h"

#define NOT_A_NUMBER  "__not_a_number__"

QString QEEnvironmentVariables::defaultPrefix = "";

//------------------------------------------------------------------------------
//
QEEnvironmentVariables::QEEnvironmentVariables ()
{
   this->prefix = defaultPrefix;
}

//------------------------------------------------------------------------------
//
QEEnvironmentVariables::QEEnvironmentVariables (const QString& prefixIn)
{
   this->prefix = prefixIn;
}

//------------------------------------------------------------------------------
//
QEEnvironmentVariables::~QEEnvironmentVariables ()
{
}

//------------------------------------------------------------------------------
//
void QEEnvironmentVariables::setDefaultPrefix (const QString& prefix)
{
   defaultPrefix = prefix;
}

//------------------------------------------------------------------------------
//
bool QEEnvironmentVariables::getBool (const QString &name, const bool defaultValue)
{
   bool result;
   QString sval;

   sval = this->getString (name, "");
   if (sval == "1" || sval.toUpper() == "TRUE") {
      result = true;
   } else if (sval == "0" || sval.toUpper() == "FALSE") {
      result = true;
   } else {
      result = defaultValue;
   }
   return result;
}

//------------------------------------------------------------------------------
//
QString QEEnvironmentVariables::getString (const QString& name, const QString& defaultValue)
{
   QString envVar;
   QByteArray byteArray;
   QString result;

   envVar = this->prefix;
   envVar.append (name.toUpper ());

   byteArray = qgetenv (envVar.toLatin1().data ());

   result = byteArray.constData ();
   if (result.isEmpty ()) {
      result = defaultValue;
   }

   return result;
}

//------------------------------------------------------------------------------
//
int QEEnvironmentVariables::getInt (const QString& name, const int defaultValue)
{
   int result = defaultValue;
   QString sval;
   bool okay;

   sval = this->getString (name, NOT_A_NUMBER);
   if (sval != NOT_A_NUMBER) {
      result = sval.toInt (&okay);
      if (!okay) result = defaultValue;
   }
   return result;
}

//------------------------------------------------------------------------------
//
double QEEnvironmentVariables::getFloat  (const QString& name, const double defaultValue)
{
   double result = defaultValue;
   QString sval;
   bool okay;

   sval = this->getString (name, NOT_A_NUMBER);
   if (sval != NOT_A_NUMBER) {
      result = sval.toDouble (&okay);
      if (!okay) result = defaultValue;
   }
   return result;
}

// end
