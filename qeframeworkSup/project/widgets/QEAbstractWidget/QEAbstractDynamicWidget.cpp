/*  QEAbstractComplexWidget.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2016-2022 Australian Synchrotron
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
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include "QEAbstractDynamicWidget.h"

#include <QDebug>
#include <QFileDialog>
#include <QECommon.h>
#include <QELabel.h>
#include <persistanceManager.h>
#include <contextMenu.h>

#define DEBUG qDebug() << "QEAbstractComplexWidget" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QEAbstractDynamicWidget::QEAbstractDynamicWidget (QWidget* parent) :
   ParentWidgetClass (parent)
{
   // Set super class properties.
   //
   this->setFrameShape (QFrame::NoFrame);
   this->setFrameShadow (QFrame::Plain);

   this->defaultDir = "";
   this->useOwnPersistantName = false;
   this->enableEditPv = false;
   this->pvLabelMode = QE::useAliasName;

   // Typically QEAbstractDynamicWidget widgets hold other QEWidgets, like 
   // QEDistrubtion, and theses widgets handle this internally.
   // 
   this->setOosAware (false);
   
   // Maybe DisplayAlarmStateOption should be set to Never here.
}

//------------------------------------------------------------------------------
//
QEAbstractDynamicWidget::~QEAbstractDynamicWidget ()
{
   // place holder
}

//------------------------------------------------------------------------------
//
void QEAbstractDynamicWidget::setDefaultDir (const QString& defaultDirIn)
{
   this->defaultDir = defaultDirIn;
}

//------------------------------------------------------------------------------
//
QString QEAbstractDynamicWidget::getDefaultDir () const
{
   return this->defaultDir;
}

//------------------------------------------------------------------------------
// Sub class utility
//
QE::UserLevels QEAbstractDynamicWidget::minimumEditPvUserLevel () const
{
   return this->getEnableEditPv () ? QE::User : QE::Engineer;
}

//------------------------------------------------------------------------------
// Sub class utility
//
void QEAbstractDynamicWidget::setStandardFormat (QELabel* dataLabel)
{
   if (!dataLabel) return;   // sanity check

   dataLabel->setPrecision (6);
   dataLabel->setForceSign (true);
   dataLabel->setUseDbPrecision (false);
   dataLabel->setNotation (QE::Automatic);
   dataLabel->setSeparator (QE::Comma);
   dataLabel->setArrayAction (QE::Index);
   dataLabel->setArrayIndex (0);
}

//------------------------------------------------------------------------------
// virtual - place holder
//
void QEAbstractDynamicWidget::pvLabelModeChanged () { }

//------------------------------------------------------------------------------
// virtual - place holder
//
void QEAbstractDynamicWidget::enableEditPvChanged () { }

//------------------------------------------------------------------------------
//
void QEAbstractDynamicWidget::setEnableEditPv (const bool isEnabled)
{
   this->enableEditPv = isEnabled;

   // Notify sub-class
   this->enableEditPvChanged ();
}

//------------------------------------------------------------------------------
//
bool QEAbstractDynamicWidget::getEnableEditPv () const
{
   return this->enableEditPv;
}

//------------------------------------------------------------------------------
//
void QEAbstractDynamicWidget::setPVLabelMode (const QE::PVLabelMode pvLabelModeIn)
{
   this->pvLabelMode = pvLabelModeIn;
   this->pvLabelModeChanged ();
}

//------------------------------------------------------------------------------
//
QE::PVLabelMode QEAbstractDynamicWidget::getPVLabelMode () const
{
   return this->pvLabelMode;
}

//------------------------------------------------------------------------------
//
void QEAbstractDynamicWidget::addPvNameList (const QStringList& pvNameList)
{
   for (int j = 0; j < pvNameList.count (); j++) {
      this->addPvName (pvNameList.value (j));
   }
}

//------------------------------------------------------------------------------
//
void QEAbstractDynamicWidget::addPvNameSet (const QString& pvNameSet)
{
   QStringList pvNameList;

   // Split input string using white space as delimiter.
   //
   pvNameList = QEUtilities::split (pvNameSet);
   this->addPvNameList (pvNameList);
}

//------------------------------------------------------------------------------
//
void QEAbstractDynamicWidget::paste (QVariant s)
{
   QStringList pvNameList;

   // s.toSring is a bit limiting when s is a StringList or a List of String, so
   // use common variantToStringList function which handles tese options.
   //
   pvNameList = QEUtilities::variantToStringList (s);
   this->addPvNameList (pvNameList);

   this->setFocus ();
}

//------------------------------------------------------------------------------
//
QMenu* QEAbstractDynamicWidget::buildContextMenu ()
{
   QMenu* menu = ParentWidgetClass::buildContextMenu ();
   QAction* action;

   menu->addSeparator ();

   action = new QAction (tr("Load Configuration"), menu);
   action->setCheckable (false);
   action->setData (ADWCM_LOAD_WIDGET_CONFIG);
   menu->addAction (action);

   action = new QAction (tr("Save Configuration"), menu);
   action->setCheckable (false);
   action->setData (ADWCM_SAVE_WIDGET_CONFIG);
   menu->addAction (action);

   // We don't always build in PVLabelMode selection.
   // Sub classes must call conveniance function below.

   return menu;
}

//------------------------------------------------------------------------------
//
void QEAbstractDynamicWidget::addPVLabelModeContextMenu (QMenu* menu)
{
   QAction* action;

   menu->addSeparator ();

   action = new QAction (tr("Use PV Names"), menu);
   action->setCheckable (true);
   action->setChecked (this->pvLabelMode == QE::usePvName);
   action->setData (ADWCM_SELECT_USE_PV_NAME);
   menu->addAction (action);

   action = new QAction (tr("Use Alias Names (if available)"), menu);
   action->setCheckable (true);
   action->setChecked (this->pvLabelMode == QE::useAliasName);
   action->setData (ADWCM_SELECT_USE_ALIAS_NAME);
   menu->addAction (action);

   action = new QAction (tr("Use Descriptions (if available)"), menu);
   action->setCheckable (true);
   action->setChecked (this->pvLabelMode == QE::useDescription);
   action->setData (ADWCM_SELECT_USE_DESCRIPTION);
   menu->addAction (action);
}

//------------------------------------------------------------------------------
//
void QEAbstractDynamicWidget::contextMenuTriggered (int selectedItemNum)
{
   switch (selectedItemNum) {

      case ADWCM_LOAD_WIDGET_CONFIG:
         this->loadWidgetConfiguration ();
         break;

      case ADWCM_SAVE_WIDGET_CONFIG:
         this->saveWidgetConfiguration ();
         break;

      case ADWCM_SELECT_USE_PV_NAME:
         this->setPVLabelMode (QE::usePvName);
         break;

      case ADWCM_SELECT_USE_ALIAS_NAME:
         this->setPVLabelMode (QE::useAliasName);
         break;

      case ADWCM_SELECT_USE_DESCRIPTION:
         this->setPVLabelMode (QE::useDescription);
         break;

      default:
         // Call parent class function.
         //
         ParentWidgetClass::contextMenuTriggered (selectedItemNum);
         break;
   }
}

//------------------------------------------------------------------------------
//
QString QEAbstractDynamicWidget::getPersistantRootName () const
{
   const QString className = this->metaObject()->className ();
   return QString ("%1Config").arg (className);
}

//------------------------------------------------------------------------------
//
QString QEAbstractDynamicWidget::getPersistantName () const
{
   const QString className = this->metaObject()->className ();

   QString result;

   if (this->useOwnPersistantName) {
      // We are saving/restoring just this component/widget. Use a fixed name.
      //
      result = QString ("%1_Widget").arg (className);
   } else {
      // Use an application generated window name.
      //
      result = this->persistantName (className);
   }

   return result;
}

//---------------------------------------------------------------------------------
//
void QEAbstractDynamicWidget::loadNamedWidetConfiguration (const QString& filename)
{
   bool okay = false;
   PersistanceManager* pm = this->getPersistanceManager ();
   if (pm && !filename.isEmpty()) {        // sanity check
      this->useOwnPersistantName = true;   // indicate that we need specific persistance name
      okay = pm->restoreWidget (this, filename, this->getPersistantRootName (), "Default");
      this->useOwnPersistantName = false;  // revert to default behaviour
   }

   if (okay) {
      // File loaded successfully - update window title.
      //
      QString title = QString("%1  %2").arg(this->metaObject()->className()).arg(filename);
      QEUtilities::setWindowTitle (this, title);
   }
}

//---------------------------------------------------------------------------------
//
void QEAbstractDynamicWidget::saveNamedWidetConfiguration (const QString& filename)
{
   bool okay = false;
   PersistanceManager* pm = this->getPersistanceManager ();
   if (pm && !filename.isEmpty()) {        // sanity check
      this->useOwnPersistantName = true;   // indicate that we need specific persistance name
      okay = pm->saveWidget (this, filename, this->getPersistantRootName (), "Default");
      this->useOwnPersistantName = false;  // revert to default behaviour
   }

   if (okay) {
      // File saved successfully - update window title.
      //
      QString title = QString("%1  %2").arg(this->metaObject()->className()).arg(filename);
      QEUtilities::setWindowTitle (this, title);
   }
}

//---------------------------------------------------------------------------------
//
void QEAbstractDynamicWidget::loadWidgetConfiguration ()
{
   QString filename = QFileDialog::getOpenFileName
         (this,tr("Select configuration input file"), this->getDefaultDir (),
          tr("Config Files(*.xml);;All files (*)"));

   // Idf user clicks on Cancel, an empty file name is returned.
   // This is handled by the loadNamedWidetConfiguration function.
   //
   this->loadNamedWidetConfiguration (filename);
}

//---------------------------------------------------------------------------------
//
void QEAbstractDynamicWidget::saveWidgetConfiguration ()
{
   // Note: saveWidget itself asks for a confirmation, so we don't ask for
   // overwrite confirmation here.
   //
   QString filename = QFileDialog::getSaveFileName
         (this, tr("Select configuration output file"), this->getDefaultDir (),
          tr("Config Files(*.xml)"), 0, QFileDialog::DontConfirmOverwrite);

   // Ensure name ends with .xml iff a name has been specified.
   //
   if (!filename.isEmpty()) {
      if (!filename.endsWith(".xml")) {
         filename.append(".xml");
      }
   }

   // Idf user clicks on Cancel, an empty file name is returned.
   // This is handled by the saveNamedWidetConfiguration function.
   //
   this->saveNamedWidetConfiguration (filename);
}

// end
