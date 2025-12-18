/*  QEGenericEdit.cpp
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


// This class is a generic CA aware line edit widget based on the Qt line edit widget.
// It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details
//
#include "QEGenericEdit.h"
#include <QDebug>
#include <QMessageBox>

#define DEBUG  qDebug () << "QEGenericEdit" << __LINE__ << __FUNCTION__ << " "

//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QEGenericEdit::QEGenericEdit( QWidget *parent ) :
   QLineEdit( parent ),
   QESingleVariableMethods( this, 0 ),
   QEWidget( this )
{
   setup ();
}


//------------------------------------------------------------------------------
// Constructor with known variable
//
QEGenericEdit::QEGenericEdit( const QString &variableNameIn, QWidget *parent) :
   QLineEdit( parent ),
   QESingleVariableMethods( this, 0 ),
   QEWidget( this )
{
   setup ();
   setVariableName( variableNameIn, 0 );
   activate();
}

//------------------------------------------------------------------------------
// Setup common to all constructors
//
void QEGenericEdit::setup()
{

   // Set up data
   // This control used a single data source
   setNumVariables( 1 );

   // Set variable index used to select write access cursor style.
   setControlPV( 0 );

   // Set up default properties
   writeOnLoseFocus = false;
   writeOnEnter = true;
   writeOnFinish = true;
   confirmWrite = false;
   isAllowFocusUpdate = false;
   isFirstUpdate = false;

   setAllowDrop( false );
   setDropOption( QE::DropToVariable );

   // Set the initial state
   isConnected = false;
   messageDialogPresent = false;
   writeFailMessageDialogPresent = false;

   // Use standard context menu
   setupContextMenu();

   // Use line edit signals
   QObject::connect( this, SIGNAL( returnPressed     () ),
                     this, SLOT  ( userReturnPressed () ) );
   QObject::connect( this, SIGNAL( editingFinished     () ),
                     this, SLOT  ( userEditingFinished () ) );

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   connectNewVariableNameProperty( SLOT( useNewVariableNameProperty( QString, QString, unsigned int ) ) );

}

// Return the Qt default context menu.
// This is added to the QE context menu
QMenu* QEGenericEdit::getDefaultContextMenu()
{
   QMenu* menu = createStandardContextMenu();
   menu->setTitle( "Edit..." );
   return menu;
}

//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the label looks and change the tool tip
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QEGenericEdit::connectionChanged( QCaConnectionInfo& connectionInfo,
                                       const unsigned int& variableIndex )
{
   // Note the connected state
   isConnected = connectionInfo.isChannelConnected();

   // Note if first update has arrived (ok to set repeatedly)
   if( isConnected )
   {
      isFirstUpdate = true;
   }

   // Display the connected state
   updateToolTipConnection( isConnected );
   processConnectionInfo( isConnected );

   // Set cursor to indicate access mode.
   setAccessCursorStyle();

   // Signal channel connection change to any (Link) widgets.
   // using signal dbConnectionChanged.
   //
   emitDbConnectionChanged( variableIndex );
}

//------------------------------------------------------------------------------
// Generic update logic.
//
void QEGenericEdit::setDataIfNoFocus( const QVariant& value, QCaAlarmInfo& alarmInfo, QCaDateTime& ) {

   // Save the most recent value.
   // If the user is editing the value updates are not applied. If the user cancels the write, the value the widget
   // should revert to the latest value.
   // This last value is also used to manage notifying user changes (save what the user will be changing from)
   lastValue = value;

   // Update the text if appropriate.
   // If the user is editing the object then updates will be
   // inapropriate, unless it is the first update and the
   // user has not started changing the text.
   // Update alays allowed iff isAllowFocusUpdate has been set true.
   if(( isAllowFocusUpdate ) ||
         ( !hasFocus() && !messageDialogPresent ) ||
         (  hasFocus() && !isModified() && isFirstUpdate ))
   {
      setValue( value );
      lastUserValue = value;
   }

   // Invoke common alarm handling processing.
   processAlarmInfo( alarmInfo );

   // First (and subsequent) update is now over
   isFirstUpdate = false;
}

//------------------------------------------------------------------------------
// The user has pressed return/enter. (Not write when user enters the widget)
// Note, it doesn't matter if the user presses return and both this function
// AND userReturnPressed() is called since setText is called in each to clear
// the 'isModified' flag. So, the first called will perform the write, the
// second (if any) will do nothing.
//
void QEGenericEdit::userReturnPressed()
{
   // If not connected, do nothing
   if( !isConnected )
   {
      return;
   }

   // Get the variable to write to
   qcaobject::QCaObject *qca = getQcaItem(0);

   // If a QCa object is present (if there is a variable to write to)
   // and the object is set up to write when the user presses return
   // then write the value.
   // Note, write even if the value has not changed (isModified() is not checked)

   if( qca && writeOnEnter )
   {
      // Note: getValue is a dispatching hook procedure.
      writeValue( qca, getValue () );
   }
}

//------------------------------------------------------------------------------
// The user has 'finished editing' such as pressed return/enter or moved
// focus from the object.
// Note, it doesn't matter if the user presses return and both this function
// AND userReturnPressed() is called since setText is called in each to clear
// the 'isModified' flag. So, the first called will perform the write, the
// second (if any) will do nothing.
//
void QEGenericEdit::userEditingFinished()
{
   // If not connected, do nothing
   if( !isConnected )
   {
      return;
   }

   // Do nothing if the user is still effectivly working with the widget (just moved to a dialog box)
   // Any signals received while messageDialogPresent is true should be ignored.
   // A signal occurs after the 'write failed' dialog closes, so a it sets
   // writeFailMessageDialogPresent to allow this code to ignore the signal.
   if( messageDialogPresent || writeFailMessageDialogPresent )
   {
      if( !messageDialogPresent )
      {
         writeFailMessageDialogPresent = false;
         setFocus();
      }
      return;
   }

   // If no changes were made by the user, do nothing
   if( !isModified() || !writeOnFinish )
   {
      return;
   }

   // Get the variable to write to
   qcaobject::QCaObject *qca = getQcaItem(0);

   // If a QCa object is present (if there is a variable to write to)
   // and the object is set up to write when the user changes focus away from the object
   // and the text has actually changed
   // then write the value
   if( qca && writeOnLoseFocus )
   {
      // Note: getValue is a dispatching hook procedure.
      writeValue( qca, getValue () );
   }

   // If, for what ever reason, the value has been changed by the user but not but not written
   // check with the user what to do about it.
   else
   {
      messageDialogPresent = true;
      int confirm = QMessageBox::warning( this, "Value changed", "You altered a value but didn't write it.\nDo you want to write this value?",
                                          QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::No );
      messageDialogPresent = false;

      switch( confirm )
      {
         // Write the value
         case QMessageBox::Yes:
            if( qca ) {
               // Note: getValue is a dispatching hook procedure.
               writeValue( qca, getValue () );
            }
            break;

            // Abort the write, revert to latest value
         case QMessageBox::No:
            setValue( lastValue );       // Note, also clears 'isModified' flag
            // setValue  is dispatching hook function
            break;

            // Don't write the value, move back to the field being edited
         case QMessageBox::Cancel:
            setFocus();
            break;
      }
   }
}

//------------------------------------------------------------------------------
// Write a value immediately.
// Used when writeOnLoseFocus, writeOnEnter, writeOnFinish are all false
// (widget will never write due to the user pressing return or leaving the widget)
//
void QEGenericEdit::writeNow ()
{
   // If not connected, do nothing
   if( !isConnected )
   {
      return;
   }

   // Get the variable to write to
   qcaobject::QCaObject *qca = getQcaItem(0);

   // If a QCa object is present (if there is a variable to write to)
   // and is of the corect type then write the value.
   //
   if ( qca ) {
      // Invokes a whole bunch of dialog logic, but eventually calls writeData.
      // Note: getValue is a dispatching hook procedure.
      //
      writeValue( qca, getValue () );
   }
}

//------------------------------------------------------------------------------
// Write a value in response to user editing the widget
// Request confirmation if required.
//
void QEGenericEdit::writeValue( qcaobject::QCaObject *, QVariant newValue )
{
   // If required, get confirmation from the user as to what to do
   int confirm = QMessageBox::Yes;
   if( confirmWrite )
   {
      messageDialogPresent = true;
      confirm = QMessageBox::warning( this, "Confirm write", "Do you want to write this value?",
                                      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes );
      messageDialogPresent = false;
   }

   // Perform the required action. Either write the value (the default) or what ever the user requested
   switch( confirm )
   {
      // Write the value and inform any derived class
      case QMessageBox::Yes:
         // Write the value
         {
            QString error;
            // Write the value - writeData is dispatching hook function
            if( !writeData( newValue, error ) )
            {
               // write failed
               // Flag what dialog activity is going on so spurious 'editing finished' signals can be ignored
               messageDialogPresent = true;
               writeFailMessageDialogPresent = true;
               // warn user
               QMessageBox::warning( this, QString( "Write failed" ), error, QMessageBox::Cancel );
               setFocus();
               // Clear flag indicating 'editing finished' signals are due to message dialog
               messageDialogPresent = false;
            }

            // Write ok
            else
            {
               // Manage notifying user changes
               emit userChange( newValue, lastUserValue, lastValue );

               // Clear 'isModified' flag
               setText( text() );
            }
         }
         break;

         // Abort the write, revert to latest value
      case QMessageBox::No:
         // Revert the text. Note, also clears 'isModified' flag
         // setValue  is dispatching hook function
         //
         setValue( lastValue );
         break;

         // Don't write the value, keep editing the field
      case QMessageBox::Cancel:
         // Do nothing
         break;
   }
}


//------------------------------------------------------------------------------
// Update variable name etc.
//
void QEGenericEdit::useNewVariableNameProperty( QString variableNameIn,
                                                QString variableNameSubstitutionsIn,
                                                unsigned int variableIndex )
{
   setVariableNameAndSubstitutions(variableNameIn, variableNameSubstitutionsIn, variableIndex);
}


//------------------------------------------------------------------------------
// Is connected state
//
bool QEGenericEdit::getIsConnected () const
{
   return isConnected;
}

//------------------------------------------------------------------------------
// Return if this is first update.
//
bool QEGenericEdit::getIsFirstUpdate () const
{
   return isFirstUpdate;
}

//==============================================================================
// Drag drop
//
void QEGenericEdit::setDrop( QVariant drop )
{
   switch (getDropOption()) {
      case QE::DropToVariable:
         setVariableName( drop.toString(), 0 );
         establishConnection( 0 );
         break;

      case QE::DropToText:
         setText(drop.toString());
         break;

      case QE::DropToTextAndWrite:
         setText(drop.toString());
         writeNow();
         break;

      default:
         DEBUG << "bad drop option " << int (getDropOption() );
         break;
   }
}

//------------------------------------------------------------------------------
//
QVariant QEGenericEdit::getDrop()
{
   return QVariant( getSubstitutedVariableName(0) );
}


//==============================================================================
// Copy / Paste
QString QEGenericEdit::copyVariable()
{
   return getSubstitutedVariableName(0);
}

QVariant QEGenericEdit::copyData()
{
   return QVariant( text() );
}

void QEGenericEdit::paste( QVariant v )
{
   if( getAllowDrop() )
   {
      setDrop( v );
   }
}

//==============================================================================
// Property convenience functions
//
// write on lose focus
void QEGenericEdit::setWriteOnLoseFocus( bool writeOnLoseFocusIn )
{
   writeOnLoseFocus = writeOnLoseFocusIn;
}
bool QEGenericEdit::getWriteOnLoseFocus()
{
   return writeOnLoseFocus;
}

//------------------------------------------------------------------------------
// write on enter
void QEGenericEdit::setWriteOnEnter( bool writeOnEnterIn )
{
   writeOnEnter = writeOnEnterIn;
}
bool QEGenericEdit::getWriteOnEnter()
{
   return writeOnEnter;
}

//------------------------------------------------------------------------------
// write on finish
void QEGenericEdit::setWriteOnFinish( bool writeOnFinishIn )
{
   writeOnFinish = writeOnFinishIn;
}
bool QEGenericEdit::getWriteOnFinish()
{
   return writeOnFinish;
}

//------------------------------------------------------------------------------
// subscribe
void QEGenericEdit::setSubscribe( bool subscribeIn )
{
   subscribe = subscribeIn;
}
bool QEGenericEdit::getSubscribe()
{
   return subscribe;
}

//------------------------------------------------------------------------------
// confirm write
void QEGenericEdit::setConfirmWrite( bool confirmWriteIn )
{
   confirmWrite = confirmWriteIn;
}
bool QEGenericEdit::getConfirmWrite()
{
   return confirmWrite;
}

//------------------------------------------------------------------------------
// set allow updatws while widget has focus.
void QEGenericEdit::setAllowFocusUpdate( bool allowFocusUpdateIn )
{
   isAllowFocusUpdate = allowFocusUpdateIn;
}

bool QEGenericEdit::getAllowFocusUpdate() const
{
   return isAllowFocusUpdate;
}

// end
