/*  QEMenuButtonArgumentsDialog.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2023-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
