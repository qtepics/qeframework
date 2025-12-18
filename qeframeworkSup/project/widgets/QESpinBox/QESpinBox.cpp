/*  QESpinBox.cpp
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
  This class is a CA aware spin box widget based on the Qt spin box widget.
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details
 */

#include "QESpinBox.h"
#include <QLineEdit>

#define DEBUG qDebug () << "QESpinBox" << __LINE__ << __FUNCTION__ << "  "

#define PV_VARIABLE_INDEX      0

/*
    Create a CA aware spin box with no variable name yet
*/
QESpinBox::QESpinBox( QWidget *parent ) :
   ParentWidget( parent ),
   QESingleVariableMethods ( this, PV_VARIABLE_INDEX ),
   QEWidget( this )
{
   this->setup();
}

/*
    Create a CA aware spin box with a variable name already known
*/
QESpinBox::QESpinBox( const QString &variableNameIn, QWidget *parent ) :
   ParentWidget( parent ),
   QESingleVariableMethods ( this, PV_VARIABLE_INDEX ),
   QEWidget( this )
{
   this->setVariableName( variableNameIn, PV_VARIABLE_INDEX );
   this->setup();
   this->activate();
}

/*
    Common construction
*/
void QESpinBox::setup() {
   // Set up data
   // This control used a single data source
   this->setNumVariables(1);

   // Set variable index used to select write access cursor style.
   this->setControlPV( PV_VARIABLE_INDEX );

   // Initialise the flag indicating the value is being changed programatically (not by the user)
   this->programaticValueChange = false;

   // Don't respond to every key stroke - just enter or loose focus
   this->setKeyboardTracking( false );

   // Set up default properties
   this->writeOnChange = true;
   this->setAllowDrop( false );
   this->addUnitsAsSuffix = false;
   this->useDbPrecisionForDecimal = true;
   this->autoScaleSpinBox = true;
   this->useAutoStepSize = false;

   // Set the initial state
   this->lastValue = 0.0;
   this->ignoreSingleShotRead = false;

   // Use standard context menu
   this->setupContextMenu();

   // Use spin box signals
   QObject::connect( this, SIGNAL( valueChanged( double ) ),
                     this, SLOT( userValueChanged( double ) ) );

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
bool QESpinBox::eventFilter (QObject *obj, QEvent *event)
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
// Credit: Christian Nothoff
//
void QESpinBox::stepBy (int steps)
{
   if (!this->useAutoStepSize) {
      // Functionality not enabled - just call THE parent method.
      ParentWidget::stepBy (steps);
      return;
   }

   QString line = this->lineEdit ()->text ();

   bool sign = false;
   double single_step = 1;
   if (line.contains ("+") || line.contains ("-")) {
      line.remove (0, 1);
      sign = true;
   }

   const QStringList linesplit = line.split (".");
   const int textLen = linesplit[0].length ();
   int cursorPos = qMax (lineEdit ()->cursorPosition () + 1 - (sign ? 1 : 0), 1);
   single_step = pow (10, textLen - cursorPos);
   if (cursorPos >= textLen + 2)
      single_step *= 10;
   if (cursorPos > line.length ()) {
      single_step *= 10;
      cursorPos--;
   }

   this->setSingleStep (single_step);
   ParentWidget::stepBy (steps);  // call parent method.

   line = this->lineEdit ()->text ();
   sign = false;
   if (line.contains ("+") || line.contains ("-")) {
      line.remove (0, 1);
      sign = true;
   }

   const QStringList linesplit_after = line.split (".");
   const int textLen_after = linesplit_after[0].length ();
   if (textLen_after < textLen)
      cursorPos -= 1;
   if (textLen_after > textLen)
      cursorPos += 1;
   this->lineEdit ()->setCursorPosition (cursorPos - 1 + (sign ? 1 : 0));
}

//------------------------------------------------------------------------------
// Return the Qt default context of embedded line edit menu.
// This is added to the QE context menu
//
QMenu* QESpinBox::getDefaultContextMenu()
{
   QMenu* menu = NULL;
   QLineEdit* edit = NULL;

   // QESpinBox doesn't have a  createStandardContextMenu or equivilent.
   // But it does have/use an embedded line edit object, which does.
   edit = this->lineEdit();
   if( edit ){
      menu = edit->createStandardContextMenu();
      menu->setTitle( "Edit..." );
   }
   return menu;
}

//------------------------------------------------------------------------------
//
void QESpinBox::useNewVariableNameProperty( QString pvName,
                                            QString substitutions,
                                            unsigned int variableIndex )
{
   this->setVariableNameAndSubstitutions(pvName, substitutions, variableIndex);
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of QCaObject required.
// For a spin box a QCaObject that streams real (double) numbers is required.
//
qcaobject::QCaObject* QESpinBox::createQcaItem( unsigned int variableIndex ) {

   qcaobject::QCaObject* result = NULL;

   // Create the item as a QEFloating
   result = new QEFloating( this->getSubstitutedVariableName( variableIndex ), this,
                            &floatingFormatting, variableIndex );

   // Apply currently defined array index/elements request values.
   this->setSingleVariableQCaProperties( result );

   return result;
}

//------------------------------------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QESpinBox::establishConnection( unsigned int variableIndex ) {

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   qcaobject::QCaObject* qca = createConnection( variableIndex );

   // If a QCaObject object is now available to supply data update signals,
   // connect it to the appropriate slots.
   if( qca ) {
      this->setValue( 0 );
      QObject::connect( qca,  SIGNAL( floatingChanged( const double&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ),
                        this, SLOT( setValueIfNoFocus( const double&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& ) ) );
      QObject::connect( qca,  SIGNAL( connectionChanged( QCaConnectionInfo&, const unsigned int&  ) ),
                        this, SLOT( connectionChanged( QCaConnectionInfo&, const unsigned int& ) ) );
   }
}

//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the label looks and change the tool tip
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QESpinBox::connectionChanged( QCaConnectionInfo& connectionInfo,
                                   const unsigned int &variableIndex )
{
   // Note the connected state
   const bool isConnected = connectionInfo.isChannelConnected();

   // Display the connected state
   this->updateToolTipConnection( isConnected );
   this->processConnectionInfo( isConnected );

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
      qcaobject::QCaObject* qca = this->getQcaItem(PV_VARIABLE_INDEX);
      if (qca) qca->singleShotRead();
      this->ignoreSingleShotRead = true;
   }

   // Set cursor to indicate access mode.
   this->setAccessCursorStyle();

   // Signal channel connection change to any (Link) widgets.
   // using signal dbConnectionChanged.
   //
   this->emitDbConnectionChanged( variableIndex );
}

/*
    Pass the update straight on to the SpinBox unless the user is changing it.
    Note, it would not be common to have a user editing a regularly updating
    value. However, this scenario should be allowed for. A reasonable reason
    for a user modified value to update on a gui is if is is written to by
    another user on another gui.
    This is the slot used to recieve data updates from a QCaObject based class.
    This is the slot used to recieve data updates from a QCaObject based class.
*/
void QESpinBox::setValueIfNoFocus( const double& value, QCaAlarmInfo& alarmInfo,
                                   QCaDateTime&, const unsigned int& variableIndex) {

   // Save the last database value
   //
   this->lastValue = value;

   qcaobject::QCaObject* qca = this->getQcaItem (variableIndex);
   if (!qca) return;   // sanity check
   const bool isMetaDataUpdate = qca->getIsMetaDataUpdate();

   if (isMetaDataUpdate && this->autoScaleSpinBox) {
      // Set the limits and step size
      double upper = qca->getControlLimitUpper();
      double lower = qca->getControlLimitLower();
      if( upper != lower)
      {
         this->setMaximum( qca->getControlLimitUpper() );
         this->setMinimum( qca->getControlLimitLower() );
      }
   }

   // Do nothing more if doing a single shot read (done when not subscribing to get range values)
   if( ignoreSingleShotRead )
   {
      this->ignoreSingleShotRead = false;
      return;
   }

   // Update the spin box only if the user is not interacting with the object, unless
   // the form designer has specifically allowed updates while the widget has focus.
   //
   if( this->isAllowFocusUpdate || !this->hasFocus() ) {
      // Update the spin box
      this->programaticValueChange = true;
      this->setDecimalsFromPrecision( qca );
      this->setSuffixEgu( qca );
      this->setValue( value );
      this->programaticValueChange = false;

      // Note the last value seen by the user
      this->lastUserValue = text();
   }

   // Invoke common alarm handling processing.
   this->processAlarmInfo( alarmInfo );

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged signals declared in header file.
   this->emitDbValueChanged( variableIndex );
}

//------------------------------------------------------------------------------
// The user has changed the spin box.
//
void QESpinBox::userValueChanged( double value )
{
   // If the user is not changing the value, or not writing on change, do nothing
   if( this->programaticValueChange || !this->writeOnChange )
   {
      return;
   }

   // Get the variable to write to
   QEFloating* qca = qobject_cast<QEFloating*>(this->getQcaItem(PV_VARIABLE_INDEX));

   // If a QCa object is present (if there is a variable to write to)
   // then write the value
   if( qca ) {
      // Write the value
      qca->writeFloatingElement( value );

      // Manage notifying user changes
      emit userChange( text(), lastUserValue, QString("%1").arg( lastValue ) );
   }
}

//------------------------------------------------------------------------------
// Write a value immedietly.
// Used when writeOnChange is false
// (widget will never write due to the user pressing return or leaving the widget)
void QESpinBox::writeNow()
{
   // Get the variable to write to
   QEFloating* qca = qobject_cast<QEFloating*>(this->getQcaItem(PV_VARIABLE_INDEX));

   // If a QCa object is present (if there is a variable to write to)
   // then write the value
   if( qca )
   {
      // Write the value
      qca->writeFloatingElement( value() );
   }
}

//------------------------------------------------------------------------------
// slot
void QESpinBox::setPvValue (const QString& text)
{
   bool okay;
   const double v = text.toDouble (&okay);
   if (okay) {
      this->setPvValue (v);
   } else {
      QString message = QString ("Cannot convert '%1' to a double").arg (text);

      message_types mt (MESSAGE_TYPE_INFO, MESSAGE_KIND_STANDARD);
      this->sendMessage (message, mt);
      DEBUG << message;
   }
}

//------------------------------------------------------------------------------
// slot
void QESpinBox::setPvValue (const int value)
{
   this->setPvValue (static_cast<double>(value));
}

//------------------------------------------------------------------------------
// slot
void QESpinBox::setPvValue (const double value)
{
   this->setValue (value);
   this->writeNow ();
}

//------------------------------------------------------------------------------
// slot
void QESpinBox::setPvValue (const bool value)
{
   this->setPvValue (value ? 1.0 : 0.0);
}

//------------------------------------------------------------------------------
// Set the EGU as the suffix
void QESpinBox::setSuffixEgu( qcaobject::QCaObject* qca )
{
   // If using the EGU as the suffix, and the EGU is available, set the suffix to the EGU
   // otherwise clear the suffix
   if( qca && this->addUnitsAsSuffix )
   {
      setSuffix( QString( " " ).append( qca->getEgu() ) );
   }
   else
   {
      setSuffix( "" );
   }
}

//------------------------------------------------------------------------------
// Set the spin box decimal places from the data precision if required
void QESpinBox::setDecimalsFromPrecision( qcaobject::QCaObject* qca )
{
   // If using the database precision to determine the number of decimal places, and it is available, then apply it
   if( qca && this->useDbPrecisionForDecimal )
   {
      this->setDecimals( qca->getPrecision() );
   }
}

//==============================================================================
// Drag drop
void QESpinBox::setDrop( QVariant drop )
{
   this->setVariableName( drop.toString(), PV_VARIABLE_INDEX );
   this->establishConnection( PV_VARIABLE_INDEX );
}

//------------------------------------------------------------------------------
//
QVariant QESpinBox::getDrop()
{
   return QVariant( this->getSubstitutedVariableName(PV_VARIABLE_INDEX) );
}

//==============================================================================
// Copy paste
QString QESpinBox::copyVariable()
{
   return this->getSubstitutedVariableName( PV_VARIABLE_INDEX );
}

//------------------------------------------------------------------------------
//
QVariant QESpinBox::copyData()
{
   return QVariant( value() );
}

//------------------------------------------------------------------------------
//
void QESpinBox::paste (QVariant s)
{
   this->setVariableName( s.toString(), 0 );
   this->establishConnection( PV_VARIABLE_INDEX );
}

//==============================================================================
// Property convenience functions

// write on change
void QESpinBox::setWriteOnChange( bool writeOnChangeIn )
{
   this->writeOnChange = writeOnChangeIn;
}

//------------------------------------------------------------------------------
//
bool QESpinBox::getWriteOnChange() const
{
   return this->writeOnChange;
}

//------------------------------------------------------------------------------
// subscribe
void QESpinBox::setSubscribe( bool subscribeIn )
{
   this->subscribe = subscribeIn;
}

//------------------------------------------------------------------------------
//
bool QESpinBox::getSubscribe() const
{
   return this->subscribe;
}

//------------------------------------------------------------------------------
// Add units (as suffix).
// Note, for most widgets with an 'addUnits' property, the property is passed to a
//       QEStringFormatting class where the units are added to the displayed string.
//       In this case, the units are added as the spin box suffix.
bool QESpinBox::getAddUnitsAsSuffix() const
{
   return addUnitsAsSuffix;
}

void QESpinBox::setAddUnitsAsSuffix( bool addUnitsAsSuffixIn )
{
   addUnitsAsSuffix = addUnitsAsSuffixIn;
   qcaobject::QCaObject* qca = getQcaItem(PV_VARIABLE_INDEX);
   setSuffixEgu( qca );
}

//------------------------------------------------------------------------------
//
void QESpinBox::setAutoScale (const bool autoScaleIn)
{
   this->autoScaleSpinBox = autoScaleIn;
}

//------------------------------------------------------------------------------
//
bool QESpinBox::getAutoScale () const
{
   return this->autoScaleSpinBox;
}

//------------------------------------------------------------------------------
//
void QESpinBox::setAutoStepSize( bool autoStepSize )
{
   this->useAutoStepSize = autoStepSize;
}

//------------------------------------------------------------------------------
//
bool QESpinBox::getAutoStepSize() const
{
   return this->useAutoStepSize;
}


//------------------------------------------------------------------------------
// useDbPrecision
// Note, for most widgets with an 'useDbPrecision' property, the property is passed to a
//       QEStringFormatting class where it is used to determine the precision when formatting numbers as a string.
//       In this case, it is used to determine the spin box number-of-decimals property.
void QESpinBox::setUseDbPrecisionForDecimals( bool useDbPrecisionForDecimalIn )
{
   useDbPrecisionForDecimal = useDbPrecisionForDecimalIn;
   qcaobject::QCaObject* qca = getQcaItem(PV_VARIABLE_INDEX);
   setDecimalsFromPrecision( qca );
}

//------------------------------------------------------------------------------
//
bool QESpinBox::getUseDbPrecisionForDecimals() const
{
   return useDbPrecisionForDecimal;
}

//------------------------------------------------------------------------------
// set allow updates while widget has focus.
void QESpinBox::setAllowFocusUpdate( bool allowFocusUpdateIn )
{
   isAllowFocusUpdate = allowFocusUpdateIn;
}

//------------------------------------------------------------------------------
//
bool QESpinBox::getAllowFocusUpdate() const
{
   return isAllowFocusUpdate;
}

// end
