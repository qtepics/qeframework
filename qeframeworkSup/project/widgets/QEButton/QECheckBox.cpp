/*  QECheckBox.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2021 Australian Synchrotron
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
  This class is a CA aware push button widget based on the Qt push button widget.
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details
 */

#include <QECheckBox.h>
#include <QProcess>
#include <QMessageBox>
#include <QMainWindow>
#include <QIcon>

/*
    Constructor with no initialisation
*/
QECheckBox::QECheckBox( QWidget *parent ) : QCheckBox( parent ), QEGenericButton( this ) {
    QEGenericButton::setup();
    setup();
}

/*
    Constructor with known variable
*/
QECheckBox::QECheckBox( const QString &variableNameIn, QWidget *parent ) : QCheckBox( parent ), QEGenericButton( this ) {
    setVariableName( variableNameIn, 0 );

    QEGenericButton::setup();
    setup();

    activate();
}

/*
    Setup common to all constructors
*/
void QECheckBox::setup() {
    // Identify the type of button
    setText( "QECheckBox" );

    // Write 1 when checked, 0 when uncheked (clicked but not checked)
    setClickText( "0" );
    setClickCheckedText( "1" );

    // Set up a connection to recieve variable name property changes
    // The variable name property manager class only delivers an updated variable name after the user has stopped typing
    connectNewVariableNameProperty( SLOT ( useNewVariableNameProperty( QString, QString, unsigned int ) ) );
}

// place holder
QECheckBox::~QECheckBox() { }

// Setup default updateOption.
//
QEGenericButton::updateOptions QECheckBox::getDefaultUpdateOption()
{
   return QEGenericButton::UPDATE_STATE;
}

// Slot to receiver a 'process completed' signal from the application launcher
void QECheckBox::programCompletedSlot()
{
    emit programCompleted();
}

//==============================================================================
// Drag drop

// All in QEGenericButton


//==============================================================================
// Copy / Paste

// Mostly in QEGenericButton

QVariant QECheckBox::copyData()
{
    return QVariant( isChecked() );
}

// end
