/*  QEAbstractDynamicWidget.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2016-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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

   action = new QAction ("Load Configuration...", menu);
   action->setCheckable (false);
   action->setData (ADWCM_LOAD_WIDGET_CONFIG);
   menu->addAction (action);

   action = new QAction ("Save Configuration...", menu);
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

   action = new QAction ("Use PV Names", menu);
   action->setCheckable (true);
   action->setChecked (this->pvLabelMode == QE::usePvName);
   action->setData (ADWCM_SELECT_USE_PV_NAME);
   menu->addAction (action);

   action = new QAction ("Use Alias Names (if available)", menu);
   action->setCheckable (true);
   action->setChecked (this->pvLabelMode == QE::useAliasName);
   action->setData (ADWCM_SELECT_USE_ALIAS_NAME);
   menu->addAction (action);

   action = new QAction ("Use Descriptions (if available)", menu);
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

   } else {
      // Load failed.
      //
      const QString errorMessage = QString("%1 configuration load from %2 failed")
                                   .arg(this->metaObject()->className())
                                   .arg (filename);
      const message_types mt (MESSAGE_TYPE_WARNING, MESSAGE_KIND_STANDARD);
      this->sendMessage (errorMessage, mt);
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
      QString title = QString("%1  %2").arg(this->metaObject()->className()).arg (filename);
      QEUtilities::setWindowTitle (this, title);

   } else {
      // Save failed.
      //
      const QString errorMessage = QString("%1 configuration save to %2 failed")
                                   .arg(this->metaObject()->className())
                                   .arg (filename);
      const message_types mt (MESSAGE_TYPE_WARNING, MESSAGE_KIND_STANDARD);
      this->sendMessage (errorMessage, mt);
   }
}

//---------------------------------------------------------------------------------
//
void QEAbstractDynamicWidget::loadWidgetConfiguration ()
{
   QString filename = QFileDialog::getOpenFileName
         (this,"Select configuration input file", this->getDefaultDir (),
          "Config Files(*.xml);;All files (*)");

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
         (this, "Select configuration output file", this->getDefaultDir (),
          "Config Files(*.xml)", 0, QFileDialog::DontConfirmOverwrite);

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
