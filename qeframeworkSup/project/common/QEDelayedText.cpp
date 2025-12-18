/*  QEDelayedText.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2014-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include <QEDelayedText.h>

//------------------------------------------------------------------------------
//
QEDelayedText::QEDelayedText (const double delayIn, QObject* parent) : QObject (parent)
{
   this->delay = delayIn;
   this->text = "";
   this->timer = new QTimer (this);

   this->timer->setSingleShot (true);
   QObject::connect (this->timer, SIGNAL (timeout ()),
                     this,        SLOT   (timeout ()));
}

//------------------------------------------------------------------------------
//
QEDelayedText::~QEDelayedText ()
{
   // place holder
}

//------------------------------------------------------------------------------
//
bool QEDelayedText::doubleConnect (const QObject* sender,   const char* signal,
                                   const QObject* receiver, const char* member) const
{
   bool result;

   result = QObject::connect (sender, signal, this, SLOT (setText (const QString&))) &&
            QObject::connect (this, SIGNAL (textChanged(const QString&)), receiver, member);


   return result;
}

//------------------------------------------------------------------------------
//
void QEDelayedText::setText (const QString& textIn)
{
   this->text = textIn;
   this->timer->start (int (this->delay * 1000.0));  // convert to mSec
}

//------------------------------------------------------------------------------
//
void QEDelayedText::timeout () {
    emit textChanged (this->text);
}

// end
