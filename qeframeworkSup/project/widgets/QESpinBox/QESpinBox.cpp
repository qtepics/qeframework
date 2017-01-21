/*  QESpinBox.cpp
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
  This class is a CA aware spin box widget based on the Qt spin box widget.
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details
 */

#include <QLineEdit>
#include "QESpinBox.h"

/*
    Create a CA aware spin box with no variable name yet
*/
QESpinBox::QESpinBox( QWidget *parent ) :
    QDoubleSpinBox( parent ),
    QESingleVariableMethods ( this, 0 ),
    QEWidget( this )
{
    setup();
}

/*
    Create a CA aware spin box with a variable name already known
*/
QESpinBox::QESpinBox( const QString &variableNameIn, QWidget *parent ) :
    QDoubleSpinBox( parent ),
    QESingleVariableMethods ( this, 0 ),
    QEWidget( this )
{
    setVariableName( variableNameIn, 0 );

    setup();
    activate();

}

/*
    Common construction
*/
void QESpinBox::setup() {
    // Set up data
    // This control used a single data source
    setNumVariables(1);

    // Set variable index used to select write access cursor style.
    setControlPV( 0 );

    // Initialise the flag indicating the value is being changed programatically (not by the user)
    programaticValueChange = false;

    // Don't respond to every key stroke - just enter or loose focus
    setKeyboardTracking( false );

    // Set up default properties
    writeOnChange = true;
    setAllowDrop( false );
    addUnitsAsSuffix = false;
    useDbPrecisionForDecimal = true;

    // Set the initial state
    lastValue = 0.0;
    isConnected = false;

    ignoreSingleShotRead = false;

    // Use standard context menu
    setupContextMenu();

    // Use spin box signals
    QObject::connect( this, SIGNAL( valueChanged( double ) ), this, SLOT( userValueChanged( double ) ) );

    // Set up a connection to recieve variable name property changes
    // The variable name property manager class only delivers an updated variable name after the user has stopped typing
    connectNewVariableNameProperty( SLOT ( useNewVariableNameProperty( QString, QString, unsigned int ) ) );
}

/*
   Return the Qt default context of embedded line edit menu.
   This is added to the QE context menu
*/
QMenu* QESpinBox::getDefaultContextMenu()
{
    QMenu* menu = NULL;
    QLineEdit* edit = NULL;

    // QESpinBox doesn't have a  createStandardContextMenu or equivilent.
    // But it does have/use an embedded line edit object, which does.
    edit = lineEdit();
    if( edit ){
       menu = edit->createStandardContextMenu();
       menu->setTitle( "Edit..." );
    }
    return menu;
}


/*
    Implementation of QEWidget's virtual funtion to create the specific type of QCaObject required.
    For a spin box a QCaObject that streams integers is required.
*/
qcaobject::QCaObject* QESpinBox::createQcaItem( unsigned int variableIndex ) {

    qcaobject::QCaObject* result = NULL;

    // Create the item as a QEFloating
    result = new QEFloating( getSubstitutedVariableName( variableIndex ), this, &floatingFormatting, variableIndex );

    // Apply current array index to new QCaObject
    setQCaArrayIndex( result );

    return result;
}

/*
    Start updating.
    Implementation of VariableNameManager's virtual funtion to establish a connection to a PV as the variable name has changed.
    This function may also be used to initiate updates when loaded as a plugin.
*/
void QESpinBox::establishConnection( unsigned int variableIndex ) {

    // Create a connection.
    // If successfull, the QCaObject object that will supply data update signals will be returned
    qcaobject::QCaObject* qca = createConnection( variableIndex );

    // If a QCaObject object is now available to supply data update signals, connect it to the appropriate slots
    if(  qca ) {
        setValue( 0 );
        QObject::connect( qca,  SIGNAL( floatingChanged( const double&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                          this, SLOT( setValueIfNoFocus( const double&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
        QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo& ) ),
                          this, SLOT( connectionChanged( QCaConnectionInfo& ) ) );
    }
}

/*
    Act on a connection change.
    Change how the label looks and change the tool tip
    This is the slot used to recieve connection updates from a QCaObject based class.
 */
void QESpinBox::connectionChanged( QCaConnectionInfo& connectionInfo )
{
    // Note the connected state
    isConnected = connectionInfo.isChannelConnected();

    // Display the connected state
    updateToolTipConnection( isConnected );
    updateConnectionStyle( isConnected );

    // !!! ??? not sure if this is right. Added as the record type was comming back as GENERIC::UNKNOWN deep in the write
    // Start a single shot read if the channel is up (ignore channel down),
    // This will allow initialisation of the widget using info from the database.
    // If subscribing, then an update will occur without having to initiated one here.
    // Note, channel up implies link up
    // Note, even though there is nothing to do to initialise the spin box if not subscribing, an
    // initial single shot read is still performed to ensure we have valid information about the
    // variable when it is time to do a write.
    if( isConnected && !subscribe )
    {
        QEFloating* qca = (QEFloating*)getQcaItem(0);
        qca->singleShotRead();
        ignoreSingleShotRead = true;
    }

    // Set cursor to indicate access mode.
    setAccessCursorStyle();
}

/*
    Pass the update straight on to the SpinBox unless the user is changing it.
    Note, it would not be common to have a user editing a regularly updating
    value. However, this scenario should be allowed for. A reasonable reason
    for a user modified value to update on a gui is if is is written to by
    another user on another gui.
    This is the slot used to recieve data updates from a QCaObject based class.
    This is the slot used to recieve data updates from a QCaObject based class.
*/
void QESpinBox::setValueIfNoFocus( const double& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& ) {

    // Save the last database value
    lastValue = value;

    // Set the limits and step size
    QEFloating* qca = (QEFloating*)getQcaItem(0);
    double upper = qca->getControlLimitUpper();
    double lower = qca->getControlLimitLower();
    if( upper != lower)
    {
        setMaximum( qca->getControlLimitUpper() );
        setMinimum( qca->getControlLimitLower() );
    }

    // Do nothing more if doing a single shot read (done when not subscribing to get range values)
    if( ignoreSingleShotRead )
    {
        ignoreSingleShotRead = false;
        return;
    }

    // Update the spin box only if the user is not interacting with the object, unless
    // the form designer has specifically allowed updates while the widget has focus.
    if( isAllowFocusUpdate || !hasFocus() ) {
        // Update the spin box
        programaticValueChange = true;
        setDecimalsFromPrecision( qca );
        setSuffixEgu( qca );
        setValue( value );
        programaticValueChange = false;

        // Note the last value seen by the user
        lastUserValue = text();
    }

    // Invoke common alarm handling processing.
    processAlarmInfo( alarmInfo );

    // Signal a database value change to any Link widgets
    emit dbValueChanged( value );
}

/*
    The user has changed the spin box.
*/
void QESpinBox::userValueChanged( double value )
{
    // If the user is not changing the value, or not writing on change, do nothing
    if( programaticValueChange || !writeOnChange )
    {
        return;
    }

    // Get the variable to write to
    QEFloating* qca = (QEFloating*)getQcaItem(0);

    // If a QCa object is present (if there is a variable to write to)
    // then write the value
    if( qca ) {
        // Write the value
        qca->writeFloatingElement( value );

        // Manage notifying user changes
        emit userChange( text(), lastUserValue, QString("%1").arg( lastValue ) );
    }
}

// Write a value immedietly.
// Used when writeOnChange is false
// (widget will never write due to the user pressing return or leaving the widget)
void QESpinBox::writeNow()
{
    // Get the variable to write to
    QEFloating* qca = (QEFloating*)getQcaItem(0);

    // If a QCa object is present (if there is a variable to write to)
    // then write the value
    if( qca )
    {
        // Write the value
        qca->writeFloatingElement( value() );
    }
}

// Set the EGU as the suffix
void QESpinBox::setSuffixEgu( qcaobject::QCaObject* qca )
{
    // If using the EGU as the suffix, and the EGU is available, set the suffix to the EGU
    // otherwise clear the suffix
    if( qca && addUnitsAsSuffix )
    {
        setSuffix( QString( " " ).append( qca->getEgu() ) );
    }
    else
    {
        setSuffix( "" );
    }
}

// Set the spin box decimal places from the data precision if required
void QESpinBox::setDecimalsFromPrecision( qcaobject::QCaObject* qca )
{
    // If using the database precision to determine the number of decimal places, and it is available, then apply it
    if( qca && useDbPrecisionForDecimal )
    {
        setDecimals( qca->getPrecision() );
    }
}

//==============================================================================
// Drag drop
void QESpinBox::setDrop( QVariant drop )
{
    setVariableName( drop.toString(), 0 );
    establishConnection( 0 );
}

QVariant QESpinBox::getDrop()
{
    return QVariant( getSubstitutedVariableName(0) );
}

//==============================================================================
// Copy paste
QString QESpinBox::copyVariable()
{
    return getSubstitutedVariableName( 0 );
}

QVariant QESpinBox::copyData()
{
   return QVariant( value() );
}

void QESpinBox::paste (QVariant s)
{
    setVariableName( s.toString(), 0 );
    establishConnection( 0 );
}

//==============================================================================
// Property convenience functions

// write on change
void QESpinBox::setWriteOnChange( bool writeOnChangeIn )
{
    writeOnChange = writeOnChangeIn;
}
bool QESpinBox::getWriteOnChange() const
{
    return writeOnChange;
}

// subscribe
void QESpinBox::setSubscribe( bool subscribeIn )
{
    subscribe = subscribeIn;
}
bool QESpinBox::getSubscribe() const
{
    return subscribe;
}

// Add units (as suffix).
// Note, for most widgets with an 'addUnits' property, the property is passed to a
//       QEStringFormatting class where the units are added to the displayed string.
//       In this case, the units are added as the spin box suffix.
bool QESpinBox::getAddUnitsAsSuffix() const
{
    return addUnitsAsSuffix;
}

void QESpinBox::setAddUnitsAsSuffix( bool addUnitsAsSuffixIn )
{
    addUnitsAsSuffix = addUnitsAsSuffixIn;
    qcaobject::QCaObject* qca = (QEFloating*)getQcaItem(0);
    setSuffixEgu( qca );
}

// useDbPrecision
// Note, for most widgets with an 'useDbPrecision' property, the property is passed to a
//       QEStringFormatting class where it is used to determine the precision when formatting numbers as a string.
//       In this case, it is used to determine the spin box number-of-decimals property.
void QESpinBox::setUseDbPrecisionForDecimals( bool useDbPrecisionForDecimalIn )
{
    useDbPrecisionForDecimal = useDbPrecisionForDecimalIn;
    qcaobject::QCaObject* qca = (QEFloating*)getQcaItem(0);
    setDecimalsFromPrecision( qca );
}

bool QESpinBox::getUseDbPrecisionForDecimals() const
{
    return useDbPrecisionForDecimal;
}

// set allow updates while widget has focus.
void QESpinBox::setAllowFocusUpdate( bool allowFocusUpdateIn )
{
    isAllowFocusUpdate = allowFocusUpdateIn;
}

bool QESpinBox::getAllowFocusUpdate() const
{
    return isAllowFocusUpdate;
}

// end
