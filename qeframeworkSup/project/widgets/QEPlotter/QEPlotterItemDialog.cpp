/*  QEPlotterItemDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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
 *  Copyright (c) 2013 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#include <QDebug>

#include <QEPlotterItemDialog.h>
#include <ui_QEPlotterItemDialog.h>


//------------------------------------------------------------------------------
//
QEPlotterItemDialog::QEPlotterItemDialog (QWidget *parent) :
      QEDialog (parent),
      ui (new Ui::QEPlotterItemDialog)
{
   this->ui->setupUi (this);

   this->returnIsMasked = false;

   QObject::connect (this->ui->clearButton, SIGNAL (clicked            (bool)),
                     this,                  SLOT   (clearButtonClicked (bool)));

   QObject::connect (this->ui->dataEdit,  SIGNAL (returnPressed ()),
                     this,                SLOT   (dataEditReturnPressed ()));

   QObject::connect (this->ui->aliasEdit, SIGNAL (returnPressed ()),
                     this,                SLOT   (aliasEditReturnPressed ()));

   QObject::connect (this->ui->sizeEdit,  SIGNAL (returnPressed ()),
                     this,                SLOT   (sizeEditReturnPressed ()));
}

//------------------------------------------------------------------------------
//
QEPlotterItemDialog::~QEPlotterItemDialog ()
{
   delete ui;
}


//------------------------------------------------------------------------------
//
void QEPlotterItemDialog::setFieldInformation (const QString dataIn,
                                               const QString aliasIn,
                                               const QString sizeIn)
{
   this->ui->dataEdit->setText (dataIn);
   this->ui->aliasEdit->setText (aliasIn);
   this->ui->sizeEdit->setText (sizeIn);
   this->ui->dataEdit->setFocus ();
}

//------------------------------------------------------------------------------
//
void QEPlotterItemDialog::getFieldInformation (QString& dataOut,
                                               QString& aliasOut,
                                               QString& sizeOut)
{
   dataOut =  this->ui->dataEdit->text ().trimmed();
   aliasOut = this->ui->aliasEdit->text ().trimmed() ;
   sizeOut =  this->ui->sizeEdit->text ().trimmed();
}

//------------------------------------------------------------------------------
//
void QEPlotterItemDialog::dataEditReturnPressed ()
{
   this->returnIsMasked = true;
   this->ui->aliasEdit->setFocus ();
}

//------------------------------------------------------------------------------
//
void QEPlotterItemDialog::aliasEditReturnPressed ()
{
   this->returnIsMasked = true;
   this->ui->sizeEdit->setFocus ();
}

//------------------------------------------------------------------------------
//
void QEPlotterItemDialog::sizeEditReturnPressed ()
{
   // place holder
}

//------------------------------------------------------------------------------
// User has pressed Clear
//
void QEPlotterItemDialog::clearButtonClicked (bool)
{
   this->ui->dataEdit->clear ();
   this->ui->aliasEdit->clear ();
   this->ui->sizeEdit->clear ();
   this->accept ();
}

//------------------------------------------------------------------------------
// User has pressed Colour
//
void QEPlotterItemDialog::colourButtonClicked (bool)
{
//   this->ui->pvNameEdit->clear ();
}

//------------------------------------------------------------------------------
// User has pressed OK
//
void QEPlotterItemDialog::on_buttonBox_accepted ()
{
   if (this->returnIsMasked) {
      this->returnIsMasked = false;
      return;
   }

   this->accept ();
}

//------------------------------------------------------------------------------
// User has pressed Cancel
//
void QEPlotterItemDialog::on_buttonBox_rejected ()
{
   this->close ();
}

// end
