/*  PeriodicDialog.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2011-2018 Australian Synchrotron
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
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
  This class manages the presenation of a periodic table and the selection of an
  element in the table by a user.
  It is used by the QEPeriodic widget.
  The QEPeriodic widget appears as a push button displaying an element name.
  When the QEPeriodic widget is pressed this dialog is presented with the focus on the
  button in the periodic table for the current element.
  When a button for an element in the periodic table is pressed the element symbol is
  saved ready for recall by the QEPeriodic widget.
 */

#include "PeriodicDialog.h"
#include "ui_PeriodicDialog.h"
#include <QtDebug>
#include <QECommon.h>
#include <QEPeriodic.h>

#define DEBUG qDebug() << "PeriodicDialog" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
// Create the periodic selection dialog
PeriodicDialog::PeriodicDialog(QWidget *parent) :
    QEDialog(parent),
    m_ui(new Ui::PeriodicDialog)
{
    m_ui->setupUi( this );

    // Ideally we would construct this dynamically using the QEPeriodic::elementInfo
    // table just as it is done for the designer user setup form. But for now,
    // we will go with what we have.
    //
    map.clear ();

    // Find all the push buttons
    QList<QPushButton *> allPButtons = this->findChildren<QPushButton *>();

    for( int i = 0; i < allPButtons.size(); i++ )
    {
        // Search for buttons using button text.
        //
        QPushButton* button = allPButtons[i];
        QString buttonText = button->text();

        for( int j = 0; j < ARRAY_LENGTH( QEPeriodic::elementInfo ) ; j++ ) {
           const QEPeriodic::elementInfoStruct* info = &QEPeriodic::elementInfo [j];

           if( buttonText.compare( info->symbol) == 0 ) {
              // We have a match
              //
              if( map.containsF (j)) {
                 DEBUG << "duplicate symbol" << buttonText << "slot" << j;
              } else {
                 map.insertF (j, button);

                 QObject::connect(button, SIGNAL (clicked()),
                                  this,   SLOT   (noteElementSelected()));

                 button->setToolTip (QString(" %1 %2 %3 ")
                                     .arg(info->number)
                                     .arg (info->symbol)
                                     .arg(info->name));

                 button->setMouseTracking ( true );
                 button->installEventFilter( this );

              }
              break;
           }
        }
    }

    selectedAtomicNumber = 0;
    elementSelected = "";
}

//------------------------------------------------------------------------------
// Delete the periodic selection dialog
PeriodicDialog::~PeriodicDialog()
{
    map.clear();
    delete m_ui;
}

//------------------------------------------------------------------------------
// Ensure last selected items are undefined.
int PeriodicDialog::exec( QWidget* targetWidget )
{
   // Clear selection
   //
   selectedAtomicNumber = 0;
   elementSelected = "";
   return QEDialog::exec( targetWidget );    // call parent
}

//------------------------------------------------------------------------------
// ???
void PeriodicDialog::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        m_ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

//------------------------------------------------------------------------------
// Return the symbolic name of the element last selected
// Used to determine what element was selected by the user.
QString PeriodicDialog::getElement() const
{
    return elementSelected;
}

//------------------------------------------------------------------------------
// Return the atomic number of the element last selected
// Used to determine what element was selected by the user.
int PeriodicDialog::getAtomicNumber() const
{
    return selectedAtomicNumber;
}


//------------------------------------------------------------------------------
// Set the focus to the push button for a an element with the specified symbol
// This is used when presenting the dialog so the dialog has focus on the element
// currently selected.
void PeriodicDialog::setElement( QString elementIn,
                                 QList<bool>& enabledList )
{
    for( int j = 0; j < ARRAY_LENGTH( QEPeriodic::elementInfo ) ; j++ ) {
        const QEPeriodic::elementInfoStruct* info = &QEPeriodic::elementInfo [j];
        QPushButton* button = map.valueF( j, NULL );
        if( !button ) continue;

        const bool itemEnabled = enabledList.value( j, false );
        button->setEnabled( itemEnabled );

        if( itemEnabled && elementIn == info->symbol ) {
           button->setFocus();
        }
    }
}

//------------------------------------------------------------------------------
// deprecated
void PeriodicDialog::setElement( QString elementIn,
                                 QList<bool>& enabledList,
                                 QList<QString>& /* elementList */ )
{
    setElement( elementIn, enabledList );
}


//------------------------------------------------------------------------------
// Save the element symbol for the widget with focus (if it is a push button)
// This is used by all the 'clicked' slots for all the element buttons
void PeriodicDialog::noteElementSelected()
{
    QPushButton* button = qobject_cast <QPushButton*>( sender () );

    int j = map.valueI( button, -1 );

    if( j >= 0 && j < NUM_ELEMENTS ){
        const QEPeriodic::elementInfoStruct* info = &QEPeriodic::elementInfo [j];
        selectedAtomicNumber = info->number;
        elementSelected = info->symbol;
        this->accept ();
    }

    this->close();
}

// end
