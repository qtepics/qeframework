/*  QECommentManager.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2025 Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
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

#include "QECommentManager.h"
#include <QDebug>
#include <QtDesigner>
#include <QtPlugin>
#include <QEComment.h>
#include <QEDesignerPluginGroupNames.h>
#include <QECommentSetupDialog.h>

#define DEBUG  qDebug () << "QECommentManager" << __LINE__ << __FUNCTION__ << "  "


//==============================================================================
// QECommentManager
//==============================================================================
//
QECommentManager::QECommentManager (QObject* parent) : QObject (parent)
{
   this->initialized = false;
}

//------------------------------------------------------------------------------
//
void QECommentManager::initialize (QDesignerFormEditorInterface* formEditor)
{
   if (this->initialized) {
      return;
   }

   QExtensionManager* manager = formEditor->extensionManager ();
   Q_ASSERT (manager != 0);

   manager->registerExtensions (new QECommentTaskMenuFactory (manager),
                                Q_TYPEID (QDesignerTaskMenuExtension));

   this->initialized = true;
}

//------------------------------------------------------------------------------
//
bool QECommentManager::isInitialized () const
{
   return this->initialized;
}

//------------------------------------------------------------------------------
// Widget factory. Creates a QEComment widget.
//
QWidget *QECommentManager::createWidget (QWidget* parent)
{
   return new QEComment (parent);
}

//------------------------------------------------------------------------------
// Name for widget. Used by Qt Designer in widget list.
//
QString QECommentManager::name () const
{
   return "QEComment";
}

//------------------------------------------------------------------------------
// Name of group Qt Designer will add widget to.
//
QString QECommentManager::group () const
{
   return QESUPPORT;
}

//------------------------------------------------------------------------------
// Icon for widget. Used by Qt Designer in widget list.
//
QIcon QECommentManager::icon () const
{
   return QIcon (":/qe/plugin/QEComment.png");
}

//------------------------------------------------------------------------------
// Tool tip for widget. Used by Qt Designer in widget list.
//
QString QECommentManager::toolTip () const
{
   return "View/edit any form comments";
}

//------------------------------------------------------------------------------
//
QString QECommentManager::whatsThis () const
{
   return "Comment";
}

//------------------------------------------------------------------------------
//
bool QECommentManager::isContainer () const
{
   return false;
}

//------------------------------------------------------------------------------
//
QString QECommentManager::includeFile () const
{
   return "QEComment.h";
}


//==============================================================================
// QECommentTaskMenu
//==============================================================================
//
QECommentTaskMenu::QECommentTaskMenu (QEComment* commentWidgetIn, QObject* parent)
   : QObject(parent)
{
   this->commentWidget = commentWidgetIn;

   this->editUserInfoAction = new QAction (tr ("Edit Comment Info..."), this);

   connect (this->editUserInfoAction, SIGNAL (triggered ()),
            this,                     SLOT   (editUserInfo ()));
}

//------------------------------------------------------------------------------
//
void QECommentTaskMenu::editUserInfo ()
{
   // Note: the use of the comment widget's parent as the dialog's parent.
   //
   QECommentSetupDialog dialog (this->commentWidget,
                                dynamic_cast <QWidget*> (this->commentWidget->parent ()));
   dialog.exec ();
}

//------------------------------------------------------------------------------
//
QAction *QECommentTaskMenu::preferredEditAction () const
{
   return this->editUserInfoAction;
}

//------------------------------------------------------------------------------
//
QList<QAction *> QECommentTaskMenu::taskActions () const
{
   QList<QAction *> list;

   list.append (this->editUserInfoAction);
   return list;
}


//==============================================================================
// QECommentTaskMenuFactory
//==============================================================================
//
QECommentTaskMenuFactory::QECommentTaskMenuFactory (QExtensionManager *parent)
   : QExtensionFactory (parent)
{
}

//------------------------------------------------------------------------------
//
QObject* QECommentTaskMenuFactory::createExtension (QObject* object,
                                                    const QString& iid,
                                                    QObject* parent) const
{
   if (iid != Q_TYPEID (QDesignerTaskMenuExtension)) {
      return 0;
   }

   QEComment* comment = qobject_cast<QEComment*>(object);
   if (comment) {
      return new QECommentTaskMenu (comment, parent);
   }

   return 0;
}

// end
