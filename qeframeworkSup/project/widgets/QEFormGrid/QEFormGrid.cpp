/*  QEFormGrid.cpp
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
#include <QECommon.h>

#include "QEFormGrid.h"

#define DEBUG qDebug () << "QEFormGrid" << __FUNCTION__ << __LINE__

#define WAIT_FOR_TYPING_TO_FINISH 1000       // One Second

// Kind of arbitary, and provids parctical upper limits to avoid excess.
//
const int MaximumForms = 2000;
const int MaximumColumns = 200;

//==============================================================================
// MacroData functions
//==============================================================================
//
QEFormGrid::MacroData::MacroData (const QString& prefixIn, QEFormGrid* formGridIn)
{
   this->prefix = prefixIn;
   this->formGrid = formGridIn;
   this->offset = 1;
   this->numberWidth = 2;
}

//------------------------------------------------------------------------------
//
void QEFormGrid::MacroData::setMacroPrefix (const QString& prefixIn)
{
   QString trimmedPrefix = prefixIn.trimmed ();

   // Do not allow null prefixes.
   //
   if (!trimmedPrefix.isEmpty()) {
      if (this->prefix != trimmedPrefix) {
         this->prefix = trimmedPrefix;
         this->formGrid->triggerReCreateAllForms ();
      }
   }
}

//------------------------------------------------------------------------------
//
QString QEFormGrid::MacroData::getMacroPrefix ()
{
   return this->prefix;
}

//------------------------------------------------------------------------------
//
void QEFormGrid::MacroData::setOffset (const int offsetIn)
{
   this->offset = offsetIn;
   this->formGrid->triggerReCreateAllForms ();
}

//------------------------------------------------------------------------------
//
int QEFormGrid::MacroData::getOffset ()
{
   return this->offset;
}

//------------------------------------------------------------------------------
//
void QEFormGrid::MacroData::setNumberWidth (const int numberWidthIn)
{
   this->numberWidth = LIMIT (numberWidthIn, 1, 6);
   this->formGrid->triggerReCreateAllForms ();
}

//------------------------------------------------------------------------------
//
int QEFormGrid::MacroData::getNumberWidth ()
{
   return this->numberWidth;
}

//------------------------------------------------------------------------------
//
void QEFormGrid::MacroData::setStrings (const QStringList& stringsIn)
{
   this->strings = stringsIn;
   this->formGrid->triggerReCreateAllForms ();
}

//------------------------------------------------------------------------------
//
QStringList QEFormGrid::MacroData::getStrings ()
{
   return this->strings;
}

//------------------------------------------------------------------------------
//
QString QEFormGrid::MacroData::genSubsitutions (const int n)
{
   QString subs;
   QString value;

   subs = "";

   // E.g. ROWNAME=Fred where prefix provides SLOT, ROW or COL.
   // Note: the string value must quoted this incase it is a null string
   // or it contailes spaces.
   //
   value = "'" + this->strings.value (n, "") + "'";
   subs.append (this->prefix).append ("NAME=").append (value);

   subs.append (", ");   // Separator

   // E.g.  ROW=09
   // Pad number with '0' to required width.
   //
   value = QString ("%1").arg (n + this->offset, this->numberWidth, 10, QChar ('0'));
   subs.append (this->prefix).append ("=").append (value);

   return subs;
}


//=============================================================================
// QEFormGrid functions
//=============================================================================
// Constructor with no initialisation
//
QEFormGrid::QEFormGrid (QWidget* parent) : QEFrame (parent)
{
   this->commonSetup ("", 4, 1);
}

//------------------------------------------------------------------------------
//
QEFormGrid::QEFormGrid (const QString& uiFileIn, const int numberIn,
                        const int colsIn, QWidget* parent) : QEFrame (parent)
{
   this->commonSetup (uiFileIn, numberIn, colsIn);
}

//------------------------------------------------------------------------------
//
void QEFormGrid::commonSetup (const QString& uiFileIn,
                              const int numberIn,
                              const int colsIn)
{
   // Save input parameters.
   //
   this->uiFile = uiFileIn;
   this->number = LIMIT (numberIn, 1, MaximumForms);
   this->columns = LIMIT (colsIn, 1, MaximumColumns);

   // If a container profile has been defined, then this widget isn't being created
   // within designer, so flag the various properties are not being modified
   // interactively.  If a user is not modifying the properties there is no need
   // to wait for a user to finish typing before using new property value.
   //
   this->interactive = QEWidget::inDesigner ();

   // Setup a timer so rapid changes to the property values are ignored.
   // Only after the user has stopped typing for a while will the entry be used.
   // The timer will be set on the first keystroke and reset with each subsequent
   // keystroke untill the keystrokes stop for longer than the timeout period.
   // Note, timers are not required if there is no user entering values.
   //
   if (this->interactive) {
       this->inputTimer = new QTimer (this);
       this->inputTimer->setSingleShot (true);
       QObject::connect (this->inputTimer, SIGNAL (timeout ()),
                         this,             SLOT   (inputDelayExpired ()));
   } else {
       this->inputTimer = NULL;
   }

   // Set up the number of variables managed by the variable name manager.
   // NOTE: there is no channel data associated with this widget, but it uses
   // the same mechanism as other data widgets to manage the UI filename and
   // macro substitutions. The standard variable name and macros mechanism is
   // used by QEFormGrid for UI file name and marcos
   //
   this->setNumVariables (1);

   // Configure the panel.
   //
   this->setFrameStyle (QFrame::Sunken);
   this->setFrameShape (QFrame::Box);

   this->gridOrder = RowMajor;

   // Set up default macro formal name prefixes.
   //
   this->rowMacroData = new MacroData ("ROW", this);
   this->colMacroData = new MacroData ("COL", this);
   this->slotMacroData = new MacroData ("SLOT", this);

   this->layout = new QGridLayout (this);
   this->layout->setMargin (2);
   this->layout->setSpacing (2);

   // Create initial number of sub-forms.
   //
   for (int j = 0; j < this->number; j++) {
      this->addSubForm ();
   }

   // Note: unlike QEFile, the grid does not need to recieve notification that
   // the ui file being displayed has changed. The QEForm does that anyways.
   //
   // Set up a connection to recieve variable name property changes (Actually
   // only interested in substitution changes.
   //
   this->variableNamePropertyManager.setVariableIndex (0);

   QObject::connect (&this->variableNamePropertyManager,
                     SIGNAL (newVariableNameProperty (QString, QString, unsigned int) ),
                     this, SLOT        (setNewUiFile (QString, QString, unsigned int) ) );
}

//------------------------------------------------------------------------------
//
QEFormGrid::~QEFormGrid ()
{
   delete this->rowMacroData;
   delete this->colMacroData;
   delete this->slotMacroData;
}

//------------------------------------------------------------------------------
//
void QEFormGrid::triggerReCreateAllForms ()
{
   if (this->interactive && this->inputTimer) {
      this->inputTimer->start (WAIT_FOR_TYPING_TO_FINISH);   // Delayed call
   } else {
      this->reCreateAllForms ();   // Immediate call
   }
}

//------------------------------------------------------------------------------
//
void QEFormGrid::inputDelayExpired ()
{
   this->reCreateAllForms ();
}

//------------------------------------------------------------------------------
//
void QEFormGrid::setNewUiFile (QString variableNameIn,
                               QString variableNameSubstitutionsIn,
                               unsigned int variableIndex)
{
   this->setVariableNameAndSubstitutions (variableNameIn,
                                          variableNameSubstitutionsIn,
                                          variableIndex);
}

//------------------------------------------------------------------------------
//
void QEFormGrid::establishConnection (unsigned int variableIndex)
{
   if (variableIndex != 0) {
      DEBUG << "Unexpected variableIndex: " << variableIndex;
      return;
   }

   // Get the fully substituted variable name.
   //
   this->uiFile = this->getSubstitutedVariableName (variableIndex);

   for (int j = 0; j < this->formsList.count (); j++) {
      QEForm* form = this->formsList.value (j, NULL);
      if (form) {
         form->setUiFileNameProperty (this->uiFile);
      }
   }
}

//------------------------------------------------------------------------------
//
QString QEFormGrid::getPrioritySubstitutions (const int slot)
{
   QString result = "";
   int row;
   int col;

   // Get row and col number from the slot number.
   //
   this->splitSlot (slot, row, col);

   result.append (this->slotMacroData->genSubsitutions (slot));
   result.append (", ");
   result.append (this->rowMacroData->genSubsitutions (row));
   result.append (", ");
   result.append (this->colMacroData->genSubsitutions (col));

   return result;
}

//------------------------------------------------------------------------------
//
QEForm* QEFormGrid::createQEForm (const int slot)
{
   QString psubs = this->getPrioritySubstitutions (slot);
   ProfilePublisher publisher (this, psubs);   // publish/release as necessary.

   QEForm* form = new QEForm (this);
   form->setUiFileNameProperty (this->uiFile);

   return form;
}

//------------------------------------------------------------------------------
//
void QEFormGrid::addSubForm ()
{
   QEForm* form;
   int slot;
   int row, col;

   slot = this->formsList.count ();
   if (slot < MaximumForms) {

      form = this->createQEForm (slot);
      this->formsList.append (form);

      // Get row and col number.
      //
      this->splitSlot (slot, row, col);
      this->layout->addWidget (form, row, col);
   }
}

//------------------------------------------------------------------------------
//
void QEFormGrid::reCreateAllForms ()
{
   while (this->formsList.count () > 0) {
      QEForm* form = this->formsList.value (0);
      this->formsList.removeFirst ();
      this->layout->removeWidget (form);
      delete form;
   }

   for (int slot = 0; slot < this->number; slot++) {
      this->addSubForm ();
   }
}

//------------------------------------------------------------------------------
//
int QEFormGrid::slotOf (const int row, const int col)
{
   int slot = 0;

   switch (this->gridOrder) {
      case RowMajor:
         slot = (row * this->getColumns ()) + col;
         break;

      case ColMajor:
         slot = (col * this->getRows ()) + row;
         break;

      default:
         DEBUG << "Unexpected gridOrder: " << this->gridOrder;
         slot = 0;
         break;
   }
   return slot;
}

//------------------------------------------------------------------------------
//
void QEFormGrid::splitSlot (const int slot, int& row, int& col)
{
   switch (this->gridOrder) {
      case RowMajor:
         row = slot / this->getColumns ();
         col = slot % this->getColumns ();
         break;
      case ColMajor:
         row = slot % this->getRows ();
         col = slot / this->getRows ();
         break;
      default:
         DEBUG << "Unexpected gridOrder: " << this->gridOrder;
         break;
   }
}

//------------------------------------------------------------------------------
//
QSize QEFormGrid::sizeHint () const
{
   return QSize (160, 200);
}


//==============================================================================
// Property access.
//==============================================================================
//
// We use the variableNamePropertyManager to manage the uiFile and local subsitutions.
//
void QEFormGrid::setUiFile (QString uiFileIn)
{
   this->variableNamePropertyManager.setVariableNameProperty (uiFileIn);
}

//------------------------------------------------------------------------------
//
QString QEFormGrid::getUiFile ()
{
   return this->variableNamePropertyManager.getVariableNameProperty ();
}

//------------------------------------------------------------------------------
//
void  QEFormGrid::setGridVariableSubstitutions (QString variableSubstitutionsIn)
{
   this->variableNamePropertyManager.setSubstitutionsProperty (variableSubstitutionsIn);
}

//------------------------------------------------------------------------------
//
QString QEFormGrid::getGridVariableSubstitutions ()
{
   return this->variableNamePropertyManager.getSubstitutionsProperty ();
}

//------------------------------------------------------------------------------
//
void QEFormGrid::setNumber (int number)
{
   const int currentNumber = this->getNumber ();

   this->number = LIMIT (number, 1, MaximumForms);

   if (this->gridOrder == RowMajor) {
      // Existing row and col numbers remain unchanged.
      //
      if (this->number > currentNumber) {
         for (int j = currentNumber; j < this->number; j++) {
            this->addSubForm ();
         }
      } else if (this->number < currentNumber) {
         for (int j = currentNumber - 1; j >= this->number; j--) {
            QEForm* form =  this->formsList.value (j, NULL);
            this->formsList.removeLast ();
            delete form;
         }
      }
   } else {
      // Existing row and col numbers may change.
      //
      while (this->formsList.count () > 0) {
         QEForm* form = this->formsList.value (0);
         this->formsList.removeFirst ();
         this->layout->removeWidget (form);
         delete form;
      }

      for (int slot = 0; slot < this->number; slot++) {
         this->addSubForm ();
      }
   }
}

//------------------------------------------------------------------------------
//
int QEFormGrid::getNumber ()
{
   return this->number;
}

//------------------------------------------------------------------------------
//
void QEFormGrid::setColumns (int number)
{
   int newColumns = LIMIT (number, 1, MaximumColumns);

   if (this->columns != newColumns) {
      this->columns = newColumns;
      this->triggerReCreateAllForms ();
   }
}

//------------------------------------------------------------------------------
//
int QEFormGrid::getColumns ()
{
   return this->columns;
}

//------------------------------------------------------------------------------
//
int QEFormGrid::getRows ()
{
   int r = (this->getNumber () + this->columns - 1) / this->columns;
   return MAX (1, r);  // always at least 1 row.
}

//------------------------------------------------------------------------------
//
void QEFormGrid::setGridOrder (GridOrders gridOrderIn)
{
   if (this->gridOrder != gridOrderIn) {
      this->gridOrder = gridOrderIn;
      this->triggerReCreateAllForms ();
   }
}

//------------------------------------------------------------------------------
//
QEFormGrid::GridOrders QEFormGrid::getGridOrder ()
{
   return this->gridOrder;
}

//------------------------------------------------------------------------------
//
void  QEFormGrid::setMargin (int margin)
{
   this->layout->setMargin (margin);
}

//------------------------------------------------------------------------------
//
int  QEFormGrid::getMargin ()
{
   return this->layout->margin ();
}

//------------------------------------------------------------------------------
//
void  QEFormGrid::setSpacing (int spacing)
{
   this->layout->setSpacing (spacing);
}

//------------------------------------------------------------------------------
//
int  QEFormGrid::getSpacing ()
{
   return this->layout->spacing ();
}

// end
