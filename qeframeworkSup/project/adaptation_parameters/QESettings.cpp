/*  QESettings.cpp $
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

#include "QESettings.h"
#include <iostream>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QEOptions.h>

#define DEBUG qDebug () << "QESettings" << __LINE__ << __FUNCTION__ << "  "

#define NOT_A_NUMBER  "__not_a_number__"


//==============================================================================
//
static QDir absCurrentPath () {
   QDir current (QDir::currentPath ());
   current.makeAbsolute ();
   return current;
}

static const QDir initialDir = absCurrentPath ();

static const QString parameterFilename = QString ("adaptation_parameters_file.ini");


//==============================================================================
//
QESettings::QESettings ()
{
   QEOptions* clo = new QEOptions ();   // command line options.
   QString fileName;

   // the defualt file is the adaptation_parameters_file.ini file in the applicatiuon's
   // start up directory.
   //
   fileName = initialDir.absolutePath ()
                  .append (QDir::separator())
                  .append (parameterFilename);

   // This may be overriden by --adaptation=../somewhere/else/some/other/file.ini
   //
   fileName = clo->getString ("adaptation", fileName);

   this->commonSetup (fileName);

   delete clo;
}

//------------------------------------------------------------------------------
//
QESettings::QESettings (const QString &fileName)
{
   this->commonSetup (fileName);
}

//------------------------------------------------------------------------------
//
void QESettings::commonSetup (const QString &fileNameIn)
{
   QString cleanName = QDir::cleanPath (fileNameIn);
   QFileInfo fileInfo (cleanName);
   QString fileName;

   // Hard-coded message Id.
   //
   this->setSourceId (9002);

   // Is the given file name an absolute path name?
   //
   if (fileInfo.isAbsolute ()) {
       // Yes - use as is?
       //
       fileName = cleanName;
   } else {
      // fileName always deemed relative to initial dir, irrespective of whether
      // applicaton has changed the current path.
      //
      fileName = initialDir.absolutePath ()
                     .append (QDir::separator ())
                     .append (cleanName);
   }

   // Access file info for modified name.
   //
   QFileInfo fileNameInfo (fileName);

   // Do we have read access (implies that file exists if true).
   //
   if (fileNameInfo.isFile () && fileNameInfo.isReadable ()) {
      this->settings = new QSettings (fileName, QSettings::IniFormat, NULL);

      // QString => QFileInfo => QDir => QString.
      this->directoryName = QFileInfo (fileName).dir ().path ();

   } else {
      this->settings = NULL;
      this->directoryName = "";
   }
}

//------------------------------------------------------------------------------
//
QString QESettings::getDirectoryName ()
{
   return this->directoryName;
}

//------------------------------------------------------------------------------
//
QESettings::~QESettings ()
{
   if (this->settings) {
     delete this->settings;
   }
}

//------------------------------------------------------------------------------
//
bool QESettings::isDefined () const
{
   return (this->settings != NULL);
}

//------------------------------------------------------------------------------
//
QString QESettings::getSettingFileName () const
{
   return this->settings ? this->settings->fileName () : "nil";
}

//------------------------------------------------------------------------------
//
QStringList QESettings::groupKeys (const QString &group)
{
   QStringList result;

   if (this->settings) {
      this->settings->beginGroup (group);
      result = this->settings->allKeys ();
      this->settings->endGroup ();
   }

   return result;
}

//------------------------------------------------------------------------------
//
QVariant QESettings::getValue (const QString &key,
                               const QVariant &defaultValue)
{
   QVariant result;

   if (this->settings) {
      result = this->settings->value (key, defaultValue);
   } else {
      result = defaultValue;
   }

   return result;
}

//------------------------------------------------------------------------------
//
bool QESettings::getBool (const QString &key, const bool defaultValue)
{
   bool result = defaultValue;
   QVariant var = this->getValue (key, QVariant (defaultValue));

   result = var.toBool ();
   return result;
}

//------------------------------------------------------------------------------
//
QString QESettings::getString (const QString &key, const QString &defaultValue)
{
   QString result = defaultValue;
   QVariant var = this->getValue (key, QVariant (defaultValue));

   result = var.toString ();
   return result;
}

//------------------------------------------------------------------------------
//
int QESettings::getInt (const QString &key, const int defaultValue)
{
   int result = defaultValue;
   QVariant var = this->getValue (key, QVariant (defaultValue));
   bool okay;

   result = var.toInt (&okay);
   if (!okay) result = defaultValue;

   return result;
}

//------------------------------------------------------------------------------
//
double QESettings::getFloat (const QString &key, const double defaultValue)
{
   double result = defaultValue;
   QVariant var = this->getValue (key, QVariant (defaultValue));
   bool okay;

   result = var.toDouble (&okay);
   if (!okay) result = defaultValue;

   return result;
}


//------------------------------------------------------------------------------
//
QString QESettings::getFilename (const QString &key, const QString &defaultValue)
{
   QString result = this->getString (key, defaultValue);

   if (!result.isEmpty ()) {
      if (QDir::isRelativePath (result)) {
         result = this->directoryName + initialDir.separator () + result;
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
QESettings* QESettings::getSettings (const QString &key)
{
   QString fileName = this->getFilename (key, "");

   if (fileName.isEmpty()) {
      this->sendMessage (QString ("QESettings: no file name found for key: ").append (key),
                         message_types (MESSAGE_TYPE_INFO));
      return NULL;
   }

   // Access file info for modified name.
   //
   QFileInfo fileNameInfo (fileName);

   // Do we have read access (implies that file exists if true).
   //
   if (!(fileNameInfo.isFile () && fileNameInfo.isReadable ())) {
      this->sendMessage (QString ("QESettings: %1 does not exist or no read access").append (fileName),
                         message_types (MESSAGE_TYPE_INFO));
      return NULL;
   }

   return new QESettings (fileName);
}

//------------------------------------------------------------------------------
// static
//
QESettings* QESettings::getConfigurationParameters (const QString & section)
{
   QESettings* result = NULL;
   QESettings mainParameters;

   result = mainParameters.getSettings (section + "/Configuration_File");
   return result;
}

// end
