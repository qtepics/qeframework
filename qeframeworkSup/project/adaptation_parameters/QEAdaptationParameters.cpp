/* QEAdaptationParameters.cpp
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

#include "QEAdaptationParameters.h"
#include <QDebug>

#define DEBUG qDebug () << "QEAdaptationParameters" << __LINE__ << __FUNCTION__ << "  "

// Note: in the original AS Delphi framework, we used the [General] section, but
// in the Qt world this has special meaning.
//
static const QString prefix = "Adaptation/";


//------------------------------------------------------------------------------
//
QEAdaptationParameters::QEAdaptationParameters ()
{
   this->environment = new QEEnvironmentVariables ();
   this->settings = new QESettings ();
   this->options = new QEOptions ();
}

//------------------------------------------------------------------------------
//
QEAdaptationParameters::QEAdaptationParameters (const QString& envPrefix)
{
   this->environment = new QEEnvironmentVariables (envPrefix);
   this->settings = new QESettings ();
   this->options = new QEOptions ();
}

//------------------------------------------------------------------------------
//
QEAdaptationParameters::~QEAdaptationParameters ()
{
   delete this->environment;
   delete this->settings;
   delete this->options;
}

//------------------------------------------------------------------------------
//
bool QEAdaptationParameters::getBool (const QString &name)
{
   bool result = false;

   result = this->environment->getBool (name, result);
   result = this->settings->getBool (prefix + name, result);
   if (!result) {
      result = this->options->getBool (name);
   }

   return result;
}

//------------------------------------------------------------------------------
//
QString QEAdaptationParameters::getString (const QString &name, const QString &defaultValue)
{
   QString result = defaultValue;

   result = this->environment->getString (name, result);
   result = this->settings->getString (prefix + name, result);
   result = this->options->getString (name, result);
   return result;
}

//------------------------------------------------------------------------------
//
int QEAdaptationParameters::getInt (const QString &name, const int defaultValue)
{
   int result = defaultValue;

   result = this->environment->getInt (name, result);
   result = this->settings->getInt (prefix + name, result);
   result = this->options->getInt (name, result);
   return result;
}

//------------------------------------------------------------------------------
//
double QEAdaptationParameters::getFloat (const QString &name, const double defaultValue)
{
   double result = defaultValue;

   result = this->environment->getFloat (name, result);
   result = this->settings->getFloat (prefix + name, result);
   result = this->options->getFloat (name, result);
   return result;
}

// end
