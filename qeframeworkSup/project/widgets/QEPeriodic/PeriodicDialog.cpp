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
PeriodicDialog::PeriodicDialog( QWidget *parent ) :
    QEDialog(parent),
    m_ui(new Ui::PeriodicDialog)
{
    m_ui->setupUi( this );

    colourise = false;
    map.clear ();    // index/button map

    // Populate the table
    // In many ways this is the same as in the plugin's PeriodicSetupDialog constructor.
    //
    QGridLayout* periodicGrid = m_ui->periodicGridLayout;
    if( periodicGrid )
    {
       periodicGrid->setSpacing( 4 );

       // Populate the table elements
       for( int j = 0; j < NUM_ELEMENTS; j++ )
       {
           const QEPeriodic::elementInfoStruct* info = &QEPeriodic::elementInfo [j];
           QPushButton* button = new QPushButton( info->symbol, this );

           map.insertF( j, button );   // allows index/button mapping

           button->setMinimumSize( 35, 23 );
           button->setMaximumSize( 350,230 );

           periodicGrid->addWidget( button, info->tableRow, info->tableCol );

           QObject::connect( button, SIGNAL ( clicked() ),
                             this,   SLOT   ( noteElementSelected()) );

           button->setToolTip ( QString(" %1  %2 ")
                                .arg( info->name )
                                .arg( info->number ) );
       }

       // Populate unused rows and columns
       QLabel* label;

       // ... Lanthanides indicators
       label = new QLabel( this );
       label->setText( "*" );
       label->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
       label->setFixedWidth (20);
       periodicGrid->addWidget( label, 5, 3 );

       label = new QLabel( this );
       label->setText( "*" );
       label->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
       label->setFixedWidth (20);
       periodicGrid->addWidget( label, 8, 3 );

       // ... Actinides indicators
       label = new QLabel( this );
       label->setText( "**" );
       label->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
       label->setFixedWidth (20);
       periodicGrid->addWidget( label, 6, 3 );

       label = new QLabel( this );
       label->setText( "**" );
       label->setAlignment(Qt::AlignVCenter|Qt::AlignRight);
       label->setFixedWidth (20);
       periodicGrid->addWidget( label, 9, 3 );

       // ... Force empty row 7 to remain
       label = new QLabel( this );
       label->setText( "" );
       periodicGrid->addWidget( label, 7, 10 );

       // Make empty row 7 narrower than the rest
       for( int i = 0; i < 10; i++ )
           periodicGrid->setRowStretch( i, (i==7) ? 2 : 10); // row 7 stretch = 2, all other rows stretch = 10

       // Make "**" col 3 narrower than the rest
       for( int i = 0; i < 19; i++ )
           periodicGrid->setColumnStretch( i, (i==3) ? 1 : 10); // col 3 stretch = 2, all other rows stretch = 10
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
// Colourise user element selection dialog.
void PeriodicDialog::setColourised (const bool colouriseIn)
{
    this->colourise = colouriseIn;
}
bool PeriodicDialog::isColourised () const
{
    return this->colourise;
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
void PeriodicDialog::changeEvent( QEvent *e )
{
    QDialog::changeEvent( e );
    switch( e->type() ){
    case QEvent::LanguageChange:
        m_ui->retranslateUi( this );
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

        if( colourise ) {
            QColor colour = QEPeriodic::categoryColour( info->category );
            if( itemEnabled ){
                button->setStyleSheet( QEUtilities::colourToStyle( colour ) );
            } else {
                // Disabled - set bland colours.
                colour = QEUtilities::blandColour( colour );
                button->setStyleSheet( QEUtilities::colourToStyle( colour, QColor( "#808080" ) ) );
            }
        }

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
// Save the element symbol and atomic number for the widget with focus
// (if it is an element push button)
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
