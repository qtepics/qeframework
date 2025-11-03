/*  QELineEdit.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2009-2025 Australian Synchrotron
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
QELineEdit::QELineEdit( QWidget *parent ) : QEGenericEdit( parent )
{
   setup();
}

/*
    Constructor with known variable
*/
QELineEdit::QELineEdit( const QString& variableNameIn, QWidget *parent ) : QEGenericEdit( variableNameIn, parent )
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
    Implementation of QEWidget's virtual funtion to create the specific type of QCaObject required.
    For a line edit a QCaObject that streams strings is required.
*/
qcaobject::QCaObject* QELineEdit::createQcaItem( unsigned int variableIndex ) {

   qcaobject::QCaObject* result = NULL;

   // Create the item as a QEString
   QString pvName = getSubstitutedVariableName( variableIndex );
   result = new QEString( pvName, this, &stringFormatting, variableIndex );

   // Apply currently defined array index/elements request values.
   setSingleVariableQCaProperties( result );

   return result;
}

/*
    Start updating.
    Implementation of VariableNameManager's virtual funtion to establish a connection to a PV as the variable name has changed.
    This function may also be used to initiate updates when loaded as a plugin.
*/
void QELineEdit::establishConnection( unsigned int variableIndex ) {

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   qcaobject::QCaObject* qca = createConnection( variableIndex );

   // If a QCaObject object is now available to supply data update signals, connect it to the appropriate slots
   if(  qca ) {
      QObject::connect( qca,  SIGNAL( stringChanged( const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                        this, SLOT( setTextIfNoFocus( const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
      QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo&, const unsigned int&  ) ),
                        this, SLOT( connectionChanged( QCaConnectionInfo&, const unsigned int&  ) ) );
      QObject::connect( this, SIGNAL( requestResend() ),
                        qca, SLOT( resendLastData() ) );
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
    This is the slot used to recieve data updates from a QCaObject based class.
*/
void QELineEdit::setTextIfNoFocus( const QString& value, QCaAlarmInfo& alarmInfo, QCaDateTime& dateTime, const unsigned int& ) {

   // Do generic update processing.
   setDataIfNoFocus (QVariant (value), alarmInfo, dateTime);

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged.
   //
   emitDbValueChanged( value, 0 );
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
