/*  QECommentManager.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2025 Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  The EPICS QT Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrews@ansto.gov.au
 */

#ifndef QE_COMMENT_MANAGER_H
#define QE_COMMENT_MANAGER_H

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

class QEComment;

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QECommentManager :
   public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEComment)
};


//------------------------------------------------------------------------------
// The QEComment provides design time extensions
//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QECommentTaskMenu :
   public QObject,
   public QDesignerTaskMenuExtension
{
    Q_OBJECT
    Q_INTERFACES (QDesignerTaskMenuExtension)

public:
    QECommentTaskMenu (QEComment* commentWidget, QObject* parent);

    QAction *preferredEditAction() const;
    QList<QAction *> taskActions() const;

private slots:
    void editUserInfo ();

private:
    QAction* editUserInfoAction;
    QEComment* commentWidget;
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QECommentTaskMenuFactory :
   public QExtensionFactory
{
    Q_OBJECT

public:
    QECommentTaskMenuFactory (QExtensionManager* parent = 0);

protected:
    QObject* createExtension (QObject* object,
                              const QString& iid,
                              QObject* parent) const;
};

#endif     // QE_COMMENT_MANAGER_H
