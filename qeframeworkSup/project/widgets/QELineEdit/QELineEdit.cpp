/*  QELineEdit.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2009-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/*
  This class is a CA aware line edit widget based on the Qt line edit widget.
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details
 */

#include <QELineEdit.h>
#include <QMessageBox>
#include <QDebug>

#define DEBUG qDebug () << "QELineEdit.cpp" << __LINE__ << __FUNCTION__ << "  "

/*
    Constructor with no initialisation
*/
QELineEdit::QELineEdit( QWidget *parent ) :
   QEGenericEdit( parent )
{
   setup();
}

/*
    Constructor with known variable
*/
QELineEdit::QELineEdit( const QString& variableNameIn, QWidget *parent ) :
   QEGenericEdit( variableNameIn, parent )
{
   setup();
   setVariableName( variableNameIn, 0 );
   activate();
}

void QELineEdit::setup()
{
   setAddUnits( false );
}

//------------------------------------------------------------------------------
//
void QELineEdit::setPvValue (const QString& text)
{
   this->setText (text);
   this->writeNow ();
}

//------------------------------------------------------------------------------
//
void QELineEdit::setPvValue (const int value)
{
   this->setPvValue (QString::number(value));
}

//------------------------------------------------------------------------------
//
void QELineEdit::setPvValue (const double value)
{
   this->setPvValue (QString::number(value));
}

//------------------------------------------------------------------------------
//
void QELineEdit::setPvValue (const bool value)
{
   this->setPvValue (value ? "true" : "false");
}


/*
    Implementation of QEWidget's virtual funtion to create the specific type of QEChannel required.
    For a line edit a QEChannel that streams strings is required.
*/
QEChannel* QELineEdit::createQcaItem( unsigned int variableIndex )
{
   // Create the item as a QEString.
   //
   const QString pvName = getSubstitutedVariableName( variableIndex );
   QEChannel* result = new QEString( pvName, this, &stringFormatting, variableIndex );

   // Apply currently defined array index/elements request values.
   //
   setSingleVariableQCaProperties( result );

   return result;
}

/*
    Start updating.
    Implementation of VariableNameManager's virtual funtion to establish a connection to a PV as the variable name has changed.
    This function may also be used to initiate updates when loaded as a plugin.
*/
void QELineEdit::establishConnection( unsigned int variableIndex )
{
   // Create a connection.
   // If successfull, the QEChannel object that will supply data update signals will be returned
   QEChannel* qca = createConnection( variableIndex );

   // If a QEChannel object is now available to supply data update signals, connect it to the appropriate slots
   if(  qca ) {
      QObject::connect( qca,  SIGNAL( valueUpdated(   const QEStringValueUpdate& ) ),
                        this, SLOT( setTextIfNoFocus( const QEStringValueUpdate& ) ) );

      QObject::connect( qca,  SIGNAL( connectionUpdated( const QEConnectionUpdate& ) ),
                        this, SLOT(   connectionUpdated( const QEConnectionUpdate& ) ) );

      QObject::connect( this, SIGNAL( requestResend() ),
                        qca,  SLOT(  resendLastData() ) );
   }
}

void QELineEdit::stringFormattingChange()
{
   emit requestResend();
}


/*
    Pass the text update straight on to the QLineEdit unless the user is
    editing the text.
    Note, it would not be common to have a user editing a regularly updating
    value. However, this scenario should be allowed for. A reasonable reason
    for a user updated value to update on a gui is if is is written to by
    another user on another gui.
    This is the slot used to recieve data updates from a QEChannel based class.
*/
void QELineEdit::setTextIfNoFocus( const QEStringValueUpdate& update )
{
   // Do generic update processing.
   //
   setDataIfNoFocus( { QVariant (update.value), update.alarmInfo,
                       update.timeStamp, update.variableIndex,
                       update.isMetaUpdate } );

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged.
   //
   emitDbValueChanged( update.value, 0 );
}

// Set widget to the given value
//
void QELineEdit::setValue (const QVariant & value)
{
   setText( value.toString() );
}

QVariant QELineEdit::getValue()
{
   return QVariant (text());
}

// Write the given value to the associated channel.
//
bool QELineEdit::writeData (const QVariant& value, QString& message)
{
   bool result = false;

   QEString *qca = dynamic_cast <QEString*> ( getQcaItem(0) );
   if( qca ) {

      // Should this logic be relocated into QEString?
      //
      switch( getArrayAction() ){

         case QE::Ascii:
            // convert string to zero terninates int array.
            //
            result = qca->writeString( value.toString (), message );
            break;

         case QE::Index:
            // Update specifiec element and write.
            //
            result = qca->writeStringElement( value.toString (), message );
            break;

         case QE::Append:
         default:
            message = "Invalid arrayAction property";
            result = false;
            break;
      }

   } else {
      message = "null qca object";
      result =  false;
   }

   return result;
}

// end
