/*  QRadioGroup.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2014  Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef Q_RADIO_GROUP_H
#define Q_RADIO_GROUP_H

#include <QGridLayout>
#include <QList>
#include <QString>
#include <QStringList>
#include <QAbstractButton>
#include <QSize>
#include <QGroupBox>

#include <QEPluginLibrary_global.h>
#include <QEOneToOne.h>

class QEPLUGINLIBRARYSHARED_EXPORT QRadioGroup : public QGroupBox {

Q_OBJECT
public:
   /// Enumrations values used to select the button style.
   /// Whereas check box buttons can/do work, this option not provided as check
   /// boxes are not assoicated with the radio button, i.e. one and only one
   /// selected, paradigm.
   //
   enum ButtonStyles { Radio,      ///< Use radio buttons - the default
                       Push };     ///< Use push buttons.

   Q_ENUMS (ButtonStyles)

   /// Enumrations values used to select the button order.
   //
   enum ButtonOrders { rowMajor,    ///< row by row button order - the default
                       colMajor };  ///< col by col button order

   Q_ENUMS (ButtonOrders)

   Q_PROPERTY (QRadioGroup::ButtonStyles buttonStyle READ getButtonStyle  WRITE setButtonStyle)
   Q_PROPERTY (QRadioGroup::ButtonOrders buttonOrder READ getButtonOrder  WRITE setButtonOrder)
   Q_PROPERTY (int columns                           READ getColumns      WRITE setColumns)
   Q_PROPERTY (int spacing                           READ getSpacing      WRITE setSpacing)
   Q_PROPERTY (QStringList strings                   READ getStrings      WRITE setStrings)   // max 256 strings.
   Q_PROPERTY (int value                             READ getValue        WRITE setValue)

public:
   /// Create with default title.
   ///
   explicit QRadioGroup (QWidget* parent = 0);

   /// Create with a group title.
   ///
   explicit QRadioGroup (const QString& title, QWidget* parent = 0);

   /// Destruction
   virtual ~QRadioGroup() { }

   int getMaximumButtons () const;

   // Property setters and getters
   //
public slots:
   void setValue (const int value);  // use -1 for no selection.
public:
   int getValue () const;

   void setStrings (const QStringList& strings);
   QStringList getStrings () const;

   void setColumns (int columns);
   int getColumns () const;

   void setSpacing (int spacing);
   int getSpacing () const;

   void setButtonStyle (const ButtonStyles buttonStyle);
   ButtonStyles getButtonStyle () const;

   void setButtonOrder (const ButtonOrders buttonOrder);
   ButtonOrders getButtonOrder () const;

signals:
   void valueChanged (const int value);

protected:
   QSize sizeHint () const;

private:
   typedef QList<QAbstractButton*> QAbstractButtonList;
   typedef QEOneToOne<int, QAbstractButton*> ValueButtonAssoications;

   ValueButtonAssoications valueToButton;
   QGridLayout* buttonLayout;
   QAbstractButtonList buttonList;
   QAbstractButton *noSelectionButton;

   QStringList strings;

   int currentIndex;
   int numberDisplayed;    // number of displayed buttons.
   int rows;
   int cols;
   int space;
   ButtonStyles buttonStyle;
   ButtonOrders buttonOrder;
   bool emitValueChangeInhibited;  // inhibits valueChanged signal when true

   void internalSetValue (const int value);
   QAbstractButton* createButton (QWidget* parent);
   void reCreateAllButtons ();
   void commonSetup ();
   void setButtonText ();
   void setButtonLayout ();

private slots:
   void buttonClicked (bool checked);
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QRadioGroup::ButtonStyles)
Q_DECLARE_METATYPE (QRadioGroup::ButtonOrders)
#endif

#endif // Q_RADIO_GROUP_H
