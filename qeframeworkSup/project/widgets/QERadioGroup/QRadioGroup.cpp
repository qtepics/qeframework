/*  QRadioGroup.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2014-2023 Australian Synchrotron
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

#include "QRadioGroup.h"
#include <QEvent>
#include <QDebug>
#include <QRadioButton>
#include <QPushButton>
#include <QECommon.h>

#define DEBUG qDebug () << "QRadioGroup"  << __LINE__<< __FUNCTION__ << " "

//-----------------------------------------------------------------------------
// Constructor with no initialisation
//
QRadioGroup::QRadioGroup (QWidget* parent) :
   QGroupBox (parent)
{
   this->commonSetup (" QRadioGroup ");
}

//-----------------------------------------------------------------------------
// Constructor with title.
//
QRadioGroup::QRadioGroup (const QString& title, QWidget* parent) :
   QGroupBox (parent)
{
   this->commonSetup (title);
}

//-----------------------------------------------------------------------------
// Setup common to all constructors
//
void QRadioGroup::commonSetup (const QString& title)
{
   this->setMinimumSize (16, 16);   // start small - let designer set larger.

   // Set up default properties
   //
   this->setOwnTitle (title);
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

   // Some events must be applied to the internal widgets
   //
   this->installEventFilter (this);
}

//---------------------------------------------------------------------------------
//
QSize QRadioGroup::sizeHint () const
{
   return QSize (140, 40);
}

//---------------------------------------------------------------------------------
// QGroupBox (parent class) captures some of these events and does not call
// appropriate virtual function.  So must intercept there events here.
//
bool QRadioGroup::eventFilter (QObject* watched, QEvent* event)
{
   const QEvent::Type type = event->type ();
   bool result = false;

   switch (type) {
      case QEvent::FontChange:
         if (watched == this) {
            // Propagate font change to embedded buttons.
            //
            for (int j = 0; j < this->buttonList.count (); j++) {
               QAbstractButton* button = this->buttonList.value (j);
               if (button) button->setFont (this->font());
            }
            result = QGroupBox::event (event);   // call parent fuction;
         }
         break;

      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonDblClick:
         // Handle by doing nothing - not even ignoring the event as base class does.
         result = false;
         break;

      default:
         result = false;
   }

   return result;
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
   result->setFont (this->font());
   result->setFocusPolicy (Qt::NoFocus);
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
   const bool doEmit = (this->currentIndex != newIndex);
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

   // Did the value change?
   //
   if (doEmit) {
      // This prevents infinite looping in the case of cyclic connections.
      //
      if (!this->emitValueChangeInhibited) {
         this->emitValueChangeInhibited = true;
         emit valueChanged (this->currentIndex);
         this->emitValueChangeInhibited = false;
      }
   }
}

//------------------------------------------------------------------------------
//
void QRadioGroup::setValue (const int indexIn)
{
   // Basically a wrapper.
   //
   this->internalSetValue (indexIn);
}

//------------------------------------------------------------------------------
//
int QRadioGroup::getValue () const
{
   return this->currentIndex;
}

//------------------------------------------------------------------------------
//
void QRadioGroup::setOwnTitle (const QString& titleIn)
{
   this->ownTitle = titleIn;
   // Treat "-" as special null value. As actual null string get re-interpreted
   // as the default, i.e. " QERadioGroup "
   this->setTitle (this->ownTitle == "-" ? "" : this->ownTitle);
}

//------------------------------------------------------------------------------
//
QString QRadioGroup::getOwnTitle () const
{
   return this->ownTitle;
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
