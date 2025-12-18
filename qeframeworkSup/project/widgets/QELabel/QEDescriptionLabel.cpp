/*  QEDescriptionLabel.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2018-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEDescriptionLabel.h"
#include <QDebug>
#include <QFont>
#include <QECommon.h>

#define DEBUG  qDebug () << "QEDescriptionLabel" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QEDescriptionLabel::QEDescriptionLabel (QWidget* parent) :
   QELabel (parent)
{
   this->descrptionLabelSetup ();
}

//------------------------------------------------------------------------------
//
QEDescriptionLabel::QEDescriptionLabel (const QString &variableName, QWidget* parent) :
    QELabel (variableName, parent)
{
   this->descrptionLabelSetup ();
}

//------------------------------------------------------------------------------
//
QEDescriptionLabel::~QEDescriptionLabel () { }

//------------------------------------------------------------------------------
//
void QEDescriptionLabel::descrptionLabelSetup ()
{
   this->setStyleSheet("");
   this->setDefaultStyle("");
   this->setIndent (-1);
   this->setDisplayAlarmStateOption (QE::Never);
   this->m_enableDots = false;

   QFont tempFont;
   tempFont = this->font();
   tempFont.setPointSize (8);
   this->setFont (tempFont);

   QObject::connect (this, SIGNAL (dbValueChanged (const QString&)),
                     this, SLOT   (dbValueChanged (const QString&)));
}

//------------------------------------------------------------------------------
//
void QEDescriptionLabel::dbValueChanged (const QString& value)
{
   if (this->m_enableDots) {
      // This over-writes what has already been set.
      //
      QEUtilities::setPaddedText (this, value);
   }
}

//------------------------------------------------------------------------------
//
void QEDescriptionLabel::setEnableDots (const bool enable)
{
   if (this->m_enableDots != enable) {
      this->m_enableDots = enable;
      // ensure label text is updated with or without pad.
      emit requestResend();
   }
}

//------------------------------------------------------------------------------
//
bool QEDescriptionLabel::getEnableDots() const
{
   return this->m_enableDots;
}

// end

