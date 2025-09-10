/*  QEDescriptionLabel.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2018-2025 Australian Synchrotron
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

