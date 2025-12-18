/*  QEComboBox.cpp
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

/*
  This class is a CA aware combo box widget based on the Qt combo box widget.
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details
 */

#include "QEComboBox.h"
#include <limits>
#include <QDebug>

#define DEBUG qDebug () << "QEComboBox" << __LINE__ << __FUNCTION__ << "  "

#define PV_VARIABLE_INDEX      0

//------------------------------------------------------------------------------
// Construct a combo box with no variable specified yet
//
QEComboBox::QEComboBox( QWidget *parent ) :
   QComboBox( parent ),
   QESingleVariableMethods ( this, PV_VARIABLE_INDEX ),
   QEWidget( this )
{
   this->setup();
}

//------------------------------------------------------------------------------
// Construct a combo box with a variable specified
//
QEComboBox::QEComboBox( const QString &variableNameIn, QWidget *parent ) :
   QComboBox( parent ),
   QESingleVariableMethods ( this, PV_VARIABLE_INDEX ),
   QEWidget( this )
{
   this->setVariableName( variableNameIn, PV_VARIABLE_INDEX );
   this->setup();
   this->activate();
}

//------------------------------------------------------------------------------
// Place holder
QEComboBox::~QEComboBox() { }

//------------------------------------------------------------------------------
// Common construction
//
void QEComboBox::setup()
{
   // Some environmnts seem to stuff this up - set explicitly.
   this->updatePropertyStyle ("QWidget { selection-background-color: rgb(80, 160, 255); } " );

   // Set up data
   // This control used a single data source
   this->setNumVariables(1);

   // Set variable index used to select write access cursor style.
   setControlPV( PV_VARIABLE_INDEX );

   // Set up default properties
   this->useDbEnumerations = true;
   this->writeOnChange = true;
   this->subscribe = true;
   this->setAllowDrop( false );
   this->setMaxVisibleItems (16);

   // Set the initial state
   this->lastValue = 0;
   this->isConnected = false;

   this->ignoreSingleShotRead = false;
   this->isAllowFocusUpdate = false;

   // Use standard context menu
   this->setupContextMenu();

   // Use line edit signals
   // Set up to write data when the user changes the value
   QObject::connect( this, SIGNAL( activated ( int ) ), this, SLOT( userValueChanged( int ) ) );

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated variable name after the user has stopped typing
   this->connectNewVariableNameProperty( SLOT ( useNewVariableNameProperty( QString, QString, unsigned int ) ) );

   // Change the default focus policy from WheelFocus to ClickFocus
   //
   this->setFocusPolicy (Qt::ClickFocus);
   this->installEventFilter( this );
}

//------------------------------------------------------------------------------
//
bool QEComboBox::eventFilter (QObject *obj, QEvent *event)
{
   const QEvent::Type type = event->type ();

   switch (type) {

      case QEvent::Wheel:
         if (obj == this) {
            // "Handle", i.e. ignore, the event if we do not have focus.
            return !this->hasFocus();
         }
         break;

      default:
         break;
   }

   return false;
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of QCaObject required.
// For a Combo box a QCaObject that streams integers is required.
//
qcaobject::QCaObject* QEComboBox::createQcaItem( unsigned int variableIndex )
{
   qcaobject::QCaObject* result = NULL;

   // Create the item as a QEInteger
   const QString pvName = this->getSubstitutedVariableName( variableIndex );
   result = new QEInteger( pvName, this, &integerFormatting, variableIndex );

   // Apply currently defined array index/elements request values.
   this->setSingleVariableQCaProperties( result );

   return result;
}

//------------------------------------------------------------------------------
// Start updating. Implementation of VariableNameManager's virtual funtion to
// establish a connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QEComboBox::establishConnection( unsigned int variableIndex )
{
   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   qcaobject::QCaObject* qca = this->createConnection( variableIndex );

   // If a QCaObject object is now available to supply data update signals, connect it to the appropriate slots
   if(  qca ) {
      this->setCurrentIndex( 0 );
      QObject::connect( qca,  SIGNAL( integerChanged( const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                        this, SLOT( setValueIfNoFocus( const long&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
      QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ),
                        this, SLOT( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ) );
   }
}

//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the label looks and change the tool tip.
// This is the slot used to recieve connection updates from a QCaObject based class.
//
// Perform initialisation that can only be completed once data from the variable has been read.
// Specifically, set up the combo box entries to match the enumerated types if required.
// This function is called when the channel is first established to the data.
// It will also be called if the channel fails
// and recovers. Subsequent calls will do nothing as the combo box is already populated.
//
void QEComboBox::connectionChanged( QCaConnectionInfo& connectionInfo, const unsigned int& variableIndex)
{
   // Note the connected state
   this->isConnected = connectionInfo.isChannelConnected();

   // Display the connected state
   this->updateToolTipConnection( isConnected,variableIndex );
   this->processConnectionInfo( isConnected, variableIndex );

   // Start a single shot read if the channel is up (ignore channel down),
   // This will allow initialisation of the widget using info from the database.
   // If the combo box is already populated, then it has been set up at design time,
   // or this is a subsequent 'channel up'
   // If subscribing, then an update will occur without having to initiated one here.
   // Note, channel up implies link up
   if( this->isConnected && !this->subscribe )
   {
      QEInteger* qca = qobject_cast<QEInteger*>(this->getQcaItem (PV_VARIABLE_INDEX));
      if (qca) qca->singleShotRead();
      this->ignoreSingleShotRead = true;
   }

   // Set cursor to indicate access mode.
   this->setAccessCursorStyle();

   // Signal channel connection change to any Link widgets,
   // using signal dbConnectionChanged.
   this->emitDbConnectionChanged( PV_VARIABLE_INDEX );
}

//------------------------------------------------------------------------------
// Pass the update straight on to the ComboBox unless the user is changing it.
// Note, it would not be common to have a user editing a regularly updating
// value. However, this scenario should be allowed for. A reasonable reason
// for a user modified value to update on a gui is if is is written to by
// another user on another gui.
//
// Note, this will still be called once if not subscribing to set up enumeration values.
// See  QEComboBox::dynamicSetup() for details.
//
void QEComboBox::setValueIfNoFocus( const long& value,QCaAlarmInfo& alarmInfo,
                                    QCaDateTime&, const unsigned int& variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   // Associated qca object - avoid any segmentation fault.
   //
   qcaobject::QCaObject* qca = this->getQcaItem (variableIndex);
   if (!qca) return;   // sanity check

   const bool isMetaDataUpdate = qca->getIsMetaDataUpdate();

   // If and only if first update (for this connection) then use enumeration
   // values to populate the combo box.
   // If not subscribing, there will still be an initial update to get enumeration values.
   //
   if( isMetaDataUpdate )
   {
      this->setComboBoxText ();
   }

   // Do nothing more if doing a single shot read (done when not subscribing to get enumeration values)
   if( this->ignoreSingleShotRead )
   {
      this->ignoreSingleShotRead = false;
      return;
   }

   // First caluate index value irrespective of whether we update or not.
   // The data HAS changed and we should signal the correct information
   int index;
   if( this->valueIndexMap.containsF (value) )
   {
      index = this->valueIndexMap.valueF (value);
   }
   else
   {
      // We haven't mapped this value.
      //
      index = -1;
   }

   // Save the last database value
   this->lastValue = value;

   // Update the text if appropriate.
   // If the user is editing the object then updates will be inapropriate, unless
   // it is the first update or allow updated while focused explicitly permitted.
   // !!It would be best to not update if the user has started interacting with
   // the combo box in a similar way to QELine edit where isModified() is used
   // to restrict updates. Allow if the form designer has specifically allowed
   // updates while the widget has focus.
   //
   if( this->isAllowFocusUpdate || !this->hasFocus() || isMetaDataUpdate )
   {
      this->setCurrentIndex( index );

      // Note the last value presented to the user
      this->lastUserValue = this->currentText();
   }

   // Invoke common alarm handling processing.
   this->processAlarmInfo( alarmInfo );

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged.
   this->emitDbValueChanged( this->itemText( index ), PV_VARIABLE_INDEX );
}

//------------------------------------------------------------------------------
// Set the text - either from the data base or from the localEnumertion
//
void QEComboBox::setComboBoxText()
{
   qcaobject::QCaObject * qca = NULL;
   QStringList enumerations;
   QString text;
   bool isMatch;
   int savedIndex;

   // Buid forward and revserse EPICS value to button position maps.
   // We do this even even using db enuberations and the mapping is trivial.
   //
   // Clear value to index mapping.
   //
   this->valueIndexMap.clear();

   if( this->useDbEnumerations ) {
      qca = this->getQcaItem( PV_VARIABLE_INDEX );
      if( qca ) {
         enumerations = qca->getEnumerations();

         // Create indentity map.
         //
         for( int j = 0; j < enumerations.count(); j++ ) {
            this->valueIndexMap.insertF ( j, j );
         }
      }

   } else {

      // Build up enumeration list using the local enumerations.  This may be
      // sparce - e.g.: 1 => Red, 5 => Blue, 63 => Green.   We create a reverse
      // map 0 => 1; 1 => 5; 2 => 63 so that when user selects the an element,
      // say Blue, we can map this directly to integer value of 5.
      //
      // Search upto values range -128 .. 128 - this is arbitary.
      // Maybe localEnumeration can be modified to provide a min/max value
      // or a list of values.
      //
      enumerations.clear();
      for( int n = -128; n <= 128; n++ ) {
         text = this->localEnumerations.valueToText( n, isMatch );

         // Unless exact match, do not use.
         //
         if( !isMatch ) continue;
         if( text.isEmpty() ) continue;

         // j is count value before we add text.
         int j = enumerations.count ();
         enumerations.append( text );

         this->valueIndexMap.insertF( n, j );
      }
   }

   // Clearing and re-inserting values "upsets" the current index value.
   //
   savedIndex = this->currentIndex();

   this->clear();
   this->insertItems( 0, enumerations );
   if( savedIndex >= count()) {
      savedIndex = -1;
   }
   this->setCurrentIndex ( savedIndex );
}

//------------------------------------------------------------------------------
// The user has changed the Combo box.
//
void QEComboBox::userValueChanged( int index )
{
   // Do nothing unless writing on change
   if( !this->writeOnChange )
      return;

   // Get the variable to write to
   QEInteger* qca = qobject_cast<QEInteger*>(this->getQcaItem (PV_VARIABLE_INDEX));

   // If a QCa object is present (if there is a variable to write to)
   // then write the value
   if( qca )
   {
      // Validate
      //
      if (!this->valueIndexMap.containsI (index)) {
         return;
      }

      // Don't write same value.
      // Is this test actully get exersized?
      //
      const int value = this->valueIndexMap.valueI (index);
      if (value == this->lastValue) {
         return;
      }

      // Write the value
      qca->writeIntegerElement( value );

      // Notify user changes
      QStringList enumerations = qca->getEnumerations();
      QString lastValueString;
      if( this->lastValue >= 0 && this->lastValue < enumerations.size() )
      {
         lastValueString = enumerations[this->lastValue];
      }
      emit userChange( this->currentText(), this->lastUserValue, lastValueString );

      // Note the last value presented to the user
      this->lastUserValue = currentText();
   }
}

//------------------------------------------------------------------------------
// Write a value immedietly.
// Used when writeOnChange are false
// (widget will never write due to the user pressing return or leaving the widget)
//
void QEComboBox::writeNow()
{
   // Get the variable to write to
   QEInteger* qca = qobject_cast<QEInteger*>(this->getQcaItem (PV_VARIABLE_INDEX));

   // If a QCa object is present (if there is a variable to write to)
   // then write the value
   if( qca )
   {
      const int index = this->currentIndex();

      // Validate
      //
      if (!this->valueIndexMap.containsI ( index )) {
         return;
      }

      const int value = this->valueIndexMap.valueI (index);

      // Write the value
      qca->writeIntegerElement( value );
   }
}

//------------------------------------------------------------------------------
// Note: keep aligned with QERadioGroup::setPvValue
// slot
void QEComboBox::setPvValue (const QString& text)
{
   // First check it text is one of the enumeration values.
   // If not, then check is a valid integer.
   //
   int value = this->findText (text);
   if (value == -1) {
      // Repeat with a trimmed string.
      value = this->findText (text.trimmed());
   }

   if (value >= 0 && !this->useDbEnumerations) {
      // We have a match and local enumeration is in use.
      // We must to the reverse map.
      //
      int temp;
      if (this->valueIndexMap.containsI(value)) {
         temp = this->valueIndexMap.valueI (value);
      } else {
         temp = -1;
      }
      value = temp;
   }

   if (value == -1) {
      // Try interpretting as an integer number.
      //
      bool okay;
      const int temp = text.toInt (&okay);
      if (okay) {
         value = temp;
      }
   }

   if (value >= 0) {
      this->setPvValue (value);
   } else {
      QString message = QString ("Cannot convert '%1' to an integer").arg (text);

      message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STANDARD);
      this->sendMessage (message, mt);
      DEBUG << message;
   }
}

//------------------------------------------------------------------------------
// slot
void QEComboBox::setPvValue (const int value)
{
   this->setCurrentIndex (value);
   // Note the last value presented to the user
   this->lastUserValue = this->currentText();
   this->writeNow ();
}

//------------------------------------------------------------------------------
// slot
void QEComboBox::setPvValue (const double value)
{
   static const int imin = std::numeric_limits<int>::min();
   static const int imax = std::numeric_limits<int>::max();

   if ((value >= imin) && (value <= imax)) {
      this->setPvValue (static_cast<int>(value));
   } else {
      QString message = QString ("Cannot convert '%1' to an integer").arg (value);

      message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STANDARD);
      this->sendMessage (message, mt);
      DEBUG << message;
   }
}

//------------------------------------------------------------------------------
// slot
void QEComboBox::setPvValue (const bool value)
{
   this->setPvValue (value ? 1 : 0);
}

//==============================================================================
// Context Menu
QMenu* QEComboBox::buildContextMenu ()
{
   // Start with the standard QE Widget menu
   //
   QMenu* menu = QEWidget::buildContextMenu ();

   QAction* action;
   action = new QAction ("Apply current selection", menu);
   action->setCheckable (false);
   action->setData (QECB_APPLY_CURRENT_SELECTION);

   contextMenu::insertBefore (menu, action, contextMenu::CM_SHOW_PV_PROPERTIES);
   contextMenu::insertSeparatorBefore (menu, contextMenu::CM_SHOW_PV_PROPERTIES);

   return menu;
}

//------------------------------------------------------------------------------
//
void  QEComboBox::contextMenuTriggered (int selectedItemNum)
{
   switch (selectedItemNum) {

      case QECB_APPLY_CURRENT_SELECTION:
         this->writeNow();
         break;

      default:
         // Call parent class function.
         //
         QEWidget::contextMenuTriggered (selectedItemNum);
         break;
   }
}

//==============================================================================
// Drag drop
void QEComboBox::setDrop( QVariant drop )
{
   this->setVariableName( drop.toString(), PV_VARIABLE_INDEX );
   this->establishConnection( PV_VARIABLE_INDEX );
}

//------------------------------------------------------------------------------
//
QVariant QEComboBox::getDrop()
{
   return QVariant( this->getSubstitutedVariableName( PV_VARIABLE_INDEX ) );
}


//==============================================================================
// Copy / paste
//
QString QEComboBox::copyVariable()
{
   return this->getSubstitutedVariableName( PV_VARIABLE_INDEX );
}

//------------------------------------------------------------------------------
//
QVariant QEComboBox::copyData()
{
   return QVariant( this->currentText() );
}

//------------------------------------------------------------------------------
//
void QEComboBox::paste (QVariant s)
{
   this->setVariableName( s.toString(), PV_VARIABLE_INDEX );
   this->establishConnection( PV_VARIABLE_INDEX );
}


//==============================================================================
// Property convenience functions
//
// write on change
void QEComboBox::setWriteOnChange( bool writeOnChangeIn )
{
   this->writeOnChange = writeOnChangeIn;
}

//------------------------------------------------------------------------------
//
bool QEComboBox::getWriteOnChange() const
{
   return this->writeOnChange;
}

//------------------------------------------------------------------------------
// subscribe
//
void QEComboBox::setSubscribe( bool subscribeIn )
{
   this->subscribe = subscribeIn;
}

//------------------------------------------------------------------------------
//
bool QEComboBox::getSubscribe() const
{
   return this->subscribe;
}

//------------------------------------------------------------------------------
// use database enumerations
//
void QEComboBox::setUseDbEnumerations( bool useDbEnumerationsIn )
{
   if( this->useDbEnumerations != useDbEnumerationsIn ) {
      this->useDbEnumerations = useDbEnumerationsIn;
      this->setComboBoxText();
   }
}

//------------------------------------------------------------------------------
//
bool QEComboBox::getUseDbEnumerations() const
{
   return this->useDbEnumerations;
}

//------------------------------------------------------------------------------
// set local enuerations
//
void QEComboBox::setLocalEnumerations( const QString & localEnumerationsIn )
{
   this->localEnumerations.setLocalEnumeration( localEnumerationsIn );
   if( !this->useDbEnumerations ) {
      this->setComboBoxText();
   }
}

//------------------------------------------------------------------------------
//
QString QEComboBox::getLocalEnumerations() const
{
   return this->localEnumerations.getLocalEnumeration();
}

//------------------------------------------------------------------------------
// set allow updates while widget has focus.
//
void QEComboBox::setAllowFocusUpdate( bool allowFocusUpdateIn )
{
   this->isAllowFocusUpdate = allowFocusUpdateIn;
}

//------------------------------------------------------------------------------
//
bool QEComboBox::getAllowFocusUpdate() const
{
   return this->isAllowFocusUpdate;
}

// end
