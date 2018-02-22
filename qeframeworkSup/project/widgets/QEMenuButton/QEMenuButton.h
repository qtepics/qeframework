/*  QEMenuButton.h
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
 *  Copyright (c) 2015,2017,2018 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_MENU_BUTTON_H
#define QE_MENU_BUTTON_H

#include <QAction>
#include <QColor>
#include <QHBoxLayout>
#include <QMetaType>
#include <QMenu>
#include <QPushButton>
#include <QSize>
#include <QString>

#include <QCaObject.h>
#include <QECommon.h>
#include <QEActionRequests.h>
#include <QEAbstractWidget.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QEWidget.h>

/// This widget provides a menu attached to a button.
/// Each menu/submenu can tigger same items as a QEPushButton, i.e:
/// a) write a value to a PV (click only); and/or
/// b) run a nominated program (with arguments); and/or
/// c) open a specified ui file.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEMenuButton : public QEAbstractWidget {
   Q_OBJECT
public:
   // Some standard properties are really not applicable to this widget.
   // These are re-declared as DESIGNABLE false.
   //
   Q_PROPERTY (bool allowDrop         READ getAllowDrop         WRITE setAllowDrop         DESIGNABLE false)
   Q_PROPERTY (bool variableAsToolTip READ getVariableAsToolTip WRITE setVariableAsToolTip DESIGNABLE false)
   Q_PROPERTY (DisplayAlarmStateOptions displayAlarmStateOption
               READ getDisplayAlarmStateOptionProperty
               WRITE setDisplayAlarmStateOptionProperty DESIGNABLE false)

   // start of QEMenuButton specific properties.
   //
   /// Default macro substitutions. The default is no substitutions. The format is NAME1=VALUE1[,] NAME2=VALUE2...
   /// Values may be quoted strings. For example, 'PUMP=PMP3, NAME = "My Pump"'
   /// These substitutions are applied to progran names and arguments etc.
   //  Unlike most widgets we skip the property manager and associated 1 second delay
   //  here as such changes do not cause numerous channel connections.
   //
   Q_PROPERTY (QString defaultSubstitutions READ getSubstitutionsProperty WRITE setSubstitutionsProperty)

   /// Label Text
   Q_PROPERTY (QString labelText READ getLabelTextProperty WRITE setLabelTextProperty)

   // This property stores the user info.
   // The "Edit User Info..." context menu item must be used to edit this.
   /// Specifies the menu entry values, encoded and an XML string.
   //
   Q_PROPERTY (QString menuEntries READ getMenuString WRITE setMenuString)
   //
   // end QEMenuButton specific properties.

public:
   explicit QEMenuButton (QWidget* parent = 0);
   virtual ~QEMenuButton ();

   void setSubstitutionsProperty (const QString& substitutions);
   QString getSubstitutionsProperty () const;

   // Label text must be mapped to the button
   void setLabelTextProperty (const QString& labelText);
   QString getLabelTextProperty () const;

   void setMenuString (const QString& s);
   QString getMenuString () const;

signals:
   /// Internal use only. Request a new GUI is created. Typically, this is caught by the QEGui application.
   void newGui (const QEActionRequests& request);

protected:
   QSize sizeHint () const;
   bool eventFilter (QObject* watched, QEvent* event);

   QMenu* buttonMainMenu;
   QString theMenuString;

private:
   QPushButton* button;    // internal menu button widget
   QHBoxLayout* layout;    // holds the button widget - any layout type will do

   void resetMenuString ();
   void writeToVariable (qcaobject::QCaObject* qca);

private slots:
   void menuTriggered (QAction *action);
   void programCompletedSlot ();

   void connectionChanged (QCaConnectionInfo& connectionInfo,
                           const unsigned int& variableIndex);

   /// Default slot used to create a new GUI if there is no slot indicated in the
   /// ContainerProfile class. This slot is typically used when the button menu item
   /// is selected within the Designer preview window to allow the operation of
   /// the button to be tested. If an application does not specify a slot to use
   /// for creating new windows (through the ContainerProfile class) a window will
   /// still be created through this slot, but it will not respect the window
   /// creation options or any other window related application constraints.
   /// For example, the QEGui application does provide a slot for creating new GUIs
   /// in the ContainerProfile class which respects the creation options, knows
   /// how to add tabs in the application, and extend the application's window
   /// menu in the menu bar.
   ///
   void requestAction (const QEActionRequests& request);
};

#endif           // QE_MENU_BUTTON_H
