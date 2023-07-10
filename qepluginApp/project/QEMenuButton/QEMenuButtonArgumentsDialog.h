/*  QEMenuButtonArgumentsDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2023 Australian Synchrotron
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
 *    andrews@ansto.gov.au
 */

#ifndef QE_MENU_BUTTON_ARGUMENTS_DIALOG_H
#define QE_MENU_BUTTON_ARGUMENTS_DIALOG_H

#include <QObject>
#include <QWidget>
#include <QListWidget>
#include <QStringList>

#include <QEDialog.h>
#include <QEPluginLibrary_global.h>

// Differed declaration.
//
namespace Ui {
   class QEMenuButtonArgumentsDialog;
}

// This class essentially tries to duplicate the designer QStringList
// property editor. While designed as a dialog for argument lists, it
// would work for any QStringList
//
class QEPLUGINLIBRARYSHARED_EXPORT QEMenuButtonArgumentsDialog : public QEDialog
{
   Q_OBJECT

public:
   explicit QEMenuButtonArgumentsDialog (QWidget* parent);
   ~QEMenuButtonArgumentsDialog ();

   void setArgumentList (const QStringList& argumentList);
   QStringList getArgumentList () const;

protected:
   void resizeEvent (QResizeEvent *event);

private:
   Ui::QEMenuButtonArgumentsDialog* ui;
   QListWidget* listWidget;   // an alias

   QStringList argumentList;  // ths list of arguments

   void widgetsEnable();
   void loadArgumentList ();

private slots:
   void setArgumentListPart2 ();

   void rowSelectionChanged ();
   void scrollValueChanged (int);
   void textEdited (const QString &);

   void onNewItemClick (bool);
   void onDeleteItemClick (bool);
   void onScrollUpClick (bool);
   void onScrollDownClick (bool);

   // These conection are made by QDialog and associates.
   //
   void on_buttonBox_rejected ();
   void on_buttonBox_accepted ();
};

# endif // QE_MENU_BUTTON_ARGUMENTS_DIALOG_H
