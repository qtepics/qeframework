/*  PeriodicElementSetupForm.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2011-2018 Australian Synchrotron
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
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef QE_PERIODIC_ELEMENT_SETUP_FORM_H
#define QE_PERIODIC_ELEMENT_SETUP_FORM_H

#include <QWidget>
#include <QEPluginLibrary_global.h>

namespace Ui {
    class PeriodicElementSetupForm;
}

class userInfoStruct;

class QEPLUGINLIBRARYSHARED_EXPORT PeriodicElementSetupForm : public QWidget
{
    Q_OBJECT

public:
    explicit PeriodicElementSetupForm(const int element,
                                      userInfoStruct* userInfo,
                                      QWidget *parent = 0);
    ~PeriodicElementSetupForm();

private:
    Ui::PeriodicElementSetupForm *ui;
};

#endif // QE_PERIODIC_ELEMENT_SETUP_FORM_H
