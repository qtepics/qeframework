/*  QEPvLoadSaveCompare.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2015-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include <QDebug>
#include <QECommon.h>

#include "QEPvLoadSave.h"
#include "QEPvLoadSaveModel.h"
#include "QEPvLoadSaveCompare.h"

#include <ui_QEPvLoadSaveCompare.h>

#define DEBUG  qDebug () << "QEPvLoadSaveCompare" << __LINE__ << __FUNCTION__

//------------------------------------------------------------------------------
//
QEPvLoadSaveCompare::QEPvLoadSaveCompare (QEPvLoadSave* ownerIn,
                                          const int sideIn,
                                          QWidget* parent) :
   QWidget (parent),
   owner (ownerIn),
   side (sideIn),
   ui (new Ui::QEPvLoadSaveCompare)
{
   this->ui->setupUi (this);

   // Direct messages to owning form.
   //
   this->setFormId (this->owner->getFormId ());

   // Create the value pair list.
   //
   this->valuePairList = new ValuePairLists ();

   if (this->side < 0 || this->side >= 2) {
      qDebug () << "QEPvLoadSaveCompare constructor: side" << this->side << "out pf range";
      return;
   }

   QObject::connect (this->ui->comparison, SIGNAL (mouseIndexChanged     (const int)),
                     this,                 SLOT   (mouseIndexChangedSlot (const int)));

   // Initialse scale based on slider setting chossen at design time.
   //
   this->scale = EXP10 (this->ui->scale_slider->getValue ());

   // Do the plumbing.
   //
   QObject::connect (this->ui->
                     scale_slider, SIGNAL (valueChanged (const double)),
                     this,         SLOT   (valueChanged (const double)));
}

//------------------------------------------------------------------------------
//
QEPvLoadSaveCompare::~QEPvLoadSaveCompare ()
{
   this->valuePairList->clear ();
   delete this->valuePairList;
   delete this->ui;
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveCompare::processModelData ()
{
   const int otherSide = 1 - this->side;

   const QEPvLoadSaveModel* aModel = this->owner->half [this->side]->model;
   const QEPvLoadSaveModel* bModel = this->owner->half [otherSide]->model;

   const QEPvLoadSaveCommon::PvNameValueMaps aMap = aModel->getPvNameValueMap ();
   const QEPvLoadSaveCommon::PvNameValueMaps bMap = bModel->getPvNameValueMap ();

   this->valuePairList->clear ();

   QList<QString> aKeys = aMap.keys ();
   int n = aKeys.count();

   for (int j = 0; j < n; j++) {
      QString pvName = aKeys.value (j, "");
      if (!pvName.isEmpty () && bMap.contains (pvName)) {
         // Item is in both maps.
         //
         ValuePairs vp;

         vp.name = pvName;
         vp.a = aMap.value (pvName);
         vp.b = bMap.value (pvName);
         vp.dB = this->safe_dB (vp.a, vp.b);

         this->valuePairList->append (vp);
      }
   }

   this->updateHistogram ();
}

//------------------------------------------------------------------------------
//
double QEPvLoadSaveCompare::dB (const double x) const
{
   return 10.0 * LOG10 (x);
}

//------------------------------------------------------------------------------
//
double QEPvLoadSaveCompare::safe_dB (const double x,
                                     const double y) const
{
   const double infinity = 999.99;
   double result;

   if (x == y) {
      // Also covers case (x = 0.0) and (x = 0.0)
      //
      result = 0.0;

   } else if (x > 0.0 && y > 0.0) {
      result = this->dB (y/x);

   } else if (x < 0.0 && y < 0.0) {
      result = this->dB (x/y);

   } else if (x <= 0.0 && y >= 0.0) {
      result = +infinity;

   } else if (x >= 0.0 && y <= 0.0) {
      result = -infinity;

   } else {
      DEBUG << "Program logic error, please refer to developer";
      result = 0.0;
   }

   return result;
}

//------------------------------------------------------------------------------
//
QColor QEPvLoadSaveCompare::calculateColour (const double q) const
{
   QColor result;
   double aq;

   aq = ABS (q) / this->scale;  // Normalise

   if (aq >= 1.0) {
      result = QColor ("#ff00ff");   // purple

   } else if (aq >= 0.6) {
      result = QColor ("#ff0000");   // red

   } else if (aq >= 0.4) {
      result = QColor ("#ff8000");   // orange

   } else if (aq >= 0.2) {
      result = QColor ("#ffff00");   // yellow

   } else if (aq > 0.0) {
      result = QColor ("#4080ff");   // light blue

   } else {
      result = QColor ("#00c000");   // green
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveCompare::updateHistogram ()
{
   static const double minScale = 1.0e-15;
   static const double maxScale = 1.0e+3;

   // Ensure scale is sensible.
   //
   if (this->scale < minScale) this->scale = minScale;
   if (this->scale > maxScale) this->scale = maxScale;

   const double minDisplay = this->scale / 25.0;

   QEHistogram* hgram = this->ui->comparison;  // create alias.
   hgram->clear();     // clear any old data
   hgram->setMinimum (-this->scale);
   hgram->setMaximum (+this->scale);
   // cannot set major/minor coords (yet).
   // cannot set digits (yet).

   const int n = this->valuePairList->count ();
   for (int j = 0; j < n; j++) {
      const ValuePairs vp = this->valuePairList->value (j);
      double q = vp.dB;
      const QColor c = this->calculateColour (q);

      // Even when really small (or zero), ensure we display
      // something to hover over.
      //
      if (ABS (q) <= minDisplay) {
         q = (q >= 0.0) ? +minDisplay : -minDisplay;
      }

      hgram->setValue (j, q);
      hgram->setColour (j, c);
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveCompare::valueChanged (const double value)
{
   this->scale = EXP10 (value);
   this->updateHistogram ();
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveCompare::updateReadout (const int index)
{
   QString text = "";
   if (index >= 0 && index < this->valuePairList->count ()) {
      const ValuePairs vp = this->valuePairList->value (index);
      text = QString ("%1 %2 dB (%3 to %4)").
             arg (vp.name).arg (vp.dB).arg (vp.a).arg (vp.b);
   }

   message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STATUS);
   this->sendMessage (text, mt);

   this->setToolTip (text);
}

//------------------------------------------------------------------------------
//
void QEPvLoadSaveCompare::mouseIndexChangedSlot (const int index)
{
   this->updateReadout (index);
}

// end
