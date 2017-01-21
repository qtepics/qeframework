/*  QRadioGroup.cpp
 *
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
 *  Copyright (c) 2013 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include <QDebug>
#include <QRadioButton>
#include <QPushButton>
#include <QECommon.h>

#include "QRadioGroup.h"

#define DEBUG qDebug () << "QRadioGroup" << __FUNCTION__ << __LINE__

//-----------------------------------------------------------------------------
// Constructor with no initialisation
//
QRadioGroup::QRadioGroup (QWidget* parent) :
   QGroupBox (" QRadioGroup ", parent)
{
   this->commonSetup ();
}

//-----------------------------------------------------------------------------
// Constructor with title and known variable
//
QRadioGroup::QRadioGroup (const QString& title, QWidget* parent) :
   QGroupBox (title, parent)
{
   this->commonSetup ();
}

//---------------------------------------------------------------------------------
//
QSize QRadioGroup::sizeHint () const
{
   return QSize (200, 80);
}

//---------------------------------------------------------------------------------
//
QAbstractButton* QRadioGroup::createButton (QWidget* parent)
{
   QAbstractButton* result = NULL;

   switch (this->buttonStyle) {
      case Radio:
         result = new QRadioButton (parent);
         break;

      case Push:
         result = new QPushButton (parent);
         break;

      default:
         DEBUG  << "Invalid button style" << (int) this->buttonStyle;
         break;
   }

   result->setAutoExclusive (true);
   result->setCheckable (true);
   return result;
}

//-----------------------------------------------------------------------------
//
void QRadioGroup::reCreateAllButtons ()
{
   int j;
   QAbstractButton *button;

   // First selete any old existing buttons.
   //
   if (this->noSelectionButton) {
      delete this->noSelectionButton;
      this->noSelectionButton = NULL;
   }

   while (this->buttonList.count() > 0) {
      button = this->buttonList.takeFirst ();
      if (button) {
         delete button;
      }
   }

   // Create new buttons -invisble for now.
   // NOTE: buttons are added/removed from layout as and when needed.
   //
   for (j = 0; j < this->getMaximumButtons (); j++) {
      button = this->createButton (this);
      button->setGeometry (-40, -40, 20, 20);
      button->setVisible (false);

      QObject::connect (button, SIGNAL (clicked (bool)),
                        this,   SLOT   (buttonClicked (bool)));

      this->buttonList.append (button);
   }

   // Hidden button set when no valid selection available.
   // We cannot (in some versions) deselect all.
   //
   this->noSelectionButton = this->createButton (this);
   this->noSelectionButton->setGeometry (-40, -40, 20, 20);
   this->noSelectionButton->setVisible (false);
}

//-----------------------------------------------------------------------------
// Setup common to all constructors
//
void QRadioGroup::commonSetup ()
{
   this->setMinimumSize (120, 40);

   // Set up default properties
   //
   this->cols = 2;
   this->space = 4;
   this->buttonStyle = Radio;
   this->buttonOrder = rowMajor;
   this->strings.clear ();

   // Set the initial state
   //
   this->currentIndex = -1;

   this->numberDisplayed = 0;
   this->rows = 0;

   this->buttonLayout = new QGridLayout (this);
   this->buttonLayout->setContentsMargins (this->space,   // left
                                           this->space,   // top
                                           this->space,   // right
                                           this->space);  // bottom
   this->buttonLayout->setSpacing (this->space);

   // Create buttons - invisble for now.
   // NOTE: radio buttons are added/removed from layout as and when needed.
   //
   this->noSelectionButton = NULL;
   this->buttonList.clear ();
   this->reCreateAllButtons ();
   this->emitValueChangeInhibited = false;
}

//---------------------------------------------------------------------------------
//
int QRadioGroup::getMaximumButtons () const
{
   return 256;  // somewhat arbitary
}

//---------------------------------------------------------------------------------
//
void QRadioGroup::buttonClicked (bool)
{
   QAbstractButton* sendingButton = NULL;

   // Determine signal sending widget
   //
   sendingButton = dynamic_cast<QAbstractButton*> (this->sender());
   if (!sendingButton) {
      return;
   }

   // Is this button in the association?
   //
   if (!this->valueToButton.containsI (sendingButton)) {
      return;
   }

   // Get the value associated with this button.
   //
   this->internalSetValue (this->valueToButton.valueI (sendingButton));
}

//---------------------------------------------------------------------------------
//
void QRadioGroup::setButtonText ()
{
   // Append spaces on end of text for Radio buttons only. This makes for a more
   // consistent selection. The length of the suffix is a bit arbitrary, but in
   // practice 40 is more than enough.
   //
   const QString suffix =
      (this->buttonStyle == Radio) ?  "                                        " : "";

   QAbstractButton* button = NULL;
   int j;

   // Build forward and revserse value to button index/position maps.
   // We do this even when using db enuberations and the mapping is trivial.
   //
   // Clear maps.
   //
   this->valueToButton.clear ();

   // Create indentity map.
   //
   for (j = 0; j < this->strings.count (); j++) {
      this->valueToButton.insertF (j, this->buttonList.value (j));
   }

   this->numberDisplayed = MIN (this->strings.count (), this->buttonList.count ());
   this->rows = (numberDisplayed + this->cols - 1) / MAX (this->cols, 1);

   for (j = 0; j < this->buttonList.count (); j++) {
      button = this->buttonList.value (j);
      button->setVisible (j < numberDisplayed);
      if (j < numberDisplayed) {
         button->setText (this->strings.value (j) + suffix);
      }
   }

   this->setButtonLayout ();
}

//---------------------------------------------------------------------------------
//
void QRadioGroup::setButtonLayout ()
{
   int j;
   int row, col;

   // Remove any existing items from the layout.
   //
   while (this->buttonLayout->takeAt (0) != NULL);

   // Add buttons that are now required.
   //
   for (j = 0; j < this->numberDisplayed && j < this->buttonList.count (); j++) {
      QAbstractButton *button = this->buttonList.value (j, NULL);
      if (button) {

         if (this->buttonOrder == colMajor) {
            // Find row and col - col major.
            //
            row = j % MAX (this->rows, 1);
            col = j / MAX (this->rows, 1);
         } else {
            // Find row and col - row major.
            //
            row = j / MAX (this->cols, 1);
            col = j % MAX (this->cols, 1);
         }

         this->buttonLayout->addWidget (button, row, col);
      }
   }
}

//------------------------------------------------------------------------------
//
void QRadioGroup::internalSetValue (const int indexIn)
{
   QAbstractButton *selectedButton = NULL;
   int newIndex;

   newIndex = LIMIT (indexIn, -1, (this->numberDisplayed - 1));

   // If value the same then nothing to do, specifically no signal to emit. This
   // is the behaviour of Qt's own widgets such as combo box, spin edit etc.
   // We try to be consistant.
   //
   if (this->currentIndex != newIndex) {
      this->currentIndex = newIndex;

      if (this->valueToButton.containsF (this->currentIndex)) {

         selectedButton = this->valueToButton.valueF (this->currentIndex, NULL);
         if (selectedButton) {
            selectedButton->setChecked (true); // this will uncheck all other (radio) buttons
         }

      } else {
         // We haven't mapped this value - use hidden selection.
         // This will uncheck all the "real" buttons.
         // Only really applicable for radio buttons, but we do it anyway.
         //
         this->noSelectionButton->setChecked (true);
         selectedButton = NULL;
      }

      // On some styles, a down push button looks very much like a non-down
      // button. To help emphasize the selected button, we set the font of
      // the selected button bold, and all the other buttons non-bold.
      //
      if (this->buttonStyle == Push) {
         for (int j = 0; j < this->numberDisplayed; j++) {
            QAbstractButton* otherButton = this->buttonList.value (j, NULL);
            if (otherButton) {
               QFont otherFont = otherButton->font ();
               otherFont.setBold (otherButton == selectedButton);
               otherButton->setFont (otherFont);
            }
         }
      }

      // This prevents infinite looping in the case of cyclic connections.
      //
      if (!this->emitValueChangeInhibited) {
         emit valueChanged (this->currentIndex);
      }
   }
}

//------------------------------------------------------------------------------
//
void QRadioGroup::setValue (const int indexIn)
{
   // This prevents infinite looping in the case of cyclic connections.
   //
   this->emitValueChangeInhibited = true;
   this->internalSetValue (indexIn);
   this->emitValueChangeInhibited = false;
}

//------------------------------------------------------------------------------
//
int QRadioGroup::getValue () const
{
   return this->currentIndex;
}

//------------------------------------------------------------------------------
//
void QRadioGroup::setStrings (const QStringList& stringsIn)
{
   this->strings = stringsIn;
   while (this->strings.count() > this->getMaximumButtons ()) {
      this->strings.removeLast ();
   }
   this->setButtonText ();
}

//------------------------------------------------------------------------------
//
QStringList QRadioGroup::getStrings () const
{
   return this->strings;
}

//------------------------------------------------------------------------------
//
void QRadioGroup::setColumns (int colsIn)
{
   int constrainedCols = LIMIT (colsIn, 1, 16);

   if (this->cols != constrainedCols) {
      this->cols = constrainedCols;
      this->rows = (this->numberDisplayed + this->cols - 1) / MAX (this->cols, 1);
      this->setButtonLayout ();
   }
}

//------------------------------------------------------------------------------
//
int QRadioGroup::getColumns () const
{
   return this->cols;
}

//------------------------------------------------------------------------------
//
void QRadioGroup::setSpacing (int spacingIn)
{
   this->space = LIMIT (spacingIn, 0, 20);

   this->buttonLayout->setContentsMargins (this->space,   // left
                                           this->space,   // top
                                           this->space,   // right
                                           this->space);  // bottom
   this->buttonLayout->setSpacing (this->space);
}

//------------------------------------------------------------------------------
//
int QRadioGroup::getSpacing () const
{
    return this->space;
}

//------------------------------------------------------------------------------
//
void QRadioGroup::setButtonStyle (const ButtonStyles buttonStyleIn)
{
   if (this->buttonStyle != buttonStyleIn) {
      this->buttonStyle = buttonStyleIn;

      this->reCreateAllButtons ();
      this->setButtonText ();
   }
}

//------------------------------------------------------------------------------
//
QRadioGroup::ButtonStyles QRadioGroup::getButtonStyle () const
{
   return this->buttonStyle;
}

//------------------------------------------------------------------------------
//
void QRadioGroup::setButtonOrder (const ButtonOrders buttonOrderIn)
{
   if (this->buttonOrder != buttonOrderIn) {
      this->buttonOrder = buttonOrderIn;
      this->setButtonLayout ();
   }
}

//------------------------------------------------------------------------------
//
QRadioGroup::ButtonOrders QRadioGroup::getButtonOrder () const
{
   return this->buttonOrder;
}

// end
