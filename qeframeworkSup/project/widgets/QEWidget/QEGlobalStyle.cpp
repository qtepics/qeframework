/*  QEGlobalStyle.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2017-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
