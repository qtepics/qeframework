/*  QEExitButton.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2025 Australian Synchrotron
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
 *    andrews@ansto.gov.au
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
