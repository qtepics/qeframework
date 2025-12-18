/*  QESlider.cpp
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
  This class is a CA aware slider widget based on the Qt slider widget.
  It is tighly integrated with the base class QEWidget. Refer to QEWidget.cpp for details
 */

#include "QESlider.h"
#include <QDebug>

#define DEBUG qDebug () << "QESlider" << __LINE__<< __FUNCTION__ << "  "

//------------------------------------------------------------------------------
// Constructor with no initialisation
//
QESlider::QESlider (QWidget* parent):
   QSlider (parent),
   QESingleVariableMethods (this, pvVariableIndex),
   QEWidget (this)
{
   this->setup ();
}

//------------------------------------------------------------------------------
// Constructor with known variable
//
QESlider::QESlider (const QString& variableNameIn, QWidget* parent):
   QSlider (parent),
   QESingleVariableMethods (this, pvVariableIndex),
   QEWidget (this)
{
   this->setup ();
   this->setVariableName (variableNameIn, pvVariableIndex);
   this->activate ();
}

//------------------------------------------------------------------------------
// Setup common to all constructors
//
void QESlider::setup ()
{
   // Set up data
   // This control used a single data source
   this->setNumVariables (1);

   // Set variable index used to select write access cursor style.
   this->setControlPV (pvVariableIndex);

   // Set up default properties
   this->updateInProgress = false;
   this->m_writeOnChange = true;
   this->m_autoScale = false;  // most widgets are true by default, however backward compatibility rules.
   this->setAllowDrop (false);

   this->m_scale = 1.0;
   this->m_offset = 0.0;
   this->m_currentValue = 0.0;

   // Set the initial state
   this->ignoreSingleShotRead = false;

   // Use standard context menu
   this->setupContextMenu ();

   // Use slider signals
   QObject::connect (this, SIGNAL (valueChanged (const int&)),
                     this, SLOT (userValueChanged (const int&)));

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated variable
   // name after the user has stopped typing.
   //
   this->connectNewVariableNameProperty (SLOT (useNewVariableNameProperty (QString, QString, unsigned int)));
}


//------------------------------------------------------------------------------
//
double QESlider::sliderToPv (const int i) const
{
   return (i / this->m_scale) + this->m_offset;
}

//------------------------------------------------------------------------------
//
int QESlider::pvToSlider (const double x) const
{
   return (x - this->m_offset) * this->m_scale;
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. For a slider a QCaObject that streams integers is required.
//
qcaobject::QCaObject * QESlider::createQcaItem (unsigned int variableIndex)
{

   qcaobject::QCaObject * result = NULL;

   // Create the item as a QEFloating
   result = new QEFloating (this->getSubstitutedVariableName (variableIndex),
                            this, &this->floatingFormatting, variableIndex);

   // Apply currently defined array index/elements request values.
   this->setSingleVariableQCaProperties (result);

   return result;
}

//------------------------------------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QESlider::establishConnection (unsigned int variableIndex)
{

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   //
   qcaobject::QCaObject * qca = this->createConnection (variableIndex);

   // If a QCaObject object is now available to supply data update signals,
   // connect it to the appropriate slots.
   //
   if (qca) {
      this->setValue (0);
      QObject::connect (qca, SIGNAL (floatingChanged (const double&, QCaAlarmInfo&,
                                                      QCaDateTime&, const unsigned int&)),
                        this, SLOT (setValueIfNoFocus (const double&, QCaAlarmInfo&,
                                                       QCaDateTime&, const unsigned int&)));
      QObject::connect (qca, SIGNAL (connectionChanged (QCaConnectionInfo&, const unsigned int)),
                        this, SLOT  (connectionChanged (QCaConnectionInfo&, const unsigned int)));
   }
}

//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the label looks and change the tool tip
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QESlider::connectionChanged (QCaConnectionInfo& connectionInfo,
                                  const unsigned int&variableIndex)
{
   // Note the connected state
   //
   const bool isConnected = connectionInfo.isChannelConnected ();

   // Display the connected state
   this->updateToolTipConnection (isConnected);
   this->processConnectionInfo (isConnected);

   // !!! ??? not sure if this is right. Added as the record type was comming back as GENERIC::UNKNOWN deep in the write
   // Start a single shot read if the channel is up (ignore channel down),
   // This will allow initialisation of the widget using info from the database.
   // If subscribing, then an update will occur without having to initiated one here.
   // Note, channel up implies link up
   // Note, even though there is nothing to do to initialise the spin box if not subscribing, an
   // initial single shot read is still performed to ensure we have valid information about the
   // variable when it is time to do a write.
   //
   if (isConnected && !this->subscribe) {
      QEFloating* qca =  qobject_cast<QEFloating*>(this->getQcaItem (pvVariableIndex));
      if (qca) qca->singleShotRead ();
      this->ignoreSingleShotRead = true;
   }

   // Set cursor to indicate access mode.
   this->setAccessCursorStyle ();

   // Signal channel connection change to any (Link) widgets.
   // using signal dbConnectionChanged.
   //
   this->emitDbConnectionChanged (variableIndex);
}

//------------------------------------------------------------------------------
// Pass the update straight on to the QSlider unless the user is moving the slider.
// Note, it would not be common to have a user editing a regularly updating value. However, this
// scenario should be allowed for. A reasonable reason for a user modified value to update on a gui is
// if is is written to by another user on another gui.
// This is the slot used to recieve data updates from a QCaObject based class.
//
void QESlider::setValueIfNoFocus (const double&value, QCaAlarmInfo& alarmInfo,
                                  QCaDateTime&, const unsigned int&variableIndex)
{

   // Do nothing if doing a single shot read
   // (done when not subscribing to get enumeration values)
   if (this->ignoreSingleShotRead) {
      this->ignoreSingleShotRead = false;
      return;
   }
   // Update the slider only if the user is not interacting with the object, unless
   // the form designer has specifically allowed updates  while the widget has focus.
   if (this->m_isAllowFocusUpdate || !this->hasFocus ()) {
      this->updateInProgress = true;
      this->m_currentValue = value;
      int intValue = this->pvToSlider (value);
      this->setValue (intValue);
      this->updateInProgress = false;
   }

   // Invoke common alarm handling processing.
   this->processAlarmInfo (alarmInfo);

   // Signal a database value change to any Link (or other) widgets using one
   // of the dbValueChanged signals declared in header file.
   //
   this->emitDbValueChanged (variableIndex);
}

//------------------------------------------------------------------------------
// The user has modified the slider position.
// This will occur as the user slides the slider if tracking is enabled,
// or when the user completes sliding if tracking is not enabled.
//
void QESlider::userValueChanged (const int&value)
{

   // If the change is due to an update (and not the user)
   // or not writing on change, then ignore the change
   if (this->updateInProgress == true || !this->m_writeOnChange) {
      return;
   }

   // Get the variable to write to
   QEFloating* qca =  qobject_cast<QEFloating*>(this->getQcaItem (pvVariableIndex));

   /* If a QCa object is present (if there is a variable to write to)
    * then write the value
    */
   if (qca) {
      // Attempt to write the data if the destination data type is known.
      // It is not known until a connection is established.
      if (qca->dataTypeKnown ()) {
         this->m_currentValue = this->sliderToPv (value);
         qca->writeFloatingElement (this->m_currentValue);
      } else {
         // Inform the user that the write could not be performed.
         // It is normally not possible to get here. If the connection or link has not
         // yet been established (and therefore the data type is unknown) then the user
         // interface object should be unaccessable. This code is here in the event that
         // the user can, by design or omision, still attempt a write.
         //
         this->sendMessage ("Could not write value as type is not known yet.",
                            "QESlider::userValueChanged()",
                            message_types (MESSAGE_TYPE_WARNING));
      }
   }
}

//------------------------------------------------------------------------------
//
void QESlider::useNewVariableNameProperty (QString pvName,
                                           QString substitutions,
                                           unsigned int index)
{
   this->setVariableNameAndSubstitutions (pvName, substitutions, index);
}

//------------------------------------------------------------------------------
// Write a value immedietly.
// Used when writeOnChange is false
// (widget will never write due to the user pressing return or leaving the widget)
//
void QESlider::writeNow ()
{
   // Get the variable to write to
   QEFloating* qca =  qobject_cast<QEFloating*>(this->getQcaItem (pvVariableIndex));

   // If a QCa object is present (if there is a variable to write to)
   // then write the value
   if (qca && qca->getChannelIsConnected ()) {
      // Attempt to write the data if the destination data type is known.
      // It is not known until a connection is established.
      //
      if (qca->dataTypeKnown ()) {
         this->m_currentValue = this->sliderToPv (QSlider::value ());
         qca->writeFloatingElement (this->m_currentValue);
      }
   }
}

//------------------------------------------------------------------------------
// slot
void QESlider::setPvValue (const QString& text)
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
void QESlider::setPvValue (const int value)
{
   this->setPvValue (static_cast<double>(value));
}

//------------------------------------------------------------------------------
// slot
void QESlider::QESlider::setPvValue (const double value)
{
   // Note: this is same logic as in setValueIfNoFocus, i.e. pv value update.
   //
   this->m_currentValue = value;
   const int intValue = this->pvToSlider (value);
   this->setValue (intValue);
   this->writeNow ();
}

//------------------------------------------------------------------------------
// slot
void QESlider::setPvValue (const bool value)
{
   this->setPvValue (value ? 1.0 : 0.0);
}

//------------------------------------------------------------------------------
//
void QESlider::setDefaultStyle (const QString& style)
{
   this->setStyleDefault (style);
}

//------------------------------------------------------------------------------
//
void QESlider::setManagedVisible (bool v)
{
   this->setRunVisible (v);
}

//==============================================================================
// Drag drop
void QESlider::setDrop (QVariant drop)
{
   this->setVariableName (drop.toString (), pvVariableIndex);
   this->establishConnection (pvVariableIndex);
}

//------------------------------------------------------------------------------
//
QVariant QESlider::getDrop ()
{
   if (this->isDraggingVariable ())
      return QVariant (this->copyVariable ());
   else
      return this->copyData ();
}

//==============================================================================
// Copy / Paste
QString QESlider::copyVariable ()
{
   return this->getSubstitutedVariableName (0);
}

//------------------------------------------------------------------------------
//
QVariant QESlider::copyData ()
{
   return QVariant (this->m_currentValue);
}

//------------------------------------------------------------------------------
//
void QESlider::paste (QVariant v)
{
   if (this->getAllowDrop ()) {
      this->setDrop (v);
   }
}

//==============================================================================
// Property convenience functions
//------------------------------------------------------------------------------
// write on change
//
void QESlider::setWriteOnChange (bool writeOnChange)
{
   this->m_writeOnChange = writeOnChange;
}

//------------------------------------------------------------------------------
//
bool QESlider::getWriteOnChange () const
{
   return this->m_writeOnChange;
}

//------------------------------------------------------------------------------
// subscribe
//
void QESlider::setSubscribe (bool subscribeIn)
{
   this->subscribe = subscribeIn;
}

//------------------------------------------------------------------------------
//
bool QESlider::getSubscribe () const
{
   return this->subscribe;
}

//------------------------------------------------------------------------------
// Set scale and offset (used to scale data when inteter scale bar min and max are not suitable)
//
void QESlider::setScale (double scale)
{
   this->m_scale = scale;
}

//------------------------------------------------------------------------------
//
double QESlider::getScale () const
{
   return this->m_scale;
}

//------------------------------------------------------------------------------
//
void QESlider::setOffset (double offset)
{
   this->m_offset = offset;
}

//------------------------------------------------------------------------------
//
double QESlider::getOffset () const
{
   return this->m_offset;
}

//------------------------------------------------------------------------------
// set allow updates while widget has focus.
//
void QESlider::setAllowFocusUpdate (bool allowFocusUpdate)
{
   this->m_isAllowFocusUpdate = allowFocusUpdate;
}

//------------------------------------------------------------------------------
//
bool QESlider::getAllowFocusUpdate () const
{
   return this->m_isAllowFocusUpdate;
}

//------------------------------------------------------------------------------
//
void QESlider::setAutoScale (const bool autoScale)
{
   this->m_autoScale = autoScale;
}

//------------------------------------------------------------------------------
//
bool QESlider::getAutoScale () const
{
   return this->m_autoScale;
}

// end
