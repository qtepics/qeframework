/*  QEMenuButtonData.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2015-2022 Australian Synchrotron
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

#include "QEMenuButtonData.h"
#include <QECommon.h>
#include <QEOneToOne.h>

#define DEBUG  qDebug () << "QEMenuButtonData" << __FUNCTION__ << __LINE__ << ":"


typedef applicationLauncher::programStartupOptions  PSO;
typedef QEStringFormatting::formats                 SFF;
typedef QEActionRequests::Options                   ARO;

static QEOneToOne<applicationLauncher::programStartupOptions, QString> startOptionsEnums;
static QEOneToOne<QEStringFormatting::formats,                QString> valueFormatEnums;
static QEOneToOne<QEActionRequests::Options,                  QString> actionOptionsEnums;


// Initializes the environment. Automatically called on program start.
//
bool QEMenuButtonData::elaborate ()
{
   qRegisterMetaType<QEMenuButtonData> ("QEMenuButtonData");

   startOptionsEnums.insertF (applicationLauncher::PSO_NONE,      "None");
   startOptionsEnums.insertF (applicationLauncher::PSO_TERMINAL,  "Terminal");
   startOptionsEnums.insertF (applicationLauncher::PSO_LOGOUTPUT, "LogOut");
   startOptionsEnums.insertF (applicationLauncher::PSO_STDOUTPUT, "StdOut");

   valueFormatEnums.insertF (QEStringFormatting::FORMAT_DEFAULT,          "Default");
   valueFormatEnums.insertF (QEStringFormatting::FORMAT_FLOATING,         "Floating");
   valueFormatEnums.insertF (QEStringFormatting::FORMAT_INTEGER,          "Integer");
   valueFormatEnums.insertF (QEStringFormatting::FORMAT_UNSIGNEDINTEGER,  "UnsignedInteger");
   valueFormatEnums.insertF (QEStringFormatting::FORMAT_TIME,             "Time");
   valueFormatEnums.insertF (QEStringFormatting::FORMAT_LOCAL_ENUMERATE,  "LocalEnumeration");
   valueFormatEnums.insertF (QEStringFormatting::FORMAT_STRING,           "String");

   actionOptionsEnums.insertF (QEActionRequests::OptionOpen,                   "Open");
   actionOptionsEnums.insertF (QEActionRequests::OptionNewTab,                 "NewTab");
   actionOptionsEnums.insertF (QEActionRequests::OptionNewWindow,              "NewWindow");
   actionOptionsEnums.insertF (QEActionRequests::OptionTopDockWindow,          "TopDockWindow");
   actionOptionsEnums.insertF (QEActionRequests::OptionBottomDockWindow,       "BottomDockWindow");
   actionOptionsEnums.insertF (QEActionRequests::OptionLeftDockWindow,         "LeftDockWindow");
   actionOptionsEnums.insertF (QEActionRequests::OptionRightDockWindow,        "RightDockWindow");
   actionOptionsEnums.insertF (QEActionRequests::OptionTopDockWindowTabbed,    "TopDockWindowTabbed");
   actionOptionsEnums.insertF (QEActionRequests::OptionBottomDockWindowTabbed, "BottomDockWindowTabbed");
   actionOptionsEnums.insertF (QEActionRequests::OptionLeftDockWindowTabbed,   "LeftDockWindowTabbed");
   actionOptionsEnums.insertF (QEActionRequests::OptionRightDockWindowTabbed,  "RightDockWindowTabbed");
   actionOptionsEnums.insertF (QEActionRequests::OptionFloatingDockWindow,     "FloatingDockWindow");

   return true;
}

const bool  QEMenuButtonData::elaborated = QEMenuButtonData::elaborate ();

//------------------------------------------------------------------------------
//
QString QEMenuButtonData::psoToString (const applicationLauncher::programStartupOptions value)
{
   return startOptionsEnums.valueF (value, "");
}

//------------------------------------------------------------------------------
//
applicationLauncher::programStartupOptions QEMenuButtonData::stringToPso (const QString& image)
{
   return startOptionsEnums.valueI (image, applicationLauncher::PSO_NONE);
}

//------------------------------------------------------------------------------
//
QString QEMenuButtonData::optionToString (const QEActionRequests::Options  value)
{
   return actionOptionsEnums.valueF (value, "");
}

//------------------------------------------------------------------------------
//
QEActionRequests::Options QEMenuButtonData::stringToOption (const QString& image)
{
    return actionOptionsEnums.valueI (image, QEActionRequests::OptionOpen);
}


//------------------------------------------------------------------------------
//
QString QEMenuButtonData::formatToString (const QEStringFormatting::formats  value)
{
   return valueFormatEnums.valueF (value, "");
}

//------------------------------------------------------------------------------
//
QEStringFormatting::formats QEMenuButtonData::stringToFormat (const QString& image)
{
   return valueFormatEnums.valueI (image, QEStringFormatting::FORMAT_DEFAULT);
}

//------------------------------------------------------------------------------
//
QString QEMenuButtonData::join  (const QStringList& x)
{
   QString result = x.join (" ");
   return result;
}

//------------------------------------------------------------------------------
//
QStringList QEMenuButtonData::split (const QString& x)
{
   QStringList result = QEUtilities::split (x);
   return result;
}

//==============================================================================
//
QEMenuButtonData::QEMenuButtonData ()
{
   this->separator = false;

   this->programName = "";
   this->programArguments.clear ();
   this->programStartupOption = applicationLauncher::PSO_NONE;

   this->uiFilename = "";
   this->prioritySubstitutions = "";
   this->customisationName = "";
   this->creationOption = QEActionRequests::OptionOpen;

   this->variable = "";
   this->variableValue = "0";
   this->format = QEStringFormatting::FORMAT_DEFAULT;
}

//------------------------------------------------------------------------------
//
QEMenuButtonData::~QEMenuButtonData () {}  // place holder


//------------------------------------------------------------------------------
//
QVariant QEMenuButtonData::toVariant () const
{
   QVariant data;
   data.setValue (*this);
   return data;
}

//------------------------------------------------------------------------------
//
bool QEMenuButtonData::setValue (const QVariant& data)
{
   bool result = data.canConvert<QEMenuButtonData> ();
   if (result) {
      *this = data.value<QEMenuButtonData> ();
   }
   return result;
}

//------------------------------------------------------------------------------
//
QDataStream& operator<< (QDataStream& stream, QEMenuButtonData& that)
{
   stream << that.separator;

   stream << that.programName;
   stream << that.programArguments;
   stream << int (that.programStartupOption);

   stream << that.uiFilename;
   stream << that.prioritySubstitutions;
   stream << int (that.creationOption);
   stream << that.customisationName;

   stream << that.variable;
   stream << that.variableValue;
   stream << int (that.format);

   return stream;
}

//------------------------------------------------------------------------------
//
QDataStream& operator>> (QDataStream& stream, QEMenuButtonData& that)
{
   int e;

   stream >> that.separator;

   stream >> that.programName;
   stream >> that.programArguments;
   stream >> e; that.programStartupOption = applicationLauncher::programStartupOptions (e);

   stream >> that.uiFilename;
   stream >> that.prioritySubstitutions;
   stream >> e; that.creationOption = QEActionRequests::Options (e);
   stream >> that.customisationName;

   stream >> that.variable;
   stream >> that.variableValue;
   stream >> e; that.format = QEStringFormatting::formats (e);

   return stream;
}

//------------------------------------------------------------------------------
//
QDebug operator<< (QDebug dbg, const QEMenuButtonData& mbd)
{
   dbg << "QEMenuButtonData("
       << mbd.separator << ","

       << mbd.programName << ","
       << mbd.programArguments << ","
       << QEMenuButtonData::psoToString (mbd.programStartupOption) << ","

       << mbd.uiFilename << ","
       << mbd.prioritySubstitutions << ","
       << QEMenuButtonData::optionToString (mbd.creationOption) << ","
       << mbd.customisationName << ","

       << mbd.variable  << ","
       << mbd.variableValue   << ","
       << QEMenuButtonData::formatToString (mbd.format) << ")";

   return dbg;
}


// end

