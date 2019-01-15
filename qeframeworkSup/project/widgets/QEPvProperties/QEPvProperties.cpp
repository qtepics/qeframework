/*  QEPvProperties.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2012-2019 Australian Synchrotron.
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include "QEPvProperties.h"
#include "QEPvPropertiesUtilities.h"

#include <QDebug>
#include <QTableWidgetItem>
#include <QComboBox>
#include <QFrame>
#include <QHeaderView>

#include <ContainerProfile.h>
#include <QEScaling.h>
#include <QECommon.h>
#include <QEAdaptationParameters.h>
#include <QCaAlarmInfo.h>
#include <QELabel.h>
#include <QEStringFormatting.h>
#include <QERecordFieldName.h>

#define DEBUG qDebug() << "QEPvProperties" << __LINE__ << __FUNCTION__ << "  "

// INP/OUT and CALC fields are 80, 120 should cover it.
//
#define MAX_FIELD_DATA_SIZE  120
#define PV_VARIABLE_INDEX      0


//==============================================================================
// class wide data
//==============================================================================
//
static const QString lightGreyStyle = QEUtilities::colourToStyle ("#e8e8e8");

static bool recordSpecsAreInitialised = false;       // setup housekeeping
static QERecordSpec *pDefaultRecordSpec = NULL;      // default for unknown record types
static QERecordSpecList recordSpecList;              // list of record type specs


//==============================================================================
// Utilities function
//==============================================================================
//
// This function is idempotent
//
static void initialiseRecordSpecs ()
{
   bool okay;
   QEAdaptationParameters ap ("QE_");

   // If already setup then exit.
   //
   if (recordSpecsAreInitialised) return;
   recordSpecsAreInitialised = true;

   recordSpecList.clear ();

   // Create a record spec to be used as default if we given an unknown record type.
   // All the common fields plus meta field RTYP plus VAL.
   //
   pDefaultRecordSpec = new QERecordSpec ("_default_");
   (*pDefaultRecordSpec)
         << "RTYP" << "NAME$" << "DESC$" << "ASG"   << "SCAN" << "PINI" << "PHAS"
         << "EVNT" << "TSE"   << "TSEL"  << "DTYP"  << "DISV" << "DISA" << "SDIS$"
         << "DISP" << "PROC"  << "STAT"  << "SEVR"  << "NSTA" << "NSEV" << "ACKS"
         << "ACKT" << "DISS"  << "LCNT"  << "PACT"  << "PUTF" << "RPRO" << "PRIO"
         << "TPRO" << "UDF"   << "FLNK$" << "VAL";

   okay = false;

   // First process the internal file list (from resource file).
   //
   okay |= recordSpecList.processRecordSpecFile (":/qe/pvproperties/record_field_list.txt");

   // Next agument from any file specified using the environment variable.
   //
   okay |= recordSpecList.processRecordSpecFile (ap.getString ("record_field_list", ""));

   if (okay == false) {
      DEBUG << "unable to read any record field files";
   }
}


//==============================================================================
// Tables columns
//
#define FIELD_COL                 0
#define VALUE_COL                 1
#define NUNBER_COLS               2
#define DEFAULT_SECTION_SIZE      22

#define WIDGET_MIN_WIDTH          340
#define WIDGET_MIN_HEIGHT         400

#define WIDGET_DEFAULT_WIDTH      448
#define WIDGET_DEFAULT_HEIGHT     696

#define ENUMERATIONS_MIN_HEIGHT   18
#define ENUMERATIONS_MAX_HEIGHT   100
#define NUMBER_OF_ENUMERATIONS    32


//==============================================================================
// QEPvProperties class functions
//==============================================================================
//
void QEPvProperties::createInternalWidgets ()
{
   const int label_height = 18;
   const int label_width = 48;

   int j;

   // Creates all the internal widgets including setting basic geometry.
   //
   this->topFrame = new QFrame (this);
   this->topFrame->setFixedHeight (128);     // go on - do the sums...
   this->topFrame->setObjectName ("topFrame");

   this->topFrameVlayout = new QVBoxLayout (this->topFrame);
   this->topFrameVlayout->setContentsMargins (0, 2, 0, 4);  // l, t, r, b
   this->topFrameVlayout->setSpacing (6);
   this->topFrameVlayout->setObjectName ("topFrameVlayout");

   for (j = 1; j <= 5; j++) {
      this->hlayouts [j] = new QHBoxLayout ();
      this->hlayouts [j]->setContentsMargins (2, 0, 2, 0);  // l, t, r, b
      this->hlayouts [j]->setSpacing (6);

      this->topFrameVlayout->addLayout (this->hlayouts [j], 1);
   }

   this->label1 = new QLabel ("NAME", this->topFrame);
   this->label1->setFixedSize (QSize (label_width, label_height));

   this->box = new QComboBox (this->topFrame);
   this->box->setFixedHeight (label_height + 9);
   this->hlayouts [1]->addWidget (this->label1, 0, Qt::AlignVCenter);
   this->hlayouts [1]->addWidget (this->box, 0, Qt::AlignVCenter);

   this->label2 = new QLabel ("VAL", this->topFrame);
   this->label2->setFixedSize (QSize (label_width, label_height));
   this->valueLabel = new QLabel (this->topFrame);
   this->valueLabel->setFixedHeight (label_height);
   this->valueLabel->setStyleSheet (lightGreyStyle);
   this->valueLabel->setIndent (6);

   this->hlayouts [2]->addWidget (this->label2);
   this->hlayouts [2]->addWidget (this->valueLabel);

   this->label3 = new QLabel ("HOST", this->topFrame);
   this->label3->setFixedSize (QSize (label_width, label_height));
   this->hostName = new QLabel (this->topFrame);
   this->hostName->setFixedHeight (label_height);
   this->hlayouts [3]->addWidget (this->label3);
   this->hlayouts [3]->addWidget (this->hostName);

   this->label4 = new QLabel ("TIME", this->topFrame);
   this->label4->setFixedSize (QSize (label_width, label_height));
   this->timeStamp = new QLabel (this->topFrame);
   this->timeStamp->setFixedHeight (label_height);
   this->hlayouts [4]->addWidget (this->label4);
   this->hlayouts [4]->addWidget (this->timeStamp);

   this->label5 = new QLabel ("DBF", this->topFrame);
   this->label5->setFixedSize (QSize (label_width, label_height));
   this->fieldType = new QLabel (this->topFrame);
   this->fieldType->setFixedHeight (label_height);
   this->label6 = new QLabel ("INDEX", this->topFrame);
   this->label6->setFixedSize (QSize (label_width, label_height));
   this->indexInfo = new QLabel (this->topFrame);
   this->indexInfo->setFixedHeight (label_height);
   this->hlayouts [5]->addWidget (this->label5);
   this->hlayouts [5]->addWidget (this->fieldType);
   this->hlayouts [5]->addWidget (this->label6);
   this->hlayouts [5]->addWidget (this->indexInfo);

   this->enumerationFrame = new QFrame (NULL); // is re-parented by enumerationScroll
   for (j = 0; j < NUMBER_OF_ENUMERATIONS; j++) {
      QLabel * item;
      item = new QLabel (this->enumerationFrame);
      item->setGeometry (0, 0, 128, label_height);
      this->enumerationLabelList.append (item);
   }

   // Create scrolling area and add pv frame.
   //
   this->enumerationScroll = new QScrollArea ();          // this will become parented by enumerationResize
   this->enumerationScroll->setVerticalScrollBarPolicy (Qt::ScrollBarAlwaysOn);
   this->enumerationScroll->setWidgetResizable (true);    // MOST IMPORTANT
   this->enumerationScroll->setWidget (this->enumerationFrame);

   // Create user controllable resize area
   //
   this->enumerationResize = new QEResizeableFrame (QEResizeableFrame::BottomEdge,
                                                    ENUMERATIONS_MIN_HEIGHT,
                                                    ENUMERATIONS_MAX_HEIGHT, this);
   this->enumerationResize->setFixedHeight (ENUMERATIONS_MIN_HEIGHT);
   this->enumerationResize->setFrameShape (QFrame::Panel);
   this->enumerationResize->setGrabberToolTip ("Re size enuerations");
   this->enumerationResize->setWidget (this->enumerationScroll);

   // We create this with 40 rows initially - this will get expanded if/when necessary.
   // Mainly want enough to make it look sensible in designer.
   //
   this->table = new QTableWidget (40, NUNBER_COLS, this);

   this->vlayout = new QVBoxLayout (this);
   this->vlayout->setMargin (4);
   this->vlayout->setSpacing (4);
   this->vlayout->addWidget (this->topFrame);
   this->vlayout->addWidget (this->enumerationResize);
   this->vlayout->addWidget (this->table);
}

//------------------------------------------------------------------------------
//
QEPvProperties::QEPvProperties (QWidget* parent) :
   QEAbstractDynamicWidget (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX),
   QEQuickSort ()
{
   this->recordBaseName = "";
   this->common_setup ();
}

//------------------------------------------------------------------------------
//
QEPvProperties::QEPvProperties (const QString& variableName, QWidget* parent) :
   QEAbstractDynamicWidget (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX),
   QEQuickSort ()
{
   this->recordBaseName = QERecordFieldName::recordName (variableName);
   this->common_setup ();
   this->setVariableName (variableName, PV_VARIABLE_INDEX);
   this->activate ();
}

//------------------------------------------------------------------------------
// NB. Need to do a deep clear to avoid memory loss.
//
QEPvProperties::~QEPvProperties ()
{
   // Free up all allocated QEString objects.
   //
   // Ensure the standardRecordType and alternateRecordType QEString objects
   // are deleted as well as the field QEString objects.
   //
   if (this->standardRecordType) {
      delete this->standardRecordType;
      this->standardRecordType = NULL;
   }

   if (this->alternateRecordType) {
      delete this->alternateRecordType;
      this->alternateRecordType = NULL;
   }

   while (!this->fieldChannels.isEmpty ()) {
      QEString *qca = this->fieldChannels.takeFirst ();
      if (qca) {
         delete qca;
      }
   }
}

//------------------------------------------------------------------------------
//
QSize QEPvProperties::sizeHint () const {
   return QSize (WIDGET_DEFAULT_WIDTH, WIDGET_DEFAULT_HEIGHT);
}

//------------------------------------------------------------------------------
//
void QEPvProperties::common_setup ()
{
   QTableWidgetItem* item;
   int j;
   QLabel* enumLabel;

   // ensure null so that if (this->valueLabel) is sensible.
   //
   this->valueLabel = NULL;
   this->standardRecordType = NULL;
   this->alternateRecordType = NULL;
   this->recordProcField = NULL;

   // This function only perform required actions on first call.
   //
   initialiseRecordSpecs ();

   this->previousRecordBaseName = "";
   this->previousRecordType = "";

   this->fieldsAreSorted = false;
   this->fieldChannels.clear ();
   this->variableIndexTableRowMap.clear ();

   // configure the panel and create contents
   //
   this->setFrameShape (QFrame::Panel);
   this->setFrameShadow (QFrame::Plain);

   // configure abstract dynamic widget - allow edit PV by default.
   //
   this->setEnableEditPv (true);

   // allocate and configure own widgets
   // ...and setup an alias
   //
   this->createInternalWidgets ();

   // Configure widgets
   //
#ifndef QT_NO_COMPLETER
   // Could not get completer to work
   this->box->setAutoCompletion (true);
   this->box->setAutoCompletionCaseSensitivity (Qt::CaseSensitive);
#endif

   this->box->setEditable (true);
   this->box->setMaxCount (36);
   this->box->setMaxVisibleItems (20);
   this->box->setEnabled (true);

   // These two don't seem to enforce what one might sensibly expect.
   this->box->setInsertPolicy (QComboBox::InsertAtTop);
   this->box->setDuplicatesEnabled (false);

   // We use the activated signal (as opposed to currentIndexChanged) as it
   // is only emmited on User change.
   //
   QObject::connect (this->box, SIGNAL (activated              (int)),
                     this,      SLOT   (boxCurrentIndexChanged (int)));

   // We allow ourselves to select the index programatically.
   //
   QObject::connect (this,      SIGNAL (setCurrentBoxIndex (int)),
                     this->box, SLOT   (setCurrentIndex    (int)));


   // We want to be general here - plenty of precision.
   //
   this->valueStringFormatting.setFormat (QEStringFormatting::FORMAT_DEFAULT);
   this->valueStringFormatting.setPrecision (9);
   this->valueStringFormatting.setUseDbPrecision (false);
   this->valueStringFormatting.setNotation (QEStringFormatting::NOTATION_AUTOMATIC);
   this->valueStringFormatting.setArrayAction (QEStringFormatting::INDEX);

   this->rtypStringFormatting.setArrayAction (QEStringFormatting::ASCII);
   this->rtypStringFormatting.setAddUnits (false);

   this->hostName->setIndent (4);
   this->hostName->setStyleSheet (lightGreyStyle);

   this->timeStamp->setIndent (4);
   this->timeStamp->setStyleSheet (lightGreyStyle);

   this->fieldType->setAlignment(Qt::AlignHCenter);
   this->fieldType->setStyleSheet (lightGreyStyle);

   this->indexInfo->setAlignment(Qt::AlignRight);
   this->indexInfo->setIndent (4);
   this->indexInfo->setStyleSheet (lightGreyStyle);

   for (j = 0; j < this->enumerationLabelList.count (); j++) {
      enumLabel = this->enumerationLabelList.value (j);
      enumLabel->setIndent (4);
      enumLabel->setStyleSheet (lightGreyStyle);
   }

   item = new QTableWidgetItem (" Field ");
   this->table->setHorizontalHeaderItem (FIELD_COL, item);

   item = new QTableWidgetItem (" Value ");
   this->table->setHorizontalHeaderItem (VALUE_COL, item);

   this->table->horizontalHeader()->setDefaultSectionSize (60);
   this->table->horizontalHeader()->setStretchLastSection (true);

   this->table->verticalHeader()->hide ();
   this->table->verticalHeader()->setDefaultSectionSize (DEFAULT_SECTION_SIZE);


   // Setup layout of widgets with the QEPvProperties QFrame
   //
   this->setMinimumWidth (WIDGET_MIN_WIDTH);
   this->setMinimumHeight(WIDGET_MIN_HEIGHT);

   this->fieldStringFormatting.setAddUnits (false);
   this->fieldStringFormatting.setUseDbPrecision (false);
   this->fieldStringFormatting.setPrecision (12);
   this->fieldStringFormatting.setNotation (QEStringFormatting::NOTATION_AUTOMATIC);
   this->fieldStringFormatting.setArrayAction (QEStringFormatting::ASCII);

   // - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   // Framework boiler-plate stuff.
   //
   // This control used a single PV via the framework.
   //
   this->setNumVariables (1);

   // Enable drag drop onto this widget by default.
   //
   this->setAllowDrop (true);

   // By default, the PV properties widget does not display the alarm state.
   // The internal VALue widget does this on our behalf.
   //
   this->setDisplayAlarmState (false);

   // Use standard context menu for overall widget.
   //
   this->setupContextMenu ();

   // Table related signals.
   // Do context menu for the table (this exludes the headings).
   //
   this->table->setContextMenuPolicy (Qt::CustomContextMenu);
   QObject::connect (this->table, SIGNAL (customContextMenuRequested      (const QPoint&)),
                     this,        SLOT   (customTableContextMenuRequested (const QPoint&)));

   QHeaderView* header = this->table->horizontalHeader ();
   QObject::connect (header, SIGNAL (sectionClicked     (int)),
                     this,   SLOT   (tableHeaderClicked (int)));


   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   this->connectNewVariableNameProperty
         (SLOT (useNewVariableNameProperty (QString, QString, unsigned int)));
}

//------------------------------------------------------------------------------
//
void QEPvProperties::resizeEvent (QResizeEvent *)
{
   QRect g;
   QLabel* enumLabel;
   int pw;
   int ew;   // enumerations width
   int epr;  // enumerations per row.
   int gap;
   int lh;   // label height
   int lw;   // label width
   int j;

   // Find scaled gap and enumeration width values.
   //
   gap = QEScaling::scale (4);
   ew  = QEScaling::scale (172);

   pw = this->enumerationFrame->width ();
   epr = MAX (1, (pw / ew));    // calc enumerations per row.
   lw = ((pw - gap)/ epr) - gap;
   lh = this->enumerationLabelList.value (0)->geometry().height();

   for (j = 0; j < this->enumerationLabelList.count (); j++) {
      enumLabel = this->enumerationLabelList.value (j);
      enumLabel->setGeometry (gap + (j%epr)*(lw + gap), gap + (j/epr)*(lh + gap), lw, lh);
   }
}

//------------------------------------------------------------------------------
// NB. Need to do a deep clear to avoid memory loss.
// qcaobject::QCaObjects aren't owned by parent widget
//
void QEPvProperties::clearFieldChannels ()
{
   QEString* qca;
   QTableWidgetItem *item;
   QString gap ("           ");  // empirically found to be quivilent width of " DESC "
   int j;

   while (!this->fieldChannels.isEmpty ()) {
      qca = this->fieldChannels.takeFirst ();
      delete qca;
   }
   this->fieldsAreSorted = false;
   this->variableIndexTableRowMap.clear ();

   for (j = 0; j < this->table->rowCount (); j++) {
      item = table->verticalHeaderItem (j);
      if (item) {
         item->setText (gap);
      }

      item = this->table->item (j, 0);
      if (item) {
         item->setText ("");
      }
   }
}

//------------------------------------------------------------------------------
// Compare two table rows
//
bool QEPvProperties::itemLessThan (const int a, const int b, QObject* context) const
{
   bool result = (a < b);

   if (context == &this->sortContext) {
      // Sort by field name
      //
      QTableWidgetItem* ai = this->table->item (a, FIELD_COL);
      QTableWidgetItem* bi = this->table->item (b, FIELD_COL);

      if (ai && bi) {
         result = ai->text() < bi->text();
      }

   } else if (context == &this->resetContext) {
      // Sort by variable index.
      //
      unsigned int avi = this->variableIndexTableRowMap.valueI (a);
      unsigned int bvi = this->variableIndexTableRowMap.valueI (b);
      result = avi < bvi;
   }

   return result;
}

//------------------------------------------------------------------------------
// Swap two table rows
//
void QEPvProperties::swapItems (const int a, const int b, QObject*)
{
   QTableWidgetItem* ai;
   QTableWidgetItem* bi;

   ai = this->table->takeItem (a, FIELD_COL);
   bi = this->table->takeItem (b, FIELD_COL);
   this->table->setItem       (a, FIELD_COL, bi);
   this->table->setItem       (b, FIELD_COL, ai);

   ai = this->table->takeItem (a, VALUE_COL);
   bi = this->table->takeItem (b, VALUE_COL);
   this->table->setItem       (a, VALUE_COL, bi);
   this->table->setItem       (b, VALUE_COL, ai);

   // Now update the variableIndex to table row mapping.
   //
   unsigned int avi = this->variableIndexTableRowMap.valueI (a);
   unsigned int bvi = this->variableIndexTableRowMap.valueI (b);

   this->variableIndexTableRowMap.removeF (avi);
   this->variableIndexTableRowMap.removeF (bvi);

   this->variableIndexTableRowMap.insertF (avi, b);
   this->variableIndexTableRowMap.insertF (bvi, a);
}

//------------------------------------------------------------------------------
//
void QEPvProperties::useNewVariableNameProperty (QString variableNameIn,
                                                 QString variableNameSubstitutionsIn,
                                                 unsigned int variableIndex)
{
   this->setVariableNameAndSubstitutions (variableNameIn, variableNameSubstitutionsIn, variableIndex);
}

//------------------------------------------------------------------------------
//
qcaobject::QCaObject* QEPvProperties::createQcaItem (unsigned int variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return NULL;
   }

   QString pvName;
   qcaobject::QCaObject* qca;

   pvName = this->getSubstitutedVariableName (variableIndex);

   // We don't need any formatting - that's looked after by the embedded QELabel,
   // but we are afrter a bit of meta data.
   //
   qca = new qcaobject::QCaObject (pvName, this, variableIndex,
                                   qcaobject::QCaObject::SIG_VARIANT);

   // Apply currently defined array index/elements request values.
   //
   this->setSingleVariableQCaProperties (qca);

   return qca;
}

//------------------------------------------------------------------------------
//
void QEPvProperties::establishConnection (unsigned int variableIndex)
{
   QString substitutedPVName;

   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   substitutedPVName = this->getSubstitutedVariableName (0).trimmed ();
   this->recordBaseName = QERecordFieldName::recordName (substitutedPVName);

   // Set up field name label.
   //
   this->label2->setText (QERecordFieldName::fieldName (substitutedPVName));

   // Clear associated data fields.
   //
   this->hostName->setText ("");
   this->timeStamp->setText ("");
   this->fieldType->setText ("");
   this->indexInfo->setText ("");
   this->valueLabel->setText ("");

   // Clear any previous cached info.
   //
   this->previousRecordBaseName = "";
   this->previousRecordType = "";

   // Clear any exiting field connections.
   //
   this->clearFieldChannels ();

   // Remove this name from mid-list if it exists and (re) insert at top of list.
   //
   this->insertIntoDropDownList (substitutedPVName);

   // Ensure CombBox consistent .
   //
   emit setCurrentBoxIndex (0);

   // Set up connections to XXXX.RTYP and XXXX.RTYP$.
   // We do this to firstly establish the record type name (e.g. ai, calcout),
   // but also to determine if the PV server (IOC) supports character array mode
   // for string PVs. This is usefull for long strings (> 40 characters).
   //
   // The order we do this here is important as this is refected in the order
   // that we will receive the connection notications.
   //
   this->setUpRecordTypeChannels (this->alternateRecordType, ReadAsCharArray);
   this->setUpRecordTypeChannels (this->standardRecordType,  StandardRead);

   // Set up XXXX.PROC client
   //
   this->setUpRecordProcChannel (this->recordProcField);

   // Lastly do the regular connection.
   //
   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject* qca = this->createConnection (variableIndex);

   // If a QCaObject object is now available to supply data update signals, connect it to the appropriate slots.
   //
   if (qca) {
      QObject::connect (qca,  SIGNAL (connectionChanged  (QCaConnectionInfo&, const unsigned int&)),
                        this, SLOT   (setValueConnection (QCaConnectionInfo&, const unsigned int&)));

      QObject::connect (qca,  SIGNAL (dataChanged   (const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                        this, SLOT   (setValueValue (const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));
   }
}

//------------------------------------------------------------------------------
//
void QEPvProperties::setUpRecordProcChannel (QEInteger* &qca)
{
   QString pvName;
   QString recordProcFieldName;

   pvName = this->getSubstitutedVariableName (0).trimmed ();
   recordProcFieldName = QERecordFieldName::fieldPvName (pvName, "PROC");

   // Delete any existing qca object if needs be.
   //
   if (qca) {
      delete qca;
      qca = NULL;
   }

   qca = new QEInteger (recordProcFieldName, this, &this->integerFormatting, 0);
   qca->subscribe ();
}

//------------------------------------------------------------------------------
//
void QEPvProperties::setUpRecordTypeChannels (QEString* &qca, const PVReadModes readMode)
{
   QString pvName;
   QString recordTypeName;

   pvName = this->getSubstitutedVariableName (0).trimmed ();
   this->recordBaseName = QERecordFieldName::recordName (pvName);

   recordTypeName = QERecordFieldName::rtypePvName (pvName);
   if (readMode == ReadAsCharArray) {
      recordTypeName.append ("$");
   }

   // Delete any existing qca object if needs be.
   //
   if (qca) {
      delete qca;
      qca = NULL;
   }

   qca = new QEString (recordTypeName, this, &this->rtypStringFormatting, (unsigned int) readMode);

   if (readMode == ReadAsCharArray)  {
      // Record type names are never longer than standard string.
      //
      qca->setRequestedElementCount (40);
   } else {
      qca->setRequestedElementCount (1);
   }

   QObject::connect (qca,  SIGNAL (connectionChanged       (QCaConnectionInfo&, const unsigned int& )),
                     this, SLOT   (setRecordTypeConnection (QCaConnectionInfo&, const unsigned int& )));

   QObject::connect (qca,  SIGNAL (stringChanged      (const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& )),
                     this, SLOT   (setRecordTypeValue (const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& )));

   qca->subscribe ();
}

//------------------------------------------------------------------------------
//
void QEPvProperties::setRecordTypeConnection (QCaConnectionInfo& connectionInfo,
                                              const unsigned int &variableIndex)
{
   const PVReadModes readMode = (PVReadModes) variableIndex;
   const bool isConnected = connectionInfo.isChannelConnected ();

   if ((readMode == ReadAsCharArray) && isConnected) {
      // XXX.RTYP$ connected - pre empty standard connection.
      //
      delete this->standardRecordType;
      this->standardRecordType = NULL;
   }

   // Update tool tip, but leave the basic widget enabled.
   //
   this->updateToolTipConnection (isConnected, variableIndex);
}

//------------------------------------------------------------------------------
// Called when notified of the (new) record type value.
//
void QEPvProperties::setRecordTypeValue (const QString& rtypeValue,
                                         QCaAlarmInfo&,
                                         QCaDateTime&,
                                         const unsigned int& variableIndex)
{
   const PVReadModes readMode = (PVReadModes) variableIndex;

   if ((this->recordBaseName == this->previousRecordBaseName) &&
       (rtypeValue == this->previousRecordType)) {
      return;   // Nothing to do - there is no significant change.
   }

   // Update for next time.
   //
   this->previousRecordBaseName = this->recordBaseName;
   this->previousRecordType = rtypeValue;

   // Look for the record spec for the given record type if it exists.
   //
   const QERecordSpec* pRecordSpec = recordSpecList.find (rtypeValue);

   // If we didn't find the specific record type, use the default record spec.
   //
   if (!pRecordSpec) {
       pRecordSpec = pDefaultRecordSpec;
   }

   // If we didn't find the specific record type or the default record spec
   // not defined then quit.
   //
   if(!pRecordSpec) {
      // Output some error???
      return;
   }

   // It is possible that a record may change RTYP (e.g. calc to calcout while IOC is off line)
   // Cannot rely soley on the clear called in createQcaItem / establish connection.
   //
   this->clearFieldChannels ();

   const int numberOfFields = pRecordSpec->size ();

   this->table->setRowCount (numberOfFields);
   for (int j = 0; j < numberOfFields; j++) {

      const QString readField = pRecordSpec->getFieldName (j);

      // Field names ending with $ inducate that the ling string mode is applicable.
      //
      const bool mayUseCharArray = readField.endsWith ('$');

      QString displayField;
      QString pvField;

      if (mayUseCharArray) {
         displayField = readField;
         displayField.chop (1);       // remove last character
      } else {
         displayField = readField;    // use as is.
      }

      const bool fieldUsingCharArray = (readMode == ReadAsCharArray) && mayUseCharArray;

      if (fieldUsingCharArray) {
         pvField = displayField;
         pvField.append ('$');        // append CA array mode qualifier.
      } else {
         pvField = displayField;
      }

      // Ensure vertical header exists and set it.
      //
      QTableWidgetItem* item = this->table->item (j, FIELD_COL);
      if (!item) {
         // We need to allocate item and insert into the table.
         item = new QTableWidgetItem ();
         this->table->setItem (j, FIELD_COL, item);
      }
      item->setText  (" " + displayField + " ");

      // Ensure table entry item exists.
      //
      item = this->table->item (j, VALUE_COL);
      if (!item) {
         // We need to allocate item and insert into the table.
         item = new QTableWidgetItem ();
         this->table->setItem (j, VALUE_COL, item);
      }

      // Form the required PV name.
      //
      const QString pvName = this->recordBaseName + "." + pvField;

      const unsigned int vi = (unsigned int) j;
      QEString* qca = new QEString (pvName, this, &this->fieldStringFormatting, vi);

      if (fieldUsingCharArray) {
         qca->setRequestedElementCount (MAX_FIELD_DATA_SIZE);
      } else {
         qca->setRequestedElementCount (1);
      }

      QObject::connect (qca, SIGNAL (connectionChanged  (QCaConnectionInfo&, const unsigned int& )),
                        this,  SLOT (setFieldConnection (QCaConnectionInfo&, const unsigned int& )));

      QObject::connect (qca, SIGNAL (stringChanged (const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& )),
                        this,  SLOT (setFieldValue (const QString&, QCaAlarmInfo&, QCaDateTime&, const unsigned int& )));

      qca->subscribe ();

      this->fieldChannels.append (qca);
      this->variableIndexTableRowMap.insertF (vi, j);
   }

   // The alarmInfo not really applicable to the RTYP field.
   // We pick up on the VAL field instead.
}

//------------------------------------------------------------------------------
//
void QEPvProperties::setValueConnection (QCaConnectionInfo& connectionInfo, const unsigned int&)
{
   qcaobject::QCaObject *qca;

   const bool isConnected = connectionInfo.isChannelConnected ();

   // These are not QELabels - so have to do this do manually.
   //
   this->valueLabel->setEnabled  (isConnected);
   this->hostName->setEnabled  (isConnected);
   this->timeStamp->setEnabled (isConnected);
   this->fieldType->setEnabled (isConnected);
   this->indexInfo->setEnabled (isConnected);

   if (connectionInfo.isChannelConnected ()) {
      // We "know" that the only/main channel is the 1st (slot 0) channel.
      //
      qca = this->getQcaItem (0);
      if (qca) {
         this->hostName->setText (qca->getHostName());
         this->isFirstUpdate = true;
      }
   }
}

//------------------------------------------------------------------------------
//
void QEPvProperties::setValueValue (const QVariant& value,
                                    QCaAlarmInfo& alarmInfo,
                                    QCaDateTime& dateTime,
                                    const unsigned int&)
{
   qcaobject::QCaObject *qca;
   QStringList enumerations;
   QLabel *enumLabel;
   QLabel *enumLast;
   int n;
   int j;
   QRect g;
   int h;

   this->timeStamp->setText (dateTime.text () + "  " + QEUtilities::getTimeZoneTLA (dateTime));

   // We "know" that the only/main channel is the 1st (slot 0) channel.
   //
   qca = this->getQcaItem (0);
   if (this->isFirstUpdate && qca) {
      // First update - fill in some of the meta data.
      //
      this->fieldType->setText (qca->getFieldType ());

      // Assume we are looking at 1st/only element for now.
      //
      QString s;
      s.sprintf ("%d / %ld", 1,  qca->getElementCount ());
      this->indexInfo->setText (s);

      // Whilst the value QELabel basically looks after itself, it benefits from
      // a helping hand. If the PV is of type DBF_CHAR and the field name
      // ends with $ then interpret as a long string.
      //
      // TODO - move this logic into QEStringFormatting.
      //
      bool isDbfChar = (qca->getFieldType () == "DBF_CHAR");

      QString pvName = qca->getRecordName ();
      QString field = QERecordFieldName::fieldName (pvName);
      bool requestedCharArray = field.endsWith ('$');

      bool longString = isDbfChar && requestedCharArray;
      if (longString) {
         this->valueStringFormatting.setArrayAction (QEStringFormatting::ASCII);
      } else {
         this->valueStringFormatting.setArrayAction (QEStringFormatting::INDEX);
      }

      // Set up variable details used by some formatting options
      //
      this->valueStringFormatting.setDbEgu (qca->getEgu ());
      this->valueStringFormatting.setDbEnumerations (qca->getEnumerations ());
      this->valueStringFormatting.setDbPrecision (qca->getPrecision ());

      // Ensure we do any required resizing.
      //
      this->resizeEvent (NULL);

      // Set up any enumeration values
      //
      enumerations = qca->getEnumerations ();
      n = enumerations.count();

      enumLast = NULL;
      for (j = 0; j < this->enumerationLabelList.count (); j++) {
         enumLabel = this->enumerationLabelList.value (j);
         if (j < n) {
            // Value is specified.
            enumLabel->setText (enumerations.value (j));
            enumLast = enumLabel;
            enumLabel->setVisible (true);
         } else {
            enumLabel->clear ();
            enumLabel->setVisible (false);
         }
      }

      if (enumLast) {
         g = enumLast->geometry ();
         h = g.top() + g.height() + 4;
      } else {
         h = 0;
      }
      this->enumerationFrame->setFixedHeight (h);

      // Set and expand to new max height.
      //
      this->enumerationResize->setAllowedMaximum (ENUMERATIONS_MIN_HEIGHT + h);
      this->enumerationResize->setFixedHeight (ENUMERATIONS_MIN_HEIGHT + h);

      this->isFirstUpdate = false;
   }

   // Update internal label.
   //
   this->valueLabel->setText (this->valueStringFormatting.formatString (value, 0));
   this->valueLabel->setStyleSheet (alarmInfo.style ());

   // Invoke common alarm handling processing.
   //
   this->processAlarmInfo (alarmInfo);
}

//------------------------------------------------------------------------------
//
void QEPvProperties::setFieldConnection (QCaConnectionInfo& connectionInfo,
                                         const unsigned int &variableIndex)
{
   const int row = this->variableIndexTableRowMap.valueF (variableIndex);
   const int numberOfRows = this->table->rowCount ();
   const bool isConnected = connectionInfo.isChannelConnected ();

   if ((row >= 0) && (row < numberOfRows)) {
      QTableWidgetItem* item = this->table->item (row, VALUE_COL);

      if (isConnected) {
         // connected
         item->setForeground (QColor (0, 0, 0));
         item->setText ("");
      } else {
         // disconnected - leave old text - readable but grayed out.
         item->setForeground (QColor (160, 160, 160));
      }
   } else {
      DEBUG << "variableIndex/row =" << variableIndex << row
            << ", out of range - must be <" << numberOfRows;
   }
}

//------------------------------------------------------------------------------
//
void QEPvProperties::setFieldValue (const QString& value,
                                    QCaAlarmInfo&,
                                    QCaDateTime&,
                                    const unsigned int& variableIndex)
{
   const int row = this->variableIndexTableRowMap.valueF (variableIndex);
   const int numberOfRows = this->table->rowCount ();

   if ((row >= 0) && (row < numberOfRows)) {
      QTableWidgetItem* item = this->table->item (row, VALUE_COL);

      if (value.length () < MAX_FIELD_DATA_SIZE) {
         item->setText  (" " + value);
      } else {
         // The string has maxed-out the read length, add ...
         item->setText  (" " + value + "...");
      }
   } else {
      DEBUG << "variableIndex/row =" << variableIndex << row
            << ", out of range - must be <" << numberOfRows;
   }
}

//------------------------------------------------------------------------------
// Unlike most widgets, the frame is not disabled if/when PVs disconnect
// Normally, standardProperties::setApplicationEnabled() is called
// For this widget our own version which just calls the widget's setEnabled is called.
//
void QEPvProperties::setApplicationEnabled (const bool& state)
{
    QWidget::setEnabled (state);
}

//==============================================================================
// ComboBox
//
void QEPvProperties::boxCurrentIndexChanged (int index)
{
   QString newPvName;
   QString oldPvName;

   if (index >= 0) {
      newPvName = this->box->itemText (index);
      oldPvName = getSubstitutedVariableName (0);

      // belts 'n' braces.
      //
      if (newPvName != oldPvName) {
         // Clear style.
         //
         this->valueLabel->setStyleSheet (lightGreyStyle);
         this->setVariableName (newPvName, 0);
         this->establishConnection (0);
      }
   }
}

//------------------------------------------------------------------------------
//
void QEPvProperties::insertIntoDropDownList (const QString& pvName)
{
   // Remove the PV name from mid-list if it exists and (re) insert at top of list.
   //
   for (int slot = this->box->count() - 1; slot >= 0; slot--) {
      if (this->box->itemText (slot).trimmed () == pvName) {
         this->box->removeItem (slot);
      }
   }

   // Make sure at least 2 free slots - one for this PV and one
   // for the user to type.
   //
   while (this->box->count() >= box->maxCount () - 2) {
      this->box->removeItem (box->count () - 1);
   }

   this->box->insertItem (0, pvName, QVariant ());
}

//------------------------------------------------------------------------------
//
QMenu* QEPvProperties::buildContextMenu ()
{
   const int number = this->fieldChannels.count ();

   QMenu* menu = ParentWidgetClass::buildContextMenu ();  // build parent context menu

   menu->addSeparator ();
   QAction* action;
   action = new QAction ("Sort By Field Name", menu);
   action->setCheckable (false);
   action->setEnabled ((number > 0) && !this->fieldsAreSorted);
   action->setData (QEPvProperties::PVPROP_SORT_FIELD_NAMES);
   menu->addAction (action);

   action = new QAction ("Reset Field Order", menu);
   action->setCheckable (false);
   action->setEnabled ((number > 0) && this->fieldsAreSorted);
   action->setData (QEPvProperties::PVPROP_RESET_FIELD_NAMES);
   menu->addAction (action);

   action = new QAction ("Process Record", menu);
   action->setCheckable (false);
   action->setEnabled (true);
   action->setData (QEPvProperties::PVPROP_PROCESS_RECORD);

   contextMenu::insertAfter (menu, action, contextMenu::CM_GENERAL_PV_EDIT);
   //menu->addAction (action);  // or insert??

   return menu;
}

//------------------------------------------------------------------------------
//
void QEPvProperties::contextMenuTriggered (int selectedItemNum)
{
   const int lastRow = this->fieldChannels.count () - 1;

   switch (selectedItemNum) {
      case QEPvProperties::PVPROP_SORT_FIELD_NAMES:
         this->sort (0, lastRow, &this->sortContext);
         this->fieldsAreSorted = true;
         break;

      case QEPvProperties::PVPROP_RESET_FIELD_NAMES:
         this->sort (0, lastRow, &this->resetContext);
         this->fieldsAreSorted = false;
         break;

      case QEPvProperties::PVPROP_PROCESS_RECORD:
         if (this->recordProcField && this->recordProcField->getChannelIsConnected ()) {
            this->recordProcField->writeInteger (1);
         }
         break;

      default:
         // process parent context menu
         //
         ParentWidgetClass::contextMenuTriggered (selectedItemNum);
         break;
   }
}

//------------------------------------------------------------------------------
//
void QEPvProperties::tableHeaderClicked (int index)
{
   if (index == FIELD_COL) {
      const int lastRow = this->fieldChannels.count () - 1;
      if (this->fieldsAreSorted) {
         this->sort (0, lastRow, &this->resetContext);
         this->fieldsAreSorted = false;
      } else {
         this->sort (0, lastRow, &this->sortContext);
         this->fieldsAreSorted = true;
      }
   }
}

//------------------------------------------------------------------------------
//
void QEPvProperties::customTableContextMenuRequested (const QPoint& posIn)
{
   QTableWidgetItem* item = NULL;
   QString trimmed;
   int row;
   int vi;
   qcaobject::QCaObject* qca = NULL;
   QString newPV = "";

   this->contextMenuPvName.clear ();

   // Find the associated item
   //
   item = this->table->itemAt (posIn);
   if (!item) {
      return;  // sainity check, just in case
   }

   switch (item->column ()) {
      case FIELD_COL:
         row = item->row ();
         // Find associated variable index...
         vi = this->variableIndexTableRowMap.valueI (row, -1);
         qca = this->fieldChannels.value (vi, NULL);
         if (qca) {
            newPV = qca->getRecordName ();
         }
         break;

      case VALUE_COL:
         trimmed = item->text ().trimmed ();
         // newPV set to empty string if input is not a valid PV name
         //
         QERecordFieldName::extractPvName (trimmed, newPV);
         break;

      default:
         DEBUG << "unexpected column number:" << item->column () << trimmed;
         return;
   }

   if (!newPV.isEmpty ()) {
      QPoint pos = posIn;
      pos.setY (pos.y () + DEFAULT_SECTION_SIZE);  // A feature of QTableWiget (because header visible maybe?).
      QPoint golbalPos = this->table->mapToGlobal (pos);
      QMenu* menu = this->buildContextMenu ();

      // Any trailing '$' is really for local usage only.
      //
      if (newPV.endsWith ("$")) {
         newPV.chop (1);
      }
      this->contextMenuPvName = newPV;
      menu->exec (golbalPos);
   }
}


//==============================================================================
// Save / restore
//
void QEPvProperties::saveConfiguration (PersistanceManager* pm)
{
   const QString formName = this->getPersistantName ();

   PMElement formElement = pm->addNamedConfiguration (formName);

   // DEBUG << "\n" << formName << "\n";

   // Note: we save the subsituted name (as opposed to template name and any macros).
   //
   formElement.addValue ("Name", this->getSubstitutedVariableName (0));

   // Add combo box drop down data.
   //
   PMElement dropDownElement = formElement.addElement ("DropDownList");
   for (int slot = 0 ; slot < this->box->count(); slot++) {
      PMElement pvElement = dropDownElement.addElement ("PV");
      pvElement.addAttribute ("id", slot);
      pvElement.addValue ("Name", this->box->itemText (slot).trimmed ());
   }
}

//------------------------------------------------------------------------------
//
void QEPvProperties::restoreConfiguration (PersistanceManager* pm, restorePhases restorePhase)
{
   if (restorePhase != FRAMEWORK) return;

   const QString formName = this->getPersistantName ();

   PMElement formElement = pm->getNamedConfiguration (formName);
   if (formElement.isNull ()) return;   // sainity check

   bool status;
   QString pvName;

   status = formElement.getValue ("Name", pvName);
   if (status) {
      this->setPvName (pvName);
   }

   // Restore each Drop Down PV.
   //
   PMElement dropDownElement = formElement.getElement ("DropDownList");
   this->box->clear();

   for (int slot = 0; slot < this->box->maxCount(); slot++) {
      PMElement pvElement = dropDownElement.getElement ("PV", "id", slot);

      if (pvElement.isNull ()) continue;

      // Attempt to extract a PV name
      //
      status = pvElement.getValue ("Name", pvName);
      if (status) {
         this->box->addItem (pvName, QVariant ());
      }
   }
}

//==============================================================================
//
void QEPvProperties::setPvName (const QString& pvNameIn)
{
   this->setVariableName (pvNameIn, 0);
   this->establishConnection (0);
}

//==============================================================================
// Copy / Paste
//
QString QEPvProperties::copyVariable ()
{
   QString result;

   if (!this->contextMenuPvName.isEmpty ()) {
      result = this->contextMenuPvName;
      this->contextMenuPvName.clear ();
   } else {
      result = this->getSubstitutedVariableName (0);
   }

   return result;
}

//------------------------------------------------------------------------------
//
QVariant QEPvProperties::copyData ()
{
   QTableWidgetItem* f;
   QTableWidgetItem* v;
   QString fieldList;
   QString field;
   QString value;
   QString line;

   // Create csv format.
   //
   fieldList.clear ();
   for (int i = 0; i < table->rowCount(); i++) {
      f = this->table->item (i, FIELD_COL);
      v = this->table->item (i, VALUE_COL);

      // Ensure both items have been allocated and assigned.
      //
      if (f && v) {

         field = f->text ().trimmed ();
         value = v->text ().trimmed ();

         // Right pad field to width of 6, suits most records.
         //
         while (field.length () < 6) field.append (" ");

         line = QString ("%1 , %2\n").arg (field).arg (value);
         fieldList.append (line);
      }
   }
   return QVariant (fieldList);
}

//------------------------------------------------------------------------------
//
void QEPvProperties::enableEditPvChanged ()
{
   const userLevelTypes::userLevels level = this->minimumEditPvUserLevel ();
   this->setEditPvUserLevel (level);
}

//------------------------------------------------------------------------------
//
int QEPvProperties::addPvName (const QString& pvName)
{
   this->insertIntoDropDownList (pvName);
   this->setPvName (pvName);
   return 0;
}

//------------------------------------------------------------------------------
//
void QEPvProperties::clearAllPvNames ()
{
   this->setPvName ("");
}

// end
