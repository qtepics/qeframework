/*  QEDelayedText.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2014 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
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
