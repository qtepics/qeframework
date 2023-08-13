/*  QEMenuButtonSetupDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2015-2023 Australian Synchrotron
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

// Ensure uint64_t is available for all compilers
//
#ifdef __cplusplus
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
#endif

#include "QEMenuButtonSetupDialog.h"
#include <QtDesigner>
#include <QDebug>
#include <QStringList>
#include <QModelIndex>
#include <QModelIndexList>
#include <QEStringFormatting.h>

#include <QEMenuButton.h>
#include <QEMenuButtonItem.h>
#include <QEMenuButtonModel.h>
#include <QEMenuButtonArgumentsDialog.h>

#include <ui_QEMenuButtonSetupDialog.h>

#define DEBUG  qDebug () << "QEMenuButtonSetupDialog" << __LINE__ << __FUNCTION__ << "  "

// Used to qualify context menu actions.
//
enum Actions {
   ADD_MENU_ITEM_ACTION = 1,
   DEL_MENU_ITEM_ACTION,
   ADD_SUB_MENU_ACTION
};


// Must be consistant with QEMenuButtonSetupDialog.ui
// We need a map as FORMAT_TIME and FORMAT_LOCAL_ENUMERATE
// are not applicable here.
//
#define NUMBER_FORMATS   5

static const QEStringFormatting::formats formatMap [NUMBER_FORMATS] = {
   QEStringFormatting::FORMAT_DEFAULT,
   QEStringFormatting::FORMAT_FLOATING,
   QEStringFormatting::FORMAT_INTEGER,
   QEStringFormatting::FORMAT_UNSIGNEDINTEGER,
   QEStringFormatting::FORMAT_STRING
};

//-----------------------------------------------------------------------------
// Create the dialog
//
QEMenuButtonSetupDialog::QEMenuButtonSetupDialog (QEMenuButton* menuButtonIn,
                                                  QWidget* parent) :
   QDialog (parent),
   ui (new Ui::QEMenuButtonSetupDialog)
{
   QAction* action = NULL;
   bool okay;

   // Ensure sensible values.
   this->model = NULL;
   this->selectedItem = NULL;
   this->contextMenuItem = NULL;
   this->treeContextMenu = NULL;
   this->treeSelectionModel = NULL;
   this->returnIsMasked = false;
   this->acceptIsInhibited = true;

   this->ui->setupUi (this);
   this->argumentsDialog = new QEMenuButtonArgumentsDialog (this);

   // Create lists of widgets.
   //
   this->lineEditList.append (this->ui->menuItemName);

   this->lineEditList.append (this->ui->programName);
   this->comboBoxList.append (this->ui->programOptions);

   this->lineEditList.append (this->ui->openUiFilename);
   this->lineEditList.append (this->ui->openPrioritySubstitutions);
   this->lineEditList.append (this->ui->openCustomisationName);
   this->comboBoxList.append (this->ui->openCreateOption);

   this->lineEditList.append (this->ui->variableName);
   this->lineEditList.append (this->ui->variableValue);
   this->comboBoxList.append (this->ui->variableFormat);

   this->resetButtonList.append (this->ui->pushButton_01);
   this->resetButtonList.append (this->ui->pushButton_02);
   this->resetButtonList.append (this->ui->pushButton_03);
   this->resetButtonList.append (this->ui->pushButton_04);
   this->resetButtonList.append (this->ui->pushButton_05);
   this->resetButtonList.append (this->ui->pushButton_06);
   this->resetButtonList.append (this->ui->pushButton_07);
   this->resetButtonList.append (this->ui->pushButton_08);
   this->resetButtonList.append (this->ui->pushButton_09);
   this->resetButtonList.append (this->ui->pushButton_10);
   this->resetButtonList.append (this->ui->pushButton_11);
   this->resetButtonList.append (this->ui->pushButton_12);

   this->itemSelected (NULL);

   this->owner = menuButtonIn;
   if (!this->owner) return;  // sanity check

   this->model = new QEMenuButtonModel (NULL);

   QString xml = this->owner->getMenuString ();

   okay = this->model->parseXml (xml);
   if (okay) {
      // Link to Treeview
      //
      this->ui->treeView->setModel (this->model);
   }
   this->acceptIsInhibited = !okay;

   // Configure basic tree setup.
   //
   this->ui->treeView->setIndentation (10);
   this->ui->treeView->setItemsExpandable (true);
   this->ui->treeView->setUniformRowHeights (true);
   this->ui->treeView->setRootIsDecorated (true);
   this->ui->treeView->setAlternatingRowColors (true);

   // Set up drag/drop processing.
   //
   this->ui->treeView->setSelectionMode (QAbstractItemView::SingleSelection);
   this->ui->treeView->setDragEnabled (true);
   this->ui->treeView->setAcceptDrops (true);
   this->ui->treeView->setDropIndicatorShown (true);
   this->ui->treeView->setDragDropMode (QAbstractItemView::InternalMove);
   //
   // Still to figure this out.


   // Set up contect menu processing.
   //
   this->ui->treeView->setContextMenuPolicy (Qt::CustomContextMenu);
   QObject::connect (this->ui->treeView, SIGNAL (customContextMenuRequested (const QPoint &)),
                     this,               SLOT   (treeMenuRequested          (const QPoint &)));

   this->treeContextMenu = new QMenu (this);
   action = new QAction ("Add Menu Item...", this->treeContextMenu);
   action->setData (QVariant (int (ADD_MENU_ITEM_ACTION)));
   this->treeContextMenu->addAction (action);

   action = new QAction ("Add Sub Menu...", this->treeContextMenu);
   action->setData (QVariant (int (ADD_SUB_MENU_ACTION)));
   this->treeContextMenu->addAction (action);

   action = new QAction ("Delete Menu Item...", this->treeContextMenu);
   action->setData (QVariant (int (DEL_MENU_ITEM_ACTION)));
   this->treeContextMenu->addAction (action);

   QObject::connect (this->treeContextMenu, SIGNAL (triggered        (QAction*)),
                     this,                  SLOT   (treeMenuSelected (QAction*)));

   // Create a tree selection model.
   //
   this->treeSelectionModel = new QItemSelectionModel (this->model, this);
   this->ui->treeView->setSelectionModel (this->treeSelectionModel);

   QObject::connect (this->treeSelectionModel, SIGNAL (selectionChanged (const QItemSelection&, const QItemSelection&)),
                     this,                     SLOT   (selectionChanged (const QItemSelection&, const QItemSelection&)));


   // Connect variable widget sigbals into this dialog.
   //
   QCheckBox* cbox = this->ui->useSeperator;
   QObject::connect (cbox, SIGNAL (stateChanged (int)),
                     this, SLOT   (stateChanged (int)));

   for (int j = 0; j < this->lineEditList.count(); j++) {
      QLineEdit* line = this->lineEditList.value (j);
      QObject::connect (line, SIGNAL (textEdited (const QString&)),
                        this, SLOT   (textEdited (const QString&)));
      QObject::connect (line, SIGNAL (returnPressed     ()),
                        this, SLOT   (editReturnPressed ()));
   }

   for (int j = 0; j < this->comboBoxList.count(); j++) {
      QComboBox* box = this->comboBoxList.value (j);
      QObject::connect (box,  SIGNAL (activated         (int)),
                        this, SLOT   (comboBoxActivated (int)));
   }

   for (int j = 0; j < this->resetButtonList.count(); j++) {
      QPushButton* btn = this->resetButtonList.value (j);
      QObject::connect (btn,  SIGNAL (clicked            (bool)),
                        this, SLOT   (resetButtonClicked (bool)));
   }

   // Edit program arguments
   //
   QObject::connect (this->ui->programArgumentsEdit,  SIGNAL (clicked                (bool)),
                     this,                            SLOT   (onEditProgramArguments (bool)));

}

//-----------------------------------------------------------------------------
// Destroy the dialog
//
QEMenuButtonSetupDialog::~QEMenuButtonSetupDialog()
{
   delete this->ui;
   delete this->model;
}

//------------------------------------------------------------------------------
//
void QEMenuButtonSetupDialog::itemSelected (QEMenuButtonItem* item)
{
    bool enable;
    QString text;

    this->selectedItem = item;   // save what's selected.

    // Set default setting depnding on selection if any and item type.
    //
    if (item) {
       if (item->getIsSubMenuContainer()) {
          enable = false;
          text = "n/a";
       } else {
          enable = true;
          text = "";
       }

    } else {
       enable = false;
       text = "";
    }

    for (int j = 0; j < this->lineEditList.count(); j++) {
       QLineEdit* w = this->lineEditList.value (j);
       w->setEnabled (enable);
       w->setText (text);
    }

   for (int j = 0; j < this->comboBoxList.count(); j++) {
      QComboBox* w = this->comboBoxList.value (j);
      w->setEnabled (enable);
      w->setCurrentIndex (0);
   }

   for (int j = 0; j < this->resetButtonList.count(); j++) {
      QPushButton* w = this->resetButtonList.value (j);
      w->setEnabled (enable);
   }

   this->ui->programArgumentsEdit->setEnabled (enable);

   if (item) {
      this->ui->menuItemName->setEnabled (true);
      this->ui->menuItemName->setText (item->getName ());

      this->ui->useSeperator->setEnabled (true);
      this->ui->useSeperator->setChecked (item->data.separator);

      if (!item->getIsSubMenuContainer()) {
         // This is not a sub menu item - set up all "properties".
         //
         this->ui->programName->setText (item->data.programName);
         this->ui->programArgumentsLabel->setText (item->data.programArguments.join("; "));
         this->ui->programOptions->setCurrentIndex ((int) item->data.programStartupOption);

         this->ui->openUiFilename->setText (item->data.uiFilename);
         this->ui->openPrioritySubstitutions->setText (item->data.prioritySubstitutions);
         this->ui->openCustomisationName->setText (item->data.customisationName);
         this->ui->openCreateOption->setCurrentIndex ((int) item->data.creationOption);

         this->ui->variableName->setText (item->data.variable);
         this->ui->variableValue->setText (item->data.variableValue);

         bool formatFound = false;
         for (int j = 0; j < NUMBER_FORMATS; j++) {
            if (item->data.format == formatMap[j]) {
               // We have a match ;-)
               this->ui->variableFormat->setCurrentIndex (j);
               formatFound = true;
               break;
            }
         }
         if (!formatFound) {
            DEBUG << "string format" << item->data.format << "not valid for QEMenuButton";
            // default to FORMAT_DEFAULT
            this->ui->variableFormat->setCurrentIndex (0);
         }
      }

   } else {
      // Clear info.
      //
      this->ui->menuItemName->setEnabled (false);
      this->ui->menuItemName->setText ("");

      this->ui->useSeperator->setEnabled (false);
      this->ui->useSeperator->setChecked (false);
   }
}


//------------------------------------------------------------------------------
//
void QEMenuButtonSetupDialog::selectionChanged (const QItemSelection& selected,
                                                const QItemSelection& /* deselected*/ )
{
   QModelIndexList list = selected.indexes ();
   const int n = list.size ();

   // We expect only one item to be selected.
   //
   if (n == 1) {
      QModelIndex s = list.value (0);
      QEMenuButtonItem* item = this->model->indexToItem (s);
      this->itemSelected (item);
   } else {
      // Don't allow multiple selections.
      //
      this->itemSelected (NULL);
   }
}

//-----------------------------------------------------------------------------
//
void QEMenuButtonSetupDialog::treeMenuRequested (const QPoint& pos)
{
   // Get the model index 'address' of the item at this positon,
   // and then get the underlying load save item.
   //
   QModelIndex index = this->ui->treeView->indexAt (pos);
   this->contextMenuItem = this->model->indexToItem (index);

   QPoint golbalPos = this->ui->treeView->mapToGlobal (pos);

   // NOTE: We want access to the tree's rowHeight function as we need this as a
   // position offset (I suspect to account for the header). But the rowHeight ()
   // function IS protected. So we get around this by deriving our own tree view
   // class that can see the protected rowHeight () function and expose this as a
   // public function.
   //
   class ExposedTreeView : QTreeView {
   public:
       int getRowHeight (const QModelIndex &index) const { return this->rowHeight (index); }
   };

   ExposedTreeView* exposedTree = (ExposedTreeView*) this->ui->treeView;
   int rowHeight = exposedTree->getRowHeight (index);

   if (rowHeight == 0) rowHeight = 20;  // scale??

   golbalPos.setX (golbalPos.x () + 4);
   golbalPos.setY (golbalPos.y () + rowHeight);
   this->treeContextMenu->exec (golbalPos, 0);
}

//-----------------------------------------------------------------------------
//
void QEMenuButtonSetupDialog::treeMenuSelected  (QAction* action)
{
   static int number = 100;

   bool okay;
   int intAction;
   QEMenuButtonItem* item = NULL;
   QEMenuButtonItem* attachTo = NULL;
   QModelIndex index;

   intAction = action->data ().toInt (&okay);
   if (!okay) return;

   switch (intAction) {

      case ADD_MENU_ITEM_ACTION:
      case ADD_SUB_MENU_ACTION:

         // Find new items parent.
         //
         if (this->contextMenuItem) {
            if (this->contextMenuItem->getIsSubMenuContainer ()) {
               attachTo = this->contextMenuItem;
            } else {
               attachTo = this->contextMenuItem->getParent ();
            }
         } else {
            attachTo = NULL;   // just add to top level.
         }

         item = new QEMenuButtonItem (QString ("MenuItem_%1").arg (++number),
                                      intAction == ADD_SUB_MENU_ACTION,
                                      NULL, NULL);
         this->model->addItemToModel (item, attachTo);

         index = this->model->getIndex (item);
         this->treeSelectionModel->select (index, QItemSelectionModel::ClearAndSelect);

         if (intAction == ADD_SUB_MENU_ACTION) {
            // New sub menu won't have any children yet but it is still "expanded".
            this->ui->treeView->expand (index);
         }
         break;

      case DEL_MENU_ITEM_ACTION:
         if (this->contextMenuItem) {
            this->model->removeItemFromModel (this->contextMenuItem);
            this->itemSelected (NULL);
         }
         break;

      default:
         break;
   }

   this->contextMenuItem = NULL;
}

//-----------------------------------------------------------------------------
//
void QEMenuButtonSetupDialog::stateChanged (int state)
{
   QEMenuButtonItem* item = this->selectedItem;
   if (!item) return;

   switch (state) {
      case Qt::Unchecked:
         item->data.separator = false;
         break;

      case Qt::Checked:
         item->data.separator = true;
         break;

      default:
         DEBUG << "Unexpected state:" << state;
         // ignore
         break;
   }
}

//-----------------------------------------------------------------------------
//
void QEMenuButtonSetupDialog::textEdited (const QString& newText)
{
   QEMenuButtonItem* item = this->selectedItem;
   if (!item) return;

   QLineEdit* lineEdit = dynamic_cast <QLineEdit*> (sender ());

#define APPLY(lne, target) if (lineEdit == lne) {                      \
   target = newText;                                                   \
} else


   if (lineEdit == this->ui->menuItemName) {
       item->name = newText;
       this->model->itemUpdated (item);
   } else
   APPLY (this->ui->programName, item->data.programName)
   APPLY (this->ui->openUiFilename, item->data.uiFilename)
   APPLY (this->ui->openPrioritySubstitutions, item->data.prioritySubstitutions)
   APPLY (this->ui->openCustomisationName, item->data.customisationName)
   APPLY (this->ui->variableName, item->data.variable)
   APPLY (this->ui->variableValue, item->data.variableValue)
   {
      qDebug() << "Unexpected " << lineEdit->objectName ();
   }

#undef APPLY

}

//-----------------------------------------------------------------------------
//
void QEMenuButtonSetupDialog::comboBoxActivated (int index)
{
   QEMenuButtonItem* item = this->selectedItem;
   if (!item) return;
   QComboBox* comboBox = dynamic_cast <QComboBox*> (sender ());

   if (comboBox == this->ui->programOptions) {
      item->data.programStartupOption = applicationLauncher::programStartupOptions (index);

   } else if (comboBox == this->ui->openCreateOption) {
      item->data.creationOption = QEActionRequests::Options (index);

   } else if (comboBox == this->ui->variableFormat) {
      item->data.format = formatMap [index];

   } else {
      qDebug() << "Unexpected " << comboBox->objectName ();
   }
}

//-----------------------------------------------------------------------------
//
void QEMenuButtonSetupDialog::resetButtonClicked (bool)
{
   QEMenuButtonItem* item = this->selectedItem;
   if (!item) return;
   QPushButton* resetButton = dynamic_cast <QPushButton*> (sender ());

#define CHECKB(btn, lne, itm, def) if (resetButton == btn) {         \
   this->ui->lne->setText (def);                                     \
   itm = def;                                                        \
} else

#define CHECKC(btn, cbx, itm, def) if (resetButton == btn) {         \
   this->ui->cbx->setCurrentIndex (def);                             \
   itm = def;                                                        \
} else


   CHECKB (this->ui->pushButton_02, programName, item->data.programName, "")
   if (resetButton == this->ui->pushButton_03) {
      this->ui->programArgumentsLabel->setText ("");
      item->data.programArguments.clear ();
   } else
   CHECKC (this->ui->pushButton_04, programOptions, item->data.programStartupOption, applicationLauncher::PSO_NONE)
   CHECKB (this->ui->pushButton_05, openUiFilename, item->data.uiFilename, "")
   CHECKC (this->ui->pushButton_06, openCreateOption, item->data.creationOption, QEActionRequests::OptionOpen)
   CHECKB (this->ui->pushButton_07, openPrioritySubstitutions, item->data.prioritySubstitutions, "")
   CHECKB (this->ui->pushButton_08, openCustomisationName, item->data.customisationName, "")
   CHECKB (this->ui->pushButton_09, variableName, item->data.variable, "")
   CHECKB (this->ui->pushButton_10, variableValue, item->data.variableValue, "0")
   CHECKC (this->ui->pushButton_11, variableFormat, item->data.format, QEStringFormatting::FORMAT_DEFAULT)
   if (resetButton == this->ui->pushButton_12) {
      this->ui->useSeperator->setChecked (false);
      item->data.separator = false;
   } else {
      qDebug() << "Unexpected " << resetButton->objectName();
   }

#undef CHECKC
#undef CHECKB
}

//-----------------------------------------------------------------------------
//
void QEMenuButtonSetupDialog::onEditProgramArguments (bool)
{
   QEMenuButtonItem* item = this->selectedItem;
   if (!item) return;

   this->argumentsDialog->setArgumentList (item->data.programArguments);
   const int status = this->argumentsDialog->exec (this->ui->programArgumentsEdit);
   if (status == 1) {
      const QStringList argList = this->argumentsDialog->getArgumentList();
      const QString argText = argList.join ("; ");
      item->data.programArguments = argList;
      this->ui->programArgumentsLabel->setText (argText);
   }
}

//-----------------------------------------------------------------------------
//
void QEMenuButtonSetupDialog::editReturnPressed ()
{
   this->returnIsMasked = true;   // for accept to essentially not accept
}

//-----------------------------------------------------------------------------
// User has pressed OK
//
void QEMenuButtonSetupDialog::on_buttonBox_accepted ()
{
   // If user presses return in a line edit, the accepted function called.
   // We need to check if this is the case.
   //
   if (this->returnIsMasked) {
      this->returnIsMasked = false;
      return;
   }

   if (this->acceptIsInhibited) return;

   // Can we find the designer form window?
   //
   QString xml = this->model->serialiseXml ();

   // Just calling this->owner->setMenuString kind of works, but doesn't update
   // in designer as expected.
   //
   QDesignerFormWindowInterface* formWindow = QDesignerFormWindowInterface::findFormWindow (this->owner);
   if (formWindow) {
      formWindow->cursor()->setProperty("menuEntries", xml);
   } else {
      this->owner->setMenuString (xml);
   }

   this->accept ();
}

//-----------------------------------------------------------------------------
// User has pressed cancel
//
void QEMenuButtonSetupDialog::on_buttonBox_rejected ()
{
   this->close ();
}

// end
