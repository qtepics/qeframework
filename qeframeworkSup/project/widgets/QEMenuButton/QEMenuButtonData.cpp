/*  QEMenuButtonData.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2015-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEMenuButtonData.h"
#include <QECommon.h>
#include <QEOneToOne.h>

#define DEBUG  qDebug () << "QEMenuButtonData" << __FUNCTION__ << __LINE__ << ":"

static QEOneToOne<QE::ProgramStartupOptions, QString> startOptionsEnums;
static QEOneToOne<QE::Formats,               QString> valueFormatEnums;
static QEOneToOne<QE::CreationOptions,       QString> actionOptionsEnums;


// Initializes the environment. Automatically called on program start.
// Shame we didn't use the moc convertion
//
bool QEMenuButtonData::elaborate ()
{
   qRegisterMetaType<QEMenuButtonData> ("QEMenuButtonData");

   startOptionsEnums.insertF (QE::ProgramStartupOptions::NoOutput,          "None");
   startOptionsEnums.insertF (QE::ProgramStartupOptions::Terminal,          "Terminal");
   startOptionsEnums.insertF (QE::ProgramStartupOptions::LogOutput,         "LogOut");
   startOptionsEnums.insertF (QE::ProgramStartupOptions::StdOutput,         "StdOut");

   valueFormatEnums.insertF (QE::Formats::Default,            "Default");
   valueFormatEnums.insertF (QE::Formats::Floating,           "Floating");
   valueFormatEnums.insertF (QE::Formats::Integer,            "Integer");
   valueFormatEnums.insertF (QE::Formats::UnsignedInteger,    "UnsignedInteger");
   valueFormatEnums.insertF (QE::Formats::Time,               "Time");
   valueFormatEnums.insertF (QE::Formats::LocalEnumeration,   "LocalEnumeration");
   valueFormatEnums.insertF (QE::Formats::String,             "String");

   actionOptionsEnums.insertF (QE::CreationOptions::Open,             "Open");
   actionOptionsEnums.insertF (QE::CreationOptions::NewTab,           "NewTab");
   actionOptionsEnums.insertF (QE::CreationOptions::NewWindow,        "NewWindow");
   actionOptionsEnums.insertF (QE::CreationOptions::DockTop,          "TopDockWindow");
   actionOptionsEnums.insertF (QE::CreationOptions::DockBottom,       "BottomDockWindow");
   actionOptionsEnums.insertF (QE::CreationOptions::DockLeft,         "LeftDockWindow");
   actionOptionsEnums.insertF (QE::CreationOptions::DockRight,        "RightDockWindow");
   actionOptionsEnums.insertF (QE::CreationOptions::DockTopTabbed,    "TopDockWindowTabbed");
   actionOptionsEnums.insertF (QE::CreationOptions::DockBottomTabbed, "BottomDockWindowTabbed");
   actionOptionsEnums.insertF (QE::CreationOptions::DockLeftTabbed,   "LeftDockWindowTabbed");
   actionOptionsEnums.insertF (QE::CreationOptions::DockRightTabbed,  "RightDockWindowTabbed");
   actionOptionsEnums.insertF (QE::CreationOptions::DockFloating,     "FloatingDockWindow");

   return true;
}

const bool QEMenuButtonData::elaborated = QEMenuButtonData::elaborate ();

//------------------------------------------------------------------------------
//
QString QEMenuButtonData::psoToString (const QE::ProgramStartupOptions value)
{
   return startOptionsEnums.valueF (value, "");
}

//------------------------------------------------------------------------------
//
QE::ProgramStartupOptions QEMenuButtonData::stringToPso (const QString& image)
{
   return startOptionsEnums.valueI (image, QE::ProgramStartupOptions::NoOutput);
}

//------------------------------------------------------------------------------
//
QString QEMenuButtonData::optionToString (const QE::CreationOptions  value)
{
   return actionOptionsEnums.valueF (value, "");
}

//------------------------------------------------------------------------------
//
QE::CreationOptions QEMenuButtonData::stringToOption (const QString& image)
{
    return actionOptionsEnums.valueI (image, QE::CreationOptions::Open);
}


//------------------------------------------------------------------------------
//
QString QEMenuButtonData::formatToString (const QE::Formats value)
{
   return valueFormatEnums.valueF (value, "");
}

//------------------------------------------------------------------------------
//
QE::Formats QEMenuButtonData::stringToFormat (const QString& image)
{
   return valueFormatEnums.valueI (image, QE::Formats::Default);
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
   this->programStartupOption = QE::ProgramStartupOptions::NoOutput;

   this->uiFilename = "";
   this->prioritySubstitutions = "";
   this->customisationName = "";
   this->creationOption = QE::CreationOptions::Open;

   this->variable = "";
   this->variableValue = "0";
   this->format = QE::Formats::Default;
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
   stream >> e; that.programStartupOption = QE::ProgramStartupOptions (e);

   stream >> that.uiFilename;
   stream >> that.prioritySubstitutions;
   stream >> e; that.creationOption = QE::CreationOptions (e);
   stream >> that.customisationName;

   stream >> that.variable;
   stream >> that.variableValue;
   stream >> e; that.format = QE::Formats (e);

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

