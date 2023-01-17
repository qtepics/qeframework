/*  QEStripChartAdjustPVDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2022 Australian Synchrotron
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
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 *
 */

#include "QEStripChartAdjustPVDialog.h"
#include <QVariant>
#include <ui_QEStripChartAdjustPVDialog.h>
#include <QDebug>

//------------------------------------------------------------------------------
//
QEStripChartAdjustPVDialog::QEStripChartAdjustPVDialog (QWidget *parent) :
      QEDialog (parent),
      ui (new Ui::QEStripChartAdjustPVDialog)
{
   this->ui->setupUi (this);

   this->chartMinimum = 0.0;
   this->chartMaximum = 100.0;
   this->returnIsMasked = false;

   // Connect user has hit return iv each edit widget.
   //
   QObject::connect (this->ui->originEdit, SIGNAL  (returnPressed ()),
                     this,                 SLOT (originReturnPressed ()));

   QObject::connect (this->ui->slopeEdit,  SIGNAL  (returnPressed ()),
                     this,                 SLOT (slopeReturnPressed ()));

   QObject::connect (this->ui->offsetEdit, SIGNAL  (returnPressed ()),
                     this,                 SLOT (offsetReturnPressed ()));


   // Connect push buttons.
   //
   QObject::connect (this->ui->resetButton,    SIGNAL (clicked            (bool)),
                     this,                     SLOT   (resetButtonClicked (bool)));

   QObject::connect (this->ui->loprHoprButton, SIGNAL (clicked              (bool)),
                     this,                     SLOT   (loprHoprButtonClicked (bool)));

   QObject::connect (this->ui->plottedButton,  SIGNAL (clicked              (bool)),
                     this,                     SLOT   (plottedButtonClicked (bool)));

   QObject::connect (this->ui->bufferedButton, SIGNAL (clicked               (bool)),
                     this,                     SLOT   (bufferedButtonClicked (bool)));

   QObject::connect (this->ui->lowerButton,    SIGNAL (clicked             (bool)),
                     this,                     SLOT   (lowerButtonClicked  (bool)));

   QObject::connect (this->ui->centreButton,   SIGNAL (clicked             (bool)),
                     this,                     SLOT   (centreButtonClicked (bool)));

   QObject::connect (this->ui->upperButton,    SIGNAL (clicked             (bool)),
                     this,                     SLOT   (upperButtonClicked  (bool)));
}

//------------------------------------------------------------------------------
//
QEStripChartAdjustPVDialog::~QEStripChartAdjustPVDialog ()
{
   delete ui;
}

//------------------------------------------------------------------------------
//
void QEStripChartAdjustPVDialog::setValueScaling (const ValueScaling & valueScaleIn)
{
   double d, m, c;
   QString text;

   this->valueScale.assign (valueScaleIn);

   this->valueScale.get (d, m, c);

   text = QString::asprintf (" %g", d);
   this->ui->originEdit->setText (text);

   text = QString::asprintf (" %g", m);
   this->ui->slopeEdit->setText (text);

   text = QString::asprintf (" %g", c);
   this->ui->offsetEdit->setText (text);
}

//------------------------------------------------------------------------------
//
ValueScaling QEStripChartAdjustPVDialog::getValueScaling () const
{
   return this->valueScale;
}

//------------------------------------------------------------------------------
//
void QEStripChartAdjustPVDialog::setSupport (const double minIn, const double maxIn,
                                             const QEDisplayRanges& loprHoprIn,
                                             const QEDisplayRanges& plottedIn,
                                             const QEDisplayRanges& buffereInd)
{
   // Current chart display range.
   //
   this->chartMinimum = minIn;
   this->chartMaximum = maxIn;

   // Various ranges of this PV.
   //
   this->loprHopr = loprHoprIn;
   this->plotted = plottedIn;
   this->buffered = buffereInd;
}


//------------------------------------------------------------------------------
//
void QEStripChartAdjustPVDialog::useSelectedRange (const QEDisplayRanges& selectedRange)
{
   ValueScaling preset;
   bool status;
   double min, max;

   status = selectedRange.getMinMax (min, max);
   if (status) {
      preset.map (min, max, this->chartMinimum, this->chartMaximum);
      this->setValueScaling (preset);
   }
}

//------------------------------------------------------------------------------
//
void QEStripChartAdjustPVDialog::originReturnPressed ()
{
   this->returnIsMasked = true;
   this->ui->slopeEdit->setFocus ();
}

//------------------------------------------------------------------------------
//
void QEStripChartAdjustPVDialog::slopeReturnPressed ()
{
   this->returnIsMasked = true;
   this->ui->offsetEdit->setFocus ();
}

//------------------------------------------------------------------------------
//
void QEStripChartAdjustPVDialog::offsetReturnPressed ()
{
   this->returnIsMasked = true;
   this->ui->buttonBox->setFocus();
}

//------------------------------------------------------------------------------
// Preset options
//
void QEStripChartAdjustPVDialog::resetButtonClicked (bool)
{
   ValueScaling preset;

   preset.reset ();
   this->setValueScaling (preset);
}

//------------------------------------------------------------------------------
//
void QEStripChartAdjustPVDialog::loprHoprButtonClicked (bool)
{
   this->useSelectedRange (this->loprHopr);
}

//------------------------------------------------------------------------------
//
void QEStripChartAdjustPVDialog::plottedButtonClicked (bool)
{
   this->useSelectedRange (this->plotted);
}

//------------------------------------------------------------------------------
//
void QEStripChartAdjustPVDialog::bufferedButtonClicked (bool)
{
   this->useSelectedRange (this->buffered);
}

//------------------------------------------------------------------------------
//
void QEStripChartAdjustPVDialog::lowerButtonClicked (bool)
{
   double min;
   double max;
   bool okay = this->plotted.getMinMax (min, max);
   double delta = max - min;

   // useSelectedRange map to chartMinMax, so we compensate.
   //
   QEDisplayRanges modified (min, max + 2.0*delta);
   this->useSelectedRange (modified);
}

//------------------------------------------------------------------------------
//
void QEStripChartAdjustPVDialog::centreButtonClicked (bool)
{
   double min;
   double max;
   bool okay = this->plotted.getMinMax (min, max);
   double delta = max - min;

   // useSelectedRange map to chartMinMax, so we compensate.
   //
   QEDisplayRanges modified (min - delta, max + delta);
   this->useSelectedRange (modified);
}

//------------------------------------------------------------------------------
//
void QEStripChartAdjustPVDialog::upperButtonClicked (bool)
{
   double min;
   double max;
   bool okay = this->plotted.getMinMax (min, max);
   double delta = max - min;

   // useSelectedRange map to chartMinMax, so we compensate.
   //
   QEDisplayRanges modified (min - 2.0*delta, max);
   this->useSelectedRange (modified);
}

//------------------------------------------------------------------------------
// User has pressed OK
//
void QEStripChartAdjustPVDialog::on_buttonBox_accepted ()
{
   QVariant varOrigin (QVariant::String);
   QVariant varSlope (QVariant::String);
   QVariant varOffset (QVariant::String);
   double d, m, c;

   bool ok1, ok2, ok3;

   if (this->returnIsMasked) {
      this->returnIsMasked = false;
      return;
   }

   // Extract and validate user entry.
   //
   varOrigin = this->ui->originEdit->text ();
   d = varOrigin.toDouble (&ok1);

   varSlope = this->ui->slopeEdit->text ();
   m = varSlope.toDouble (&ok2);

   varSlope = this->ui->offsetEdit->text ();
   c = varSlope.toDouble (&ok3);

   if (ok1 && ok2 && ok3) {
      // All okay - assign values to object.
      //
      this->valueScale.set (d, m, c);

      // Proceed with 'good' dialog exit
      //
      QDialog::accept();
   }
}

//------------------------------------------------------------------------------
// User has pressed cancel
//
void QEStripChartAdjustPVDialog::on_buttonBox_rejected ()
{
   this->close ();
}

// end
