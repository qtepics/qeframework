/*  QEOpaque.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2018-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEOpaque.h"

#include <QAction>
#include <QColor>
#include <QDebug>
#include <QTimer>

#include <QECommon.h>
#include <QEPlatform.h>
#include <QEFloating.h>
#include <QHeaderView>
#include <QEOpaqueData.h>

#define DEBUG qDebug () << "QEOpaque" << __LINE__ << __FUNCTION__ << "  "

#define PV_VARIABLE_INDEX       0

#define DEFAULT_CELL_HEIGHT     22
#define NULL_SELECTION          (-1)

//=============================================================================
// Constructor with no initialisation
//=============================================================================
//
QEOpaque::QEOpaque (QWidget* parent) :
   QEAbstractWidget (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)
{
   this->commonConstruct ();
}

//---------------------------------------------------------------------------------
//
QEOpaque::QEOpaque (const QString& variableNameIn, QWidget* parent) :
   QEAbstractWidget (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)
{
   this->commonConstruct ();
   this->setVariableName (variableNameIn, PV_VARIABLE_INDEX);
   this->activate ();
}

//---------------------------------------------------------------------------------
//
void QEOpaque::commonConstruct ()
{
   // Create table data object.
   //
   this->opaqueData = new QEOpaqueData ();
   this->opaqueData->clear();

   // Create internal widgets.
   //
   this->pvNameLabel = new QLabel();
   this->pvNameLabel->setFixedHeight (24);
   this->pvNameLabel->setAlignment (Qt::AlignHCenter | Qt::AlignVCenter);
   this->pvNameLabel->setStyleSheet (QEUtilities::colourToStyle (QColor (0xf0f0f0)));
   QFont font = this->pvNameLabel->font();
   font.setPointSize (font.pointSize () + 1);
   this->pvNameLabel->setFont (font);

   this->plainText = new QPlainTextEdit ();
   this->plainText->setPlaceholderText ("-- opaque data --");
   this->plainText->setLineWrapMode (QPlainTextEdit::NoWrap);
   this->plainText->setReadOnly (true);
   this->plainText->setContextMenuPolicy (Qt::NoContextMenu);

   // Copy actual widget size policy to the containing widget, then ensure
   // internal widget will expand to fill container widget.
   //
   this->setSizePolicy (this->plainText->sizePolicy ());
   this->plainText->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);

   this->layout = new QVBoxLayout (this);
   this->layout->setContentsMargins (0, 0, 0, 0);    // extact fit.
   this->layout->setSpacing (2);
   this->layout->addWidget (this->pvNameLabel);
   this->layout->addWidget (this->plainText);

   // Set default property values
   //
   this->showPvName = false;
   this->setShowPvName (false);

   // This widget uses a single data source
   //
   this->setNumVariables (1);
   this->setMinimumSize (120, 50);
   this->setVariableAsToolTip (true);
   this->setAllowDrop (true);

   // Use default standard context menu.

   this->setupContextMenu ();

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   this->connectPvNameProperties
         (SLOT (usePvNameProperties (const QEPvNameProperties&)));

   // Some events must be applied to the internal widget
   //
   this->installEventFilter (this);
}

//---------------------------------------------------------------------------------
//
QEOpaque::~QEOpaque ()
{
   // opaqueData is not a QObject - it must be explicitly deleted.
   //
   if (this->opaqueData) {
      delete this->opaqueData;
   }
}

//---------------------------------------------------------------------------------
//
QSize QEOpaque::sizeHint () const
{
   return QSize (222, 140);
}

//---------------------------------------------------------------------------------
//
bool QEOpaque::eventFilter (QObject* watched, QEvent* event)
{
   const QEvent::Type type = event->type ();
   bool result = false;

   switch (type) {
      case QEvent::FontChange:
         if (watched == this) {
            // Font changes must be aapplied to the internal widget.
            //
            if (this->plainText) {
               this->plainText->setFont (this->font ());
            }
         }
         break;

      default:
         result = false;
         break;
   }

   return result;
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QEChannel required. A QEChannel that streams integers is required.
//
QEChannel* QEOpaque::createQcaItem (unsigned int variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variable index" << variableIndex;
      return NULL;
   }

   const QString pvName = this->getSubstitutedVariableName (variableIndex);
   QEChannel* result = new QEChannel (pvName, this, PV_VARIABLE_INDEX);

   // using setSingleVariableQCaProperties not applicable here

   return result;
}

//------------------------------------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QEOpaque::establishConnection (unsigned int variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variable index" << variableIndex;
   }

   // Create a connection.
   // If successfull, the QEChannel object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QEChannel.
   //
   QEChannel* qca = createConnection (variableIndex);

   if (!qca) return;  // Sanity check

   // If a QEChannel object is now available to supply data update signals,
   // connect it to the appropriate slots.
   //
   QObject::connect (qca,  SIGNAL (connectionUpdated (const QEConnectionUpdate&)),
                     this, SLOT   (connectionUpdated (const QEConnectionUpdate&)));

   // Note: we connect to receive the 'raw' variant data.
   //
   QObject::connect (qca, SIGNAL (valueUpdated (const QEVariantUpdate&)),
                     this, SLOT  (valueUpdated (const QEVariantUpdate&)));
}

//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the s looks and change the tool tip
// This is the slot used to recieve connection updates from a QEChannel based class.
// slot
void QEOpaque::connectionUpdated (const QEConnectionUpdate& update)
{
   const unsigned int vi = update.variableIndex;

   if (vi != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variable index" << vi;
   }

   // Note the connected state.
   //
   const bool isConnected = update.connectionInfo.isChannelConnected ();

   // If this is a connect (as oppose to a disconnect) clear the
   // widget contents. Om disconnext, this leva last know values
   // on display, albeit grayed out.
   //
   if (isConnected) this->plainText->clear();

   // Enable internal widget iff connected.
   // Container widget remains enabled, so menues etc. still work.
   //
   this->plainText->setEnabled (isConnected);

   // Display the connected state
   //
   this->updateToolTipConnection (isConnected, vi);
   this->processConnectionInfo (isConnected, vi);

   // Set cursor to indicate access mode.
   //
   this->setAccessCursorStyle ();

   // Signal channel connection change to any (Link) widgets.
   // using signal dbConnectionChanged.
   //
   this->emitDbConnectionChanged (PV_VARIABLE_INDEX);
}

//-----------------------------------------------------------------------------
// slot
void QEOpaque::valueUpdated (const QEVariantUpdate& update)
{
   const unsigned int vi = update.variableIndex;

   if (vi != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << vi;
      return;
   }

   QEChannel* qca = this->getQcaItem (vi);
   if (!qca) return;  // sanity check

   QString pvName = this->getSubstitutedVariableName (vi);

   if (!this->opaqueData->assignFromVariant (update.value)) {
      if (update.isMetaUpdate) {
         DEBUG << "PV" << pvName << "does not provide OpaqueData data";
      }
      return;
   }

   if (update.isMetaUpdate) {
      // Append the description, if it exists, to the PV name label.
      //
      const QString desc = qca->getDescription();
      if (!desc.isEmpty()) {
         pvName.append(" : ").append (desc);
      }
      this->pvNameLabel->setText (pvName);
   }

   this->plainText->setPlainText (this->opaqueData->getText());

   // Invoke common alarm handling processing.
   //
   this->processAlarmInfo (update.alarmInfo, vi);

   // Signal a database value change to any Link widgets
   //
   emit this->dbValueChanged (update.value);
   emit this->dbValueChanged (*this->opaqueData);
}

//------------------------------------------------------------------------------
// slot
void QEOpaque::usePvNameProperties (const QEPvNameProperties& pvNameProperties)
{
   // Note: essentially calls createQcaItem.
   //
   this->setVariableNameAndSubstitutions (pvNameProperties.pvName,
                                          pvNameProperties.substitutions,
                                          pvNameProperties.index);

   const QString pvName = this->getSubstitutedVariableName (pvNameProperties.index);
   this->pvNameLabel->setText (pvName);
}

//==============================================================================
// Properties
//==============================================================================
//
void QEOpaque::setShowPvName (const bool showNameIn)
{
   this->showPvName = showNameIn;
   this->pvNameLabel->setVisible (this->showPvName);
}

//------------------------------------------------------------------------------
//
bool QEOpaque::getShowPvName () const
{
   return this->showPvName;
}

//==============================================================================
// Copy / Paste
//==============================================================================
//
QString QEOpaque::copyVariable ()
{
   return this->getSubstitutedVariableName (PV_VARIABLE_INDEX);
}

//------------------------------------------------------------------------------
//
QVariant QEOpaque::copyData ()
{
   return  QVariant(this->opaqueData->getText());
}

//------------------------------------------------------------------------------
//
void QEOpaque::paste (QVariant s)
{
   const QString pvName = s.toString ();
   this->pvNameLabel->setText (pvName);
   this->plainText->clear();
   this->setVariableName (pvName, PV_VARIABLE_INDEX);
   this->establishConnection (PV_VARIABLE_INDEX);
}

// end
