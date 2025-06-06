/*  QEComboBox.cpp
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
 *    andrew.rhyder@synchrotron.org
 */

/*
  This class is a CA aware combo box widget based on the Qt combo box widget.
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details
 */

#include "QEComboBox.h"
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
   setup();
}

//------------------------------------------------------------------------------
// Construct a combo box with a variable specified
//
QEComboBox::QEComboBox( const QString &variableNameIn, QWidget *parent ) :
   QComboBox( parent ),
   QESingleVariableMethods ( this, PV_VARIABLE_INDEX ),
   QEWidget( this )
{
   setVariableName( variableNameIn, PV_VARIABLE_INDEX );

   setup();

   activate();
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
   updatePropertyStyle ("QWidget { selection-background-color: rgb(80, 160, 255); } " );

   // Set up data
   // This control used a single data source
   setNumVariables(1);

   // Set variable index used to select write access cursor style.
   setControlPV( PV_VARIABLE_INDEX );

   // Set up default properties
   useDbEnumerations = true;
   writeOnChange = true;
   subscribe = true;
   setAllowDrop( false );
   setMaxVisibleItems (16);

   // Set the initial state
   lastValue = 0;
   isConnected = false;

   ignoreSingleShotRead = false;
   isAllowFocusUpdate = false;

   // Use standard context menu
   setupContextMenu();

   // Use line edit signals
   // Set up to write data when the user changes the value
   QObject::connect( this, SIGNAL( activated ( int ) ), this, SLOT( userValueChanged( int ) ) );

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated variable name after the user has stopped typing
   connectNewVariableNameProperty( SLOT ( useNewVariableNameProperty( QString, QString, unsigned int ) ) );

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
   result = new QEInteger( getSubstitutedVariableName( variableIndex ), this, &integerFormatting, variableIndex );

   // Apply currently defined array index/elements request values.
   setSingleVariableQCaProperties( result );

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
   qcaobject::QCaObject* qca = createConnection( variableIndex );

   // If a QCaObject object is now available to supply data update signals, connect it to the appropriate slots
   if(  qca ) {
      setCurrentIndex( 0 );
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
   isConnected = connectionInfo.isChannelConnected();

   // Display the connected state
   updateToolTipConnection( isConnected,variableIndex );
   processConnectionInfo( isConnected, variableIndex );

   // Start a single shot read if the channel is up (ignore channel down),
   // This will allow initialisation of the widget using info from the database.
   // If the combo box is already populated, then it has been set up at design time,
   // or this is a subsequent 'channel up'
   // If subscribing, then an update will occur without having to initiated one here.
   // Note, channel up implies link up
   if( isConnected && !subscribe )
   {
      QEInteger* qca = (QEInteger*)getQcaItem( PV_VARIABLE_INDEX );
      qca->singleShotRead();
      ignoreSingleShotRead = true;
   }

   // Set cursor to indicate access mode.
   setAccessCursorStyle();

   // Signal channel connection change to any Link widgets,
   // using signal dbConnectionChanged.
   emitDbConnectionChanged( PV_VARIABLE_INDEX );
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
      setComboBoxText ();
   }

   // Do nothing more if doing a single shot read (done when not subscribing to get enumeration values)
   if( ignoreSingleShotRead )
   {
      ignoreSingleShotRead = false;
      return;
   }

   // First caluate index value irrespective of whether we update or not.
   // The data HAS changed and we should signal the correct information
   int index;
   if( valueIndexMap.containsF (value) )
   {
      index = valueIndexMap.valueF (value);
   }
   else
   {
      // We haven't mapped this value.
      //
      index = -1;
   }

   // Save the last database value
   lastValue = value;

   // Update the text if appropriate.
   // If the user is editing the object then updates will be inapropriate, unless
   // it is the first update or allow updated while focused explicitly permitted.
   // !!It would be best to not update if the user has started interacting with
   // the combo box in a similar way to QELine edit where isModified() is used
   // to restrict updates. Allow if the form designer has specifically allowed
   // updates while the widget has focus.
   //
   if( isAllowFocusUpdate || !hasFocus() || isMetaDataUpdate )
   {
      setCurrentIndex( index );

      // Note the last value presented to the user
      lastUserValue = currentText();
   }

   // Invoke common alarm handling processing.
   processAlarmInfo( alarmInfo );

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged.
   emitDbValueChanged( this->itemText( index ), PV_VARIABLE_INDEX );
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
   valueIndexMap.clear();

   if( useDbEnumerations ) {
      qca = getQcaItem( PV_VARIABLE_INDEX );
      if( qca ) {
         enumerations = qca->getEnumerations();

         // Create indentity map.
         //
         for( int j = 0; j < enumerations.count(); j++ ) {
            valueIndexMap.insertF ( j, j );
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
         text = localEnumerations.valueToText( n, isMatch );

         // Unless exact match, do not use.
         //
         if( !isMatch ) continue;
         if( text.isEmpty() ) continue;

         // j is count value before we add text.
         int j = enumerations.count ();
         enumerations.append( text );

         valueIndexMap.insertF( n, j );
      }
   }

   // Clearing and re-inserting values "upsets" the current index value.
   //
   savedIndex = currentIndex();

   clear();
   insertItems( 0, enumerations );
   if( savedIndex >= count()) {
      savedIndex = -1;
   }
   setCurrentIndex ( savedIndex );

}

//------------------------------------------------------------------------------
// The user has changed the Combo box.
//
void QEComboBox::userValueChanged( int index )
{
   // Do nothing unless writing on change
   if( !writeOnChange )
      return;

   // Get the variable to write to
   QEInteger* qca = (QEInteger*)getQcaItem( PV_VARIABLE_INDEX );

   // If a QCa object is present (if there is a variable to write to)
   // then write the value
   if( qca )
   {
      // Validate
      //
      if (!valueIndexMap.containsI (index)) {
         return;
      }

      // Don't write same value.
      // Is this test actully get exersized?
      //
      const int value = valueIndexMap.valueI (index);
      if (value == lastValue) {
         return;
      }

      // Write the value
      qca->writeIntegerElement( value );

      // Notify user changes
      QStringList enumerations = qca->getEnumerations();
      QString lastValueString;
      if( lastValue >= 0 && lastValue < enumerations.size() )
      {
         lastValueString = enumerations[lastValue];
      }
      emit userChange( currentText(), lastUserValue, lastValueString );

      // Note the last value presented to the user
      lastUserValue = currentText();
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
   QEInteger* qca = (QEInteger*)getQcaItem( PV_VARIABLE_INDEX );

   // If a QCa object is present (if there is a variable to write to)
   // then write the value
   if( qca )
   {
      const int index = currentIndex();

      // Validate
      //
      if (!valueIndexMap.containsI ( index )) {
         return;
      }

      const int value = valueIndexMap.valueI (index);

      // Write the value
      qca->writeIntegerElement( value );
   }
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
   setVariableName( drop.toString(), PV_VARIABLE_INDEX );
   establishConnection( PV_VARIABLE_INDEX );
}

//------------------------------------------------------------------------------
//
QVariant QEComboBox::getDrop()
{
   return QVariant( getSubstitutedVariableName( PV_VARIABLE_INDEX ) );
}


//==============================================================================
// Copy / paste
//
QString QEComboBox::copyVariable()
{
   return getSubstitutedVariableName( PV_VARIABLE_INDEX );
}

//------------------------------------------------------------------------------
//
QVariant QEComboBox::copyData()
{
   return QVariant( currentText() );
}

//------------------------------------------------------------------------------
//
void QEComboBox::paste (QVariant s)
{
   setVariableName( s.toString(), PV_VARIABLE_INDEX );
   establishConnection( PV_VARIABLE_INDEX );
}


//==============================================================================
// Property convenience functions
//
// write on change
void QEComboBox::setWriteOnChange( bool writeOnChangeIn )
{
   writeOnChange = writeOnChangeIn;
}

//------------------------------------------------------------------------------
//
bool QEComboBox::getWriteOnChange() const
{
   return writeOnChange;
}

//------------------------------------------------------------------------------
// subscribe
//
void QEComboBox::setSubscribe( bool subscribeIn )
{
   subscribe = subscribeIn;
}

//------------------------------------------------------------------------------
//
bool QEComboBox::getSubscribe() const
{
   return subscribe;
}

//------------------------------------------------------------------------------
// use database enumerations
//
void QEComboBox::setUseDbEnumerations( bool useDbEnumerationsIn )
{
   if( useDbEnumerations != useDbEnumerationsIn ) {
      useDbEnumerations = useDbEnumerationsIn;
      setComboBoxText();
   }
}

//------------------------------------------------------------------------------
//
bool QEComboBox::getUseDbEnumerations() const
{
   return useDbEnumerations;
}

//------------------------------------------------------------------------------
// set local enuerations
//
void QEComboBox::setLocalEnumerations( const QString & localEnumerationsIn )
{
   localEnumerations.setLocalEnumeration( localEnumerationsIn );
   if( !useDbEnumerations ) {
      setComboBoxText();
   }
}

//------------------------------------------------------------------------------
//
QString QEComboBox::getLocalEnumerations() const
{
   return localEnumerations.getLocalEnumeration();
}

//------------------------------------------------------------------------------
// set allow updates while widget has focus.
//
void QEComboBox::setAllowFocusUpdate( bool allowFocusUpdateIn )
{
   isAllowFocusUpdate = allowFocusUpdateIn;
}

//------------------------------------------------------------------------------
//
bool QEComboBox::getAllowFocusUpdate() const
{
   return isAllowFocusUpdate;
}

// end
