/*  QENTTable.cpp
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

#include "QENTTable.h"

#include <QAction>
#include <QColor>
#include <QDebug>
#include <QTimer>

#include <QECommon.h>
#include <QEFloating.h>
#include <QHeaderView>
#include <QENTTableData.h>

#define DEBUG qDebug () << "QENTTable" << __LINE__ << __FUNCTION__ << "  "

#define PV_VARIABLE_INDEX       0

#define DEFAULT_CELL_HEIGHT     22
#define NULL_SELECTION          (-1)

//=============================================================================
// Constructor with no initialisation
//=============================================================================
//
QENTTable::QENTTable (QWidget* parent) :
   QEAbstractWidget (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)
{
   this->commonConstruct ();
}

//---------------------------------------------------------------------------------
//
QENTTable::QENTTable (const QString& variableNameIn, QWidget* parent) :
   QEAbstractWidget (parent),
   QESingleVariableMethods (this, PV_VARIABLE_INDEX)
{
   this->commonConstruct ();
   this->setVariableName (variableNameIn, PV_VARIABLE_INDEX);
   this->activate ();
}

//---------------------------------------------------------------------------------
//
void QENTTable::commonConstruct ()
{
   // Create table data object.
   //
   this->tableData = new QENTTableData ();
   this->tableData->clear();

   // Create internal widget. We always have at least one row and one col.
   //
   this->table = new QTableWidget (1, 1, this);

   // Copy actual widget size policy to the containing widget, then ensure
   // internal widget will expand to fill container widget.
   //
   this->setSizePolicy (this->table->sizePolicy ());
   this->table->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);

   this->layout = new QHBoxLayout (this);
   this->layout->setContentsMargins (0, 0, 0, 0);    // extact fit.
   this->layout->addWidget (this->table);

   // Set default property values
   //
   this->displayMaximum = 0x1000;
   this->selection = NULL_SELECTION;
   this->selectionChangeInhibited = false;

   this->columnWidthMinimum = 80;
   this->orientation = Qt::Vertical;

   // This widget uses a single data source
   //
   this->setNumVariables (1);
   this->setMinimumSize (120, 50);
   this->setVariableAsToolTip (true);
   this->setAllowDrop (true);

   this->table->setSelectionBehavior (QAbstractItemView::SelectRows);
   this->table->verticalHeader()->setDefaultSectionSize (DEFAULT_CELL_HEIGHT);

   this->rePopulateTimer = new QTimer (this);
   QObject::connect (this->rePopulateTimer, SIGNAL (timeout ()),
                     this, SLOT (timeout ()));

   this->rePopulateData = false;
   this->rePopulateTimer->start (100);   // 10Hz  ???

   // Use default standard context menu less drag related items.
   // Note we overide buildContextMenu to add QENTTable specific items.
   //
   ContextMenuOptionSets tableMenuSet = this->defaultMenuSet ();
   tableMenuSet.remove (CM_DRAG_VARIABLE);
   tableMenuSet.remove (CM_DRAG_DATA);
   this->setupContextMenu (tableMenuSet);

   // Set up a connection to recieve variable name property changes
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   this->connectNewVariableNameProperty
         (SLOT (setNewVariableName (QString, QString, unsigned int)));

   // Table related signals
   //
   QObject::connect (this->table, SIGNAL (cellClicked      (int, int)),
                     this,        SLOT   (gridCellClicked  (int, int)));

   QObject::connect (this->table, SIGNAL (cellEntered      (int, int)),
                     this,        SLOT   (gridCellEntered  (int, int)));

   this->table->setMouseTracking (true);   // need this for cell entered.

   // Some events must be applied to the internal widget
   //
   this->installEventFilter (this);
}

//---------------------------------------------------------------------------------
//
QENTTable::~QENTTable ()
{
   // tableData is not a QObject - must be explicitly deleted.
   if (this->tableData) {
      delete this->tableData;
   }
}

//---------------------------------------------------------------------------------
//
QSize QENTTable::sizeHint () const
{
   return QSize (222, 118);
}

//---------------------------------------------------------------------------------
//
bool QENTTable::eventFilter (QObject* watched, QEvent* event)
{
   const QEvent::Type type = event->type ();
   bool result = false;

   switch (type) {
      case QEvent::FontChange:
         if (watched == this) {
            // Font must be mapped to the internal table
            //
            if (this->table) {
               this->table->setFont (this->font ());
            }
         }
         break;

      default:
         result = false;
         break;
   }

   return result;
}

//---------------------------------------------------------------------------------
//
void QENTTable::resizeEvent (QResizeEvent*)
{
   this->resizeCoulumns ();
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. A QCaObject that streams integers is required.
//
qcaobject::QCaObject* QENTTable::createQcaItem (unsigned int variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variable index" << variableIndex;
      return NULL;
   }

   qcaobject::QCaObject* result = NULL;
   result = new qcaobject::QCaObject (this->getSubstitutedVariableName (variableIndex),
                                      this, PV_VARIABLE_INDEX);

   // using setSingleVariableQCaProperties not applicable here

   return result;
}

//------------------------------------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QENTTable::establishConnection (unsigned int variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variable index" << variableIndex;
   }

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject* qca = createConnection (variableIndex);

   if (!qca) return;  // Sanity check

   // If a QCaObject object is now available to supply data update signals,
   // connect it to the appropriate slots.
   //
   QObject::connect (qca, SIGNAL (connectionChanged (QCaConnectionInfo&, const unsigned int&)),
                     this, SLOT  (connectionChanged (QCaConnectionInfo&, const unsigned int&)));

   // Note: we connect to receive the 'raw' variant data.
   //
   QObject::connect (qca, SIGNAL (dataChanged      (const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)),
                     this, SLOT  (tableDataChanged (const QVariant&, QCaAlarmInfo&, QCaDateTime&, const unsigned int&)));
}

//------------------------------------------------------------------------------
//
void QENTTable::activated ()
{
   //
}

//------------------------------------------------------------------------------
//
QMenu* QENTTable::buildContextMenu ()
{
   QMenu* menu = QEAbstractWidget::buildContextMenu ();
   QAction* action;

   menu->addSeparator ();

   action = new QAction ("Vertical table", menu);
   action->setCheckable (true);
   action->setChecked (this->isVertical ());
   action->setData (CM_VERTICAL_TABLE);
   menu->addAction (action);

   action = new QAction ("Horizontal table", menu);
   action->setCheckable (true);
   action->setChecked (!this->isVertical ());
   action->setData (CM_HORIZONTAL_TABLE);
   menu->addAction (action);

   return menu;
}

//------------------------------------------------------------------------------
//
void QENTTable::contextMenuTriggered (int selectedItemNum)
{
   switch (selectedItemNum) {

      case CM_HORIZONTAL_TABLE:
         this->setOrientation (Qt::Horizontal);
         break;

      case CM_VERTICAL_TABLE:
         this->setOrientation (Qt::Vertical);
         break;

      default:
         // Call parent class function.
         //
         QEAbstractWidget::contextMenuTriggered (selectedItemNum);
         break;
   }
}

//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the s looks and change the tool tip
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QENTTable::connectionChanged (QCaConnectionInfo& connectionInfo,
                                   const unsigned int& variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variable index" << variableIndex;
   }

   // Note the connected state
   //
   this->isConnected = connectionInfo.isChannelConnected ();

   // Enable internal widget iff connected.
   // Container widget remains enabled, so menues etc. still work.
   //
   this->table->setEnabled (this->isConnected);

   // Display the connected state
   //
   this->updateToolTipConnection (this->isConnected, variableIndex);
   this->processConnectionInfo (this->isConnected, variableIndex);

   // Set cursor to indicate access mode.
   //
   this->setAccessCursorStyle ();

   // Signal channel connection change to any (Link) widgets.
   // using signal dbConnectionChanged.
   //
   this->emitDbConnectionChanged (PV_VARIABLE_INDEX);
}

//-----------------------------------------------------------------------------
//
void QENTTable::tableDataChanged (const QVariant& value,
                                  QCaAlarmInfo& alarmInfo,
                                  QCaDateTime&,
                                  const unsigned int& variableIndex)
{
   if (variableIndex != PV_VARIABLE_INDEX) {
      DEBUG << "unexpected variableIndex" << variableIndex;
      return;
   }

   qcaobject::QCaObject* qca = this->getQcaItem (variableIndex);
   if (!qca) return;  // sanity check

   const bool isMetaDataUpdate = qca->getIsMetaDataUpdate();

   if (!this->tableData->assignFromVariant (value)) {
      if (isMetaDataUpdate) {
         QString pvname = this->getSubstitutedVariableName (variableIndex);
         DEBUG << "PV" << pvname << "does not provides NTTable data";
      }
      return;
   }

   this->populateTable ();

   // Invoke common alarm handling processing.
   //
   this->processAlarmInfo (alarmInfo, variableIndex);

   // Signal a database value change to any Link widgets
   //
   emit this->dbValueChanged (value);
   emit this->dbValueChanged (*this->tableData);
}

//---------------------------------------------------------------------------------
//
bool QENTTable::isVertical () const
{
   return (this->orientation != Qt::Horizontal);
}

//---------------------------------------------------------------------------------
//
void QENTTable::resizeCoulumns ()
{
   int count;
   int otherStuff;
   int colWidth;

   count = this->table->columnCount ();
   count = MAX (1, count);

   // Allow for side headers and scroll bar.
   //
   otherStuff = this->table->verticalHeader()->width () + 20;
   colWidth = (this->table->width () - otherStuff) / count;
   colWidth = MAX (this->columnWidthMinimum, colWidth);

   for (int col = 0; col < count; col++) {
      if (this->table->columnWidth (col) != colWidth) {
         this->table->setColumnWidth (col, colWidth);
      }
   }
}

//------------------------------------------------------------------------------
//
void QENTTable::populateTable ()
{
   if (this->isVertical ()) {
      this->populateVerticalTable ();
   }  else {
      this->populateHorizontalTable ();
   }
   this->rePopulateData = false;
}

//------------------------------------------------------------------------------
//
void QENTTable::populateVerticalTable ()
{
   if (!this->table) return;      // sanity check
   if (!this->tableData) return;  // sanity check

   const int cols = this->tableData->getColCount ();
   const int rows = this->tableData->getRowCount ();
   const QStringList headers = this->tableData->getLabels();

   QTableWidgetItem* item;

   this->table->setColumnCount (MAX (cols, 1));
   this->table->setRowCount (MAX (rows, 1));

   for (int col = 0; col < cols; col++) {
      // Set the title - allocate title item if needs be.
      //
      item = this->table->horizontalHeaderItem (col);
      if (!item) {
         item = new QTableWidgetItem ();
         this->table->setHorizontalHeaderItem (col, item);
      }
      item->setText (headers.value (col, "-"));

      QVariantList dataSet = this->tableData->getColData (col);

      for (int row = 0; row < rows; row++) {

         item = this->table->verticalHeaderItem (row);
         if (!item) {
            item = new QTableWidgetItem ();
            this->table->setVerticalHeaderItem (row, item);
         }
         item->setText (QString::number(row + 1));

         item = this->table->item (row, col);
         if (!item) {
            // We need to allocate item and insert it into the table.
            //
            item = new QTableWidgetItem ();
            item->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
            item->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            this->table->setItem (row, col, item);
         }

         QVariant datum = dataSet.value (row, QVariant ("-"));
         QString image = datum.toString ();
         item->setText (image);
      }
   }
}

//------------------------------------------------------------------------------
// keep in step with populateVerticalTable
//
void QENTTable::populateHorizontalTable ()
{
   if (!this->table) return;      // sanity check
   if (!this->tableData) return;  // sanity check

   const int cols = this->tableData->getRowCount ();
   const int rows = this->tableData->getColCount ();
   const QStringList headers = this->tableData->getLabels();

   QTableWidgetItem* item;

   this->table->setColumnCount (MAX (cols, 1));
   this->table->setRowCount (MAX (rows, 1));


   for (int row = 0; row < rows; row++) {
      // Set the title - allocate title item if needs be.
      //
      item = this->table->verticalHeaderItem (row);
      if (!item) {
         item = new QTableWidgetItem ();
         this->table->setVerticalHeaderItem (row, item);
      }
      item->setText (headers.value (row, "-"));

      QVariantList dataSet = this->tableData->getColData (row);

      for (int col = 0; col < cols; col++) {

         item = this->table->horizontalHeaderItem (col);
         if (!item) {
            item = new QTableWidgetItem ();
            this->table->setHorizontalHeaderItem (col, item);
         }
         item->setText (QString::number(col + 1));

         QTableWidgetItem* item = this->table->item (row, col);

         if (!item) {
            // We need to allocate item and insert it into the table.
            //
            item = new QTableWidgetItem ();
            item->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
            item->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            this->table->setItem (row, col, item);
         }

         QVariant datum = dataSet.value (col, QVariant ("-"));
         QString image = datum.toString ();
         item->setText (image);
      }
   }
}

//------------------------------------------------------------------------------
//
void QENTTable::timeout ()
{
   this->resizeCoulumns ();
   if (this->rePopulateData) {
      this->populateTable ();
   }
}

//------------------------------------------------------------------------------
//
void QENTTable::setNewVariableName (QString variableName,
                                    QString substitutions,
                                    unsigned int variableIndex)
{
   // Note: essentially calls createQcaItem.
   //
   this->setVariableNameAndSubstitutions (variableName, substitutions, variableIndex);
}

//---------------------------------------------------------------------------------
// User has clicked on cell or used up/down/left/right key to select cell,
// or we have programtically selected a row/coll.
//
void QENTTable::gridCellClicked (int row, int column)
{
   this->selection = (this->isVertical () ? row : column);

   // This prevents infinite looping in the case of cyclic connections.
   //
   this->selectionChangeInhibited = true;
   emit this->selectionChanged (this->selection);
   this->selectionChangeInhibited = false;
}

//------------------------------------------------------------------------------
//
void QENTTable::gridCellEntered (int, int)
{
   // place holder
   // qDebug () << __FUNCTION__ << row << column;
}

//------------------------------------------------------------------------------
//
void QENTTable::setSelection (int selectionIn)
{
   // Guard against circular signal-slot connections.
   //
   if (this->selectionChangeInhibited) return;

   // A negative selection means no selection
   //
   if (selectionIn < 0) selectionIn = NULL_SELECTION;
   if (this->selection != selectionIn) {
      this->selection = selectionIn;

      if (selectionIn >= 0) {
         if (this->isVertical ()) {
            this->table->selectRow (selectionIn);
         } else {
            this->table->selectColumn (selectionIn);
         }
      } else {
         this->table->clearSelection ();
      }
   }
}

//------------------------------------------------------------------------------
//
QStringList QENTTable::getTitles () const
{
   return this->tableData->getLabels ();
}

//------------------------------------------------------------------------------
//
int QENTTable::getSelection () const {
   return this->selection;
}


//==============================================================================
// Properties
//------------------------------------------------------------------------------
//
void QENTTable::setDisplayMaximum (const int displayMaximumIn)
{
   int temp = LIMIT (displayMaximumIn, 1, 0x10000);

   if (this->displayMaximum != temp) {
      this->displayMaximum = temp;
      this->rePopulateData = true;
   }
}

//------------------------------------------------------------------------------
//
int QENTTable::getDisplayMaximum () const
{
    return this->displayMaximum;
}

//------------------------------------------------------------------------------
//
void QENTTable::setColumnWidthMinimum (const int minimumColumnWidthIn)
{
   int temp = LIMIT (minimumColumnWidthIn, 20, 320);

   if (this->columnWidthMinimum != temp) {
      this->columnWidthMinimum = temp;
      this->resizeCoulumns ();
   }
}

//------------------------------------------------------------------------------
//
int QENTTable::getColumnWidthMinimum () const
{
   return this->columnWidthMinimum;
}

//------------------------------------------------------------------------------
//
void QENTTable::setOrientation (const Qt::Orientation orientationIn)
{
   if (this->orientation != orientationIn) {
      this->orientation = orientationIn;
      if (this->isVertical ()) {
         this->table->setSelectionBehavior (QAbstractItemView::SelectRows);
      } else {
         this->table->setSelectionBehavior (QAbstractItemView::SelectColumns);
      }
      this->populateTable ();
   }
}

//------------------------------------------------------------------------------
//
Qt::Orientation QENTTable::getOrientation () const
{
   return this->orientation;
}

//==============================================================================
// Copy / Paste
//
QString QENTTable::copyVariable ()
{
   return this->getSubstitutedVariableName (PV_VARIABLE_INDEX);
}

//------------------------------------------------------------------------------
//
QVariant QENTTable::copyData ()
{
   return this->tableData->toVariant();
}

// end
