/*  QEMenuButtonData.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2015-2024 Australian Synchrotron
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

#ifndef QE_MENU_BUTTON_DATA_H
#define QE_MENU_BUTTON_DATA_H

#include <QDebug>
#include <QDataStream>
#include <QVariant>
#include <QString>
#include <QStringList>

#include <QEEnums.h>
#include <QEFrameworkLibraryGlobal.h>
#include <applicationLauncher.h>
#include <QEActionRequests.h>
#include <QEStringFormatting.h>

// Own variant type associated with QAction data.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEMenuButtonData {
public:
   explicit QEMenuButtonData ();
   ~QEMenuButtonData ();

   QVariant toVariant () const;                // convert to variant format
   bool setValue (const QVariant& data);       // convert from variant format

   bool separator;

   QString programName;
   QStringList programArguments;
   QE::ProgramStartupOptions programStartupOption;  // default is QE::NoOutput

   QString uiFilename;
   QString prioritySubstitutions;
   QE::CreationOptions creationOption;  // default is QE::Open
   QString customisationName;

   QString variable;
   QString variableValue;               // value to write to the variable
   QE::Formats format;                  // default is QE::Default

   // Utility functions to convert enum to String and vice versa.
   //
   // These enum types used here are declared in non-QObject derviced classes, and
   // thus the meta data enumeration to/from string conversion is not available.
   // Options are: hand craft (as we do now), convert defining classes to QObject
   // classes, replicate this functionality as is in the applicable class, create
   // a separate QObject name class to define these enums, or use rebadged types
   // declared in QEPushButton.
   //
   static QString psoToString (const QE::ProgramStartupOptions  value);
   static QE::ProgramStartupOptions stringToPso (const QString& image);

   static QString optionToString (const QE::CreationOptions  value);
   static QE::CreationOptions stringToOption (const QString& image);

   static QString formatToString (const QE::Formats  value);
   static QE::Formats stringToFormat (const QString& image);

   // Utility functions for dealying with arguments, essentially because the
   // dialog currently uses a QLineEdit as opposed to a string list edit.
   //
   static QString join  (const QStringList& x);
   static QStringList split (const QString& x);

private:
   // Initializes the environment. Automatically called on program start.
   //
   static bool elaborate ();

   // Used to automatically run ::init()
   //
   static const bool elaborated;
};

Q_DECLARE_METATYPE (QEMenuButtonData)   // Allow use as variant, esp. QAction

// Conversion between object and data stream encoding.
//
QDataStream& operator<< (QDataStream& stream, QEMenuButtonData& that);
QDataStream& operator>> (QDataStream& stream, QEMenuButtonData& that);

QDebug operator<< (QDebug dbg, const QEMenuButtonData& mbd);  // Allow streaming to debug.

#endif // QE_MENU_BUTTON_DATA_H
