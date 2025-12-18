/*  QEExitButton.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2025-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEExitButton.h"
#include <QDebug>
#include <QColor>
#include <QMainWindow>
#include <QCoreApplication>
#include <QECommon.h>

#define DEBUG qDebug () << "QEExitButton" << __LINE__ << __FUNCTION__ << "  "


//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QEExitButton::QEExitButton (QWidget * parent) : QPushButton (parent)
{
   this->setText ("Exit");
   this->setExitProgram (false);

   QObject::connect (this, SIGNAL(clicked (bool)),
                     this, SLOT  (onClick (bool)));
}

//------------------------------------------------------------------------------
//
QEExitButton::~QEExitButton() { }   // place holder

//------------------------------------------------------------------------------
//
QSize QEExitButton::sizeHint () const
{
   return QSize (60, 25);
}

//------------------------------------------------------------------------------
//
void QEExitButton::setExitProgram (const bool exitProgram)
{
   this->mExitProgram = exitProgram;

   QFont font = this->font();
   if (this->mExitProgram) {
      this->setStyleSheet (QEUtilities::colourToStyle (QColor(0xff0000)));
      font.setBold (true);
      this->setToolTip (" This will close the whole application ");
   } else {
      font.setBold (false);
      this->setToolTip (" This will close the current window ");
   }
   this->setFont (font);
}

//------------------------------------------------------------------------------
//
bool QEExitButton::getExitProgram () const
{
   return this->mExitProgram;
}

//------------------------------------------------------------------------------
//
void QEExitButton::onClick (bool)
{
   if (this->mExitProgram) {
      QCoreApplication::exit (0);
   } else {
      // Find enclose-ing main form
      //
      QMainWindow* mainWindow = QEUtilities::findMainWindow (this);
      if (mainWindow) {
         mainWindow->close();
      }
   }
}

// end
