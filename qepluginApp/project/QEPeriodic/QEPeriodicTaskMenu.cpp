/*  QEPeriodicTaskMenu.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2011-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
