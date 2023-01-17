/*  QEPvLoadSave.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2022 Australian Synchrotron
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

#include <QECommon.h>
#include <QEScaling.h>
#include "QEPvLoadSaveAccessFail.h"
#include "QEPvLoadSaveCompare.h"
#include "QEPvLoadSaveItem.h"
#include "QEPvLoadSaveModel.h"
#include "QEPvLoadSaveUtilities.h"

#define DEBUG  qDebug () << "QEPvLoadSave.cpp" << __LINE__ << __FUNCTION__ << "  "

// Compare with QEStripChartToolBar
//
struct PushButtonSpecifications {
   int side;
   int gap;
   int width;
   bool isIcon;  // when false is caption
   const QString captionOrIcon;
   const QString toolTip;
   const char * member;
};

// Most buttons are on both sides - some are on one or the other.
//
#define BOTH 0
#define LHS  1
#define RHS  2

#define NL               (-99)        // new line gap
#define ICW                40         // icon width
#define NCW                68         // normal caption width
#define WCW                80         // wide caption width

static const struct PushButtonSpecifications buttonSpecs [QEPvLoadSave::NumberOfButtons] = {
   { BOTH, 0,   ICW, true,  QString ("write_all.png"),     QString ("Write all PV values to the system"),          SLOT (writeAllClicked (bool))    },
   { BOTH, 0,   ICW, true,  QString ("read_all.png"),      QString ("Read all PV values from the system"),         SLOT (readAllClicked (bool))     },
   { BOTH, 12,  ICW, true,  QString ("write_subset.png"),  QString ("Write selected PV values to the system"),     SLOT (writeSubsetClicked (bool)) },
   { BOTH, 0,   ICW, true,  QString ("read_subset.png"),   QString ("Read selected PV values from the system"),    SLOT (readSubsetClicked (bool))  },

   { BOTH, 12,  ICW, true,  QString ("archive_time.png"),  QString ("Extract all PV values from the archiver for the specified time"),
                                                                                                                   SLOT (archiveTimeClicked (bool)) },
   { LHS,  12,  ICW, true,  QString ("copy_all.png"),      QString ("Copy all PV values to other workspace"),      SLOT (copyAllClicked (bool))     },
   { LHS,  0,   ICW, true,  QString ("copy_subset.png"),   QString ("Copy selected PV values to other workspace"), SLOT (copySubsetClicked (bool))  },
   { RHS,  12,  ICW, true,  QString ("ypoc_all.png"),      QString ("Copy all PV values to other workspace"),      SLOT (copyAllClicked (bool))     },
   { RHS,  0,   ICW, true,  QString ("ypoc_subset.png"),   QString ("Copy selected PV values to other workspace"), SLOT (copySubsetClicked (bool))  },

   { BOTH, NL,  NCW, false, QString ("Load..."),           QString ("Load node tree from file"),                   SLOT (loadClicked (bool))        },
   { BOTH, 2,   NCW, false, QString ("Save..."),           QString ("Save node tree to file"),                     SLOT (saveClicked (bool))        },
   { BOTH, 2,   NCW, false, QString ("Delete"),            QString ("Delete node"),                                SLOT (deleteClicked (bool))      },
   { BOTH, 2,   NCW, false, QString ("Edit..."),           QString ("Edit node"),                                  SLOT (editClicked (bool))        },
   { BOTH, 2,   NCW, false, QString ("Sort"),              QString ("Sort node tree by PV name"),                  SLOT (sortClicked (bool))        },
   { BOTH, 2,   WCW, false, QString ("Compare"),           QString ("Compare workspaces"),                         SLOT (compareClicked (bool))     }
};


static const QString abortEnabledStyle  = QEUtilities::colourToStyle (QColor (145, 200, 255));
static const QString abortDisabledStyle = QEUtilities::colourToStyle (QColor (200, 200, 200));

//=============================================================================
// Halves
//=============================================================================
//
QEPvLoadSave::Halves::Halves (const Sides sideIn, QEPvLoadSave* ownerIn, QBoxLayout* layout)
{
   int j;
   int left;
   int top;
   int gap;
   QString iconPathName;
   QPushButton* button;

   this->side = sideIn;
   this->owner = ownerIn;

   this->container = new QFrame ();
   this->container->setFrameShape (QFrame::Panel);
   this->container->setFrameShadow (QFrame::Plain);
   // This re-parents container.
   layout->addWidget (container);

   this->halfLayout = new QVBoxLayout (this->container);
   this->halfLayout->setContentsMargins (2, 2, 2, 2);
   this->halfLayout->setSpacing (2);

   this->header = new QFrame ();
   this->header->setFrameShape (QFrame::NoFrame);
   this->header->setFrameShadow (QFrame::Plain);
   this->header->setFixedHeight (92);
   this->halfLayout->addWidget (this->header);

   // Create add header buttons.
   //
   left = 4;
   top = 2;
   for (j = 0 ; j < ARRAY_LENGTH (buttonSpecs); j++) {

      this->headerPushButtons [j] = NULL;

      // Some buttons are only on one side.
      //
      if ((this->side == LeftSide)  && (buttonSpecs[j].side == RHS)) continue;
      if ((this->side == RightSide) && (buttonSpecs[j].side == LHS)) continue;

      button = new QPushButton (this->header);

      // Set up icon or caption text.
      //
      if (buttonSpecs[j].isIcon) {
         iconPathName = ":/qe/pvloadsave/";
         iconPathName.append (buttonSpecs[j].captionOrIcon);
         button->setIcon (QIcon (iconPathName));
      } else {
         button->setText (buttonSpecs[j].captionOrIcon);
      }

      button->setFocusPolicy (Qt::NoFocus);
      button->setToolTip (buttonSpecs[j].toolTip);
      gap = buttonSpecs[j].gap;

      if (gap == NL) {
         // There is only one "newline".
         //
         if (this->side == LeftSide) {
            this->checkBox = new QCheckBox ("Show 2nd tree", this->header);
            this->checkBox->setGeometry (left + 2, top, 120, 26);
            this->checkBox->setFocusPolicy (Qt::NoFocus);

            QObject::connect (this->checkBox, SIGNAL (stateChanged (int)),
                              this->owner,    SLOT   (checkBoxStateChanged (int)));
         } else {
            this->checkBox = NULL;
         }

         left = 4;
         top += 32;
         gap = 0;
      }

      button->setGeometry (left + gap, top, buttonSpecs[j].width, 26);
      left += gap + buttonSpecs[j].width + 2;
      if (buttonSpecs[j].member != NULL) {
         QObject::connect (button,      SIGNAL (clicked (bool)),
                           this->owner, buttonSpecs [j].member);
      }

      if (j == 14) {
         button->setStyleSheet (QEUtilities::colourToStyle (QColor (155, 205, 255)));
      }
      this->headerPushButtons [j] = button;
   }

   this->macroString = new QLineEdit (this->header);
   this->macroString->setGeometry (4, top + 32, 444, 23);
   this->macroString->setToolTip (" Define macro substitions - applies to PV names, \n"
                                  " group names and values when loaded from a file. ");

   this->tree = new QTreeView (this->header);

   this->halfLayout->addWidget (this->tree);
   this->tree->setAcceptDrops (true);
   this->tree->setContextMenuPolicy (Qt::CustomContextMenu);

   QObject::connect (this->tree,  SIGNAL (customContextMenuRequested (const QPoint &)),
                     this->owner, SLOT   (treeMenuRequested          (const QPoint &)));

   this->footer = new QFrame ();
   this->footer->setFrameShape (QFrame::NoFrame);
   this->footer->setFrameShadow (QFrame::Plain);
   this->footer->setFixedHeight (40);
   this->halfLayout->addWidget (this->footer);

   // Configure basic tree setup.
   //
   this->tree->setIndentation (10);
   this->tree->setItemsExpandable (true);
   this->tree->setUniformRowHeights (true);
   this->tree->setRootIsDecorated (true);
   this->tree->setAlternatingRowColors (true);

   // Create the graphical PV value compare widget.
   // There are two, one for each side, to allow Left v. Right as well as
   // Right vs. Left. These should be same except for opposite sign.
   //
   this->graphicalCompare = new QEPvLoadSaveCompare (this->owner, this->side, NULL);

   // Create an essentially empty model.
   //
   this->model = new QEPvLoadSaveModel (this->tree, this->owner);  // not a widget

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

   this->vnpm.setVariableIndex ((int) this->side);

   // Set up a connection to recieve configuration file name property changes.
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   QObject::connect (&this->vnpm, SIGNAL (newVariableNameProperty (QString, QString, unsigned int)),
                     this->owner, SLOT   (useNewConfigurationFileProperty (QString, QString, unsigned int)));
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

   rootItem = QEPvLoadSaveUtilities::readTree (configurationFile, this->macroString->text ());
   if (!rootItem) {
       DEBUG << "file read fail " << configurationFile;
       return;
   }

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
   okay = QEPvLoadSaveUtilities::writeTree (configurationFile, rootItem);
   if (okay) {
      this->model->setHeading (configurationFile);
   }
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
   this->setMinimumSize (932, 400);

   // Set default QEFrame properties.
   //
   this->setAllowDrop (false); // applies to widget as a whole, not the tree view.
   this->setVariableAsToolTip (false);
   this->setDisplayAlarmStateOption (standardProperties::DISPLAY_ALARM_STATE_NEVER);

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

   QObject::connect (this->abortButton, SIGNAL (clicked (bool)), this, SLOT (abortClicked (bool)));

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
   this->createAction (this->treeContextMenu, "Examine Properties",   false, TCM_SHOW_PV_PROPERTIES);
   this->createAction (this->treeContextMenu, "Plot in StripChart",   false, TCM_ADD_TO_STRIPCHART);
   this->createAction (this->treeContextMenu, "Show in Scatch Pad",   false, TCM_ADD_TO_SCRATCH_PAD);
   this->treeContextMenu->addSeparator ();
   this->createAction (this->treeContextMenu, "Edit PV Name...",      false, TCM_EDIT_PV_NAME);
   this->createAction (this->treeContextMenu, "Edit PV Value...",     false, TCM_EDIT_PV_VALUE);
   this->createAction (this->treeContextMenu, "Copy variable name",   false, TCM_COPY_VARIABLE);
   this->createAction (this->treeContextMenu, "Copy data",            false, TCM_COPY_DATA);

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

   this->half [LeftSide]->checkBox->setChecked (false);
   this->checkBoxStateChanged (Qt::Unchecked);
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
bool QEPvLoadSave::eventFilter (QObject* /* obj*/ , QEvent* /* event */ )
{
   return false; // place holder - we did not handle this event
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::useNewConfigurationFileProperty (QString configurationFileIn,
                                                    QString configurationFileSubstitutionsIn,
                                                    unsigned int variableIndex)
{
   this->setVariableNameAndSubstitutions (configurationFileIn, configurationFileSubstitutionsIn, variableIndex);
}

//------------------------------------------------------------------------------
//
qcaobject::QCaObject* QEPvLoadSave::createQcaItem (unsigned int variableIndex)
{
   DEBUG << "unexpected - variableIndex =" << variableIndex;
   return NULL;
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::establishConnection (unsigned int variableIndex)
{
   QString configurationFile;

   if (variableIndex >= ARRAY_LENGTH (this->half)) {
      DEBUG << "unexpected - variableIndex =" << variableIndex;
      return;
   }

   configurationFile = this->getSubstitutedVariableName (variableIndex);
   this->half [variableIndex]->open (configurationFile);
}

//------------------------------------------------------------------------------
//
QEPvLoadSave::Sides QEPvLoadSave::objectSide (QObject* obj)
{
   int s;
   int j;

   for (s = 0; s < ARRAY_LENGTH (this->half); s++) {

      if (obj == this->half [s]->tree) {
         // found a match.
         return Sides (s);
      }

      if (obj == this->half [s]->model) {
         // found a match.
         return Sides (s);
      }

      // Check push buttons.
      //
      for (j = 0; j < ARRAY_LENGTH (this->half [s]->headerPushButtons); j++) {
         if (obj == this->half [s]->headerPushButtons [j]) {
            // found a match.
            return Sides (s);
         }
      }
   }

   DEBUG  << "no match found";
   return ErrorSide;
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
      // Is is a leaf/PV node or a gruop node?
      //
      if (this->contextMenuItem->getIsPV ()) {
         for (j = TCM_COPY_VARIABLE; j <= TCM_EDIT_PV_VALUE; j++) {
            this->actionList [j]->setVisible (true);
         }
      } else {
         this->actionList [TCM_ADD_GROUP]->setVisible (true);
         if (this->contextMenuItem != model->getRootItem ()) {
            // Renaming the 'ROOT' node prohibited.
            this->actionList [TCM_RENAME_GROUP]->setVisible (true);
         }
         this->actionList [TCM_ADD_PV]->setVisible (true);
      }

   } else
   // no item selected - is there a root item??
   //
   if (!model->getRootItem ()) {
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
   QVariant nilValue (QVariant::Invalid);
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
         /// TODO - create group name dialog - re-purposing pvNameSelectDialog here for now
         //
         this->groupNameDialog->setWindowTitle ("QEPvLoadSave - Add Group");
         this->groupNameDialog->setGroupName ("");
         n = this->groupNameDialog->exec (tree);
         if (n == 1) {
            item = new QEPvLoadSaveGroup (this->groupNameDialog->getGroupName (), NULL);
            model->addItemToModel (item, this->contextMenuItem);
         }
         break;

      case TCM_RENAME_GROUP:
         /// TODO - create group name dialog - re-purposing pvNameSelectDialog here for now
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

      case TCM_EDIT_PV_NAME:
         leaf = dynamic_cast <QEPvLoadSaveLeaf*> (this->contextMenuItem);
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
         this->editItemValue (this->contextMenuItem, this->contextMenuHalf, tree);
         break;

      case TCM_COPY_VARIABLE:
         leaf = dynamic_cast <QEPvLoadSaveLeaf*> (this->contextMenuItem);
         if (leaf) {   // sanity check
            QApplication::clipboard ()->setText (leaf->copyVariables());
         }
         break;

      case TCM_COPY_DATA:
         nodeValue = this->contextMenuItem->getNodeValue ();

         // Need be aware of lists.
         if (nodeValue.type() == QVariant::List) {
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
         leaf = dynamic_cast <QEPvLoadSaveLeaf*> (this->contextMenuItem);
         if (leaf) {   // sanity check
            emit this->requestAction (QEActionRequests (QEActionRequests::actionPvProperties (), leaf->copyVariables()));
         }
         break;

      case TCM_ADD_TO_STRIPCHART:
         leaf = dynamic_cast <QEPvLoadSaveLeaf*> (this->contextMenuItem);
         if (leaf) {   // sanity check
            emit this->requestAction (QEActionRequests (QEActionRequests::actionStripChart (), leaf->copyVariables()));
         }
         break;

      case TCM_ADD_TO_SCRATCH_PAD:
         leaf = dynamic_cast <QEPvLoadSaveLeaf*> (this->contextMenuItem);
         if (leaf) {   // sanity check
            emit this->requestAction (QEActionRequests (QEActionRequests::actionScratchPad (), leaf->copyVariables()));
         }
         break;

      default:
         DEBUG << "Unexpected action: " << menuAction;
         break;
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::editItemValue (QEPvLoadSaveItem* item, Halves* half, QWidget* centerOver)
{
   if (!item) return;
   if (!half) return;

   // Can only edit PV values.
   //
   if (!item->getIsPV ()) return;

   this->valueEditDialog->setPvName (item->getNodeName ());
   this->valueEditDialog->setValue (item->getNodeValue ());
   int n = this->valueEditDialog->exec (centerOver);
   if (n == 1) {
      item->setNodeValue (this->valueEditDialog->getValue ());
      half->model->modelUpdated ();
   }
}


//==============================================================================
// Button and box signal functions
//
void QEPvLoadSave::checkBoxStateChanged (int state)
{
   const bool selected = (state == Qt::Checked);
   this->half [RightSide]->container->setVisible (selected);

   // Increase/Decrease minimum width
   int mw = this->minimumWidth ();
   this->setMinimumWidth (selected ? (mw*2) : (mw/2));
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
      if (this->pvWriteIsPermitted ()) {
         this->loadSaveAction = "Apply";
         this->progressBar->setMaximum (MAX (1, number));
         this->progressBar->setValue (0);
         this->abortButton->setStyleSheet (abortEnabledStyle);
         this->abortButton->setEnabled (true);
         model->applyPVData ();
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
         item->applyPVData ();
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
   if (!model) return;  // sainity check

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
//
void QEPvLoadSave::editClicked (bool)
{
   VERIFY_SENDER;

   QEPvLoadSaveItem* item = this->half [side]->model->getSelectedItem ();

   if (item && item->getIsPV ()) {
      QWidget* centreOver = dynamic_cast <QWidget*> (this->sender ());
      this->editItemValue (item, this->half [side], centreOver);
   }
}

//------------------------------------------------------------------------------
//
void QEPvLoadSave::sortClicked (bool)
{
   VERIFY_SENDER;
   DEBUG << side;
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
                                     QEActionRequests::OptionFloatingDockWindow,
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
   this->half [0]->model->abortAction ();
   this->half [1]->model->abortAction ();

   QString title = this->loadSaveAction + " failures";
   if (this->hostSlotAvailable) {
      // If the graphicalCompare widget already visible - skip this part
      //
      if (!this->accessFail->isVisible()) {
         // Create component item and associated request.
         //
         componentHostListItem item (this->accessFail,
                                     QEActionRequests::OptionFloatingDockWindow,
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
