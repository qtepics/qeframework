/*  QEAbstractWidget.cpp
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

#include "QEAbstractWidget.h"
#include <QDebug>

#define DEBUG qDebug () << "QEAbstractWidget" << __LINE__ << __FUNCTION__  << "  "


//------------------------------------------------------------------------------
//
QEAbstractWidget::QEAbstractWidget (QWidget* parent) :
   QFrame (parent), QEWidget (this)
{
   // The default is as if just a plain widget container.
   //
   this->setFrameShape (QFrame::NoFrame);
   this->setFrameShadow (QFrame::Plain);

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
