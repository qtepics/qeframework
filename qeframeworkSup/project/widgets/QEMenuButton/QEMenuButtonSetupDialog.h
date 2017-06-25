/*  QEMenuButtonSetupDialog.h
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
 *  Copyright (c) 2015,2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
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

   QEMenuButtonItem* selectedItem;    // saved on selection change
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

   void editReturnPressed ();
   void on_buttonBox_accepted ();
   void on_buttonBox_rejected ();
};

#endif // QE_MENU_BUTTON_SETUP_DIALOG_H
