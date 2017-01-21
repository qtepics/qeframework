/*  QEPushButton.cpp
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
 *  Copyright (c) 2009, 2010 Australian Synchrotron
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

#include <QEPushButton.h>
#include <QProcess>
#include <QMessageBox>
#include <QMainWindow>
#include <QIcon>

/*
    Constructor with no initialisation
*/
QEPushButton::QEPushButton( QWidget *parent ) : QPushButton( parent ), QEGenericButton( this ) {
    QEGenericButton::setup();
    setup();
}

/*
    Constructor with known variable
*/
QEPushButton::QEPushButton( const QString &variableNameIn, QWidget *parent ) : QPushButton( parent ), QEGenericButton( this ) {
    setVariableName( variableNameIn, 0 );

    QEGenericButton::setup();
    setup();

    activate();
}

/*
    Setup common to all constructors
*/
void QEPushButton::setup() {
    // Create second single variable methods object for the alt readback PV.
    //
    altReadback = new QESingleVariableMethods (this, VAR_READBACK);

    // Identify the type of button
    setText( "QEPushButton" );

    // For each variable name property manager, set up an index to identify it when it signals and
    // set up a connection to recieve variable name property changes.
    // The variable name property manager class only delivers an updated variable name after the user has stopped typing
    connectNewVariableNameProperty( SLOT ( useNewVariableNameProperty( QString, QString, unsigned int ) ) );
    altReadback->connectNewVariableNameProperty( SLOT ( useNewVariableNameProperty( QString, QString, unsigned int ) ) );
}

/*
    Destructor function
*/
QEPushButton::~QEPushButton()
{
    if( altReadback ) delete altReadback;
}

/*
    Set variable name substitutions.
    Must set all - as each variable name proprty manager needs it's own copy.
 */
void QEPushButton::setVariableNameSubstitutionsProperty( const QString& substitutions )
{
    QESingleVariableMethods::setVariableNameSubstitutionsProperty (substitutions);
    altReadback->setVariableNameSubstitutionsProperty( substitutions );
}

/*
    Set/get alternative readback PV name.
 */
void QEPushButton::setAltReadbackProperty( const QString& variableName )
{
    altReadback->setVariableNameProperty( variableName );
}

QString QEPushButton::getAltReadbackProperty() const
{
    return altReadback->getVariableNameProperty();
}


/*
    Set/get alternative readback PV attay index.
 */
void QEPushButton::setAltReadbackArrayIndex( const int arrayIndex )
{
    altReadback->setArrayIndex( arrayIndex );
}

int QEPushButton::getAltReadbackArrayIndex () const
{
    return altReadback->getArrayIndex();
}

// Slot to receiver a 'process completed' signal from the application launcher
void QEPushButton::programCompletedSlot()
{
    emit programCompleted();
}

//==============================================================================
// Drag drop

// All in QEGenericButton


//==============================================================================
// Copy / Paste

// Mostly in QEGenericButton

QVariant QEPushButton::copyData()
{
    return QVariant( getButtonText() );
}

// end
