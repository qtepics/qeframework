/*  QEPvLoadSave.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEPvLoadSave.h"

#include <QDebug>
#include <QFileDialog>
#include <QModelIndex>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>
#include <QMetaType>

#include <QECommon.h>
#include <QEPlatform.h>
#include <QEScaling.h>

#include <ui_QEPvLoadSaveHalf.h>

#include "QEPvLoadSaveAccessFail.h"
#include "QEPvLoadSaveCompare.h"
#include "QEPvLoadSaveItem.h"
#include "QEPvLoadSaveModel.h"
#include "QEPvLoadSaveUtilities.h"

#define DEBUG  qDebug () << "QEPvLoadSave" << __LINE__ << __FUNCTION__ << "  "

static const QVariant nilValue = QVariant();

static const QString abortEnabledStyle  = QEUtilities::colourToStyle (QColor (145, 200, 255));
static const QString abortDisabledStyle = QEUtilities::colourToStyle (QColor (200, 200, 200));
static const QString controlDisabledStyle = QEUtilities::colourToStyle (QColor(0xffeecc), QColor (0xa0a0a0));
static const QString iconPathName = ":/qe/pvloadsave";

static const int minWidth = 472;
static const int minHeight = 400;

//=============================================================================
// Halves
//=============================================================================
//
QEPvLoadSave::Halves::Halves (const Sides sideIn,
                              QEPvLoadSave* ownerIn,
                              QBoxLayout* layout) :
   side (sideIn),
   owner (ownerIn)
{
   this->container = new QFrame ();
   this->container->setFrameShape (QFrame::Panel);
   this->container->setFrameShadow (QFrame::Plain);
   layout->addWidget (container);   // This re-parents container.

   this->ui = new Ui::QEPvLoadSaveHalf();
   this->ui->setupUi (this->container);

   // Can't do this in designer ... yet.
   //
   this->commentTextEdit = new QPlainTextEdit();
   this->ui->commentEditFrame->setWidget (this->commentTextEdit);

   // Connect signals and tag buttons, so we know which side/half
   // the signal comes from.
   //
   QObject::connect (this->ui->writeAllButton, SIGNAL (clicked(bool)),
                     this->owner, SLOT (writeAllClicked (bool)));
   QEUtilities::tagObject (this->ui->writeAllButton, this->side);

   QObject::connect (this->ui->readAllButton, SIGNAL (clicked(bool)),
                     this->owner, SLOT (readAllClicked (bool)));
   QEUtilities::tagObject (this->ui->readAllButton, this->side);

   QObject::connect (this->ui->writeSelectedButton, SIGNAL (clicked(bool)),
                     this->owner, SLOT (writeSubsetClicked (bool)));
   QEUtilities::tagObject (this->ui->writeSelectedButton, this->side);

   QObject::connect (this->ui->readSelectedButton, SIGNAL (clicked(bool)),
                     this->owner, SLOT (readSubsetClicked (bool)));
   QEUtilities::tagObject (this->ui->readSelectedButton, this->side);

   QObject::connect (this->ui->archiveExtractButton, SIGNAL (clicked(bool)),
                     this->owner, SLOT (archiveTimeClicked (bool)));
   QEUtilities::tagObject (this->ui->archiveExtractButton, this->side);

   QObject::connect (this->ui->copyAllButton, SIGNAL (clicked(bool)),
                     this->owner, SLOT (copyAllClicked (bool)));
   QEUtilities::tagObject (this->ui->copyAllButton, this->side);

   QObject::connect (this->ui->copySelectedButton, SIGNAL (clicked(bool)),
                     this->owner, SLOT (copySubsetClicked (bool)));
   QEUtilities::tagObject (this->ui->copySelectedButton, this->side);

   QObject::connect (this->ui->loadButton, SIGNAL (clicked(bool)),
                     this->owner, SLOT (loadClicked (bool)));
   QEUtilities::tagObject (this->ui->loadButton, this->side);

   QObject::connect (this->ui->saveButton, SIGNAL (clicked(bool)),
                     this->owner, SLOT (saveClicked (bool)));
   QEUtilities::tagObject (this->ui->saveButton, this->side);

   QObject::connect (this->ui->deleteButton, SIGNAL (clicked(bool)),
                     this->owner, SLOT (deleteClicked (bool)));
   QEUtilities::tagObject (this->ui->deleteButton, this->side);

   QObject::connect (this->ui->editButton, SIGNAL (clicked(bool)),
                     this->owner, SLOT (editClicked (bool)));
   QEUtilities::tagObject (this->ui->editButton, this->side);

   QObject::connect (this->ui->sortButton, SIGNAL (clicked(bool)),
                     this->owner, SLOT (sortClicked (bool)));
   QEUtilities::tagObject (this->ui->sortButton, this->side);

   QObject::connect (this->ui->compareButton, SIGNAL (clicked(bool)),
                     this->owner, SLOT (compareClicked (bool)));
   QEUtilities::tagObject (this->ui->compareButton, this->side);

   // A few left/right parity violations...
   //
   switch (this->side) {
      case LeftSide:
         QObject::connect (this->ui->show2ndTreeCheckBox, SIGNAL (stateChanged (int)),
                           this->owner, SLOT (showHide2ndTree (int)));
         break;

      case RightSide:
         // Update copy buttons.
         //
         this->ui->copyAllButton->setIcon (QIcon (iconPathName + "/ypoc_all.png"));
         this->ui->copySelectedButton->setIcon (QIcon (iconPathName + "/ypoc_subset.png"));

         // Hide 2nd check box.
         //
         this->ui->show2ndTreeCheckBox->setVisible (false);

         break;

      default:
         DEBUG << this->side;
         break;
   }

   this->tree = this->ui->treeView;   // alias
   QEUtilities::tagObject (tree, this->side);

   this->tree->setAcceptDrops (true);
   this->tree->setContextMenuPolicy (Qt::CustomContextMenu);
   QObject::connect (this->tree,  SIGNAL (customContextMenuRequested (const QPoint&)),
                     this->owner, SLOT   (treeMenuRequested          (const QPoint&)));

   // Configure basic tree setup.
   //
   this->tree->setIndentation (10);
   this->tree->setItemsExpandable (true);
   this->tree->setUniformRowHeights (true);
   this->tree->setRootIsDecorated (true);
   this->tree->setAlternatingRowColors (true);

   this->tree->installEventFilter (this->owner);

   // Create the graphical PV value compare widget.
   // There are two, one for each side, to allow Left v. Right as well as
   // Right vs. Left. These should be same except for opposite sign.
   //
   this->graphicalCompare = new QEPvLoadSaveCompare (this->owner, this->side, NULL);

   // Create an essentially empty model.
   //
   this->model = new QEPvLoadSaveModel (this->tree, this->owner);  // not a widget
   QEUtilities::tagObject (this->model, this->side);

   QObject::connect (this->model, SIGNAL (reportActionComplete (const QEPvLoadSaveItem*, const QEPvLoadSaveCommon::ActionKinds, const bool)),
                     this->owner, SLOT   (acceptActionComplete (const QEPvLoadSaveItem*, const QEPvLoadSaveCommon::ActionKinds, const bool)));

   QObject::connect (this->model, SIGNAL (reportActionInComplete (const QEPvLoadSaveItem*, const QEPvLoadSaveCommon::ActionKinds)),
                     this->owner, SLOT   (acceptActionInComplete (const QEPvLoadSaveItem*, const QEPvLoadSaveCommon::ActionKinds)));


   // Create an essentially empty model.
   //
   if (this->tree->header()) {
      this->tree->header()->setStretchLastSection (true);
      this->tree->header()->resizeSection (0, 240);
   }

   this->vnpm.setVariableIndex (static_cast<unsigned int>(this->side));

   // Set up a connection to receive configuration file name property changes.
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   QObject::connect (&this->vnpm, SIGNAL (newPvNameProperties   (const QEPvNameProperties&)),
                     this->owner, SLOT   (useFilenameProperties (const QEPvNameProperties&)));
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::Halves::setRoot (QEPvLoadSaveItem* rootItem, const QString& heading)
{
   QModelIndex topIndex;

   this->model->setupModelData (rootItem, heading);

   // Ensure top level is expanded.
   // Get first/only child of core index.
   //
   topIndex = this->model->getRootIndex ();
   this->tree->expand (topIndex);
}

//------------------------------------------------------------------------------
// Called by establishConnection
//
void QEPvLoadSave::Halves::open (const QString& configurationFile)
{
   QEPvLoadSaveItem* rootItem = NULL;

   if (configurationFile.isEmpty ()) {
      return;
   }

   QString errorMessage;
   QString comment;
   rootItem = QEPvLoadSaveUtilities::readTree (configurationFile,
                                               this->ui->macroStringEdit->text (),
                                               comment, errorMessage);
   if (!rootItem) {
      DEBUG << errorMessage;
      const message_types mt = message_types (MESSAGE_TYPE_WARNING,
                                              MESSAGE_KIND_STANDARD);
      this->owner->sendMessage (errorMessage, mt);
      return;
   }

   this->setComment (comment);
   this->setRoot (rootItem, configurationFile);
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::Halves::save (const QString& configurationFile)
{
   bool okay;

   if (configurationFile.isEmpty ()) {
      return;
   }

   QEPvLoadSaveItem* rootItem = this->model->getRootItem ();
   QString errorMessage;
   okay = QEPvLoadSaveUtilities::writeTree (configurationFile, rootItem,
                                            this->getComment(), errorMessage);
   if (okay) {
      this->model->setHeading (configurationFile);
   } else {
      message_types mt (MESSAGE_TYPE_WARNING, MESSAGE_KIND_STANDARD);
      this->owner->sendMessage (errorMessage, mt);
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::Halves::setEnabled (const bool enabled)
{
   QWidget* buttons = this->ui->buttonFrame;
   buttons->setEnabled (enabled);
   buttons->setStyleSheet (enabled ? "" : controlDisabledStyle);
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::Halves::shuffleUp()
{
   QEPvLoadSaveItem* item = this->model->getSelectedItem ();
   if (!item) return;   // sanity check
   this->model->shufflePosition (item, -1);
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::Halves::shuffleDown()
{
   QEPvLoadSaveItem* item = this->model->getSelectedItem ();
   if (!item) return;   // sanity check
   this->model->shufflePosition (item, +1);
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::Halves::setConfigurationFile  (const QString& configurationFile)
{
   this->vnpm.setVariableNameProperty (configurationFile);
}

//------------------------------------------------------------------------------
//
QString QEPvLoadSave::Halves::getConfigurationFile ()
{
   return this->vnpm.getVariableNameProperty ();
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::Halves::setConfigurationSubstitutions (const QString& substitutions)
{
   this->vnpm.setSubstitutionsProperty (substitutions);
}

//------------------------------------------------------------------------------
//
QString QEPvLoadSave::Halves::getConfigurationSubstitutions ()
{
   return this->vnpm.getSubstitutionsProperty ();
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::Halves::setComment (const QString& comment)
{
   this->commentTextEdit->setPlainText (comment);
}

//------------------------------------------------------------------------------
//
QString QEPvLoadSave::Halves::getComment() const
{
   return this->commentTextEdit->toPlainText();
}

//=============================================================================
// QEPvLoadSave functions
//=============================================================================
// Constructor with no initialisation
//
QEPvLoadSave::QEPvLoadSave (QWidget* parent) : QEFrame (parent)
{
   this->setNumVariables (ARRAY_LENGTH (this->half));

   // Configure the panel and create contents
   //
   this->setFrameShape (QFrame::Panel);
   this->setFrameShadow (QFrame::Plain);
   this->setMinimumSize (minWidth, minHeight);

   // Set default QEFrame properties.
   //
   this->setAllowDrop (false); // applies to widget as a whole, not the tree view.
   this->setVariableAsToolTip (false);
   this->setDisplayAlarmStateOption (QE::Never);

   this->defaultDir = "";
   this->confirmRequired = true;

   // Create internal widgets.
   //
   this->overallLayout = new QVBoxLayout (this);
   this->overallLayout->setContentsMargins (2, 2, 2, 2);
   this->overallLayout->setSpacing (2);

   this->sidesFrame = new QFrame ();    // Note addwidget sets parent
   this->sidesFrame->setFrameShape (QFrame::NoFrame);
   this->sidesFrame->setFrameShadow (QFrame::Plain);
   this->overallLayout->addWidget (sidesFrame);

   this->sideBySidelayout = new QHBoxLayout (this->sidesFrame);
   this->sideBySidelayout->setContentsMargins (0, 0, 0, 0);
   this->sideBySidelayout->setSpacing (2);

   for (int t = 0; t < ARRAY_LENGTH (this->half); t++) {
      this->half [t] = new Halves ((Sides) t, this, this->sideBySidelayout);
   }

   this->loadSaveStatusFrame = new QFrame ();
   this->loadSaveStatusFrame->setFrameShape (QFrame::Panel);
   this->loadSaveStatusFrame->setFrameShadow (QFrame::Plain);
   this->loadSaveStatusFrame->setFixedHeight (76);
   this->overallLayout->addWidget (this->loadSaveStatusFrame);

   this->loadSaveTitle = new QLabel ("Transfer to/from System or from Archive", this->loadSaveStatusFrame);
   QFont tf = this->loadSaveTitle->font ();
   tf.setPointSize (8);
   this->loadSaveTitle->setFont (tf);
   this->loadSaveTitle->setGeometry (12, 4, 400, 15);

   this->progressBar = new QProgressBar (this->loadSaveStatusFrame);
   this->progressBar->setGeometry (12, 24, 400, 26);

   this->progressStatus = new QLabel ("", this->loadSaveStatusFrame);
   this->progressStatus->setGeometry (12, 54, 400, 16);

   this->abortButton = new QPushButton (this->loadSaveStatusFrame);
   this->abortButton->setText ("Abort");
   this->abortButton->setGeometry (424, 24, 80, 26);
   this->abortButton->setStyleSheet (abortDisabledStyle);
   this->abortButton->setEnabled (false);

   QObject::connect (this->abortButton, SIGNAL (clicked (bool)),
                     this, SLOT (abortClicked (bool)));

   // Initate gathering of archive data - specifically the PV name list.
   //
   this->archiveAccess = new QEArchiveAccess (this);

   // Create forms/dialogs.
   //
   this->accessFail = new QEPvLoadSaveAccessFail (NULL);
   this->groupNameDialog = new QEPvLoadSaveGroupNameDialog (this);
   this->valueEditDialog = new QEPvLoadSaveValueEditDialog (this);
   this->pvNameSelectDialog = new QEPVLoadSaveNameSelectDialog (this);
   this->archiveTimeDialog = new QEPvLoadSaveTimeDialog (this);

   this->setAllowDrop (false);

   // Set the initial state
   //
   QWidget::setEnabled (true);

   // Use widget specific context menu.
   //
   for (int j = 0; j < ARRAY_LENGTH (this->actionList); j++) {
      this->actionList [j] = NULL;
   }

   // TODO - leverage of standard context menu if we can.
   //
   this->treeContextMenu = new QMenu (this);
   this->createAction (this->treeContextMenu, "Create Root",          false, TCM_CREATE_ROOT);
   this->createAction (this->treeContextMenu, "Add Group...",         false, TCM_ADD_GROUP);
   this->createAction (this->treeContextMenu, "Rename Group...",      false, TCM_RENAME_GROUP);
   this->createAction (this->treeContextMenu, "Add PV...",            false, TCM_ADD_PV);
   this->createAction (this->treeContextMenu, "Add Pause Delay...",   false, TCM_ADD_PAUSE);
   this->createAction (this->treeContextMenu, "Examine Properties",   false, TCM_SHOW_PV_PROPERTIES);
   this->createAction (this->treeContextMenu, "Plot in StripChart",   false, TCM_ADD_TO_STRIPCHART);
   this->createAction (this->treeContextMenu, "Show in Scatch Pad",   false, TCM_ADD_TO_SCRATCH_PAD);

   // Array PV releated.
   //
   this->createAction (this->treeContextMenu, "Show in Plotter",      false, TCM_ADD_TO_PLOTTER);
   this->createAction (this->treeContextMenu, "Show in Historgram",   false, TCM_ADD_TO_HISTORGRAM);
   this->createAction (this->treeContextMenu, "Show in Table",        false, TCM_ADD_TO_TABLE);

   this->treeContextMenu->addSeparator ();
   this->createAction (this->treeContextMenu, "Edit PV Name...",      false, TCM_EDIT_PV_NAME);
   this->createAction (this->treeContextMenu, "Edit PV Value...",     false, TCM_EDIT_PV_VALUE);
   this->createAction (this->treeContextMenu, "Copy variable name",   false, TCM_COPY_VARIABLE);
   this->createAction (this->treeContextMenu, "Copy data",            false, TCM_COPY_DATA);
   this->createAction (this->treeContextMenu, "Edit Pause Delay...",  false, TCM_EDIT_PAUSE_VALUE);

   QObject::connect (this->treeContextMenu, SIGNAL (triggered        (QAction*)),
                     this,                  SLOT   (treeMenuSelected (QAction*)));

   // Gui requests.
   //
   this->hostSlotAvailable = false;
   QObject* consumer = this->getGuiLaunchConsumer ();
   if (consumer) {
      this->hostSlotAvailable =
         QObject::connect (this,     SIGNAL (requestAction (const QEActionRequests& )),
                           consumer, SLOT   (requestAction (const QEActionRequests& )));
   }

   this->half [LeftSide]->ui->show2ndTreeCheckBox->setChecked (false);
   this->showHide2ndTree (Qt::Unchecked);
}

//-----------------------------------------------------------------------------
//
QEPvLoadSave::~QEPvLoadSave ()
{
}

//------------------------------------------------------------------------------
//
QAction* QEPvLoadSave::createAction (QMenu *parent,
                                     const QString &caption,
                                     const bool checkable,
                                     const TreeContextMenuActions treeAction)
{
   QAction* action = NULL;

   action = new QAction (caption + " ", parent);
   action->setCheckable (checkable);
   action->setData (QVariant (int (treeAction)));
   parent->addAction (action);

   this->actionList [treeAction] = action;
   return action;
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::setReadOut (const QString& text)
{
   message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STATUS);
   this->sendMessage (text, mt);
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::resizeEvent (QResizeEvent* )
{
   int fw = this->geometry ().width ();
   QRect pg = this->progressBar->geometry ();
   QRect ag = this->abortButton->geometry ();

   // Need to take scaling into account.
   //
   int margin = QEScaling::scale (20);
   int space =  QEScaling::scale (12);

   int dx = (fw - (margin + ag.width ()) - ag.x ());
   ag.translate (dx, 0);
   pg.setWidth (ag.x() - pg.x() - space);

   this->progressBar->setGeometry (pg);
   this->abortButton->setGeometry (ag);
}

//------------------------------------------------------------------------------
//
QSize QEPvLoadSave::sizeHint () const
{
    return QSize (500, 500);
}

//------------------------------------------------------------------------------
//
bool QEPvLoadSave::eventFilter (QObject* watched, QEvent* event)
{
   const QEvent::Type type = event->type ();
   bool result = false;

   switch (type) {
      case QEvent::KeyPress:
         if ((watched == this->half[0]->tree) ||
             (watched == this->half[1]->tree)) {
            QKeyEvent* keyEvent = dynamic_cast< QKeyEvent*>(event);
            if (!keyEvent) break;

            const Qt::KeyboardModifiers modifier = keyEvent->modifiers();
            const bool isShift = ((modifier & Qt::ShiftModifier) != 0);
            if (!isShift)  break;

            const int index = watched == this->half[0]->tree ? 0 : 1;
            const int key = keyEvent->key ();
            switch (key) {
               case Qt::Key_Up:
                  this->half[index]->shuffleUp();
                  result = true;    // event handled
                  break;

               case Qt::Key_Down:
                  this->half[index]->shuffleDown();
                  result = true;    // event handled
                  break;

               default:
                  result = false;
                  break;
            }
         }
         break;

      default:
         result = false;
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::useFilenameProperties (const QEPvNameProperties& fileNameProperties)
{
   this->setVariableNameAndSubstitutions (fileNameProperties.pvName,
                                          fileNameProperties.substitutions,
                                          fileNameProperties.index);
}

//------------------------------------------------------------------------------
//
QEChannel* QEPvLoadSave::createQcaItem (unsigned int variableIndex)
{
   DEBUG << "unexpected - variableIndex =" << variableIndex;
   return NULL;
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::establishConnection (unsigned int variableIndex)
{
   if (variableIndex >= ARRAY_LENGTH (this->half)) {
      DEBUG << "unexpected - variableIndex =" << variableIndex;
      return;
   }

   const QString configurationFile = this->getSubstitutedVariableName (variableIndex);
   this->half [variableIndex]->open (configurationFile);
}

//------------------------------------------------------------------------------
//
QEPvLoadSave::Sides QEPvLoadSave::objectSide (QObject* obj)
{
   if (!obj) return ErrorSide;

   int tag = QEUtilities::objectTag (obj, ErrorSide);
   if ((tag != LeftSide) && (tag != RightSide)) {
      DEBUG << obj->objectName() << " : " << obj->metaObject()->className();
      return ErrorSide;
   }
   return static_cast<Sides>(tag);
}

//------------------------------------------------------------------------------
// Common macro verification
//
#define VERIFY_SIDE(item, default)                                 \
   Sides side = this->objectSide (item);                           \
   if ((side != LeftSide) && (side != RightSide)) {                \
      DEBUG << "Unexpected side" << side;                          \
      return default;                                              \
   }

// Wrapper for void slot functions
//
#define VERIFY_SENDER  VERIFY_SIDE (this->sender (),)

//------------------------------------------------------------------------------
//
QEPvLoadSave::Halves* QEPvLoadSave::halfAssociatedWith (QObject* obj)
{
   VERIFY_SIDE (obj, NULL);
   return this->half [side];
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::acceptActionComplete (const QEPvLoadSaveItem*,
                                         const QEPvLoadSaveCommon::ActionKinds,
                                         const bool okay)
{
   if (okay) {
      int v = this->progressBar->value () + 1;
      this->progressBar->setValue (v);
      int n = this->progressBar->maximum();
      QString status = QString ("%1: Processed %2 of %3 items")
            .arg (this->loadSaveAction)
            .arg (v).arg (n);
      this->progressStatus->setText (status);
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::acceptActionInComplete (const QEPvLoadSaveItem* item,
                                           const QEPvLoadSaveCommon::ActionKinds action)
{
   const QEPvLoadSaveLeaf* leaf = dynamic_cast<const QEPvLoadSaveLeaf*> (item);
   if (leaf) {
      QString pvName;
      switch (action) {
         case QEPvLoadSaveCommon::Apply:
            pvName = leaf->getSetPointPvName();
            this->accessFail->addPVName (pvName);
            break;

         case QEPvLoadSaveCommon::Extract:
            pvName = leaf->getReadBackPvName();
            this->accessFail->addPVName (pvName);
            break;

         case QEPvLoadSaveCommon::ReadArchive:
            pvName = leaf->getArchiverPvName();
            this->accessFail->addPVName (pvName);
            break;

         default:
            DEBUG << "unexpected action" << action
                  << " current action" << this->loadSaveAction;
            break;
      }
   } else {
      DEBUG << "null leaf, item" << item
            << " reported action" << action
            << " current action" << this->loadSaveAction;
   }
}

//==============================================================================
// Menu request/select
//
void QEPvLoadSave::treeMenuRequested (const QPoint& pos)
{
   VERIFY_SENDER;

   this->contextMenuItem = NULL;
   this->contextMenuHalf = this->half [side];
   QTreeView* tree = this->contextMenuHalf->tree;
   QEPvLoadSaveModel* model = this->contextMenuHalf->model;
   QModelIndex index;
   int j;

   // Get the model index 'address' of the item at this positon,
   // and then get the underlying load save item.
   //
   index = tree->indexAt (pos);
   this->contextMenuItem = model->indexToItem (index);

   // Make all actions invisible, then set visible required actions.
   //
   for (j = 0; j < ARRAY_LENGTH (this->actionList); j++) {
      this->actionList [j]->setVisible (false);
   }

   // Does item even exit at this position.
   //
   if (this->contextMenuItem) {
      // Is is a leaf/PV node or a group node?
      //
      const QEPvLoadSaveItem::ItemType itemType = this->contextMenuItem->getItemType();
      switch (itemType) {
         case QEPvLoadSaveItem::Group: {
               this->actionList [TCM_ADD_GROUP]->setVisible (true);
               if (this->contextMenuItem != model->getRootItem ()) {
                  // Renaming the 'ROOT' node prohibited.
                  this->actionList [TCM_RENAME_GROUP]->setVisible (true);
               }
               this->actionList [TCM_ADD_PV]->setVisible (true);
               this->actionList [TCM_ADD_PAUSE]->setVisible (true);
            }
            break;

         case QEPvLoadSaveItem::PV:{
               for (j = TCM_COPY_VARIABLE; j <= TCM_EDIT_PV_VALUE; j++) {
                  this->actionList [j]->setVisible (true);
               }

               // Specials for if/when is an arrays PV.
               //
               bool isAnArrayPv = false;
               QEPvLoadSaveLeaf* leaf = qobject_cast <QEPvLoadSaveLeaf*> (this->contextMenuItem);
               if (leaf) {   // just in case
                  const QVariant data = leaf->getNodeValue ();
                  const QVariantList valueList = data.toList ();
                  const int number = valueList.size ();
                  isAnArrayPv = (number >= 2);
               }
               this->actionList [TCM_ADD_TO_PLOTTER]->setVisible (isAnArrayPv);
               this->actionList [TCM_ADD_TO_HISTORGRAM]->setVisible (isAnArrayPv);
               this->actionList [TCM_ADD_TO_TABLE]->setVisible (isAnArrayPv);

            }
            break;

         case QEPvLoadSaveItem::Pause:
            this->actionList [TCM_EDIT_PAUSE_VALUE]->setVisible (true);
            break;

         default:
            DEBUG << "Unhandled" << itemType;
            break;
      }

   } else if (!model->getRootItem ()) {
      // no item selected - is there a root item??
      //
      // No "ROOT" node - allow it to be created.
      //
      this->actionList [TCM_CREATE_ROOT]->setVisible (true);

   } else {
      return;  // forget it.
   }

   QPoint golbalPos = tree->mapToGlobal (pos);

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

   ExposedTreeView* exposedTree = (ExposedTreeView*) tree;
   int rowHeight = exposedTree->getRowHeight (index);

   if (rowHeight == 0) rowHeight = 20;  // scale??

   golbalPos.setY (golbalPos.y () + rowHeight);
   this->treeContextMenu->exec (golbalPos, 0);
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::treeMenuSelected (QAction* action)
{
   if (!this->contextMenuHalf) return;   // sanity check

   QTreeView* tree = this->contextMenuHalf->tree;
   QEPvLoadSaveModel* model = this->contextMenuHalf->model;

   bool okay;
   int intAction;
   TreeContextMenuActions menuAction;
   QEPvLoadSaveItem* item = NULL;
   QEPvLoadSaveLeaf* leaf = NULL;
   QString nodeName = "";
   QVariant nodeValue;
   int n;

   intAction = action->data ().toInt (&okay);
   if (!okay) {
      return;
   }
   menuAction = (TreeContextMenuActions) intAction;

   // Extract current node name - need it is most case options.
   //
   if (this->contextMenuItem) {
      nodeName = this->contextMenuItem->getNodeName ();
   }

   switch (menuAction) {

      case TCM_CREATE_ROOT:
         item = new QEPvLoadSaveGroup ("ROOT", NULL);
         this->contextMenuHalf->setRoot (item, "");
         break;

      case TCM_ADD_GROUP:
         this->groupNameDialog->setWindowTitle ("QEPvLoadSave - Add Group");
         this->groupNameDialog->setGroupName ("");
         n = this->groupNameDialog->exec (tree);
         if (n == 1) {
            item = new QEPvLoadSaveGroup (this->groupNameDialog->getGroupName (), NULL);
            model->addItemToModel (item, this->contextMenuItem);
         }
         break;

      case TCM_RENAME_GROUP:
         this->groupNameDialog->setWindowTitle ("QEPvLoadSave - Rename Group");
         this->groupNameDialog->setGroupName (nodeName);
         n = this->groupNameDialog->exec (tree);
         if (n == 1) {
            this->contextMenuItem->setNodeName (this->groupNameDialog->getGroupName ());
            model->modelUpdated ();
         }
         break;

      case TCM_ADD_PV:
         this->pvNameSelectDialog->setWindowTitle ("QEPvLoadSave - Add PV");
         this->pvNameSelectDialog->setPvNames ("", "", "");
         n = this->pvNameSelectDialog->exec (tree);
         if (n == 1) {
            QString setPoint;
            QString readBack;
            QString archiver;
            this->pvNameSelectDialog->getPvNames (setPoint, readBack, archiver);
            leaf = new QEPvLoadSaveLeaf (setPoint, readBack, archiver, nilValue, NULL);
            model->addItemToModel (leaf, this->contextMenuItem);
         }
         break;

      case TCM_ADD_PAUSE:
         // For now we will go with a fixed pause of 5 seconds.
         //
         item = new QEPvLoadSavePause (5.0, NULL);
         model->addItemToModel (item, this->contextMenuItem);
         break;

      case TCM_EDIT_PV_NAME:
         leaf = qobject_cast <QEPvLoadSaveLeaf*> (this->contextMenuItem);
         if (leaf) {   // sanity check
            this->pvNameSelectDialog->setWindowTitle ("QEPvLoadSave - edit PV");
            this->pvNameSelectDialog->setPvNames (leaf->getSetPointPvName(),
                                                  leaf->getReadBackPvName(),
                                                  leaf->getArchiverPvName());
            n = this->pvNameSelectDialog->exec (tree);
            if (n == 1) {
               QString setPoint;
               QString readBack;
               QString archiver;
               this->pvNameSelectDialog->getPvNames (setPoint, readBack, archiver);
               leaf->setPvNames (setPoint, readBack, archiver);
               model->modelUpdated ();
            }
         }
         break;

      case TCM_EDIT_PV_VALUE:
      case TCM_EDIT_PAUSE_VALUE:
         this->editItemValue (this->contextMenuItem, this->contextMenuHalf, tree);
         break;

      case TCM_COPY_VARIABLE:
         leaf = qobject_cast <QEPvLoadSaveLeaf*> (this->contextMenuItem);
         if (leaf) {   // sanity check
            QApplication::clipboard ()->setText (leaf->copyVariables());
         }
         break;

      case TCM_COPY_DATA:
         nodeValue = this->contextMenuItem->getNodeValue ();

         // Need to be aware of lists.
         //
         if (QEPlatform::metaType (nodeValue) == QMetaType::QVariantList) {
            QStringList sl = nodeValue.toStringList ();
            QString text = "( ";

            for (int j = 0; j < sl.size (); j++) {
               if (j > 0) text.append(", ");
               text.append (sl.value(j));
            }
            text.append(" )");
            QApplication::clipboard ()->setText (text);
         } else {
            // Not a list - easy.
            QApplication::clipboard ()->setText (nodeValue.toString ());
         }
         break;

      case TCM_SHOW_PV_PROPERTIES:
         leaf = qobject_cast <QEPvLoadSaveLeaf*> (this->contextMenuItem);
         if (leaf) {   // sanity check
            emit this->requestAction (QEActionRequests (QEActionRequests::actionPvProperties (),
                                                        leaf->copyVariables()));
         }
         break;

      case TCM_ADD_TO_STRIPCHART:
         leaf = qobject_cast <QEPvLoadSaveLeaf*> (this->contextMenuItem);
         if (leaf) {   // sanity check
            emit this->requestAction (QEActionRequests (QEActionRequests::actionStripChart (),
                                                        leaf->copyVariables()));
         }
         break;

      case TCM_ADD_TO_SCRATCH_PAD:
         leaf = qobject_cast <QEPvLoadSaveLeaf*> (this->contextMenuItem);
         if (leaf) {   // sanity check
            emit this->requestAction (QEActionRequests (QEActionRequests::actionScratchPad (),
                                                        leaf->copyVariables()));
         }
         break;

      case TCM_ADD_TO_PLOTTER:
         leaf = qobject_cast <QEPvLoadSaveLeaf*> (this->contextMenuItem);
         if (leaf) {   // sanity check
            emit this->requestAction (QEActionRequests (QEActionRequests::actionPlotter (),
                                                        leaf->copyVariables()));
         }
         break;

      case TCM_ADD_TO_HISTORGRAM:
         leaf = qobject_cast <QEPvLoadSaveLeaf*> (this->contextMenuItem);
         if (leaf) {   // sanity check
            emit this->requestAction (QEActionRequests (QEActionRequests::actionShowInHistogram (),
                                                        leaf->copyVariables()));
         }
         break;

      case TCM_ADD_TO_TABLE:
         leaf = qobject_cast <QEPvLoadSaveLeaf*> (this->contextMenuItem);
         if (leaf) {   // sanity check
            emit this->requestAction (QEActionRequests (QEActionRequests::actionTable (),
                                                        leaf->copyVariables()));
         }
         break;

      default:
         DEBUG << "Unexpected action: " << menuAction;
         break;
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::editItemValue (QEPvLoadSaveItem* item,
                                  Halves* half, QWidget* centerOver)
{
   if (!item) return;
   if (!half) return;

   int n;
   QEPvLoadSavePause* pause;

   const QEPvLoadSaveItem::ItemType itemType = item->getItemType();
   switch (itemType) {
      case QEPvLoadSaveItem::Group:
         // pass
         break;

      case QEPvLoadSaveItem::PV:
         this->valueEditDialog->setPvName (item->getNodeName ());
         this->valueEditDialog->setValue (item->getNodeValue ());
         n = this->valueEditDialog->exec (centerOver);
         if (n == 1) {
            item->setNodeValue (this->valueEditDialog->getValue ());
            half->model->modelUpdated ();
         }
         break;

      case QEPvLoadSaveItem::Pause:
         pause = qobject_cast<QEPvLoadSavePause*>(item);
         if (pause) {
            // Create a bespoke dialog?
            //
            this->valueEditDialog->setPvName (pause->getNodeName ());
            this->valueEditDialog->setValue (pause->getDelay());
            n = this->valueEditDialog->exec (centerOver);
            if (n == 1) {
               QVariant value = this->valueEditDialog->getValue ();
               bool okay;
               double delay = value.toDouble(&okay);
               if (okay) {
                  pause->setDelay (delay);
                  half->model->modelUpdated ();
               } else {
                  const message_types mt = message_types (MESSAGE_TYPE_WARNING,
                                                          MESSAGE_KIND_STANDARD);
                  QString message = value.toString() + " cannot be converted to a delay value";
                  this->sendMessage (message, mt);
               }
            }
         } else {
            DEBUG << itemType << "qobject_cast fail";
         }
         break;

      default:
         DEBUG << "Unhandled" << itemType;
         break;
   }
}

//==============================================================================
// Button and box signal functions
//
void QEPvLoadSave::showHide2ndTree (int state)
{
   const bool selected = (state == Qt::Checked);
   this->half [RightSide]->container->setVisible (selected);

   // Increase/Decrease minimum width.
   //
   const int mw = QEScaling::scale (minWidth);  // modify by current scaling
   int newmw = selected ? (mw*2) : mw;
   this->setMinimumWidth (newmw);
}

//------------------------------------------------------------------------------
//
bool QEPvLoadSave::pvWriteIsPermitted ()
{
   bool result;
   if (this->confirmRequired) {
      int confirm = QMessageBox::warning
            (this, "PV Write Confirmation",
             "You are about to write to one or more system Process\n"
             "Variables. This may adversely affect the operation of\n"
             "the system. Are you sure you wish to processed?\n"
             "Click OK to proceed or Cancel for no change.",
             QMessageBox::Ok | QMessageBox::Cancel, QMessageBox::Cancel);
      result = (confirm == QMessageBox::Ok);
   } else {
      result = true;
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::writeAllClicked (bool)
{
   VERIFY_SENDER;
   QEPvLoadSaveModel* model = this->half [side]->model;

   const int number =  model->leafCount ();
   if (number > 0) {
      if (this->pvWriteIsPermitted ()) {  // query for user approval
         this->loadSaveAction = "Apply";
         this->progressBar->setMaximum (MAX (1, number));
         this->progressBar->setValue (0);
         this->abortButton->setStyleSheet (abortEnabledStyle);
         this->abortButton->setEnabled (true);

         QEPvLoadSavePause::setPauseEnabled (true);
         this->half [side]->setEnabled (false);
         model->applyPVData ();
         this->half [side]->setEnabled (true);
      }
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::readAllClicked (bool)
{
   VERIFY_SENDER;
   QEPvLoadSaveModel* model = this->half [side]->model;

   this->loadSaveAction = "Extract";
   this->progressBar->setMaximum (MAX (1, model->leafCount ()));
   this->progressBar->setValue (0);
   this->abortButton->setStyleSheet (abortEnabledStyle);
   this->abortButton->setEnabled (true);
   model->extractPVData ();
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::writeSubsetClicked (bool)
{
   VERIFY_SENDER;
   QEPvLoadSaveItem* item = this->half [side]->model->getSelectedItem ();

   const int number = item ? item->leafCount() : 0;
   if (number > 0) {
      if (this->pvWriteIsPermitted ()) {
         this->loadSaveAction = "Apply";
         this->progressBar->setMaximum (MAX (1, number));
         this->progressBar->setValue (0);
         this->abortButton->setStyleSheet (abortEnabledStyle);
         this->abortButton->setEnabled (true);

         QEPvLoadSavePause::setPauseEnabled (true);
         this->half [side]->setEnabled (false);
         item->applyPVData ();
         this->half [side]->setEnabled (true);
      }
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::readSubsetClicked (bool)
{
   VERIFY_SENDER;

   QEPvLoadSaveItem* item = this->half [side]->model->getSelectedItem ();
   if (item) {
      this->loadSaveAction = "Extract";
      this->progressBar->setMaximum (MAX (1, item->leafCount ()));
      this->progressBar->setValue (0);
      this->abortButton->setStyleSheet (abortEnabledStyle);
      this->abortButton->setEnabled (true);
      item->extractPVData ();
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::archiveTimeClicked (bool)
{
   QWidget* theSender = dynamic_cast <QWidget*> (this->sender ());

   VERIFY_SENDER;
   QEPvLoadSaveModel* model = this->half [side]->model;
   if (!model) return;  // sanity check

   const int number = model->leafCount ();

   // Any PVs to worry about?
   //
   if (number > 0) {
      QDateTime timeNow = QDateTime::currentDateTime().toLocalTime ();
      this->archiveTimeDialog->setMaximumDateTime (timeNow);
      int n = this->archiveTimeDialog->exec (theSender);
      if (n == 1) {
         // User has selected okay.
         //
         QDateTime selectedDataTime = this->archiveTimeDialog->getDateTime ();

         this->loadSaveAction = "Read Archive";
         this->progressBar->setMaximum (MAX (1, number));
         this->progressBar->setValue (0);
         this->abortButton->setStyleSheet (abortEnabledStyle);
         this->abortButton->setEnabled (true);
         model->readArchiveData (selectedDataTime);
      }
   }
}


//------------------------------------------------------------------------------
//
void QEPvLoadSave::copyAllClicked (bool)
{
   VERIFY_SENDER;
   QEPvLoadSaveItem* item = this->half [side]->model->getRootItem ();
   if (item) {
 //     DEBUG << item;
      Sides otherSide = (side == LeftSide) ? RightSide : LeftSide;
      this->half [otherSide]->model->mergeItemInToModel (item);
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::copySubsetClicked (bool)
{
   VERIFY_SENDER;

   QEPvLoadSaveItem* item = this->half [side]->model->getSelectedItem ();
   if (item) {
//      DEBUG << item;
      Sides otherSide = (side == LeftSide) ? RightSide : LeftSide;
      this->half [otherSide]->model->mergeItemInToModel (item);
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::setDefaultDir (const QString& defaultDirIn)
{
    this->defaultDir = defaultDirIn;
}

//------------------------------------------------------------------------------
//
QString QEPvLoadSave::getDefaultDir () const
{
    return this->defaultDir;
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::setConfirmAction (bool confirmRequiredIn)
{
   this->confirmRequired = confirmRequiredIn;
}

//------------------------------------------------------------------------------
//
bool QEPvLoadSave::getConfirmAction () const
{
   return this->confirmRequired;
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::loadClicked (bool)
{
   VERIFY_SENDER;
   QString filename;

   filename = QFileDialog::getOpenFileName
         (this,
          "Select input file", this->getDefaultDir (),
          "PV Config Files(*.xml);;All files (*.*)");

   if (!filename.isEmpty()) {
      this->half [side]->open (filename);

      // Extract and save new default directory.
      //
      QFileInfo fileInfo (filename);
      this->setDefaultDir (fileInfo.absolutePath ());
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::saveClicked (bool)
{
   VERIFY_SENDER;
   QString filename;

   // Create a defualt file name based on current time of day.
   //
   QDateTime timeNow = QDateTime::currentDateTime ();
   QString defaultDir = this->getDefaultDir ();
   QString defaultName;

   defaultName = QString ("%1%2.xml")
         .arg (defaultDir.isEmpty() ? "" : "/")
         .arg (timeNow.toString ("yyyyMMdd_hhmm"));

   filename = QFileDialog::getSaveFileName
         (this, "Select output file",
          defaultDir + defaultName,
          "PV Config Files(*.xml)");

   // Ensure file has xml extension
   //
   if (!filename.endsWith(".xml")) {
      filename.append(".xml");
   }

   if (!filename.isEmpty()) {
      this->half [side]->save (filename);

      // Extract and save new default directory.
      //
      QFileInfo fileInfo (filename);
      this->setDefaultDir (fileInfo.absolutePath ());
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::deleteClicked (bool)
{
   VERIFY_SENDER;
   QEPvLoadSaveModel* model = this->half [side]->model;
   QEPvLoadSaveItem* item;

   // Get and cleare selected item
   //
   item = this->half [side]->model->getSelectedItem ();
   /// this->half [side]->selectedItem = NULL;
   if (item) {
      model->removeItemFromModel (item);
   }
}

//------------------------------------------------------------------------------
// slot (from Edit button)
void QEPvLoadSave::editClicked (bool)
{
   VERIFY_SENDER;

   QEPvLoadSaveItem* item = this->half [side]->model->getSelectedItem ();
   if (!item) return;   // sanity check

   const QEPvLoadSaveItem::ItemType itemType = item->getItemType();
   if (itemType & (QEPvLoadSaveItem::PV | QEPvLoadSaveItem::Pause)) {
      QWidget* centreOver = dynamic_cast <QWidget*> (this->sender ());
      this->editItemValue (item, this->half [side], centreOver);
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::sortClicked (bool)
{
   VERIFY_SENDER;

   QEPvLoadSaveItem* item = this->half [side]->model->getSelectedItem ();
   QEPvLoadSaveGroup* group = qobject_cast<QEPvLoadSaveGroup*>(item);

   if (!group) return;   // can only sort groups, not leaf nodes.
   this->half [side]->model->sort (group);
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::compareClicked (bool)
{
   VERIFY_SENDER;

   // TODO: Verification checks:
   //   Do both sides contain at least a data stub (user root) node.
   //   Is item already on display?
   //
   QEPvLoadSaveCompare* graphicalCompare = this->half [side]->graphicalCompare;  // alias
   graphicalCompare->processModelData ();

   QString title = "Process Variable Comparison";
   if (this->hostSlotAvailable) {
      // If the graphicalCompare widget already visible - skip this part
      //
      if (!graphicalCompare->isVisible()) {
         // Create component item and associated request.
         //
         componentHostListItem item (graphicalCompare,
                                     QE::DockFloating,
                                     false, title);

         // ... and request this hosted by the support application.
         //
         emit requestAction (QEActionRequests (item));
      }
   } else {
      // Just show it.
      //
      graphicalCompare->setWindowTitle (title);
      graphicalCompare->show ();
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::abortClicked (bool)
{
   this->accessFail->clear ();

   QEPvLoadSavePause::setPauseEnabled (false);
   for (int s = 0; s < 2; s++) {
      this->half [s]->model->abortAction ();
   }

   QString title = this->loadSaveAction + " failures";
   if (this->hostSlotAvailable) {
      // If the graphicalCompare widget already visible - skip this part
      //
      if (!this->accessFail->isVisible()) {
         // Create component item and associated request.
         //
         componentHostListItem item (this->accessFail,
                                     QE::DockFloating,
                                     false, title);

         // ... and request this hosted by the support application.
         //
         emit requestAction (QEActionRequests (item));
      }
   } else {
      // Just show it.
      //
      this->accessFail->setWindowTitle (title);
      this->accessFail->show ();
   }

   this->abortButton->setStyleSheet (abortDisabledStyle);
   this->abortButton->setEnabled (false);
   this->progressStatus->setText ("");
}

//==============================================================================
// Property functions
//
void QEPvLoadSave::setSubstitutions (QString configurationFileSubstitutions)
{
   // Use same substituitions for both halves.
   //
   this->half [0]->setConfigurationSubstitutions (configurationFileSubstitutions);
   this->half [1]->setConfigurationSubstitutions (configurationFileSubstitutions);
}

//------------------------------------------------------------------------------
//
QString QEPvLoadSave::getSubstitutions ()
{
   // We use same substitutions for each half. WEither will do for return value.
   //
   return this->half [0]->getConfigurationSubstitutions ();
}

//==============================================================================
// Copy/paste = tbd
//

// end
