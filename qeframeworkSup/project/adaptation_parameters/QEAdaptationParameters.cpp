/*  QEAdaptationParameters.cpp
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

#include "QEAdaptationParameters.h"

#include <QDebug>
#include <QDir>

#define DEBUG qDebug () << "QEAdaptationParameters" << __LINE__ << __FUNCTION__ << "  "

// Note: in the original AS Delphi framework, we used the [General] section, but
// in the Qt world this has special meaning, so now use [Adaptation]
//
static const QString prefix = "Adaptation/";

static const QChar nullLetter = QChar ((ushort) 0xDEAD);   // A bit arbitary but does the job


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
   // Not QObjects - we must explicitly delete these.
   delete this->environment;
   delete this->settings;
   delete this->options;
}

//------------------------------------------------------------------------------
//
bool QEAdaptationParameters::getBool (const QString &name)
{
   return QEAdaptationParameters::getBool (name, nullLetter);
}

//------------------------------------------------------------------------------
//
bool QEAdaptationParameters::getBool (const QString &name, const QChar letter)
{
   bool result = false;

   result = this->environment->getBool (name, result);
   result = this->settings->getBool (prefix + name, result);
   if (!result) {
      result = this->options->getBool (name, letter);
   }

   return result;
}

//------------------------------------------------------------------------------
//
QString QEAdaptationParameters::getString (const QString &name,
                                           const QString &defaultValue)
{
   return QEAdaptationParameters::getString (name, nullLetter, defaultValue);
}

//------------------------------------------------------------------------------
//
QString QEAdaptationParameters::getString (const QString &name,
                                           const QChar letter,
                                           const QString &defaultValue)
{
   QString result = defaultValue;

   result = this->environment->getString (name, result);
   result = this->settings->getString (prefix + name, result);
   result = this->options->getString (name, letter, result);
   return result;
}

//------------------------------------------------------------------------------
//
int QEAdaptationParameters::getInt (const QString &name,
                                    const int defaultValue)
{
   return QEAdaptationParameters::getInt (name, nullLetter, defaultValue);
}

//------------------------------------------------------------------------------
//
int QEAdaptationParameters::getInt (const QString &name,
                                    const QChar letter,
                                    const int defaultValue)
{
   int result = defaultValue;

   result = this->environment->getInt (name, result);
   result = this->settings->getInt (prefix + name, result);
   result = this->options->getInt (name, letter, result);
   return result;
}


//------------------------------------------------------------------------------
//
double QEAdaptationParameters::getFloat (const QString &name,
                                         const double defaultValue)
{
   return QEAdaptationParameters::getFloat (name, nullLetter, defaultValue);
}

//------------------------------------------------------------------------------
//
double QEAdaptationParameters::getFloat (const QString &name,
                                         const QChar letter,
                                         const double defaultValue)
{
   double result = defaultValue;

   result = this->environment->getFloat (name, result);
   result = this->settings->getFloat (prefix + name, result);
   result = this->options->getFloat (name, letter, result);
   return result;
}

//------------------------------------------------------------------------------
//
QString  QEAdaptationParameters::getFilename (const QString& name,
                                              const QString& defaultValue)
{
   return QEAdaptationParameters::getFilename (name, nullLetter, defaultValue);
}

//------------------------------------------------------------------------------
//
QString  QEAdaptationParameters::getFilename (const QString& name,
                                              const QChar letter,
                                              const QString& defaultValue)
{
   QString result = defaultValue;

   result = this->environment->getString (name, result);
   
   // getFilename understands filenames relavtive to the settings file directory.
   //
   result = this->settings->getFilename (prefix + name, result);
   result = this->options->getString (name, letter, result);

   if (result.startsWith ("~/")) {
      result = QDir::homePath() + QDir::separator () + result.mid(1);
   }

   return result;
}

// end
