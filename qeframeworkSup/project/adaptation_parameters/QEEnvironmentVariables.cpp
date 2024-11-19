/*  QEAdaptationParameters.cpp $
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2018 Australian Synchrotron.
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
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include "QEEnvironmentVariables.h"

#include <QDebug>
#include <QByteArray>
#include <QtGlobal>
#include <QEStringFormatting.h>

#define DEBUG qDebug () << "QEEnvironmentVariables" << __LINE__ << __FUNCTION__ << "  "

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
// static
void QEEnvironmentVariables::setDefaultPrefix (const QString& prefix)
{
   QEEnvironmentVariables::defaultPrefix = prefix;
}

//------------------------------------------------------------------------------
// static
QString QEEnvironmentVariables::getDefaultPrefix ()
{
   return QEEnvironmentVariables::defaultPrefix;
}

//------------------------------------------------------------------------------
//
bool QEEnvironmentVariables::getBool (const QString &name, const bool defaultValue) const
{
   bool result;
   QString sval;

   sval = this->getString (name, "");
   if (sval == "1" || sval.toUpper() == "TRUE" || sval.toUpper() == "YES") {
      result = true;
   } else if (sval == "0" || sval.toUpper() == "FALSE" || sval.toUpper() == "NO") {
      result = false;
   } else {
      result = defaultValue;
   }
   return result;
}

//------------------------------------------------------------------------------
//
QString QEEnvironmentVariables::getString (const QString& name, const QString& defaultValue) const
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
int QEEnvironmentVariables::getInt (const QString& name, const int defaultValue) const
{
   int result = defaultValue;
   QString sval;
   bool okay;

   sval = this->getString (name, NOT_A_NUMBER);
   if (sval != NOT_A_NUMBER) {
      QEStringFormatting fmt;  // go with defaults
      result = fmt.toInt (sval, okay);
      if (!okay) result = defaultValue;
   }
   return result;
}

//------------------------------------------------------------------------------
//
double QEEnvironmentVariables::getFloat  (const QString& name, const double defaultValue) const
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
