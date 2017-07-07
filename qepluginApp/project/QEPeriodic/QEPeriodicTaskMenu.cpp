/*
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
 *  Copyright (c) 2011 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */


#include "QEPeriodic.h"
#include "PeriodicSetupDialog.h"
#include "QEPeriodicTaskMenu.h"

QEPeriodicTaskMenu::QEPeriodicTaskMenu(QEPeriodic *periodicIn, QObject *parent)
    : QObject(parent)
{
    periodic = periodicIn;

    editUserInfoAction = new QAction(tr("Edit User Info..."), this);
    connect(editUserInfoAction, SIGNAL(triggered()), this, SLOT(editUserInfo()));
}

void QEPeriodicTaskMenu::editUserInfo()
{
    PeriodicSetupDialog dialog(periodic);
    dialog.exec();
}

QAction *QEPeriodicTaskMenu::preferredEditAction() const
{
    return editUserInfoAction;
}

QList<QAction *> QEPeriodicTaskMenu::taskActions() const
{
    QList<QAction *> list;
    list.append(editUserInfoAction);
    return list;
}

QEPeriodicTaskMenuFactory::QEPeriodicTaskMenuFactory(QExtensionManager *parent)
    : QExtensionFactory(parent)
{
}

QObject *QEPeriodicTaskMenuFactory::createExtension(QObject *object,
                                                   const QString &iid,
                                                   QObject *parent) const
{
    if (iid != Q_TYPEID(QDesignerTaskMenuExtension))
        return 0;

    if (QEPeriodic *pp = qobject_cast<QEPeriodic*>(object))
        return new QEPeriodicTaskMenu(pp, parent);

    return 0;
}
