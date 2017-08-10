/*
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
  This class is a CA aware label widget based on the Qt label widget.
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details
 */
#include <QFile>
#include <QEFileImage.h>

/*
    Constructor with no initialisation
*/
QEFileImage::QEFileImage( QWidget *parent ) : QLabel( parent ), QEWidget( this ) {
    setup();
}

/*
    Constructor with known variable
*/
QEFileImage::QEFileImage( const QString &variableNameIn, QWidget *parent ) : QLabel( parent ), QEWidget( this )  {
    setup();
    setVariableName( variableNameIn, 0 );
    activate();
}

/*
    Setup common to all constructors
*/
void QEFileImage::setup() {

    // Set up data
    // This control used a single data source
    setNumVariables(1);

    // Set minimum size
    setMinimumSize( 100, 100 );

    // Set up default properties
    setAllowDrop( false );

    // Set the initial state
    isConnected = false;

    // Use standard context menu
    setupContextMenu();

//    defaultStyleSheet = styleSheet();
    // Use label signals
    // --Currently none--

    // Set up a connection to recieve variable name property changes
    // The variable name property manager class only delivers an updated variable name after the user has stopped typing
    QObject::connect( &variableNamePropertyManager, SIGNAL( newVariableNameProperty( QString, QString, unsigned int ) ), this, SLOT( useNewVariableNameProperty( QString, QString, unsigned int) ) );

}

/*
    Implementation of QEWidget's virtual funtion to create the specific type of QCaObject required.
    For a label a QCaObject that streams strings is required.
*/
qcaobject::QCaObject* QEFileImage::createQcaItem( unsigned int variableIndex ) {
    // Create the item as a QEString
   return new QEString( getSubstitutedVariableName( variableIndex ), this, &stringFormatting, variableIndex );
}

/*
    Start updating.
    Implementation of VariableNameManager's virtual funtion to establish a connection to a PV as the variable name has changed.
    This function may also be used to initiate updates when loaded as a plugin.
*/
void QEFileImage::establishConnection( unsigned int variableIndex ) {

    // Create a connection.
    // If successfull, the QCaObject object that will supply data update signals will be returned
    qcaobject::QCaObject* qca = createConnection( variableIndex );

    // If a QCaObject object is now available to supply data update signals, connect it to the appropriate slots
    if(  qca ) {
        QObject::connect( qca,  SIGNAL( stringChanged( const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                          this, SLOT( setLabelImage( const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
        QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo&, const unsigned int&  ) ),
                          this, SLOT( connectionChanged( QCaConnectionInfo&, const unsigned int&  ) ) );
        QObject::connect( this, SIGNAL( requestResend() ),
                          qca, SLOT( resendLastData() ) );
    }
}


/*
    Act on a connection change.
    Change how the label looks and change the tool tip
    This is the slot used to recieve connection updates from a QCaObject based class.
 */
void QEFileImage::connectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex )
{
    // Note the connected state
    isConnected = connectionInfo.isChannelConnected();

    // Display the connected state
    updateToolTipConnection( isConnected );
    processConnectionInfo( isConnected );

    // Signal channel connection change to any (Link) widgets.
    // using signal dbConnectionChanged.
    //
    emitDbConnectionChanged( variableIndex );
}

/*
    Update the label pixmap from variable data.
    This is the slot used to recieve data updates from a QCaObject based class.
 */
void QEFileImage::setLabelImage( const QString& textIn, QCaAlarmInfo& alarmInfo,
                                 QCaDateTime&, const unsigned int& variableIndex ) {

    // Signal a database value change to any Link widgets
    emit dbValueChanged( textIn );

    // Update the image
    setImageFileName( textIn );

    // Invoke common alarm handling processing.
    processAlarmInfo( alarmInfo );

    // Signal a database value change to any Link (or other) widgets using one
    // of the dbValueChanged signals declared in header file.
    emitDbValueChanged( variableIndex );
}

/*
    Slot to update the label pixmap from any source
 */
void QEFileImage::setImageFileName( const QString& text )
{
    // Find the file
    QFile* imageFile =  QEWidget::findQEFile( text );

    // If filename not found
    if( !imageFile )
    {
        QPixmap emptyPixmap;
        setPixmap( emptyPixmap );
        fileName = text;
        setText( QString( "File not found: " ).append( text ) );
        return;
    }

    // Get the filename and discard the image file object
    fileName = imageFile->fileName();
    delete imageFile;

    // Clear any text
    clear();

    // Update the label pixmap
    QPixmap pixmap( fileName );
    setPixmap( pixmap.scaled( size(), Qt::KeepAspectRatio ));

    // Ensure no other files are being monitored
    QStringList monitoredPaths = fileMon.files();
    if( monitoredPaths.count())
    {
        fileMon.removePaths( monitoredPaths );
    }

    // Monitor this file
    fileMon.addPath( fileName );

    // Prepare to recieve notification of changes to this file
    QObject::connect( &fileMon, SIGNAL( fileChanged( const QString & ) ), this, SLOT( setImageFileName( const QString & ) ) );
}

//==============================================================================
// Drag drop
void QEFileImage::setDrop( QVariant drop )
{
    setVariableName( drop.toString(), 0 );
    establishConnection( 0 );
}

QVariant QEFileImage::getDrop()
{
    if( isDraggingVariable() )
        return QVariant( copyVariable() );
    else
        return copyData();
}

//==============================================================================
// Copy / Paste
QString QEFileImage::copyVariable()
{
    return getSubstitutedVariableName(0);
}

QVariant QEFileImage::copyData()
{
    return QVariant( fileName );
}

void QEFileImage::paste( QVariant v )
{
    if( getAllowDrop() )
    {
        setDrop( v );
    }
}

//==============================================================================
// Property convenience functions

// None at the moment
