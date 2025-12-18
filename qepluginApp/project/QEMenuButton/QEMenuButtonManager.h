/*  QEMenuButtonManager.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2014-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_MENU_BUTTON_MANAGER_H
#define QE_MENU_BUTTON_MANAGER_H

#include <QtGlobal>

// The QT_VERSION check is in QEDesignerPluginCommon.h, but does not
// work with moc unless restated here.  Possibly relevant:
// https://stackoverflow.com/questions/4119688/macro-expansion-in-moc
// Provide Qt version independent plugin-related includes.
//
#if (QT_VERSION >= 0x050500)
#include <QtUiPlugin/QDesignerCustomWidgetInterface>
#else
#include <QDesignerCustomWidgetInterface>
#endif

#include <QEDesignerPluginCommon.h>
#include <QDesignerTaskMenuExtension>
#include <QEPluginLibrary_global.h>
#include <QExtensionFactory>

// Differed class declarations
//
QT_BEGIN_NAMESPACE
class QAction;
class QExtensionManager;
QT_END_NAMESPACE

class QEMenuButton;

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEMenuButtonManager :
   public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEMenuButton)
};


//------------------------------------------------------------------------------
// The QEMenuButton provides design time extensions
//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEMenuButtonTaskMenu : public QObject, public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES (QDesignerTaskMenuExtension)

public:
    QEMenuButtonTaskMenu (QEMenuButton* menuButton, QObject* parent);

    QAction *preferredEditAction() const;
    QList<QAction *> taskActions() const;

private slots:
    void editUserInfo ();

private:
    QAction *editUserInfoAction;
    QEMenuButton* menuButton;
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEMenuButtonTaskMenuFactory : public QExtensionFactory
{
    Q_OBJECT

public:
    QEMenuButtonTaskMenuFactory (QExtensionManager* parent = 0);

protected:
    QObject *createExtension (QObject* object,
                              const QString& iid,
                              QObject* parent) const;
};

#endif     // QE_MENU_BUTTON_MANAGER_H
