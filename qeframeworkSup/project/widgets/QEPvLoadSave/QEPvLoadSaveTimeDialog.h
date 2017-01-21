/*  QEPvLoadSaveTimeDialog.h
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
 *  Copyright (c) 2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_PV_LOAD_SAVE_TIME_DIALOG_H
#define QE_PV_LOAD_SAVE_TIME_DIALOG_H

#include <QString>
#include <QColor>
#include <QEDialog.h>
#include <QDateTime>
#include <QEPluginLibrary_global.h>

namespace Ui {
    class QEPvLoadSaveTimeDialog;
}

/*
 * Manager class for the QEPvLoadSaveTimeDialog.ui compiled form.
 *
 * This dialog form allows the user to select a datetime.
 */
class QEPLUGINLIBRARYSHARED_EXPORT QEPvLoadSaveTimeDialog : public QEDialog
{
    Q_OBJECT

public:
   explicit QEPvLoadSaveTimeDialog (QWidget* parent = 0);
   ~QEPvLoadSaveTimeDialog ();

   void setMaximumDateTime (const QDateTime& datetime);

   void setDateTime (const QDateTime& datetime);
   QDateTime getDateTime ();

private:
   Ui::QEPvLoadSaveTimeDialog* ui;

   QDateTime savedDateTime;

private slots:
   void dateClicked (const QDate & date);
   void timeChanged (const QTime &time);
   void sliderValueChanged (int value);

   void on_buttonBox_rejected ();
   void on_buttonBox_accepted ();
};

#endif  // QE_PV_LOAD_SAVE_TIME_DIALOG_H
