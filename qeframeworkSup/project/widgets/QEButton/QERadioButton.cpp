/*  QERadioButton.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2009-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
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
QERadioButton::QERadioButton( QWidget *parent ) :
   QRadioButton( parent ),
   QEGenericButton( this )
{
    QEGenericButton::setup();
    setup();
}

/*
    Constructor with known variable
*/
QERadioButton::QERadioButton( const QString &variableNameIn, QWidget *parent ) :
   QRadioButton( parent ),
   QEGenericButton( this )
{
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

// place holder
QERadioButton::~QERadioButton() { }

// Setup default updateOption.
//
QE::UpdateOptions QERadioButton::getDefaultUpdateOption()
{
   return QE::State;
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
