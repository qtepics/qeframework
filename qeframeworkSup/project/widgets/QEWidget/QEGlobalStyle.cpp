/*  QEGlobalStyle.cpp
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
 *  Copyright (c) 2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include "QEGlobalStyle.h"

#if QT_VERSION >= 0x050000
#include <QtWidgets/QApplication>
#else
#include <QtGui/QApplication>
#endif

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QEAdaptationParameters.h>


#define DEBUG qDebug () << "QEGlobalStyle" << __LINE__ << __FUNCTION__ << "  "


bool QEGlobalStyle::styleAlreadyApplied = false;

//------------------------------------------------------------------------------
//
QEGlobalStyle::QEGlobalStyle() {} // place holder

//------------------------------------------------------------------------------
//
QEGlobalStyle::~QEGlobalStyle () {} // place holder

//------------------------------------------------------------------------------
//
void QEGlobalStyle::apply ()
{
   if (QEGlobalStyle::styleAlreadyApplied) return;  // Don't keep re-applying style
   QEGlobalStyle::styleAlreadyApplied = true;       // Set to ensure idempotent

   QEAdaptationParameters ap ("QE_");
   QString globalStyle = ap.getString ("global_style_sheet", "").trimmed ();

   // Does the environment variable specify a file ?
   //
   if (globalStyle.startsWith ("file://")) {
      // Yes - extract filename and read contents.
      //
      QString filename = globalStyle.mid (7).trimmed ();

      QFile styleSheetFile (filename);
      if (styleSheetFile.open (QIODevice::ReadOnly | QIODevice::Text)) {
         QTextStream source (&styleSheetFile);
         globalStyle = source.readAll ().trimmed ();
      } else {
         DEBUG << "cannot open file global style sheet file:" << filename;
         globalStyle = "";
      }
   }

   // Has a global style been specified ?
   //
   if (!globalStyle.isEmpty ()) {
      // Yes - find core application and caste to allow access to the styleSheet
      // and setStyleSheet functions.
      //
      QApplication* app = dynamic_cast <QApplication*>(QCoreApplication::instance ());
      if (app) {
         QString styleSheet = app->styleSheet ();
         if (!styleSheet.isEmpty()) {
            // We prepend global style to the existing style so that we honor
            // any style already set up by the display manager.
            //
            globalStyle.append (";");
            globalStyle.append (styleSheet);
         }
         app->setStyleSheet (globalStyle);

      } else {
         DEBUG << "Could not find the QApplication instance";
      }
   }
}

// end
