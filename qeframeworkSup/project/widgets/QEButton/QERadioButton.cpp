/*  QERadioButton.cpp
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
 *  Copyright (c) 2009,2010,2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
  This class is a CA aware push button widget based on the Qt push button widget.
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details
 */

#include <QERadioButton.h>
#include <QProcess>
#include <QMessageBox>
#include <QMainWindow>
#include <QIcon>

/*
    Constructor with no initialisation
*/
QERadioButton::QERadioButton( QWidget *parent ) : QRadioButton( parent ), QEGenericButton( this ) {
    QEGenericButton::setup();
    setup();
}

/*
    Constructor with known variable
*/
QERadioButton::QERadioButton( const QString &variableNameIn, QWidget *parent ) : QRadioButton( parent ), QEGenericButton( this ) {
    setVariableName( variableNameIn, 0 );

    QEGenericButton::setup();
    setup();

    activate();
}

/*
    Setup common to all constructors
*/
void QERadioButton::setup() {
    // Identify the type of button
    setText( "QERadioButton" );

    // Write 1 when checked, 0 when uncheked (clicked but not checked)
    setClickText( "0" );
    setClickCheckedText( "1" );

    // Subscribe by default
    setSubscribe( true );

    // Set up a connection to recieve variable name property changes
    // The variable name property manager class only delivers an updated variable name after the user has stopped typing
    connectNewVariableNameProperty( SLOT ( useNewVariableNameProperty( QString, QString, unsigned int ) ) );
}

// Slot to receiver a 'process completed' signal from the application launcher
void QERadioButton::programCompletedSlot()
{
    emit programCompleted();
}

//==============================================================================
// Drag drop

// All in QEGenericButton


//==============================================================================
// Copy / Paste

// Mostly in QEGenericButton

QVariant QERadioButton::copyData()
{
    return QVariant( isChecked() );
}

// end
