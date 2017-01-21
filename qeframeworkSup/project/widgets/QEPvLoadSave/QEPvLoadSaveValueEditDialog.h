/*  QEPvLoadSaveValueEditDialog.h
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
 */

#ifndef QEPV_LOAD_SAVE_VALUE_EDIT_DIALOG_H
#define QEPV_LOAD_SAVE_VALUE_EDIT_DIALOG_H

#include <QString>
#include <QVariant>

#include <QEDialog.h>

namespace Ui {
   class QEPvLoadSaveValueEditDialog;
}

/*
 * Manager class for the QEPvLoadSaveValueEditDialog.ui compiled form.
 */
class QEPvLoadSaveValueEditDialog : public QEDialog
{
   Q_OBJECT

public:
   explicit QEPvLoadSaveValueEditDialog (QWidget *parent = 0);
   ~QEPvLoadSaveValueEditDialog ();

   void setPvName (const QString& pvName);

   void setValue (const QVariant& valueList);

   QVariant getValue () const;

private:
   void captureText ();      // copy edit widget text into selected valueList element.
   void outputText ();       // copy selected valueList element to edit widget text.

   QVariantList valueList;   // we work with a list, even for scaler values.
   int currentIndex;
   Ui::QEPvLoadSaveValueEditDialog *ui;

private slots:
   void elementIndexChanged (int newIndex);
   void numberElementsChanged (int numberOfElements);
   void on_buttonBox_rejected ();
   void on_buttonBox_accepted ();
};

#endif  // QEPV_LOAD_SAVE_VALUE_EDIT_DIALOG_H
