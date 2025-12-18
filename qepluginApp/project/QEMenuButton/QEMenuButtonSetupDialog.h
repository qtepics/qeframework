/*  QEMenuButtonSetupDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2015-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_MENU_BUTTON_SETUP_DIALOG_H
#define QE_MENU_BUTTON_SETUP_DIALOG_H

#include <QAction>
#include <QDialog>
#include <QComboBox>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QList>
#include <QPoint>
#include <QEPluginLibrary_global.h>

// Differed declaration.
//
namespace Ui {
   class QEMenuButtonSetupDialog;
}

class QEMenuButton;
class QEMenuButtonModel;
class QEMenuButtonItem;
class QEMenuButtonArgumentsDialog;

class QEPLUGINLIBRARYSHARED_EXPORT QEMenuButtonSetupDialog : public QDialog
{
   Q_OBJECT

public:
   explicit QEMenuButtonSetupDialog (QEMenuButton* owner, QWidget* parent);
   ~QEMenuButtonSetupDialog ();

private:
   Ui::QEMenuButtonSetupDialog* ui;
   QEMenuButton* owner;
   QEMenuButtonModel* model;
   QItemSelectionModel* treeSelectionModel;   // manages tree selections
   QMenu* treeContextMenu;

   QList<QLineEdit*> lineEditList;
   QList<QComboBox*> comboBoxList;
   QList<QPushButton*> resetButtonList;
   QEMenuButtonArgumentsDialog* argumentsDialog;

   QEMenuButtonItem* selectedItem;       // saved on selection change
   QEMenuButtonItem* contextMenuItem;    // saved on treeMenuRequested
   bool acceptIsInhibited;
   bool returnIsMasked;

   void itemSelected (QEMenuButtonItem* item);

private slots:
   void treeMenuRequested (const QPoint& pos);
   void treeMenuSelected  (QAction* action);
   void selectionChanged (const QItemSelection& selected, const QItemSelection& deselected);

   void stateChanged (int state);
   void textEdited (const QString& newText);
   void comboBoxActivated (int index);
   void resetButtonClicked (bool);
   void onEditProgramArguments (bool);

   void editReturnPressed ();
   void on_buttonBox_accepted ();
   void on_buttonBox_rejected ();
};

#endif // QE_MENU_BUTTON_SETUP_DIALOG_H
