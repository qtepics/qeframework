/*  QEGroupBox.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2019 Australian Synchrotron
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

#include "QEGroupBox.h"
#include <QDebug>

#define DEBUG qDebug () << "QEGroupBox"  << __LINE__<< __FUNCTION__ << " "

//------------------------------------------------------------------------------
//
QEGroupBox::QEGroupBox (QWidget *parent) :
   QEGroupBox (" QEGroupBox ", parent) {}

//------------------------------------------------------------------------------
//
QEGroupBox::QEGroupBox (const QString& title, QWidget* parent) :
   QGroupBox (title, parent), QEWidget (this)
{
   this->setSubstitutedTitleProperty (title);

   // This is not an EPICS aware widget.
   //
   this->setVariableAsToolTip (false);
   this->setAllowDrop (false);
   this->setNumVariables (0);
}

//------------------------------------------------------------------------------
//
QEGroupBox::~QEGroupBox () { }

//------------------------------------------------------------------------------
//
QSize QEGroupBox::sizeHint () const
{
   return QSize (120, 80);
}


//==============================================================================
// Property convenience functions
// label text (prior to substitution)
//
void QEGroupBox::setSubstitutedTitleProperty (const QString substitutedTitleIn)
{
   this->ownSubstitutedTitle = substitutedTitleIn;

   // Now update the QGroupBox title.
   // Treat "-" as special null value. As actual null string get re-interpreted
   // as the default, i.e. " QEGroupBox "
   //
   const QString ownTitle = this->substituteThis (this->ownSubstitutedTitle);
   this->setTitle (ownTitle == "-" ? "" : ownTitle);
}

//------------------------------------------------------------------------------
//
QString QEGroupBox::getSubstitutedTitleProperty() const
{
   return this->ownSubstitutedTitle;
}

//------------------------------------------------------------------------------
// title text substitutions
void QEGroupBox::setSubstitutionsProperty(const QString macroSubstitutionsIn)
{
   // Set the substitutions
   //
   this->setVariableNameSubstitutions (macroSubstitutionsIn);

   // Treat "-" as special null value. As actual null string get re-interpreted
   // as the default, i.e. " QEGroupBox "
   //
   const QString ownTitle = this->substituteThis (this->ownSubstitutedTitle);
   this->setTitle (ownTitle == "-" ? "" : ownTitle);
}

//------------------------------------------------------------------------------
//
QString QEGroupBox::getSubstitutionsProperty() const
{
   return this->getVariableNameSubstitutions();
}

// end
