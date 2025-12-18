/*  QEScratchPad.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2013-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_SCRATCH_PAD_H
#define QE_SCRATCH_PAD_H

#include <QMenu>
#include <QObject>
#include <QPushButton>
#include <QTimer>
#include <QVariant>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>

#include <QEEnums.h>
#include <QEArchiveAccess.h>
#include <QEQuickSort.h>
#include <QEAbstractDynamicWidget.h>
#include <QELabel.h>
#include <QCaVariableNamePropertyManager.h>
#include <persistanceManager.h>

#include <QEPVNameSelectDialog.h>
#include "QEScratchPadMenu.h"

/// This class provides a flexible scratch pad form, to which any Process Variable
/// may be added. It displays the PV Name, the Description, i.e. the content of
/// the DESC field together with the value of PV itself.
///
/// This class is a direct re-implementation of TScratch_Pad_Form out of the
/// Delphi OPI framework.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEScratchPad : 
   public QEAbstractDynamicWidget, 
   protected QEQuickSort 
{
   Q_OBJECT
public:
   static const int NUMBER_OF_ITEMS = 72;

   explicit QEScratchPad (QWidget* parent = 0);
   ~QEScratchPad ();
   QSize sizeHint () const;

   // Set (and clear if pvName is null).
   //
   void    setPvName (const int slot, const QString& pvName);
   QString getPvName (const int slot) const;

   int addPvName (const QString& pvName);
   void clearAllPvNames ();

   // Selects/highlights row.
   //
public slots:
   void setSelection (int value);

public:
   int getSelection () const;

signals:
   void selectionChanged (int value);

   // Set, get and emit set of active PV names.
   //
public slots:
   void setPvNameSet (const QStringList& pvNameSet);

public:
   QStringList getPvNameSet () const;

signals:
   void pvNameSetChanged (const QStringList& nameSet);

protected:
   // Override super class functions.
   //
   void resizeEvent (QResizeEvent* event);
   void activated ();

   QMenu* buildContextMenu ();                        // Build the Scratch Pad specific context menu
   void contextMenuTriggered (int selectedItemNum);   // An action was selected from the context menu

   // Override QEAbstractDynamicWidget functions.
   //
   void enableEditPvChanged ();

   // Override QEQuickSort functions.
   //
   bool itemLessThan (const int a, const int b, QObject* context = NULL) const;
   void swapItems (const int a, const int b, QObject* context = NULL);

   // Drag and Drop - no drop to self.
   //
   void mousePressEvent (QMouseEvent *event)    { qcaMousePressEvent (event); }
   void dragEnterEvent (QDragEnterEvent *event) { qcaDragEnterEvent (event);  }
   void dragMoveEvent (QDragMoveEvent *event);  // we do a special to avoid self drop.
   void dropEvent (QDropEvent *event)           { qcaDropEvent (event, true); }
   // This widget uses the setDrop/getDrop defined in QEWidget.

   // Copy/Paste
   //
   QString copyVariable ();
   QVariant copyData ();
   void paste (QVariant s);

   // override other virtual functions
   //
   void saveConfiguration (PersistanceManager* pm);
   void restoreConfiguration (PersistanceManager* pm, restorePhases restorePhase);

   void showEvent (QShowEvent* event);
   void keyPressEvent (QKeyEvent* event);
   bool eventFilter (QObject* obj, QEvent* event);

   int findSlot (QObject *obj) const;

private:
   // Internal widgets.
   //
   QVBoxLayout* vLayout;
   QFrame* titleFrame;
   QLabel* titlePvName;
   QLabel* titleRecordType;
   QLabel* titleDescription;
   QLabel* titleValue;
   QPushButton* loadButton;
   QPushButton* saveButton;

   QScrollArea* scrollArea;
   QWidget*     scrollContents;
   QVBoxLayout* scrollLayout;   // manages DataSets::frame items in scrollContents

   QEArchiveAccess* archiveAccess;
   QEPVNameSelectDialog* pvNameSelectDialog;

   int selectedItem;
   bool emitSelectionChangeInhibited;
   bool emitPvNameSetChangeInhibited;

   class DataSets {
   public:
      explicit DataSets ();
      ~DataSets ();

      bool isInUse () const;
      void setHighLighted (const bool isHigh);

      QString thePvName;
      bool isHighLighted;

      // Widgets.
      //
      QHBoxLayout* hLayout;
      QFrame* frame;
      QLabel* pvName;
      QELabel* recordType;
      QELabel* description;
      QELabel* value;
   };

   // We keep menus and items in separate arrays. Items may be swapped, however
   // menu items are fixed, always associated with the same slot position.
   //
   DataSets* items [NUMBER_OF_ITEMS];

   void createInternalWidgets ();
   void setSelectItem (const int slot, const bool toggle);
   int numberSlotsUsed () const;  // includes cleared slots
   void calcMinimumHeight ();

   // Perform a pvNameDropEvent 'drop' when dropped onto internal widget.
   //
   void pvNameDropEvent (const int slot, QDropEvent* event);

private slots:
   void intialResize ();
   void contextMenuRequested (const QPoint& pos);
   void contextMenuSelected  (const int slot, const QEScratchPadMenu::ContextMenuOptions option);
};

#endif // QE_SCRATCH_PAD_H
