/*  QEFormGroupBox.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2021-2022 Australian Synchrotron
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
 *    andrews@ansto.gov.au
 */

#include "QEFormGroupBox.h"
#include <QDebug>
#include <QFont>
#include <QEScaling.h>

#define DEBUG qDebug () << "QEFormGroupBox"  << __LINE__<< __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QEFormGroupBox::QEFormGroupBox (QWidget *parent) : QEGroupBox (parent)
{
   // Set default property values.
   //
   this->setSubstitutedTitleProperty ("QEFormGroupBox ");
   this->setCheckable (true);
   this->setChecked (true);

   this->hideInnerFrameShape = false;
   this->loadedFrameShape = QFrame::NoFrame;

   this->useFormTitle = false;
   this->loadedFormTitle = "";

   // Create the internal layout
   //
   this->layout = new QHBoxLayout (this);
   this->layout->setContentsMargins (
            QEScaling::scale (4),   // left
            QEScaling::scale (12),  // top
            QEScaling::scale (4),   // right
            QEScaling::scale (4));  // bottom
   this->layout->setSpacing (0);

   this->form = new QEForm (this);
   this->layout->addWidget (this->form);

   // Ensure the QEFrom has enough height to display any error message.
   // If we set a min height on the QEFrom itself, it breaks the layout
   // being driven by the item loaded within in the QEFrom.
   // Maybe that is a Qt thing or maybe a QEFrom thing - not sure.
   //
   this->spacer = new QWidget (this);
   this->spacer->setFixedWidth (1);
   this->spacer->setMinimumHeight (QEScaling::scale (20));
   this->layout->addWidget (this->spacer);

   // Get form loaded notification - connection to onFormLoaded.
   //
   QObject::connect (this->form, SIGNAL (formLoaded   (bool)),
                     this,       SLOT   (onFormLoaded (bool)));

   // Connect the clicked signal - connection to onGroupBoxClick.
   //
   QObject::connect (this,  SIGNAL (clicked         (bool)),
                     this,  SLOT   (onGroupBoxClick (bool)));
}

//------------------------------------------------------------------------------
//
QEFormGroupBox::~QEFormGroupBox () { }

//------------------------------------------------------------------------------
//
QSize QEFormGroupBox::sizeHint () const
{
   return QSize (200, 100);
}

//------------------------------------------------------------------------------
// On show, particularly the first show, honor the isChecked setting.
//
void QEFormGroupBox::showEvent (QShowEvent* event)
{
   QEGroupBox::showEvent (event);  // call parent first
   this->updateBoxMinMaxSizes();
}

//------------------------------------------------------------------------------
//
QFrame* QEFormGroupBox::findUniqueFrame ()
{
   if (!this->form) return NULL;   // sanity check

   QFrame* result = NULL;
   int frameCount = 0;

   QObjectList objList1 = this->form->children ();
   const int number1 = objList1.count ();

   for (int i = 0; i < number1; i++) {
      QObject* obj1 = objList1.value (i, NULL);

      // Looking for widgets
      QWidget* widget = qobject_cast <QWidget*> (obj1);

      if (widget) {
         QObjectList objList2 = widget->children ();
         const int number2  = objList2.count();

         for (int j = 0; j <  number2 ; j++) {
            QObject* obj2 = objList2.value (j, NULL);

            // Looking for QFrame inc QEFrame
            result = qobject_cast <QFrame*> (obj2);
            if (result) {
               frameCount++;
            }
         }
      }
   }

   if (frameCount != 1) result = NULL;  // must be unique.
   return result;
}

//------------------------------------------------------------------------------
//
void QEFormGroupBox::setHideInnerFrameShape (const bool hideInnerFrameShapeIn)
{
   this->hideInnerFrameShape = hideInnerFrameShapeIn;
   this->updateInnerFrameShape();
}

//------------------------------------------------------------------------------
//
bool QEFormGroupBox::getHideInnerFrameShape () const
{
   return this->hideInnerFrameShape;
}

//------------------------------------------------------------------------------
//
void QEFormGroupBox::setUseFormTitle (const bool useFormTitleIn)
{
   this->useFormTitle = useFormTitleIn;
   this->updateBoxTitle();
}

//------------------------------------------------------------------------------
//
bool QEFormGroupBox::getUseFormTitle () const
{
   return this->useFormTitle;
}

//------------------------------------------------------------------------------
// slot
void QEFormGroupBox::setUiFileName (const QString& uiFileName)
{
   this->setUiFileNameProperty (uiFileName);
}

//------------------------------------------------------------------------------
// slot
void QEFormGroupBox::setUiFileSubstitutions (const QString& uiFileNameSubstitutions)
{
   this->setVariableNameSubstitutionsProperty (uiFileNameSubstitutions);
}

//------------------------------------------------------------------------------
//
void QEFormGroupBox::saveConfiguration (PersistanceManager* pm)
{
   // Prepare to save configuration for this widget.
   // We only save the is checked state.
   //
   const QString name = this->persistantName ("QEFormGroupBox");
   PMElement instanceElement = pm->addNamedConfiguration (name);
   instanceElement.addValue ("isChecked", this->isChecked());
}

//------------------------------------------------------------------------------
//
void QEFormGroupBox::restoreConfiguration (PersistanceManager* pm,
                                           restorePhases restorePhase)
{
   // Do nothing during the framework phase.
   //
   if (restorePhase != FRAMEWORK) return;

   // Retrieve our configuration
   const QString name = this->persistantName ("QEFormGroupBox");
   PMElement instanceElement = pm->getNamedConfiguration( name );
   if (!instanceElement.isNull()) {
      bool value;
      bool status = instanceElement.getValue ("isChecked", value);
      if (status) {
         this->setChecked (value);

         // setChecked does not trigger signal, so must call this ourselves.
         //
         this->updateBoxMinMaxSizes();
      }
   }
}

//------------------------------------------------------------------------------
//
void QEFormGroupBox::updateBoxTitle()
{
   if (this->useFormTitle && !this->loadedFormTitle.isEmpty()) {
      this->setSubstitutedTitleProperty (this->loadedFormTitle);
   }
}

//------------------------------------------------------------------------------
//
void QEFormGroupBox::updateInnerFrameShape()
{
   QFrame* frame = this->findUniqueFrame ();
   if (frame) {
      if (this->hideInnerFrameShape) {
         frame->setFrameShape (QFrame::NoFrame);
      } else {
         frame->setFrameShape (this->loadedFrameShape);
      }
   }
}

//------------------------------------------------------------------------------
//
void QEFormGroupBox::updateBoxMinMaxSizes()
{
   if (this->isChecked()) {
      // By setting the minmum to 0,0, this allows the widgit loaded ui file
      // to detrmine the minimum displayed size of the group box.
      //
      this->setMinimumSize (0, 0);
      this->setMaximumSize (QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
   } else {
      this->setFixedHeight (QEScaling::scale (20));
      this->setMinimumWidth (QEScaling::scale (40));
   }
}

//------------------------------------------------------------------------------
// slot
void QEFormGroupBox::onFormLoaded (bool formLoaded)
{
   if (formLoaded) {
      this->loadedFormTitle = this->form->getQEGuiTitle() + " ";
      this->updateBoxTitle();

      QFrame* frame = this->findUniqueFrame ();
      if (frame) {
         this->loadedFrameShape = frame->frameShape();
      }
      this->updateInnerFrameShape();
   }
}

//------------------------------------------------------------------------------
// slot
void QEFormGroupBox::onGroupBoxClick (bool)
{
   this->updateBoxMinMaxSizes();
}

// end
