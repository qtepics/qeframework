/*  QEGenericButton.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2023 Australian Synchrotron
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

#include <alarm.h>
#include <UserMessage.h>
#include <QEGenericButton.h>
#include <QDebug>
#include <QMessageBox>
#include <QIcon>
#include <QInputDialog>
#include <QStyle>
#include <QERecordFieldName.h>

#define DEBUG qDebug () << "QEGenericButton" << __LINE__ << __FUNCTION__ << "  "

// Style option dynamic property name.
#define STYLE_OPTION  "StyleOption"

QEGenericButton::QEGenericButton( QAbstractButton *owner ) :
    QEWidget( owner ),
    QESingleVariableMethods( this, (unsigned int) VAR_PRIMARY )
{
    altReadback = NULL;
}

/*
    Setup common to all button constructors
*/
void QEGenericButton::setup()
{
    dataSetup();
//  commandSetup();
    guiSetup();

    QAbstractButton* button = getButtonQObject();
    // Use push button signals
    QObject::connect( button, SIGNAL( pressed() ),       button, SLOT( userPressed() ) );
    QObject::connect( button, SIGNAL( released() ),      button, SLOT( userReleased() ) );
    QObject::connect( button, SIGNAL( clicked( bool ) ), button, SLOT( userClicked( bool ) ) );
}

/*
    Setup for reading and writing data
*/
void QEGenericButton::dataSetup()
{
    // Set up data
    // This control uses two data sources, the first is written to and (by default) read from.
    // The second is the alternative read back
    //
    setNumVariables( NUMBER_OF_VARIABLES );

    // Set up default properties
    disabledRecordPolicy = QE::ignore;
    writeOnPress = false;
    writeOnRelease = false;
    writeOnClick = true;
    setAllowDrop( false );
    confirmRequired = false;
    confirmText = "Do you want to perform this action?";

    // Set text alignment to the default for a push button
    // This will make no visual difference unless the style has been changed from the default
    textAlignment = Qt::AlignHCenter | Qt::AlignVCenter;
    this->setTextAlignment( textAlignment );

    pressText = "1";
    releaseText = "0";
    clickText = "1";
    clickCheckedText = "0";

    // Override default QEWidget properties
    subscribe = false;

    // Set the initial state
    isConnected = false;
    updateOption = getDefaultUpdateOption();

    // Initially, there is no specific style based on the usage of this button.
    getQWidget()->setProperty( STYLE_OPTION, "" );

    // set control PV for cursor style
    setControlPV(0);
}

///*
//    Setup for running commands
//*/
//void QEGenericButton::commandSetup()
//{
//    programStartupOption = PSO_NONE;
//}

/*
    Setup for starting new GUIs
*/
void QEGenericButton::guiSetup()
{

    // Set default properties
    creationOption = QE::Open;

    // Use standard context menu
    setupContextMenu();

    // If a profile is define by whatever contains the button, use it
    if( isProfileDefined() )
    {
        // Setup a signal to launch a new gui
        // The signal will be used by whatever the button is in
        QObject* launcher = getGuiLaunchConsumer();
        if( launcher )
        {
            QObject::connect( getButtonQObject(), SIGNAL( newGui( const QEActionRequests& ) ),
                              launcher, SLOT( requestAction( const QEActionRequests& ) ) );
        }
    }

    // A profile is not already defined, create one. This is the case if this class is used by an application that does not set up a profile, such as 'designer'.
    else
    {
        // Set up the button's own gui form launcher
        QObject::connect( getButtonQObject(), SIGNAL( newGui( const QEActionRequests& ) ),
                          getButtonQObject(), SLOT( requestAction( const QEActionRequests& ) ) );
    }
}

/*
    Implementation of QEWidget's virtual funtion to create the specific type of QCaObject required.
    For a push button a QCaObject that streams strings is required.
*/
qcaobject::QCaObject* QEGenericButton::createQcaItem( unsigned int variableIndex ) {

    qcaobject::QCaObject* result = NULL;

    // Fetch reference to the target button widget and get pv name.
    QString pvName = getSubstitutedVariableName( variableIndex );
    QAbstractButton* target = getButtonQObject();

    // Create the item as a QEString
    switch (variableIndex) {
    case VAR_PRIMARY:
        result = new QEString( pvName, target, &stringFormatting, variableIndex );
        // Apply currently defined array index.
        setSingleVariableQCaProperties( result );
        break;

    case VAR_READBACK:
        result = new QEString( pvName, target, &stringFormatting, variableIndex );
        if( altReadback ) altReadback->setSingleVariableQCaProperties( result );
        break;

    }
    return result;
}

/*
    Start updating.
    Implementation of VariableNameManager's virtual funtion to establish a connection to a PV as the variable name has changed.
    This function may also be used to initiate updates when loaded as a plugin.
*/
void QEGenericButton::establishConnection( unsigned int variableIndex ) {

    // Create a connection.
    // If successfull, the QCaObject object that will supply data update signals will be returned.
    qcaobject::QCaObject* qca = NULL;
    switch (variableIndex) {

    case VAR_PRIMARY:
        // We always subscribe for primary irrsepective of the subscribe property
        // so that we get status. If subscribe false, we ignore the value update.
        qca = createConnection( variableIndex, true );
        break;

    case VAR_READBACK:
        qca = createConnection( variableIndex );
        break;

    default:
        qca = NULL;
        break;
    }

    if( !qca )return;

    // Fetch reference to the target button widget.
    QAbstractButton* target = getButtonQObject();

    // If a QCaObject object is now available to supply data update signals, connect it to the appropriate slots
    switch (variableIndex) {

    case VAR_PRIMARY:     // Primary readback variable
        // Get updates if subscribing and  there is no alternate read back.
        if( subscribe && getSubstitutedVariableName( VAR_READBACK ).isEmpty() ){
            if( (updateOption & QE::Text) == QE::Text )
            {
                setButtonText( "" );
            }
        }

        // We always subscribe for and handle primary data changes, because we
        // want the STATus to check againtts the disabled record policy.
        //
        connectButtonDataChange( qca );

        // Get conection status changes always (subscribing or not)
        QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ),
                          target, SLOT( connectionChanged( QCaConnectionInfo&, const unsigned int&) ) );
        break;

    case VAR_READBACK:     // Alternate readback variable
        // Get updates if subscribing.
        if( subscribe ){
            if( (updateOption & QE::Text) == QE::Text )
            {
                setButtonText( "" );
            }
            connectButtonDataChange( qca );
        }

        // Get conection status changes always (subscribing or not)
        QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ),
                          target, SLOT( connectionChanged( QCaConnectionInfo&, const unsigned int&) ) );
        break;

    default:
        break;
    }
}

/*
 Set/get disabledRecordPolicy state.
 */
void QEGenericButton::setDisabledRecordPolicy( const QE::DisabledRecordPolicy policyIn )
{
   disabledRecordPolicy = policyIn;
}

QE::DisabledRecordPolicy QEGenericButton::getDisabledRecordPolicy() const
{
   return disabledRecordPolicy;
}

/*
 Common handler for setting variable names.
 */
void QEGenericButton::useGenericNewVariableName( const QString& variableNameIn,
                                                 const QString& variableNameSubstitutionsIn,
                                                 const unsigned int variableIndex )
{
    setVariableNameAndSubstitutions( variableNameIn, variableNameSubstitutionsIn, variableIndex );

    // Update the labelText property with itself.
    // This will apply any macro substitutions changes since the labelText property was last changed
    setLabelTextProperty( getLabelTextProperty() );
    calcStyleOption();
}

/*
 * Handle changes to primary record disable state based on the current disabled record policy.
 */
void QEGenericButton::processRecordDisableState()
{
    QAbstractButton* button = getButtonQObject();
    if( !button ) return;                       // sanity check

    qcaobject::QCaObject* qca = this->getQcaItem (VAR_PRIMARY);
    if( !qca ) return;                         // sanity check
    if( !qca->getDataIsAvailable() ) return;  // sanity check

    QCaAlarmInfo alarmInfo = qca->getAlarmInfo();
    const bool isDisabled = alarmInfo.getStatus() == epicsAlarmDisable;

    switch( disabledRecordPolicy ) {
    case QE::ignore:
        // Do nothing
        break;

    case QE::grayout:
        // Treat disabled like not connected wrt style.
        updateConnectionStyle( !isDisabled );
        break;

    case QE::disable:
        button->setEnabled ( !isDisabled );
        break;
    }
}

/*
    Act on a connection change.
    Change how the label looks and change the tool tip
    This is the slot used to recieve connection updates from a QCaObject based class.
 */
void QEGenericButton::connectionChanged( QCaConnectionInfo& connectionInfo,
                                         const unsigned int& variableIndex )
{
    // Do nothing if no variable name, but there is a program to run or a new gui to open.
    // Most widgets will be dissabled at this point if there is no good connection to a PV,
    // but this widget may be doing other stuff (running a program of starting a GUI)
    if( getSubstitutedVariableName( variableIndex ).isEmpty() )
        return;

    // Note the connected state for primary PV only.
    //
    if( variableIndex  == VAR_PRIMARY ) {
       isConnected = connectionInfo.isChannelConnected();
    }

    // Display the connected state
    updateToolTipConnection( isConnected, variableIndex );
    updateConnectionStyle( isConnected );

    // set cursor to indicate access mode
    setAccessCursorStyle();

    // Signal channel connection change to any Link  (or other) widgets,
    // using signal dbConnectionChanged.
    if( variableIndex  == VAR_PRIMARY ) {
        emitDbConnectionChanged( variableIndex );
    }
}

/*
  Implement a slot to set the current text of the push button
  This is the slot used to recieve data updates from a QCaObject based class.
*/
void QEGenericButton::setGenericButtonText( const QString& text, QCaAlarmInfo& alarmInfo,
                                            QCaDateTime&, const unsigned int& variableIndex )
{
    if( variableIndex == VAR_PRIMARY ) {
       // Modify style/enable state basesd on disabledRecordPolicy and record state.
       processRecordDisableState();
    }

    // If not subscribing, or subscribing but update is not for the readback variable, then do nothing.
    //
    // Note, This will still be called even if not subscribing as there may be an initial single shot read
    // to ensure we have valid information about the variable when it is time to do a write.
    //
    // Note, variableIndex = 0 = Primary readback variable, variableIndex = 1 = Alternate readback variable,
    // so an update for variable 1 is always OK, an update from variable 0 is OK as long as there is no variable 1
    //
    if( !subscribe || ( variableIndex == VAR_PRIMARY && !getSubstitutedVariableName( VAR_READBACK ).isEmpty() ))
    {
        return;
    }

    // Update the button state if required
    // Display checked if text matches what is written when checked
    // See GUI-323
    //
    if( (updateOption & QE::State) == QE::State )
    {
        if( text.compare( clickCheckedText ) == 0){
            setButtonState( true );

        } else if( text.compare( clickText ) == 0 ){
            setButtonState( false );

        } else {
            // Can we be clever/helpfull? Not too clever I hope.
            //
            qcaobject::QCaObject* qca = this->getQcaItem( VAR_PRIMARY );
            const int state = qca ? qca->getIntegerValue() : -1;
            bool b1, b2;
            const int s1 = clickCheckedText.toInt( &b1 );
            const int s2 = clickText.toInt( &b2 );

            if( b1 && (state == s1) ){
                setButtonState( true );

            } else if( b2 && (state == s2) ){
                setButtonState( false );

            } else {
                QString message;
                message = QString( "%1: '%2' (%3) is not one of: '%4' or '%5'" )
                      .arg( getQWidget()->metaObject()->className() )
                      .arg( text )
                      .arg( state )
                      .arg( clickText )
                      .arg( clickCheckedText );

                message_types mt( MESSAGE_TYPE_WARNING, MESSAGE_KIND_STATUS );
                this->sendMessage( message, mt );
                DEBUG << message;
            }
        }
    }

    // Update the text if required
    if( (updateOption & QE::Text) == QE::Text )
    {
        setButtonText( text );
    }

    // Update the icon if required
    if( (updateOption & QE::Icon) == QE::Icon )
    {
        QIcon icon;
        icon.addPixmap( getDataPixmap( text ) );
        setButtonIcon( icon );
    }

    // Invoke common alarm handling processing.
    processAlarmInfo( alarmInfo, variableIndex );

    // Signal a database value change to any Link (or other) widgets using one
    // of the dbValueChanged. Must be subscribed.
    //
    if( variableIndex == VAR_PRIMARY ) {
        emitDbValueChanged( text, variableIndex );
    }
}

/*
 Returns true if no user confirmation required, or if user confirms a button press acion
 */
bool QEGenericButton::confirmAction()
{
    // Return OK if no confirmation required
    if( !confirmRequired )
        return true;

    // Get confirmation from the user as to what to do
    int confirm = QMessageBox::Yes;
    confirm = QMessageBox::warning( (QWidget*)getButtonQObject(), "Confirm write", confirmText,
                                    QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes );

    // Return yes/no
    switch( confirm )
    {
        case QMessageBox::Yes:  return true;
        case QMessageBox::No:   return false;
        default:                return false;   // Sanity check
    }

}

/*
    Button press event.
*/
void QEGenericButton::userPressed()
{
    // Do nothing if not acting on button press, or user confirmation required but not given, or password required but not given
    if( !writeOnPress ||  !confirmAction() || !checkPassword() )
        return;

    // Determine the string to write
    QString writeText = substituteThis( pressText );

    // Emit a 'pressed' signal
    emitPressed( writeText.toInt() );

    // Get the variable to write to
    QEString *qca = (QEString*)getQcaItem(0);

    // If a QCa object is present (if there is a variable to write to) then write the value
    if( qca )
    {
        QString error;
        if( !qca->writeStringElement( writeText, error ) )
        {
            QMessageBox::warning( (QWidget*)getButtonQObject(), QString( "Write failed" ), error, QMessageBox::Cancel );
        }
    }
}

/*
    Button release event.
*/

void QEGenericButton::userReleased()
{
    // Do nothing if not acting on button release, or user confirmation required but not given, or password required but not given
    if( !writeOnRelease ||  !confirmAction() || !checkPassword() )
        return;

    // Determine the string to write
    QString writeText = substituteThis( releaseText );

    // Emit a 'released' signal
    emitReleased( writeText.toInt() );

    // Get the variable to write to
    QEString *qca = (QEString*)getQcaItem(0);

    // If a QCa object is present (if there is a variable to write to) then write the value
    if( qca )
    {
        QString error;
        if( !qca->writeStringElement( writeText, error ) )
        {
            QMessageBox::warning( (QWidget*)getButtonQObject(), QString( "Write failed" ), error, QMessageBox::Cancel );
        }
    }
}

/*
    Button click event.
*/
void QEGenericButton::userClicked( bool checked )
{
    // Do nothing if nothing to do. (no point asking for confirmation or password)
    if( !writeOnClick && programLauncher.getProgram().isEmpty() && guiName.isEmpty() )
    {
        return;
    }

    // Keep doing nothing if user confirmation required but not given, or password required but not given
    // If confirmation not given or password verification failed, then reset widget checked state if checkable.
    // Should this also apply to userPressed and/or userReleased functions?
    if( !confirmAction() || !checkPassword() )
    {
        QAbstractButton* button = getButtonQObject();
        if( button->isCheckable() ) {
            // Un-check/re-check.
            button->setChecked (!checked);
        }
        return;
    }

    // Get the variable to write to
    QEString *qca = (QEString*)getQcaItem(0);

    // If the object is set up to write when the user clicks the button
    // emit a signal
    // Also, if a QCa object is present (if there is a variable to write to)
    // then write the value
    if( writeOnClick )
    {
        // Determine the string to write
        QString writeText;
        if( !checked )
        {
            writeText = clickText;
        }
        else
        {
            writeText = clickCheckedText;
        }

        writeText = substituteThis( writeText );

        // Emit a 'clicked' signal
        emitClicked( writeText.toInt() );

        // Write to the variable if present
        if( qca )
        {
            QString error;
            if( !qca->writeStringElement( writeText, error ) )
            {
                QMessageBox::warning( (QWidget*)getButtonQObject(), QString( "Write failed" ), error, QMessageBox::Cancel );
            }
        }
    }

    // If there is a command to run, run it, with substitutions applied to the command and arguments
    programLauncher.launch( (VariableNameManager*)this, getButtonQObject() );

    // If a new GUI is required, start it
    if( !guiName.isEmpty() )
    {
        ProfilePublisher publisher( this, prioritySubstitutions ); // publish, setup and reslease profile

        // Start the GUI
        emitNewGui( QEActionRequests( substituteThis( guiName ), customisationName, creationOption ) );
    }
}

/*
   Replicates, to a certain extents part of userClicked, save that there is no reference to writeOnClick
 */
void QEGenericButton::writeClickedNow (const bool checked)
{
    // Get the variable to write to (if any).
    QEString* qca = (QEString*)getQcaItem(0);

    if( qca ) {
        QString writeText;
        writeText = checked ? clickCheckedText : clickText;

        // Write to the variable
        QString error;
        if( !qca->writeStringElement( writeText, error ) )
        {
            message_types mt( MESSAGE_TYPE_WARNING, MESSAGE_KIND_EVENT | MESSAGE_KIND_STATUS );
            error.prepend( qca->getRecordName() + ": " );
            this->sendMessage( error, mt );
        }
    }
}

/*
   Process the QEWidget virtual writeNow hook function.
   Replicates, to a certain extents part of userPressed, userReleased and userClicked.
 */
void QEGenericButton::processWriteNow ( const bool checked )
{
    // Get the variable to write to (if any).
    QEString* qca = (QEString*)getQcaItem(0);

    if( qca ) {
        QString writeText;

        // Determine the string to write
        // For now (at least) we assume only one of the witeOnXxxx bools set true.
        if( writeOnClick )
        {
            writeText = checked ? clickCheckedText : clickText;
        }
        else if( writeOnPress )
        {
            writeText = pressText;
        }
        else if( writeOnRelease )
        {
            writeText = releaseText;
        }
        else
        {
            return;   // no write action required
        }
        writeText = substituteThis( writeText );

        // Write to the variable
        QString error;
        if( !qca->writeStringElement( writeText, error ) )
        {
            message_types mt( MESSAGE_TYPE_WARNING, MESSAGE_KIND_EVENT | MESSAGE_KIND_STATUS );
            error.prepend( qca->getRecordName() + ": " );
            this->sendMessage( error, mt );
        }
    }
}

/*
 Provides default functionality.
 */
void QEGenericButton::connectButtonDataChange( qcaobject::QCaObject* qca )
{
    QAbstractButton* target = getButtonQObject();

    QObject::connect( qca,  SIGNAL( stringChanged( const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                      target, SLOT( setButtonText( const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
    QObject::connect( target, SIGNAL( requestResend() ),
                      qca,      SLOT( resendLastData() ) );
}

/*
  Check the password.
  Return true if there is no password, or if the user enters it correctly.
  Return false if the user cancels, or enteres an incorrect password.
  Give the user a warning message if a password is entered incorrectly.
*/
bool QEGenericButton::checkPassword()
{
    // All OK if there is no password
    if( password.isEmpty() )
       return true;

    // Ask the user what the password is
    bool ok;
    QString text = QInputDialog::getText( (QWidget*)getButtonQObject(), "Password", "Password:", QLineEdit::Password, "", &ok );

    // If the user canceled, silently return password failure
    if( !ok )
        return false;

    // If the user entered the wrong password, show a warning, then return password failure
    if ( text.compare( password ) )
    {
        QMessageBox::warning( (QWidget*)getButtonQObject(), "Incorrect Password", "You entered the wrong password. No action will be taken" );
        return false;
    }

    // All OK, return password success
    return true;
}

//==============================================================================
// Copy / Paste
QString QEGenericButton::copyVariable()
{
    return getSubstitutedVariableName( 0 );
}

void QEGenericButton::paste( QVariant v )
{
    setVariableName( v.toString(), 0 );
    establishConnection( 0 );
}

//==============================================================================

// Update option Property convenience function
void QEGenericButton::setUpdateOption( QE::UpdateOptions updateOptionIn )
{
    updateOption = updateOptionIn;
}
QE::UpdateOptions QEGenericButton::getUpdateOption()
{
    return updateOption;
}

// 'Data button' Property convenience functions

// subscribe
void QEGenericButton::setSubscribe( bool subscribeIn )
{
    subscribe = subscribeIn;
}
bool QEGenericButton::getSubscribe()
{
    return subscribe;
}

// text alignment
void QEGenericButton::setTextAlignment( Qt::Alignment textAlignmentIn )
{
    // Keep a local copy of the alignment
    textAlignment = textAlignmentIn;

    // Update the horizontal style to match the property
    QString styleSheetString;

    if( textAlignmentIn & Qt::AlignLeft )
        styleSheetString.append( "text-align: left;");
    else if( textAlignment & Qt::AlignRight )
        styleSheetString.append( "text-align: right;");
// Do nothing for center. It is the default
//    else if( textAlignment & Qt::AlignHCenter )
//        styleSheetString.append( "text-align: center;");

    // Update the vertical style to match the property
    if( textAlignment & Qt::AlignTop )
        styleSheetString.append( "text-align: top;");
    else if( textAlignment & Qt::AlignBottom )
        styleSheetString.append( "text-align: bottom;");

    // Setting the justification in the style also appears to set the padding
    // to greater than default so if the button is constrained in width, text
    // doesn't fit once the justification style is applied, so force the padding
    // if applying a justification style
    if( !styleSheetString.isEmpty() )
        styleSheetString.append( " padding-left: 0px; padding-right: 0px; padding-top: 3px; padding-bottom: 4px;" );

    updatePropertyStyle( styleSheetString );
}
Qt::Alignment QEGenericButton::getTextAlignment()
{
    return textAlignment;
}

// password
void QEGenericButton::setPassword( QString password )
{
    QEGenericButton::password = password;
}
QString QEGenericButton::getPassword()
{
    return QEGenericButton::password;
}

// confirm action
void QEGenericButton::setConfirmAction( bool confirmRequiredIn )
{
    confirmRequired = confirmRequiredIn;
}
bool QEGenericButton::getConfirmAction()
{
    return confirmRequired;
}

// confirm text
void QEGenericButton::setConfirmText( QString confirmTextIn )
{
    confirmText = confirmTextIn;
}
QString QEGenericButton::getConfirmText()
{
    return confirmText;
}

// write on press
void QEGenericButton::setWriteOnPress( bool writeOnPress )
{
    QEGenericButton::writeOnPress = writeOnPress;
}
bool QEGenericButton::getWriteOnPress()
{
    return QEGenericButton::writeOnPress;
}

// write on release
void QEGenericButton::setWriteOnRelease( bool writeOnRelease )
{
    QEGenericButton::writeOnRelease = writeOnRelease;
}
bool QEGenericButton::getWriteOnRelease()
{
    return QEGenericButton::writeOnRelease;
}

// write on click
void QEGenericButton::setWriteOnClick( bool writeOnClick )
{
    QEGenericButton::writeOnClick = writeOnClick;
}
bool QEGenericButton::getWriteOnClick()
{
    return QEGenericButton::writeOnClick;
}

// press value
void QEGenericButton::setPressText( QString pressText )
{
    QEGenericButton::pressText = pressText;
}
QString QEGenericButton::getPressText()
{
    return QEGenericButton::pressText;
}

// release value
void QEGenericButton::setReleaseText( QString releaseTextIn )
{
    releaseText = releaseTextIn;
}
QString QEGenericButton::getReleaseText(){ return releaseText; }

// click value
void QEGenericButton::setClickText( QString clickTextIn )
{
    clickText = clickTextIn;
}
QString QEGenericButton::getClickText()
{
    return clickText;
}

// click off value
void QEGenericButton::setClickCheckedText( QString clickCheckedTextIn )
{
    clickCheckedText = clickCheckedTextIn;
}
QString QEGenericButton::getClickCheckedText()
{
    return clickCheckedText;
}

//==============================================================================
// 'Command button' Property convenience functions

// Program String
void QEGenericButton::setProgram( QString program ){
    programLauncher.setProgram( program );
    calcStyleOption ();
}

QString QEGenericButton::getProgram(){ return programLauncher.getProgram(); }

// Arguments String
void QEGenericButton::setArguments( QStringList arguments ){ programLauncher.setArguments( arguments ); }
QStringList QEGenericButton::getArguments(){ return  programLauncher.getArguments(); }

// Startup option
void QEGenericButton::setProgramStartupOption( QE::ProgramStartupOptions programStartupOption )
{
   programLauncher.setProgramStartupOption( programStartupOption );
}

QE::ProgramStartupOptions QEGenericButton::getProgramStartupOption()
{
   return programLauncher.getProgramStartupOption();
}

//==============================================================================
// 'Start new GUI' Property convenience functions

// GUI name
void QEGenericButton::setGuiName( QString guiNameIn )
{
    guiName = guiNameIn;
    calcStyleOption();
}

QString QEGenericButton::getGuiName()
{
    return guiName;
}

// Qt Designer Properties Creation options
void QEGenericButton::setCreationOption( QE::CreationOptions creationOptionIn )
{
    creationOption = creationOptionIn;
}

QE::CreationOptions QEGenericButton::getCreationOption()
{
    return creationOption;
}


// Priority substitutions
void QEGenericButton::setPrioritySubstitutions( QString prioritySubstitutionsIn )
{
    prioritySubstitutions = prioritySubstitutionsIn;
}
QString QEGenericButton::getPrioritySubstitutions()
{
    return prioritySubstitutions;
}

// Window customisation name
void QEGenericButton::setCustomisationName( QString customisationNameIn )
{
    customisationName = customisationNameIn;
}
QString QEGenericButton::getCustomisationName()
{
    return customisationName;
}

//==============================================================================


// label text (prior to substitution)
void QEGenericButton::setLabelTextProperty( QString labelTextIn )
{
    bool wasBlank = labelText.isEmpty();
    labelText = labelTextIn;

    // Update the button's text.
    // But don't do it if the labelText property is just changing from blank to blank. This behaviour will
    // mean the normal label 'text' property can be used if text substitution is
    // not required. Without this the user would always have to use the labelText property.
    if( !(wasBlank && labelText.isEmpty() ))
    {
        setButtonText( substituteThis( labelText ));
    }
}

QString QEGenericButton::getLabelTextProperty()
{
    return labelText;
}

// Calculate style based on the widget usage and set a dynamic propert for style options.
// When the dynamic property is set it can be used in style sheets to target a style at
// all QE buttons with a particular function as follows:
//  QEPushButton[StyleOption="PV"]      {color:purple}
//  QEPushButton[StyleOption="Program"] {color:red}
//  QEPushButton[StyleOption="UI"]      {color:green}
//  QEPushButton                        {color:blue}
//  QEPushButton:!enabled               {color:grey}

void QEGenericButton::calcStyleOption ()
{
    // Get the button
    QWidget* button = getQWidget();
    if (!button) return;    // sanity check

    // Set the dynamic property based on a prioritised button usage.
    if( !getSubstitutedVariableName( 0 ).isEmpty() ){
       button->setProperty( STYLE_OPTION, "PV");

    } else if( !getProgram().isEmpty() ){
       button->setProperty( STYLE_OPTION, "Program");

    } else if( !getGuiName().isEmpty() ){
       button->setProperty( STYLE_OPTION, "UI");

    } else {
       button->setProperty( STYLE_OPTION, "");
    }

    // Ensure all dynamic aspects of the button style is reapplied
    button->style()->unpolish(button);
    button->style()->polish(button);
    button->update();
}

// end
