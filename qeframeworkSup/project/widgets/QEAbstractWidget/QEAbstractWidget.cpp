/*  QEAbstractWidget.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2014  Australian Synchrotron
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

#include "QEAbstractWidget.h"
#include <QDebug>

#define DEBUG qDebug () << "QEAbstractWidget" << __LINE__ << __FUNCTION__  << "  "


//------------------------------------------------------------------------------
//
QEAbstractWidget::QEAbstractWidget (QWidget *parent) : QWidget (parent), QEWidget (this)
{
   // This is not an EPICS aware widget.
   //
   this->setVariableAsToolTip (false);
   this->setAllowDrop (false);
   this->setNumVariables (0);
}

//------------------------------------------------------------------------------
//
QEAbstractWidget::~QEAbstractWidget () {
}

//------------------------------------------------------------------------------
//
void QEAbstractWidget::setManagedVisible (bool visibleIn)
{
   this->setRunVisible (visibleIn);
}

// end
