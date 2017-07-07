/*  QETwinScaleSelectDialog.h
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
 *  Copyright (c) 2017 Australian Synchrotron
 *
 * Author:            Andraz Pozar
 * Contact details:   andraz.pozar@synchrotron.org.au
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
