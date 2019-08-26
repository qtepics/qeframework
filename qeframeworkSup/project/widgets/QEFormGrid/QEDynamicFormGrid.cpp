/*  QEDynamicFormGrid.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2019 Australian Synchrotron
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

#include "QEDynamicFormGrid.h"

#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QTimer>
#include <QWidget>
#include <QDebug>
#include <QECommon.h>

#include <ContainerProfile.h>
#include <QECommon.h>
#include <QEScaling.h>
#include <QEForm.h>
#include <ui_QEDynamicFormGrid.h>

// #include <ui_zgrid_spaces.h>

#define DEBUG qDebug () << "QEDynamicFormGrid" << __FUNCTION__ << __LINE__

// Kind of arbitary, however provides parctical upper limits to avoid excess.
//
const int MaximumColumns = 20;

//=============================================================================
// QEDynamicFormGrid::SingleItem functions
//=============================================================================
//
QEDynamicFormGrid::SingleItemContainer::
SingleItemContainer (const QString& actualArgIn,
                     QEDynamicFormGrid* ownerIn) :
   QWidget (NULL),    // no parent for now
   owner (ownerIn)
{
   this->actualArg = actualArgIn;

   this->horizontalLayout = new QHBoxLayout (this);
   this->horizontalLayout->setSpacing(2);
   this->horizontalLayout->setContentsMargins (0, 0, 0, 0);

   {
      const QString formalArg = this->owner->getFormalArg();
      const QStringList formalArgList = formalArg.split (",");
      const QStringList actualArgList = this->actualArg.split (",");
      QString substitutions;

      for (int j = 0; j < formalArgList.count(); j++) {
         const QString f = formalArgList.value (j);
         const QString a = actualArgList.value (j, "");

         if (!substitutions.isEmpty()) {
            substitutions.append(",");
         }
         substitutions.append (f).append ("=").append (a);
      }

      ProfilePublisher publisher (this->owner, substitutions);   // publish/release as necessary.
      this->qeform = new QEForm (this);
   }
   this->horizontalLayout->addWidget(qeform);

   this->controlContainer = new QWidget (this);
   this->controlContainer->setMinimumSize (QSize (20, 20));
   this->controlContainer->setMaximumSize (QSize (20, 16777215));

   this->verticalLayout = new QVBoxLayout (controlContainer);
   this->verticalLayout->setSpacing (0);
   this->verticalLayout->setContentsMargins (0, 2, 0, 0);

   this->closeButton = new QPushButton ("X", controlContainer);
   this->closeButton->setMinimumSize (QSize (18, 18));
   this->closeButton->setMaximumSize (QSize (18, 18));
   QFont font;
   font.setPointSize (10);
   font.setBold (true);
   font.setWeight (75);
   this->closeButton->setFont (font);
   this->closeButton->setStyleSheet (QLatin1String ("background-color: rgb(105, 155, 205);\n"
                                                    "color: rgb(255, 255, 255);"));
   this->closeButton->setToolTip (QString (" Remove %1 from this display ").arg (this->actualArg));

   this->verticalLayout->addWidget(closeButton);
   this->verticalSpacer = new QSpacerItem(20, 52, QSizePolicy::Minimum, QSizePolicy::Expanding);
   this->verticalLayout->addItem(verticalSpacer);
   this->horizontalLayout->addWidget(controlContainer);

   QObject::connect (this->closeButton, SIGNAL (clicked (bool)),
                     owner, SLOT(onCloseButtonClick (bool)));

   // Get the fully substituted ui file name.
   //
   QString uiFileName;
   uiFileName = this->owner->getSubstitutedVariableName (UIFILE_NAME_VARIABLE);
   this->qeform->setUiFileNameProperty (uiFileName);
}

//------------------------------------------------------------------------------
//
QEDynamicFormGrid::SingleItemContainer::~SingleItemContainer () { }

//------------------------------------------------------------------------------
//
QString QEDynamicFormGrid::SingleItemContainer::getActualArg () const
{
   return this->actualArg;
}

//------------------------------------------------------------------------------
// static
QEDynamicFormGrid::SingleItemContainer*
QEDynamicFormGrid::SingleItemContainer::containerOf (QObject* theSender)
{
   QPushButton* button = dynamic_cast<QPushButton*> (theSender);
   if (!button) return NULL;

   QWidget* controlContainer = dynamic_cast<QWidget*> (button->parent());
   if (!controlContainer) return NULL;

   SingleItemContainer* item = dynamic_cast<SingleItemContainer*> (controlContainer->parent());
   return item;
}

//=============================================================================
// QEDynamicFormGrid functions
//=============================================================================
// Constructor with no initialisation
//
QEDynamicFormGrid::QEDynamicFormGrid (QWidget* parent) :
   QEFrame (parent),
   ui (new Ui::QEDynamicFormGrid)
{
   this->ui->setupUi (this);
   this->commonSetup ("");
}

//------------------------------------------------------------------------------
//
QEDynamicFormGrid::QEDynamicFormGrid (const QString& uiFileIn, QWidget* parent) :
   QEFrame (parent),
   ui (new Ui::QEDynamicFormGrid)
{
   this->ui->setupUi (this);
   this->commonSetup (uiFileIn);
}

//------------------------------------------------------------------------------
//
void QEDynamicFormGrid::commonSetup (const QString& uiFile)
{
   // Save input parameters.
   //
   this->setVariableName (uiFile, UIFILE_NAME_VARIABLE);

   // Configure the panel.
   //
   this->setFrameStyle (QFrame::Sunken);
   this->setFrameShape (QFrame::Box);

   this->setAllowDrop (false);
   this->setVariableAsToolTip (false);
   this->setDisplayAlarmStateOption (DISPLAY_ALARM_STATE_NEVER);

   // There are no variables.
   //
   this->setNumVariables (NUMBER_OF_VARIABLES);

   this->formList.clear ();
   this->formalArg = "DEVICE";
   this->actualArgList.clear();
   this->columns = 1;
   this->gridOrder = RowMajor;
   this->margin = 2;
   this->spacing = 2;

   this->ui->scrollArea->setWidgetResizable (true);

   // Create empty selection menu and assign to the select button.
   //
   this->selectionMenu = new QMenu (this);
   this->ui->selectButton->setMenu (this->selectionMenu);

   // The connection applies to all added the sub-menus and actions.
   //
   QObject::connect (this->selectionMenu, SIGNAL (triggered                (QAction *)),
                     this,                SLOT   (onSelectionMenuTriggered (QAction *)));


   // We could not create an empty layout when we designed QEDynamicFormGrid.ui
   //
   this->scrollFrameLayout = new QGridLayout (this->ui->scrollFrame);

   // We use a plain widgets as spacers - I couldn't get a QSpacerItem to behave
   // as I wanted.
   //
   this->rowScrollSpacer = new QWidget (NULL);
   this->rowScrollSpacer->setMinimumHeight (0);
   this->rowScrollSpacer->setMaximumHeight (QWIDGETSIZE_MAX);

   this->colScrollSpacer = new QWidget (NULL);
   this->colScrollSpacer->setMinimumWidth (0);
   this->colScrollSpacer->setMaximumWidth (QWIDGETSIZE_MAX);

   for (unsigned int v = 0; v < NUMBER_OF_VARIABLES; v++) {
      this->vnpm [v].setVariableIndex (v);
      QObject::connect (&this->vnpm [v],
                        SIGNAL (newVariableNameProperty (QString, QString, unsigned int) ),
                        this, SLOT          (setNewName (QString, QString, unsigned int) ) );
   }
}

//------------------------------------------------------------------------------
//
QEDynamicFormGrid::~QEDynamicFormGrid () { }   // place holder

//------------------------------------------------------------------------------
//
void QEDynamicFormGrid::constructItem (const QString& actualArg)
{
   // First update the active list.
   //
   const int n = this->formList.count();
   bool alreadyActive = false;
   for (int j = 0; j < n; j++) {
      SingleItemContainer* activeItem = this->formList.value (j, NULL);
      if (activeItem && activeItem->getActualArg() == actualArg) {
         // Found it - relocate to the end of the list
         //
         this->formList.removeAt (j);
         this->formList.append (activeItem);
         alreadyActive = true;
         break;
      }
   }

   if (!alreadyActive) {
      // Not in the list - create a new instance.
      //
      SingleItemContainer* item = new SingleItemContainer (actualArg, this);

      // Scale to the current application scaling.
      // We can't do anything about any current form scaling
      // This info is held in the display manager (qegui, kubili).
      //
      QEScaling::applyToWidget (item);

      this->formList.append (item);
   }

   this->reorderItems ();
}

//------------------------------------------------------------------------------
//
void QEDynamicFormGrid::reorderItems ()
{
   // First remove all items from the scrollFrameLayout
   //
   int nr = this->scrollFrameLayout->rowCount();
   int nc = this->scrollFrameLayout->columnCount();
   for (int r = 0 ; r < nr ; r++) {
      for (int c = 0 ; c < nc ; c++) {
         QLayoutItem* item = this->scrollFrameLayout->itemAtPosition (r, c);
         if (item) {
            this->scrollFrameLayout->removeItem (item);
         }
      }
   }

   // Now add again in the new order within grid.
   //
   nr = this->getRows ();
   nc = this->getColumns ();
   int actualColumnsUsed = 0;

   const int m = this->formList.count();
   for (int j = 0; j < m; j++) {
      int r;
      int c;
      if (this->gridOrder == RowMajor) {
         r = j / nc;
         c = j % nc;
      } else { // must be col major order
         c = j / nr;
         r = j % nr;
      }
      actualColumnsUsed = MAX (actualColumnsUsed, c+1);

      SingleItemContainer* activeItem = this->formList.value (j, NULL);
      if (activeItem) {
         this->scrollFrameLayout->addWidget (activeItem, r, c, 1, 1);
         this->scrollFrameLayout->setRowStretch (r, 1);      // can't be zero
         this->scrollFrameLayout->setColumnStretch (c, 1);   // can't be zero
      }
   }

   // Lastly add the spacers.
   //
   static const int bigSize = 10000;
   
   this->scrollFrameLayout->addWidget (this->rowScrollSpacer, nr, 0, 1, 1);
   this->scrollFrameLayout->setRowStretch (nr, bigSize);

   this->scrollFrameLayout->addWidget (this->colScrollSpacer, 0, actualColumnsUsed, 1, 1);
   this->scrollFrameLayout->setColumnStretch (actualColumnsUsed, bigSize);

   // We set the size of the scrollFrame really big, enough room for all the
   // items with no overlap. Because of the bigSize:1 stretch factor the top/left
   // of the spacers essentialy give us the required height/width. We then resize
   // the scrollFrame so that the scroll bars reflects reality.
   //
   this->ui->scrollFrame->setFixedSize (bigSize, bigSize);

   // Allow time for the resize to actually take place.
   //
   QTimer::singleShot (20,  this, SLOT (resetScrollFrameSize()));
}

//------------------------------------------------------------------------------
//
void QEDynamicFormGrid::resetScrollFrameSize ()
{
   // A little bit of wiggle room seems to help.
   //
   const int wiggle = QEScaling::scale (8);

   const int requiredHeight =
         this->rowScrollSpacer->geometry().top() +
         this->rowScrollSpacer->minimumHeight () +
         this->margin + wiggle;

   const int requiredWidth =
         this->colScrollSpacer->geometry().left() +
         this->colScrollSpacer->minimumWidth () +
         this->margin + wiggle;

   this->ui->scrollFrame->setFixedSize (requiredWidth, requiredHeight);
}

//------------------------------------------------------------------------------
//
QSize QEDynamicFormGrid::sizeHint () const
{
   return QSize (276, 112);
}

//------------------------------------------------------------------------------
//
void QEDynamicFormGrid::establishConnection (unsigned int index)
{
   QString title;

   switch (index) {
      case UIFILE_NAME_VARIABLE:
         // do nothing
         break;

      case TITLE_VARIABLE:
         title = this->getSubstitutedVariableName (TITLE_VARIABLE);
         this->ui->titleLabel->setText (title);
         break;

      default:
         DEBUG << "unexpected index value:" << index;
         break;
   }
}


//------------------------------------------------------------------------------
//
void QEDynamicFormGrid::setNewName (QString name, QString substitutions,
                                    unsigned int index)
{
   this->setVariableNameAndSubstitutions (name, substitutions, index);
}

//------------------------------------------------------------------------------
//
void QEDynamicFormGrid::onSelectionMenuTriggered (QAction* action)
{
   if (!action) return;  // sanity check
   QString formalArg = action->text ();
   this->constructItem (formalArg);
}

//------------------------------------------------------------------------------
//
void QEDynamicFormGrid::onCloseButtonClick (bool)
{
   // Find the SingleItemContainer associated with the sender.
   //
   SingleItemContainer* item = SingleItemContainer::containerOf (QObject::sender());
   if (!item) return;   // sainity check

   // Remove this item from the form list.
   //
   if (this->formList.removeOne (item)) {
      item->setParent (NULL);
      delete item;
      this->reorderItems ();
   } else {
      DEBUG << item->getActualArg() << "not in current form list";
   }
}

//------------------------------------------------------------------------------
//
int QEDynamicFormGrid::getRows () const
{
   int r = (this->formList.count() + this->columns - 1) / this->columns;
   return r;
}

//==============================================================================
// Property access.
//==============================================================================
//
// We use the variableNamePropertyManager to manage the uiFile and local subsitutions.
//
void QEDynamicFormGrid::setUiFile (const QString& uiFileName)
{
   this->vnpm [UIFILE_NAME_VARIABLE].setVariableNameProperty (uiFileName);
}

//------------------------------------------------------------------------------
//
QString QEDynamicFormGrid::getUiFile () const
{
   return this->vnpm [UIFILE_NAME_VARIABLE].getVariableNameProperty ();
}

//------------------------------------------------------------------------------
//
void QEDynamicFormGrid::setTitle (const QString& title)
{
   this->vnpm [TITLE_VARIABLE].setVariableNameProperty (title);
}

//------------------------------------------------------------------------------
//
QString QEDynamicFormGrid::getTitle () const
{
   return this->vnpm [TITLE_VARIABLE].getVariableNameProperty ();
}

//------------------------------------------------------------------------------
//
void QEDynamicFormGrid::setNameSubstitutions (const QString& substitutions)
{
   // Set same substitutions for all name property managers.
   //
   for (int v = 0; v < NUMBER_OF_VARIABLES; v++) {
      this->vnpm [v].setSubstitutionsProperty (substitutions);
   }
}

//------------------------------------------------------------------------------
//
QString QEDynamicFormGrid::getNameSubstitutions () const
{
   // Return substitutions from first property manager
   //
   return this->vnpm [0].getSubstitutionsProperty ();
}

//------------------------------------------------------------------------------
//
void QEDynamicFormGrid::setFormalArg (const QString& formalArgIn)
{
   this->formalArg = formalArgIn;
}

//------------------------------------------------------------------------------
//
QString QEDynamicFormGrid::getFormalArg () const
{
   return this->formalArg;
}

//------------------------------------------------------------------------------
//
void QEDynamicFormGrid::setActualArgList (const QStringList& actualArgListIn)
{
   this->actualArgList = actualArgListIn;

   this->selectionMenu->clear();
   for (int j = 0; j < this->actualArgList.count (); j++) {
      QString actualArg = this->actualArgList.value(j);
      this->selectionMenu->addAction (actualArg);
   }
}

//------------------------------------------------------------------------------
//
QStringList QEDynamicFormGrid::getActualArgList  () const
{
   return this->actualArgList;
}

//------------------------------------------------------------------------------
//
void QEDynamicFormGrid::setColumns (const int number)
{
   int newColumns = LIMIT (number, 1, MaximumColumns);

   if (this->columns != newColumns) {
      this->columns =newColumns;
   }
}

//------------------------------------------------------------------------------
//
int QEDynamicFormGrid::getColumns () const
{
   return this->columns;
}

//------------------------------------------------------------------------------
//
void QEDynamicFormGrid::setGridOrder (const GridOrders gridOrderIn)
{
   if (this->gridOrder != gridOrderIn) {
      this->gridOrder = gridOrderIn;
   }
}

//------------------------------------------------------------------------------
//
QEDynamicFormGrid::GridOrders QEDynamicFormGrid::getGridOrder () const
{
   return this->gridOrder;
}

//------------------------------------------------------------------------------
//
void  QEDynamicFormGrid::setMargin (const int marginIn)
{
   this->margin = LIMIT (marginIn, 0, 20);
   this->scrollFrameLayout->setMargin (this->margin);
}

//------------------------------------------------------------------------------
//
int  QEDynamicFormGrid::getMargin () const
{
   return this->margin;
}

//------------------------------------------------------------------------------
//
void  QEDynamicFormGrid::setSpacing (const int spacingIn)
{
   this->spacing = LIMIT (spacingIn, 0, 20);
   this->scrollFrameLayout->setSpacing (this->spacing);
}

//------------------------------------------------------------------------------
//
int  QEDynamicFormGrid::getSpacing () const
{
   return this->spacing;
}

//==============================================================================
// Save/Restore.
//==============================================================================
//
void QEDynamicFormGrid::saveConfiguration (PersistanceManager* pm)
{
   if (!pm) {
      DEBUG << "pm arg is null pointer";
      return;
   }

   // Prepare to save configuration for this widget
   //
   const QString formName = persistantName ("QEDynamicFormGrid");
   PMElement formElement = pm->addNamedConfiguration (formName);

   const int number = this->formList.count ();
   for (int j = 0; j < number; j++) {
      SingleItemContainer* item = this->formList.value (j, NULL);
      if (item) {
         PMElement actualElement = formElement.addElement ("actual");
         actualElement.addValue ("value", item->getActualArg ());
      }
   }
}

//------------------------------------------------------------------------------
//
void QEDynamicFormGrid::restoreConfiguration (PersistanceManager* pm, restorePhases restorePhase)
{
   if (!pm) {
      DEBUG << "pm arg is null pointer";
      return;
   }

   if (restorePhase != FRAMEWORK) return;

   const QString formName = persistantName ("QEDynamicFormGrid");
   PMElement formElement = pm->getNamedConfiguration (formName);

   if (formElement.isNull ()) {
      DEBUG << "formElement is null element";
      return;
   }

   const int number = formElement.getElementList ("actual").count ();
   for (int j = 0; j < number; j++) {
      PMElement actualElement = formElement.getElement ("actual", j);
      if (actualElement.isNull ()) {
         DEBUG << "element" << j << "is null";
         continue;
      }

      QString actual;
      bool okay;
      okay = actualElement.getValue ("value", actual);
      if (!okay) {
         DEBUG << "element" << j << "is invalid";
         continue;
      }

      this->constructItem (actual);
   }
}

// end
