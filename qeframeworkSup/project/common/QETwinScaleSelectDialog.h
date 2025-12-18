/*  QETwinScaleSelectDialog.h
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

#ifndef QE_TWIN_SCALE_SELECT_DIALOG_H
#define QE_TWIN_SCALE_SELECT_DIALOG_H

#include <QWidget>
#include <QPointF>
#include <QEDialog.h>
#include <QEFrameworkLibraryGlobal.h>

namespace Ui {
   class QE_TWin_Scale_Select_Dialog;   // differed
}

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QETwinScaleSelectDialog : public QEDialog {
   Q_OBJECT
public:
   // Define scale min and max
   // min - minumum scale value
   // max - maximum scale value
   //
   typedef struct {
      double min;
      double max;
   } ScaleLimit;

   explicit QETwinScaleSelectDialog (const QString& windowTitle,
                                     const QString& scaleOneName,
                                     const QString& scaleTwoName,
                                     QWidget* parent = 0);
   ~QETwinScaleSelectDialog ();

   void setActiveMap (const ScaleLimit& scaleOne, const ScaleLimit& scaleTwo);
   void getActiveMap (ScaleLimit& scaleOne, ScaleLimit& scaleTwo);

private:
   Ui::QE_TWin_Scale_Select_Dialog* ui;

   ScaleLimit scaleOne;
   ScaleLimit scaleTwo;

   bool returnIsMasked;

private slots:
   void scaleOneMinReturnPressed ();
   void scaleOneMaxReturnPressed ();
   void scaleTwoMinReturnPressed ();
   void scaleTwoMaxReturnPressed ();

   // The connection to these slots are made by QDialog and associates.
   //
   void on_buttonBox_rejected ();
   void on_buttonBox_accepted ();
};

#endif // QE_TWIN_SCALE_SELECT_DIALOG_H
