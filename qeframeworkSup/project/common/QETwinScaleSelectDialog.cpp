/*  QETwinScaleSelectDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2017-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andraz Pozar
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QETwinScaleSelectDialog.h"

#include <QDebug>
#include <QECommon.h>
#include <ui_QETwinScaleSelectDialog.h>

#define DEBUG qDebug () << "QETwinScaleSelectDialog" << __LINE__ << __FUNCTION__ << "  "

#define NOMAP   0

//------------------------------------------------------------------------------
//
QETwinScaleSelectDialog::QETwinScaleSelectDialog(
      const QString& windowTitle,
      const QString& scaleOneName,
      const QString& scaleTwoName,
      QWidget* parent) :
   QEDialog (parent)
{
   this->ui = new Ui::QE_TWin_Scale_Select_Dialog ();
   this->ui->setupUi (this);
   this->setWindowTitle(windowTitle);
   this->ui->scaleOneNameLabel->setText(scaleOneName);
   this->ui->scaleTwoNameLabel->setText(scaleTwoName);

   this->returnIsMasked = false;

   // Connect user has hit return iv each edit widget.
   //
   QObject::connect (this->ui->scaleOneMinEdit, SIGNAL   (returnPressed ()),
                     this,                   SLOT (scaleOneMinReturnPressed ()));

   QObject::connect (this->ui->scaleOneMaxEdit, SIGNAL   (returnPressed ()),
                     this,                   SLOT (scaleOneMaxReturnPressed ()));

   QObject::connect (this->ui->scaleTwoMinEdit, SIGNAL   (returnPressed ()),
                     this,                   SLOT (scaleTwoMinReturnPressed ()));

   QObject::connect (this->ui->scaleTwoMaxEdit, SIGNAL   (returnPressed ()),
                     this,                   SLOT (scaleTwoMaxReturnPressed ()));

}

//------------------------------------------------------------------------------
//
QETwinScaleSelectDialog::~QETwinScaleSelectDialog ()
{
}

//------------------------------------------------------------------------------
//
void QETwinScaleSelectDialog::setActiveMap (const ScaleLimit& scaleOne, const ScaleLimit& scaleTwo)
{
   QString text;

   this->scaleOne = scaleOne;
   this->scaleTwo = scaleTwo;

   text = QString::asprintf (" %g", this->scaleOne.min);
   this->ui->scaleOneMinEdit->setText (text);

   text = QString::asprintf (" %g", this->scaleOne.max);
   this->ui->scaleOneMaxEdit->setText (text);

   text = QString::asprintf (" %g", this->scaleTwo.min);
   this->ui->scaleTwoMinEdit->setText (text);

   text = QString::asprintf (" %g", this->scaleTwo.max);
   this->ui->scaleTwoMaxEdit->setText (text);

   this->ui->scaleOneMinEdit->setFocus ();
}

//------------------------------------------------------------------------------
//
void QETwinScaleSelectDialog::getActiveMap (ScaleLimit& scaleOne, ScaleLimit& scaleTwo)
{
   scaleOne = this->scaleOne;
   scaleTwo = this->scaleTwo;
}

//------------------------------------------------------------------------------
//
void QETwinScaleSelectDialog:: scaleOneMinReturnPressed ()
{
   this->returnIsMasked = true;
   this->ui->scaleOneMaxEdit->setFocus ();
}

//------------------------------------------------------------------------------
//
void QETwinScaleSelectDialog:: scaleOneMaxReturnPressed ()
{
   this->returnIsMasked = true;
   this->ui->scaleTwoMinEdit->setFocus ();
}

//------------------------------------------------------------------------------
//
void QETwinScaleSelectDialog:: scaleTwoMinReturnPressed ()
{
   this->returnIsMasked = true;
   this->ui->scaleTwoMaxEdit->setFocus ();
}

//------------------------------------------------------------------------------
//
void QETwinScaleSelectDialog:: scaleTwoMaxReturnPressed ()
{
   this->returnIsMasked = true;
   this->ui->buttonBox->setFocus();
}

//------------------------------------------------------------------------------
// User has pressed cancel
//
void QETwinScaleSelectDialog::on_buttonBox_rejected ()
{
   // Proceed with 'ignore' dialog exit
   //
   QDialog::reject ();
}

//------------------------------------------------------------------------------
// User has pressed OK
//
void QETwinScaleSelectDialog::on_buttonBox_accepted ()
{

   if (this->returnIsMasked) {
      this->returnIsMasked = false;
      return;
   }

   bool  b1, b2, b3, b4;
   double t1, t2, t3, t4;

   t1 = this->ui->scaleOneMinEdit->text().toDouble (&b1);
   t2 = this->ui->scaleOneMaxEdit->text().toDouble (&b2);
   t3 = this->ui->scaleTwoMinEdit->text().toDouble (&b3);
   t4 = this->ui->scaleTwoMaxEdit->text().toDouble (&b4);

   if (b1 && b2 && b3 && b4) {

      // All okay - assign values to object.
      //
      t2 = MAX (t1, t2);
      t4 = MAX (t3, t4);

      this->scaleOne.min = t1;
      this->scaleOne.max = t2;
      this->scaleTwo.min = t3;
      this->scaleTwo.max = t4;

      // Proceed with 'good' dialog exit
      //
      QDialog::accept ();
   }
}

// end
