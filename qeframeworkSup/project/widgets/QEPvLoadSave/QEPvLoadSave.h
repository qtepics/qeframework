/*  QEPvLoadSave.h
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
 *  Copyright (c) Australian Synchrotron 2013,2016,2017
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_PV_LOAD_SAVE_H
#define QE_PV_LOAD_SAVE_H

#include <QAction>
#include <QMenu>
#include <QPoint>
#include <QString>
#include <QAbstractItemModel>
#include <QLineEdit>
#include <QTreeView>
#include <QItemSelectionModel>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QProgressBar>

#include <QCaObject.h>
#include <QEFrame.h>
#include <QEActionRequests.h>
#include <QEWidget.h>
#include <QEPluginLibrary_global.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEPVNameSelectDialog.h>
#include <QEPvLoadSaveGroupNameDialog.h>
#include <QEPvLoadSaveValueEditDialog.h>
#include <QEPvLoadSaveTimeDialog.h>

#include "QEPvLoadSaveCommon.h"

/// This class provides the means to create/manages prescribed set of PVs and
/// their associated values, but more importantly the capability to:
/// a) read the current value for each PV from the system (via Channel Access);
/// b) write the set of names/values to a file;
/// c) read a the set of names/values from a file; and
/// d) wite the values to the associated PV (via Channel Access).
///
/// Note: This widget is intented to be use as the sole widget of a built in QeGUI form
/// However, the widget may be used in any form if needs be.
///

// Differed declaration - avoids mutual header inclusions.
//
class QEPvLoadSaveAccessFail;
class QEPvLoadSaveCompare;
class QEPvLoadSaveItem;
class QEPvLoadSaveModel;

class QEPLUGINLIBRARYSHARED_EXPORT QEPvLoadSave : public QEFrame  {

   Q_OBJECT
   // QEPvLoadSave specific properties ==============================================
   //
public:
   /// configurationFile
   ///
   Q_PROPERTY (QString configurationFileLeft  READ getConfigurationFileLeft  WRITE setConfigurationFileLeft)
   Q_PROPERTY (QString configurationFileRight READ getConfigurationFileRight WRITE setConfigurationFileRight)

   /// Macro substitutions. The default is no substitutions. The format is NAME1=VALUE1[,] NAME2=VALUE2...
   /// Values may be quoted strings. For example, 'PUMP=PMP3, NAME = "My Pump"'
   /// These substitutions are applied to variable names for all QE widgets.
   /// In some widgets are are also used for other purposes.
   ///
   Q_PROPERTY (QString defaultSubstitutions READ getSubstitutions WRITE setSubstitutions)

   /// Default directory used for loading/saving files. Default to null string which is
   /// interpreted as the current directory.
   ///
   Q_PROPERTY (QString defaultDir           READ getDefaultDir    WRITE setDefaultDir)

   /// If true, a dialog will be presented asking the user to confirm if the PV write actions
   /// should be carried out. Defaults to true.
   ///
   Q_PROPERTY (bool confirmAction READ getConfirmAction WRITE setConfirmAction)

   //
   // End of QEPvLoadSave specific properties =====================================


   // Standard properties are inherited from QEFrame.
   // variableAsToolTip and displayAlarmState prob not sensible, but part of standard set.
   //
public:
   /// Create without a nominated config file.
   ///
   QEPvLoadSave (QWidget *parent = 0);

   /// Destruction
   virtual ~QEPvLoadSave ();

   /// Size hint
   virtual QSize sizeHint () const;

   // Property access functions.
   //
   // We use same mechanism that is used to manage variable names to manage the configuration file name.
   //
   void    setConfigurationFileLeft (QString configurationFile)  { this->half [0]->setConfigurationFile (configurationFile); }
   QString getConfigurationFileLeft ()                    { return this->half [0]->getConfigurationFile (); }

   void    setConfigurationFileRight (QString configurationFile) { this->half [1]->setConfigurationFile (configurationFile); }
   QString getConfigurationFileRight ()                   { return this->half [1]->getConfigurationFile (); }

   void    setSubstitutions (QString configurationFileSubstitutions);
   QString getSubstitutions ();

   void setDefaultDir (const QString& defaultDir);
   QString getDefaultDir () const;

   // confirm
   void setConfirmAction (bool confirmRequiredIn );
   bool getConfirmAction () const;

   // Used internally but needs to be public.
   static const int NumberOfButtons = 15;

signals:
    void requestAction (const QEActionRequests&);       // Signal 'launch a GUI'

protected:
   // We don't expect these to be called - but do override and output debug error
   //
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);
   void establishConnection (unsigned int variableIndex);

   void resizeEvent (QResizeEvent* );
   bool eventFilter (QObject *obj, QEvent* event);

   // No Drag and Drop yet
   // No copy paste yet

private:
   enum TreeContextMenuActions {
      TCM_CREATE_ROOT,
      TCM_ADD_GROUP,
      TCM_RENAME_GROUP,
      TCM_ADD_PV,

      // This group similar to, but not same as, a regular widget.
      TCM_COPY_VARIABLE,
      TCM_COPY_DATA,
      TCM_SHOW_PV_PROPERTIES,
      TCM_ADD_TO_STRIPCHART,
      TCM_ADD_TO_SCRATCH_PAD,

      TCM_EDIT_PV_NAME,
      TCM_EDIT_PV_VALUE,
      TCM_NUMBER           // must be last
   };

   typedef QAction* TreeContextMenuActionLists [TCM_NUMBER];

   enum Sides { LeftSide = 0, RightSide = 1, ErrorSide = 2 };

   class Halves {
   public:
      Halves (const Sides side, QEPvLoadSave* owner, QBoxLayout* layout);

      // Used to store/access properties.
      //
      void setConfigurationFile  (const QString& configurationFile);
      QString getConfigurationFile ();

      void setConfigurationSubstitutions (const QString& substitutions);
      QString getConfigurationSubstitutions ();

      void setRoot (QEPvLoadSaveItem* rootItem, const QString& heading);
      void open (const QString& configurationFile);
      void save (const QString& configurationFile);

      QFrame* container;
      QVBoxLayout* halfLayout;
      QFrame* header;
      QCheckBox* checkBox;
      QPushButton *headerPushButtons [NumberOfButtons];

      QLineEdit* macroString;
      QTreeView* tree;                        // the tree widget
      QFrame* footer;

      QEPvLoadSaveModel* model;               // manages tree data
      QCaVariableNamePropertyManager vnpm;    // manages filenames
      QEPvLoadSaveCompare* graphicalCompare;

   private:
      Sides side;
      QEPvLoadSave* owner;
   };

   QVBoxLayout* overallLayout;
   QFrame* sidesFrame;
   QHBoxLayout* sideBySidelayout;
   Halves* half [2];  // two halves make a whole ;-)

   QString loadSaveAction;
   QFrame* loadSaveStatusFrame;
   QLabel* loadSaveTitle;
   QProgressBar* progressBar;
   QLabel* progressStatus;
   QPushButton* abortButton;
   QEPvLoadSaveAccessFail* accessFail;

   QEPvLoadSaveGroupNameDialog* groupNameDialog;
   QEPvLoadSaveValueEditDialog* valueEditDialog;
   QEPVNameSelectDialog* pvNameSelectDialog;
   QEPvLoadSaveTimeDialog* archiveTimeDialog;

   QMenu* treeContextMenu;
   TreeContextMenuActionLists actionList;
   QString defaultDir;
   bool confirmRequired;

   // Only meaningfull for context menu processing, i.e. after treeMenuRequested
   // called and up until treeMenuSelected.
   //
   Halves* contextMenuHalf;
   QEPvLoadSaveItem* contextMenuItem;

   // Use the objectSide object to determine which side sent the signal.
   //
   Sides objectSide (QObject* obj);
   Halves* halfAssociatedWith (QObject* obj);
   bool hostSlotAvailable;

   void editItemValue (QEPvLoadSaveItem* item, Halves* half, QWidget* centerOver);

   // Utility function to create and set up an action.
   //
   QAction* createAction (QMenu *parent,
                          const QString &caption,
                          const bool checkable,
                          const TreeContextMenuActions treeAction);

   void setReadOut (const QString& text);

   // Utility function get write confirmation if needed.
   //
   bool pvWriteIsPermitted ();

   friend class QEPvLoadSaveCompare;
   friend class QEPvLoadSaveModel;

private slots:
   void useNewConfigurationFileProperty (QString configurationFileIn,
                                         QString configurationFileSubstitutionsIn,
                                         unsigned int variableIndex );

   void acceptActionComplete (const QEPvLoadSaveItem*, const QEPvLoadSaveCommon::ActionKinds, const bool);
   void acceptActionInComplete (const QEPvLoadSaveItem*, const QEPvLoadSaveCommon::ActionKinds);

   void treeMenuRequested (const QPoint& pos);
   void treeMenuSelected  (QAction* action);

   void checkBoxStateChanged (int);
   void writeAllClicked (bool);
   void readAllClicked (bool);
   void writeSubsetClicked (bool);
   void readSubsetClicked (bool);
   void archiveTimeClicked (bool);
   void copyAllClicked (bool);
   void copySubsetClicked (bool);
   void loadClicked (bool);
   void saveClicked (bool);
   void deleteClicked (bool);
   void editClicked (bool);
   void sortClicked (bool);
   void compareClicked (bool);
   void abortClicked (bool);
};

#endif // QE_PV_LOAD_SAVE_H
