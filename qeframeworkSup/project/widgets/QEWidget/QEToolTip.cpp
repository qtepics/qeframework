/*  QEToolTip.cpp
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
 *  Copyright (c) 2009, 2010, 2016 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#include <QApplication>
#include <QDebug>
#include <QEWidget.h>
#include <QERecordFieldName.h>

#include "QEToolTip.h"


#define DEBUG qDebug () << "QEToolTip" << __FUNCTION__ << __LINE__

QEToolTipSingleton* toolTipSingleton = NULL;

#define PROPERTY_NAME  "QE_TOOL_TIP_QEWIDGET_ADDRESS"

//==============================================================================
// static: call constructor - this function is idempotent.
void QEToolTipSingleton::constructSingleton ()
{
    if( !toolTipSingleton ) {
        // Use the application itself as parent.
        toolTipSingleton = new QEToolTipSingleton( QApplication::instance() );
    }
}

// Constructor
QEToolTipSingleton::QEToolTipSingleton ( QObject* parent ) : QObject ( parent )
{
    currentQEWidget = NULL;
    descriptionStringList.clear ();
}

// Decontruct the singleton.
QEToolTipSingleton::~QEToolTipSingleton()
{
    clear ();
    toolTipSingleton = NULL;
}

// Emtpy description list and delete associated qca objects.
void QEToolTipSingleton::clear ()
{
    while( !descriptionStringList.isEmpty() ){
       QEString* qca = descriptionStringList.takeFirst ();
       if (qca) delete qca;
    }
}

// Installs and event handler for the specified widget.
void QEToolTipSingleton::registerWidget( QWidget* widget )
{
    if( widget ){
        widget->installEventFilter( this );
    }
}

// Widget isbeing deleted: If it is the current qe widget, then tidy up.
void QEToolTipSingleton::deregisterWidget( QWidget* widget )
{
    QEWidget* qewidget = dynamic_cast <QEWidget*> (widget);
    if( qewidget ==  currentQEWidget ){
        currentQEWidget = NULL;
        clear ();
    }
}

// Use the received value to update the current widget's tool tip.
void QEToolTipSingleton::descriptionUpdate (const QString &value, QCaAlarmInfo &,
                                            QCaDateTime&, const unsigned int& variableIndex)
{
    QObject* theSender = sender ();   // who sent this update ?
    QVariant propValue = theSender->property( PROPERTY_NAME );
    bool okay;
    qlonglong sainityCheck = propValue.toLongLong( &okay );

    if( currentQEWidget && okay && ( (qlonglong) currentQEWidget == sainityCheck) ){
        currentQEWidget->updateToolTipDescription( value, variableIndex );
    }
}


// Form DESCription PV name and request data.
// Note: the assumption here is that the PV is a the name of a record or a record field hosted
// on an IOC. However if this is a PV hosted on a Portable Channel Access Server (PCAS) such as
// a gateway generated PV or a pycas PV, the <name>.DESC PV won't exist.
void QEToolTipSingleton::enterQEWidget ( QEWidget* qewidget )
{
    // Do nothing if not displaying Variable names in the tool tip
    if( !qewidget->variableAsToolTip )
    {
        return;
    }

    currentQEWidget = qewidget;  // save a reference widget of interest.

    // First delete any existing QEString objects. The API provides no means to
    // reassign the PV name, so we can't reuse these objects.
    clear ();

    const unsigned int number = (unsigned int) qewidget->getNumberVariables ();
    for (unsigned int j = 0; j < number; j++) {
        qewidget->updateToolTipDescription ("", j);  // blank until we know better.
        QString pvName = qewidget->getSubstitutedVariableName (j);
        if( !pvName.isEmpty() ) {
            QString descName = QERecordFieldName::fieldPvName (pvName, "DESC");
            QEString* qca;

            qca = new QEString (descName, this, &descriptionFormatting, j);
            // Add a reference to the QEWidget into this QEString.
            qca->setProperty (PROPERTY_NAME, QVariant ((qlonglong) currentQEWidget));

            QObject::connect (qca, SIGNAL (stringChanged     (const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& )),
                              this,  SLOT (descriptionUpdate (const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& )));

            // we don't really need to monitor DESC fields as these are basically static.
            qca->singleShotRead ();
            this->descriptionStringList.append( qca );
        }
    }
}

// Process leaving the widget.
void QEToolTipSingleton::leaveQEWidget ( QEWidget* /* qewidget */ )
{
    currentQEWidget = NULL;
    clear ();
}

// Common event filter for all widgets that arer also QEWidgets.
bool QEToolTipSingleton::eventFilter( QObject* obj, QEvent* event )
{
    const QEvent::Type type = event->type ();
    QWidget* widget;
    QEWidget* qewidget;

    switch (type) {
    case QEvent::Enter:
        widget = static_cast <QWidget*> (obj);
        qewidget = dynamic_cast <QEWidget*> (widget);
        if ( qewidget ) enterQEWidget ( qewidget );
        break;

    case QEvent::Leave:
        widget = static_cast <QWidget*> (obj);
        qewidget = dynamic_cast <QEWidget*> (widget);
        if ( qewidget ) leaveQEWidget ( qewidget );
        break;

    default:
        break;
    }

    // We don't handle events per se - just intercept them.
    return false;
}


//==============================================================================
// Constructor
QEToolTip::QEToolTip(  QWidget* ownerIn )
{
    // Sanity check.
    if( ownerIn == NULL )
    {
        qWarning( "QEToolTip constructor called with a null 'owner'" );
        exit( EXIT_FAILURE );
    }

    // Keep a handle on the underlying QWidget of the QE widgets
    owner = ownerIn;

    // Initially there are no variables associated with the tool tip
    number = 0;
    variableAsToolTip = true;

    // Create singleton object if needs be.
    QEToolTipSingleton::constructSingleton ();
    toolTipSingleton->registerWidget( owner );
}

QEToolTip::~QEToolTip()
{
    toolTipSingleton->deregisterWidget( owner );
}


// Property set: variable as tool tip
void QEToolTip::setVariableAsToolTip( const bool variableAsToolTipIn )
{
    // If and only if turning off variable as tool tip then clear the tool tip
    if( variableAsToolTip && !variableAsToolTipIn )
    {
        owner->setToolTip( "" );
    }

    // Set the new tool tip type
    variableAsToolTip = variableAsToolTipIn;

    // Update the tool tip to match the new state
    displayToolTip();
}

// Property get: variable as tool tip
bool QEToolTip::getVariableAsToolTip() const
{
    return variableAsToolTip;
}

// Ensures list are large enough.
//
void QEToolTip::setNumberToolTipVariables (const unsigned int numberIn)
{
    number = numberIn;

    while (toolTipVariable.count()    < (int) number) toolTipVariable << "";
    while (toolTipDescription.count() < (int) number) toolTipDescription << "";
    while (toolTipAlarm.count()       < (int) number) toolTipAlarm << "";
    while (toolTipIsConnected.count() < (int) number) toolTipIsConnected << false;

    while (toolTipVariable.count()    > (int) number) toolTipVariable.removeLast();
    while (toolTipDescription.count() > (int) number) toolTipDescription.removeLast();
    while (toolTipAlarm.count()       > (int) number) toolTipAlarm.removeLast();
    while (toolTipIsConnected.count() > (int) number) toolTipIsConnected.removeLast();
}

// Update the variable used in the tool tip
// (Used when displaying a dynamic tool tip only)
void QEToolTip::updateToolTipVariable ( const QString& variable, const unsigned int variableIndex )
{
    if ((int) variableIndex < toolTipVariable.count ()) {
        toolTipVariable.replace( variableIndex, variable );
        displayToolTip();
    }
}

// Update the description associated with the variable.
// (Used when displaying a dynamic tool tip only)
void QEToolTip::updateToolTipDescription ( const QString& desc, const unsigned int variableIndex )
{
    if ((int) variableIndex < toolTipDescription.count ()) {
        toolTipDescription.replace( variableIndex, desc );
        displayToolTip();
    }
}

// Update the variable alarm status used in the tool tip
// (Used when displaying a dynamic tool tip only)
void QEToolTip::updateToolTipAlarm ( const QString& alarm, const unsigned int variableIndex )
{
    if ((int) variableIndex < toolTipAlarm.count ()) {
        toolTipAlarm .replace( variableIndex, alarm );
        displayToolTip();
    }
}

// Update the variable custom information used in the tool tip
// (Used when displaying a dynamic tool tip only)
void QEToolTip::updateToolTipCustom ( const QString& custom )
{
    toolTipCustom = custom;
    displayToolTip();
}

// Update the variable connection status used in the tool tip
// (Used when displaying a dynamic tool tip only)
void QEToolTip::updateToolTipConnection ( bool isConnectedIn, const unsigned int variableIndex )
{
    if ((int) variableIndex < toolTipIsConnected.count ()) {
        toolTipIsConnected.replace (variableIndex, isConnectedIn);
        displayToolTip();
    }
}


// Build and display the tool tip from the name and state if dynamic
void QEToolTip::displayToolTip()
{
    // If using the variable name as the tool tip, build the tool tip
    if( variableAsToolTip )
    {
        int count = 0;
        QString toolTip = "";

        for (unsigned int j = 0; j < number; j++) {
            QString pvName = toolTipVariable.value (j, "");
            if( pvName.size() ) {
                if (count > 0) {
                    toolTip.append( "\n" );
                }
                count++;
                toolTip.append( pvName );

                QString desc = toolTipDescription.value( j, "" );
                if ( !desc.isEmpty() ) {
                   toolTip.append( " : " );
                   toolTip.append( desc );
                }

                if( toolTipIsConnected.value (j, false ) ){
                    // Only connected PVs have an alarm state.
                    if( toolTipAlarm [j].size() )
                        toolTip.append( " - " ).append( toolTipAlarm.value (j, "") );

                } else {
                    toolTip.append( " - Disconnected" );
                }
            }
        }
        if (count == 0) {
            toolTip = "No variables defined";
        }
        if( toolTipCustom.size() )
            toolTip.append( " - " ).append( toolTipCustom );

        owner->setToolTip( toolTip );
    }
}

// end
