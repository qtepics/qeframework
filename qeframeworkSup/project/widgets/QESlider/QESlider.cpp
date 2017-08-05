/*  QESlider.cpp
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
  This class is a CA aware slider widget based on the Qt slider widget.
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details
 */

#include <QESlider.h>

/*
    Constructor with no initialisation
*/
QESlider::QESlider( QWidget *parent ) :
    QSlider( parent ),
    QESingleVariableMethods( this, 0 ),
    QEWidget( this )
{
    setup();
}

/*
    Constructor with known variable
*/
QESlider::QESlider( const QString &variableNameIn, QWidget *parent ) :
    QSlider( parent ),
    QESingleVariableMethods( this, 0 ),
    QEWidget( this )
{
    setup();
    setVariableName( variableNameIn, 0 );
    activate();
}

/*
    Setup common to all constructors
*/
void QESlider::setup() {
    // Set up data
    // This control used a single data source
    setNumVariables(1);

    // Set variable index used to select write access cursor style.
    setControlPV( 0 );

    // Set up default properties
    updateInProgress = false;
    writeOnChange = true;
    setAllowDrop( false );

    scale = 1.0;
    offset = 0.0;
    currentValue = 0.0;

    // Set the initial state
    isConnected = false;

    ignoreSingleShotRead = false;

    // Use standard context menu
    setupContextMenu();

    // Use slider signals
    QObject::connect( this, SIGNAL( valueChanged( const int &) ), this, SLOT( userValueChanged( const int & ) ) );

    // Set up a connection to recieve variable name property changes
    // The variable name property manager class only delivers an updated variable name after the user has stopped typing
    connectNewVariableNameProperty( SLOT( useNewVariableNameProperty( QString, QString, unsigned int ) ) );
}

/*
    Implementation of QEWidget's virtual funtion to create the specific type of QCaObject required.
    For a slider a QCaObject that streams integers is required.
*/
qcaobject::QCaObject* QESlider::createQcaItem( unsigned int variableIndex ) {

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
void QESlider::establishConnection( unsigned int variableIndex ) {

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
void QESlider::connectionChanged( QCaConnectionInfo& connectionInfo )
{
    // Note the connected state
    isConnected = connectionInfo.isChannelConnected();

    // Display the connected state
    updateToolTipConnection( isConnected );
    processConnectionInfo( isConnected );

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
    Pass the update straight on to the QSlider unless the user is moving the slider.
    Note, it would not be common to have a user editing a regularly updating value. However, this
    scenario should be allowed for. A reasonable reason for a user modified value to update on a gui is
    if is is written to by another user on another gui.
    This is the slot used to recieve data updates from a QCaObject based class.
*/
void QESlider::setValueIfNoFocus( const double& value, QCaAlarmInfo& alarmInfo, QCaDateTime&, const unsigned int& ) {

    // Do nothing if doing a single shot read (done when not subscribing to get enumeration values)
    if( ignoreSingleShotRead )
    {
        ignoreSingleShotRead = false;
        return;
    }

    // Signal a database value change to any Link widgets
    emit dbValueChanged( qlonglong( value ) );

    // Update the slider only if the user is not interacting with the object, unless
    // the form designer has specifically allowed updates  while the widget has focus.
    if( isAllowFocusUpdate || !hasFocus() ) {
        updateInProgress = true;
        currentValue = value;
        int intValue = int( (value - offset) * scale );
        setValue( intValue );
        updateInProgress = false;
    }

    // Invoke common alarm handling processing.
    processAlarmInfo( alarmInfo );
}

/*
    The user has modified the slider position.
    This will occur as the user slides the slider if tracking is enabled,
    or when the user completes sliding if tracking is not enabled.
*/
void QESlider::userValueChanged( const int &value) {

    // If the change is due to an update (and not the user)
    // or not writing on change, then ignore the change
    if( updateInProgress == true || !writeOnChange )
    {
        return;
    }

    // Get the variable to write to
    QEFloating* qca = (QEFloating*)getQcaItem(0);

    /* If a QCa object is present (if there is a variable to write to)
     * then write the value
     */
    if( qca )
    {
        // Attempt to write the data if the destination data type is known.
        // It is not known until a connection is established.
        if( qca->dataTypeKnown() )
        {
            currentValue = (value/scale) + offset;
            qca->writeFloatingElement( currentValue );
        }
        else
        {
            // Inform the user that the write could not be performed.
            // It is normally not possible to get here. If the connection or link has not
            // yet been established (and therefore the data type is unknown) then the user
            // interface object should be unaccessable. This code is here in the event that
            // the user can, by design or omision, still attempt a write.
            sendMessage( "Could not write value as type is not known yet.", "QESlider::userValueChanged()",
                          message_types ( MESSAGE_TYPE_WARNING ) );
        }
    }
}

// Write a value immedietly.
// Used when writeOnChange is false
// (widget will never write due to the user pressing return or leaving the widget)
void QESlider::writeNow()
{
    // Get the variable to write to
    QEFloating* qca = (QEFloating*)getQcaItem(0);

    // If a QCa object is present (if there is a variable to write to)
    // then write the value
    if( qca )
    {
        // Attempt to write the data if the destination data type is known.
        // It is not known until a connection is established.
        if( qca->dataTypeKnown() )
        {
            currentValue = (value()/scale) + offset;
            qca->writeFloatingElement( currentValue );
        }
    }
}

//==============================================================================
// Drag drop
void QESlider::setDrop( QVariant drop )
{
    setVariableName( drop.toString(), 0 );
    establishConnection( 0 );
}

QVariant QESlider::getDrop()
{
    if( isDraggingVariable() )
        return QVariant( copyVariable() );
    else
        return copyData();
}

//==============================================================================
// Copy / Paste
QString QESlider::copyVariable()
{
    return getSubstitutedVariableName(0);
}

QVariant QESlider::copyData()
{
    return QVariant( currentValue );
}

void QESlider::paste( QVariant v )
{
    if( getAllowDrop() )
    {
        setDrop( v );
    }
}

//==============================================================================
// Property convenience functions

// write on change
void QESlider::setWriteOnChange( bool writeOnChangeIn )
{
    writeOnChange = writeOnChangeIn;
}
bool QESlider::getWriteOnChange() const
{
    return writeOnChange;
}

// subscribe
void QESlider::setSubscribe( bool subscribeIn )
{
    subscribe = subscribeIn;
}
bool QESlider::getSubscribe() const
{
    return subscribe;
}

// Set scale and offset (used to scale data when inteter scale bar min and max are not suitable)
void QESlider::setScale( double scaleIn )
{
    scale = scaleIn;
}

double QESlider::getScale() const
{
    return scale;
}

void QESlider::setOffset( double offsetIn )
{
    offset = offsetIn;
}

double QESlider::getOffset() const
{
    return offset;
}

// set allow updates while widget has focus.
void QESlider::setAllowFocusUpdate( bool allowFocusUpdateIn )
{
    isAllowFocusUpdate = allowFocusUpdateIn;
}

bool QESlider::getAllowFocusUpdate() const
{
    return isAllowFocusUpdate;
}

// end
