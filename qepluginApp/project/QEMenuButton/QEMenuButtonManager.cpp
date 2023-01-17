/*  QEMenuButtonManager.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2014-2022 Australian Synchrotron.
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
 *    andrew.starritt@synchrotron.org.au
 */


// Ensure uint64_t is available for all compilers
//
#ifdef __cplusplus
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
#endif

#include <QtDesigner>
#include <QtPlugin>

#include <QEMenuButtonManager.h>
#include <QEMenuButton.h>
#include <QEDesignerPluginGroupNames.h>

#include <QEMenuButtonSetupDialog.h>

//==============================================================================
// QEMenuButtonManager
//==============================================================================
//
QEMenuButtonManager::QEMenuButtonManager (QObject * parent) : QObject (parent)
{
   initialized = false;
}

//------------------------------------------------------------------------------
//
void QEMenuButtonManager::initialize (QDesignerFormEditorInterface* formEditor)
{
   if (initialized) {
      return;
   }

   QExtensionManager* manager = formEditor->extensionManager ();
   Q_ASSERT (manager != 0);

   manager->registerExtensions (new QEMenuButtonTaskMenuFactory (manager),
                                Q_TYPEID (QDesignerTaskMenuExtension));

   initialized = true;
}

//------------------------------------------------------------------------------
//
bool QEMenuButtonManager::isInitialized () const
{
   return initialized;
}

//------------------------------------------------------------------------------
// Widget factory. Creates a QEMenuButton widget.
//
QWidget *QEMenuButtonManager::createWidget (QWidget * parent)
{
   return new QEMenuButton (parent);
}

//------------------------------------------------------------------------------
// Name for widget. Used by Qt Designer in widget list.
//
QString QEMenuButtonManager::name () const
{
   return "QEMenuButton";
}

//------------------------------------------------------------------------------
// Name of group Qt Designer will add widget to.
//
QString QEMenuButtonManager::group () const
{
   return QESUPPORT;    // make a control??
}

//------------------------------------------------------------------------------
// Icon for widget. Used by Qt Designer in widget list.
//
QIcon QEMenuButtonManager::icon () const
{
   return QIcon (":/qe/plugin/QEMenuButton.png");
}

//------------------------------------------------------------------------------
// Tool tip for widget. Used by Qt Designer in widget list.
//
QString QEMenuButtonManager::toolTip () const
{
   return "EPICS Menu Button";
}

//------------------------------------------------------------------------------
//
QString QEMenuButtonManager::whatsThis () const
{
   return "EPICS Menu Button.";
}

//------------------------------------------------------------------------------
//
bool QEMenuButtonManager::isContainer () const
{
   return false;
}

//------------------------------------------------------------------------------
//
QString QEMenuButtonManager::includeFile () const
{
   return "QEMenuButton.h";
}


//==============================================================================
// QEMenuButtonTaskMenu
//==============================================================================
//
QEMenuButtonTaskMenu::QEMenuButtonTaskMenu (QEMenuButton* menuButtonIn, QObject* parent)
   : QObject(parent)
{
   this->menuButton = menuButtonIn;

   this->editUserInfoAction = new QAction (tr ("Edit Menu Info..."), this);

   connect (this->editUserInfoAction, SIGNAL (triggered ()),
            this,                     SLOT   (editUserInfo ()));
}

//------------------------------------------------------------------------------
//
void QEMenuButtonTaskMenu::editUserInfo ()
{
   // Note: the use of the menu button widget's parent as the dialogs parent.
   //
   QEMenuButtonSetupDialog dialog (this->menuButton,
                                   dynamic_cast <QWidget*> (this->menuButton->parent ()));
   dialog.exec ();
}

//------------------------------------------------------------------------------
//
QAction *QEMenuButtonTaskMenu::preferredEditAction () const
{
   return this->editUserInfoAction;
}

//------------------------------------------------------------------------------
//
QList<QAction *> QEMenuButtonTaskMenu::taskActions () const
{
   QList<QAction *> list;

   list.append (this->editUserInfoAction);
   return list;
}


//==============================================================================
// QEMenuButtonTaskMenuFactory
//==============================================================================
//
QEMenuButtonTaskMenuFactory::QEMenuButtonTaskMenuFactory (QExtensionManager *parent)
   : QExtensionFactory (parent)
{
}

//------------------------------------------------------------------------------
//
QObject* QEMenuButtonTaskMenuFactory::createExtension (QObject* object,
                                                       const QString& iid,
                                                       QObject* parent) const
{
   if (iid != Q_TYPEID (QDesignerTaskMenuExtension)) {
      return 0;
   }

   if (QEMenuButton *pp = qobject_cast<QEMenuButton*>(object)) {
      return new QEMenuButtonTaskMenu (pp, parent);
   }

   return 0;
}

// end
