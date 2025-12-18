/*  QEGroupBox.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEGroupBox.h"
#include <QDebug>

#define DEBUG qDebug () << "QEGroupBox"  << __LINE__<< __FUNCTION__ << " "

//------------------------------------------------------------------------------
//
QEGroupBox::QEGroupBox (QWidget *parent) :
   QGroupBox (parent), QEWidget (this)
{
   this->setSubstitutedTitleProperty (" QEGroupBox ");

   // This is not an EPICS aware widget.
   //
   this->setVariableAsToolTip (false);
   this->setAllowDrop (false);
   this->setNumVariables (0);
}

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
