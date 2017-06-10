/*  QEPvLoadSaveAccessFail.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
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
 *  Copyright (c) 2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_PV_LOAD_SAVE_ACCESS_FAIL_H
#define QE_PV_LOAD_SAVE_ACCESS_FAIL_H

#include <QString>
#include <QStringList>
#include <QWidget>
#include <QEPluginLibrary_global.h>

namespace Ui {
   class QEPvLoadSaveAccessFail;
}

class QEPvLoadSave;   // differed.

/// This widget displays a list of PVs that failed to extracted, applied or read from the archive,
///
class QEPLUGINLIBRARYSHARED_EXPORT QEPvLoadSaveAccessFail : public QWidget
{
   Q_OBJECT
public:
   // constructor.
   explicit QEPvLoadSaveAccessFail (QEPvLoadSave* owner,
                                    QWidget* parent = 0);
   ~QEPvLoadSaveAccessFail ();

   void clear ();
   void addPVName (const QString& pvName);
   void addPVNames (const QStringList& pvNameList);

protected:

private:
   QEPvLoadSave* owner;
   Ui::QEPvLoadSaveAccessFail* ui;


private slots:
};

#endif  // QE_PV_LOAD_SAVE_ACCESS_FAIL_H
