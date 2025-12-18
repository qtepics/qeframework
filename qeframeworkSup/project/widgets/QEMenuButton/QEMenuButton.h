/*  QEMenuButton.h
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

#ifndef QE_MENU_BUTTON_H
#define QE_MENU_BUTTON_H

#include <QAction>
#include <QColor>
#include <QHBoxLayout>
#include <QIcon>
#include <QMetaType>
#include <QMenu>
#include <QPushButton>
#include <QSize>
#include <QString>

#include <QECommon.h>
#include <QEEnums.h>
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
   Q_PROPERTY (QE::DisplayAlarmStateOptions displayAlarmStateOption
               READ getDisplayAlarmStateOption
               WRITE setDisplayAlarmStateOption DESIGNABLE false)

   // start of QEMenuButton specific properties.
   //
   /// Default macro substitutions. The default is no substitutions.
   /// The format is NAME1=VALUE1[,] NAME2=VALUE2...
   /// Values may be quoted strings. For example, 'PUMP=PMP3, NAME = "My Pump"'
   /// These substitutions are applied to progran names and arguments etc.
   //  Unlike most widgets we skip the property manager and associated 1 second
   //  delay here as such changes do not cause numerous channel connections.
   //
   Q_PROPERTY (QString defaultSubstitutions READ  getSubstitutionsProperty
                                            WRITE setSubstitutionsProperty)

   /// Button label text.
   Q_PROPERTY (QString labelText READ getButtonTextProperty WRITE setButtonTextProperty)

   ///
   Q_PROPERTY (QIcon icon     READ getIcon WRITE setIcon)
   Q_PROPERTY (QSize iconSize READ getIconSize WRITE setIconSize)

   // This property stores the user info.
   // The "Edit User Info..." context menu item must be used to edit this.
   /// Specifies the menu entry values, encoded as an XML string.
   //
   Q_PROPERTY (QString menuEntries READ getMenuString WRITE setMenuString)
   //
   // end QEMenuButton specific properties.

public:
   explicit QEMenuButton (QWidget* parent = 0);
   virtual ~QEMenuButton ();

   void setSubstitutionsProperty (const QString& substitutions);
   QString getSubstitutionsProperty () const;

   // Label text must be mapped to the button.
   // Note: currently, any substitutions apply to the menu items only, not
   // the button text.
   //
   void setButtonTextProperty (const QString& buttonText);
   QString getButtonTextProperty () const;

   void setIcon (const QIcon& icon);
   QIcon getIcon () const;

   void setIconSize (const QSize& size);
   QSize getIconSize () const;

   void setMenuString (const QString& s);
   QString getMenuString () const;

signals:
   /// Internal use only. Request a new GUI is created. Typically, this is caught
   /// by the QEGui or bespoke display manager application's requestAction slot.
   void newGui (const QEActionRequests& request);

protected:
   QSize sizeHint () const;
   bool eventFilter (QObject* watched, QEvent* event);

   QMenu* buttonMainMenu;
   QString theMenuString;

private:
   QPushButton* button;    // internal menu button widget
   QHBoxLayout* layout;    // holds the button widget - any layout type will do
   QString buttonText;     // fixed un-subsitituted text.

   void resetMenuString ();

private slots:
   void menuTriggered (QAction *action);
   void programCompletedSlot ();

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
