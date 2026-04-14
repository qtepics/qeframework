/*  QELabel.cpp
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
  This class is a CA aware label widget based on the Qt label widget.
  It is tighly integrated with the base class QEWidget.
  Refer to QEWidget.cpp for details
 */

#include "QELabel.h"
#include <QDebug>
#include <QECommon.h>

#define DEBUG  qDebug () << "QELabel" << __LINE__ << __FUNCTION__ << "  "

#define PV_VARIABLE_INDEX      0

/*
    Constructor with no initialisation
*/
QELabel::QELabel( QWidget *parent ) :
   QLabel( parent ),
   QEWidget( this ),
   QEManagePixmaps (),
   QESingleVariableMethods ( this, PV_VARIABLE_INDEX )
{
   setup();
}

/*
    Constructor with known variable
*/
QELabel::QELabel( const QString &variableNameIn, QWidget *parent ) :
   QLabel( parent ),
   QEWidget( this ),
   QEManagePixmaps (),
   QESingleVariableMethods ( this, PV_VARIABLE_INDEX )
{
   setup();
   setVariableName( variableNameIn, PV_VARIABLE_INDEX );
   activate();
}

// Destructor
//
QELabel::~QELabel() {} // place holder

/*
    Setup common to all constructors
*/
void QELabel::setup()
{
   // Set up data
   // This control used a single data source
   setNumVariables( 1 );

   // Set up default properties
   setAllowDrop( false );

   // Set the initial state
   this->setText( "----" );
   this->setIndent( 6 );
   this->processConnectionInfo( false, 0 );
   this->mUpdateOption = Text;

   // Use standard context menu
   setupContextMenu();

   setStyleSheet( QEUtilities::offBackgroundStyle() );    // By pass the normal designer check
   setDefaultStyle( QEUtilities::offBackgroundStyle() );  // This will kick in at runtime

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   connectPvNameProperties( SLOT ( usePvNameProperties( const QEPvNameProperties& ) ) );
}

/*
   Allows the default style to be set at run time.
 */
void QELabel::setDefaultStyle( const QString& style )
{
   setStyleDefault( style );
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type
// of QEChannel required.  For a label a QEChannel that streams strings is
// required.
//
QEChannel* QELabel::createQcaItem( unsigned int variableIndex )
{
   // Create the item as a QEString.
   //
   const QString pvName = this->getSubstitutedVariableName( variableIndex );
   QEChannel* result = new QEString (pvName, this, &this->stringFormatting, variableIndex);

   // Apply currently defined array index/elements request values.
   //
   this->setSingleVariableQCaProperties (result);

   return result;
}

//------------------------------------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QELabel::establishConnection( unsigned int variableIndex )
{
   // Create a connection.  If successfull, the QEChannel object that will
   // supply data update signals will be returned.
   //
   QEChannel* qca = this->createConnection( variableIndex );

   // If a QEChannel object is now available to supply data update signals,
   // connect it to the appropriate slots.
   //
   if (qca) {
      QObject::connect( qca,  SIGNAL( valueUpdated( const QEStringValueUpdate& ) ),
                        this, SLOT(   setLabelText( const QEStringValueUpdate& ) ) );
      qca->setRequestedElementCount( 10000 );

      QObject::connect( qca,  SIGNAL( connectionUpdated( const QEConnectionUpdate& ) ),
                        this, SLOT(   connectionUpdated( const QEConnectionUpdate&) ) );

      QObject::connect( this, SIGNAL( requestResend() ),
                        qca,  SLOT(   resendLastData() ) );
   }
}


//------------------------------------------------------------------------------
// Act on a connection change.  This is the slot used to recieve connection
// updates from a QEChannel based class.
// Change how the label looks and change the tool tip.
//
void QELabel::connectionUpdated (const QEConnectionUpdate& update)
{
   const unsigned int vi = update.variableIndex;

   // Note the connected state.
   //
   bool isConnected = update.connectionInfo.isChannelConnected();

   // Display the connected state.
   //
   this->updateToolTipConnection (isConnected, vi);
   this->processConnectionInfo (isConnected, vi);

   // Signal channel connection change to any Link widgets,
   // using signal dbConnectionChanged.
   //
   this->emitDbConnectionChanged (vi);
}


//------------------------------------------------------------------------------
// Update the label text.
// This is the slot used to recieve data updates from a QEChannel based class.
//
void QELabel::setLabelText( const QEStringValueUpdate& update )
{
   const unsigned int vi = update.variableIndex;

   // Extract any formatting info from the text.
   // For example "<background-color: red>Engineering Mode" or "<color: red>not selected"
   // Does anyone use this???
   //
   currentText = update.value;
   QString textStyle;
   int textStyleStart = currentText.indexOf( '<' );
   if( textStyleStart >= 0 )
   {
      int textStyleEnd = currentText.indexOf( '>', textStyleStart );
      if( textStyleEnd >= 1 )
      {
         textStyle = currentText.mid( textStyleStart+1, textStyleEnd-textStyleStart-1 );
         currentText = currentText.left( textStyleStart ).append( currentText.right( currentText.length()-textStyleEnd-1 ));
      }
   }

   // Update the color.
   //
   if( textStyle.compare( lastTextStyle ) )
   {
      if( !textStyle.isEmpty() )
      {
         this->updateDataStyle( QString( "QWidget { " ).append( textStyle ).append( "; }") );
      }
      else
      {
         this->updateDataStyle( "" );
      }
      lastTextStyle = textStyle;
   }

   switch (this->mUpdateOption) {
      case Text:
         // Update the text if required.
         this->setText( currentText );
         break;

      case Picture:
         // Update the pixmap if required
         this->setPixmap( getDataPixmap( currentText ).scaled( size() ) );
         break;
   }

   // Invoke common alarm handling processing.
   //
   this->processAlarmInfo (update.alarmInfo);

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged.
   //
   this->emitDbValueChanged (currentText, vi);
}

//------------------------------------------------------------------------------
//
void QELabel::usePvNameProperties (const QEPvNameProperties& pvNameProperties)
{
   this->setVariableNameAndSubstitutions (pvNameProperties.pvName,
                                          pvNameProperties.substitutions,
                                          pvNameProperties.index);
}


//==============================================================================
// Drag drop
void QELabel::setDrop( QVariant drop )
{
   setVariableName( drop.toString(), PV_VARIABLE_INDEX );
   establishConnection( PV_VARIABLE_INDEX );
}

QVariant QELabel::getDrop()
{
   if( isDraggingVariable() )
      return QVariant( copyVariable() );
   else
      return copyData();
}

//==============================================================================
// Copy / Paste
QString QELabel::copyVariable()
{
   return getSubstitutedVariableName( PV_VARIABLE_INDEX );
}

QVariant QELabel::copyData()
{
   return QVariant( currentText );
}

void QELabel::paste( QVariant v )
{
   if( getAllowDrop() )
   {
      setDrop( v );
   }
}

//==============================================================================
// Property convenience functions

// Update option Property convenience function
void QELabel::setUpdateOption(const UpdateOptions updateOptionIn)
{
   this->mUpdateOption = updateOptionIn;
}

QELabel::UpdateOptions QELabel::getUpdateOption() const
{
   return this->mUpdateOption;
}

// end
