/*  QETable.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2014-2020 Australian Synchrotron.
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
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#include "QETable.h"

#include <QAction>
#include <QColor>
#include <QDebug>
#include <QTimer>

#include <QECommon.h>
#include <QEFloating.h>
#include <QHeaderView>


#define DEBUG qDebug () << "QETable" << __LINE__ << __FUNCTION__ << "  "

#define DEFAULT_CELL_HEIGHT     22
#define NULL_SELECTION          (-1)


//=============================================================================
// DataSets class - manages a single PV
//=============================================================================
//
QETable::DataSets::DataSets ()
{
   this->pvName = "";
   this->title = "";
   this->isConnected = false;
   this->owner = NULL;
   this->slot = -1;
}

//-----------------------------------------------------------------------------
//
QETable::DataSets::~DataSets ()
{
   // place holder
}

//-----------------------------------------------------------------------------
//
void QETable::DataSets::setContext (QETable* ownerIn, int slotIn)
{
   this->owner = ownerIn;
   this->slot = slotIn;
   this->index = -1;
}

//-----------------------------------------------------------------------------
//
void QETable::DataSets::clear ()
{
   this->isConnected = false;
   this->data.clear ();
   this->alarmInfo = QCaAlarmInfo (0, 0);
}

//-----------------------------------------------------------------------------
//
void QETable::DataSets::setPvName (const QString& pvNameIn)
{
   this->clear ();
   this->pvName = pvNameIn;
}

//-----------------------------------------------------------------------------
//
QString QETable::DataSets::getPvName () const
{
    return this->pvName;
}

//-----------------------------------------------------------------------------
//
bool QETable::DataSets::isInUse () const
{
   // Data set item in use if PV name defined (as opposed to PV actually exists).
   //
   return !this->pvName.isEmpty ();
}

//-----------------------------------------------------------------------------
//
void QETable::DataSets::rePopulateTable ()
{
   QTableWidget* table = this->owner->table;  // alias
   QString titleText;
   int currentSize;
   QTableWidgetItem* item;

   if (!table) return;                   // sainity check
   if (!this->isInUse ()) return;        // nothing to see here ... move along ...

   // Find own row/col index
   //
   this->index = 0;
   for (int j = 0; j < this->slot; j++) {
      if (this->owner->dataSet [j].isInUse ()) {
         this->index++;
      }
   }

   // Extract the title for this row/col.
   // If null just use the index, if <> use PV name.
   //
   titleText = this->title;
   if (titleText.isEmpty ()) {
      titleText.setNum (this->index + 1);
   } else if (titleText == "<>") {
      titleText = this->pvName;
   }

   // Ensure table large enough to accomodate all row/cols.
   //
   if (this->owner->isVertical ()) {
      currentSize = table->columnCount ();
      table->setColumnCount (MAX (currentSize, this->index + 1));

      // Set the title - allocate title item if needs be.
      //
      item = table->horizontalHeaderItem (this->index);
      if (!item) {
         item = new QTableWidgetItem ();
         table->setHorizontalHeaderItem (index, item);
      }
      item->setText (titleText);

      // Ensure other title is "1" - only need to do the first header item.
      //
      item = table->verticalHeaderItem (0);
      if (!item) {
         item = new QTableWidgetItem ();
         table->setVerticalHeaderItem (0, item);
      }
      item->setText ("1");

   } else {
      currentSize = table->rowCount ();
      table->setRowCount (MAX (currentSize, this->index + 1));

      // Set the title - allocate title item if needs be.
      //
      item = table->verticalHeaderItem (this->index);
      if (!item) {
         item = new QTableWidgetItem ();
         table->setVerticalHeaderItem (index, item);
      }
      item->setText (titleText);

      // Ensure other title is "1" - only need to do the first header item.
      //
      item = table->horizontalHeaderItem (0);
      if (!item) {
         item = new QTableWidgetItem ();
         table->setHorizontalHeaderItem (0, item);
      }
      item->setText ("1");
   }
}

//-----------------------------------------------------------------------------
//
void QETable::DataSets::rePopulateData ()
{
   QTableWidget* table = this->owner->table;  // alias
   QColor backgroundColour;
   QColor textColour;

   if (!table) return;                   // sainity check
   if (!this->isInUse ()) return;        // nothing to see here ... move along ...
   if (this->index < 0) return;

   // Use connected/alarm state to find background/foreground colours.
   //
   if (this->isConnected) {
      backgroundColour = QColor ("#e0e0e0");   // hypothrsize no alram colour.

      displayAlarmStateOptions daso = this->owner->getDisplayAlarmStateOption ();
      switch (daso) {
         case DISPLAY_ALARM_STATE_NEVER:
            backgroundColour = QColor ("#e0e0e0");
            break;

         case DISPLAY_ALARM_STATE_ALWAYS:
            backgroundColour = QColor (alarmInfo.getStyleColorName ());
            break;

         case DISPLAY_ALARM_STATE_WHEN_IN_ALARM :
            if (alarmInfo.isInAlarm ()) {
               backgroundColour = QColor (alarmInfo.getStyleColorName ());
            }
            break;
      }
      textColour = QColor ("black");

   } else {
      backgroundColour = QColor ("white");
      textColour = QColor ("grey");
   }

   // Get current table size - set up eslewhere.
   //
   int currentSize;
   if (this->owner->isVertical ()) {
      currentSize = table->rowCount ();
   } else {
      currentSize = table->columnCount ();
   }

   for (int j = 0; j <  currentSize; j++) {
      int row = this->owner->isVertical () ? j : this->index;
      int col = this->owner->isVertical () ? this->index : j;

      QTableWidgetItem* item = table->item (row, col);

      if (!item) {
         // We need to allocate item and insert it into the table.
         //
         item = new QTableWidgetItem ();
         item->setTextAlignment (Qt::AlignRight | Qt::AlignVCenter);
         item->setFlags (Qt::ItemIsSelectable | Qt::ItemIsEnabled);
         table->setItem (row, col, item);
      }

      QString image;
      if (j < this->data.count ()) {
         double value = this->data.value (j);
         image = QString ("%1 ").arg (value);    // no EGU or formatting (yet).
      } else {
         // Beyond end of data
         image = "";
         backgroundColour = QColor ("#c8c8c8");  // light gray
      }

      item->setText (image);
      item->setBackgroundColor (backgroundColour);
      item->setTextColor (textColour);
   }
}

//==============================================================================
// Slot range checking macro function.
// Set defaultValue to nil for void functions.
//
#define SLOT_CHECK(slot, defaultValue) {                           \
   if ((slot < 0) || (slot >= ARRAY_LENGTH (this->dataSet))) {     \
      DEBUG << "slot out of range: " << slot;                      \
      return defaultValue;                                         \
   }                                                               \
}


//=============================================================================
// Constructor with no initialisation
//=============================================================================
//
QETable::QETable (QWidget* parent) : QEAbstractDynamicWidget (parent)
{
   // Create internal widget. We always have at least one row and one col.
   //
   this->table = new QTableWidget (1, 1, this);

   // Copy actual widget size policy to the containing widget, then ensure
   // internal widget will expand to fill container widget.
   //
   this->setSizePolicy (this->table->sizePolicy ());
   this->table->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);

   this->layout = new QHBoxLayout (this);
   this->layout->setMargin (0);    // extact fit.
   this->layout->addWidget (this->table);

   // Initialise data set objects.
   // These are declared as array as opposed to being dynamically allocated,
   // so we need need post contruction configuration.
   //
   for (int slot = 0 ; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      this->dataSet [slot].setContext (this, slot);
   }

   // Set default property values
   //
   this->displayMaximum = 0x1000;
   this->selection = NULL_SELECTION;
   this->selectionChangeInhibited = false;
   this->pvNameSetChangeInhibited = false;
   this->titlesChangeInhibited = false;

   this->columnWidthMinimum = 80;
   this->orientation = Qt::Vertical;
   this->setNumVariables (ARRAY_LENGTH (this->dataSet));
   this->setMinimumSize (120, 50);
   this->setVariableAsToolTip (true);
   this->setAllowDrop (true);

   this->table->setSelectionBehavior (QAbstractItemView::SelectRows);
   this->table->verticalHeader()->setDefaultSectionSize (DEFAULT_CELL_HEIGHT);

   this->rePopulateTimer = new QTimer (this);
   QObject::connect (this->rePopulateTimer, SIGNAL (timeout ()),
                     this, SLOT (timeout ()));

   this->rePopulateAll = true;
   this->rePopulateTitles = false;
   this->rePopulateData = false;
   this->rePopulateTimer->start (100);   // 10Hz

   // Use default standard context menu less drag related items.
   // Note we overide buildContextMenu to add QETable specific items.
   //
   ContextMenuOptionSets tableMenuSet = this->defaultMenuSet ();
   tableMenuSet.remove (CM_DRAG_VARIABLE);
   tableMenuSet.remove (CM_DRAG_DATA);
   this->setupContextMenu (tableMenuSet);

   // Set up a connections to receive variable name property changes
   //
   // The variable name property manager class only delivers an updated
   // variable name after the user has stopped typing.
   //
   for (int slot = 0 ; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      QCaVariableNamePropertyManager* vpnm;

      vpnm = &this->dataSet [slot].variableNameManager;
      vpnm->setVariableIndex (slot);   // Use slot as variable index.

      QObject::connect (vpnm, SIGNAL (newVariableNameProperty (QString, QString, unsigned int)),
                        this, SLOT   (setNewVariableName      (QString, QString, unsigned int)));
   }

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
QETable::~QETable () { }

//---------------------------------------------------------------------------------
//
QSize QETable::sizeHint () const
{
   return QSize (222, 118);
}

//---------------------------------------------------------------------------------
//
bool QETable::eventFilter (QObject* watched, QEvent* event)
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
void QETable::resizeEvent (QResizeEvent*)
{
   this->resizeCoulumns ();
}

//------------------------------------------------------------------------------
// Implementation of QEWidget's virtual funtion to create the specific type of
// QCaObject required. A QCaObject that streams integers is required.
//
qcaobject::QCaObject* QETable::createQcaItem (unsigned int variableIndex)
{
   const int slot = this->slotOf (variableIndex);
   SLOT_CHECK (slot, NULL);

   qcaobject::QCaObject* result = NULL;
   QString pvName;

   pvName = this->getSubstitutedVariableName (variableIndex).trimmed ();
   result = new QEFloating (pvName, this, &this->floatingFormatting, variableIndex);

   return result;
}

//------------------------------------------------------------------------------
// Start updating.
// Implementation of VariableNameManager's virtual funtion to establish a
// connection to a PV as the variable name has changed.
// This function may also be used to initiate updates when loaded as a plugin.
//
void QETable::establishConnection (unsigned int variableIndex)
{
   const int slot = this->slotOf (variableIndex);
   SLOT_CHECK (slot, );

   // Create a connection.
   // If successfull, the QCaObject object that will supply data update signals will be returned
   // Note createConnection creates the connection and returns reference to existing QCaObject.
   //
   qcaobject::QCaObject* qca = createConnection (variableIndex);
   if (!qca) return;  // Sanity check

   QObject::connect (qca, SIGNAL (connectionChanged (QCaConnectionInfo &, const unsigned int &)),
                     this, SLOT  (connectionChanged (QCaConnectionInfo &, const unsigned int &)));

   QObject::connect (qca, SIGNAL (floatingArrayChanged (const QVector<double>&, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)),
                     this, SLOT  (dataArrayChanged     (const QVector<double>&, QCaAlarmInfo &, QCaDateTime &, const unsigned int &)));
}

//------------------------------------------------------------------------------
//
void QETable::activated ()
{
   // Until we know better, all PV are disconnected.
   //
   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      this->dataSet [slot].clear ();
   }
   this->rePopulateData = true;

   // This prevents infinite looping in the case of cyclic connections.
   //
   this->pvNameSetChangeInhibited = true;
   emit this->pvNameSetChanged (this->getPvNameSet ());
   this->pvNameSetChangeInhibited = false;

   this->titlesChangeInhibited = true;
   emit this->titlesChanged (this->getTitles ());
   this->titlesChangeInhibited = false;
}

//------------------------------------------------------------------------------
//
QMenu* QETable::buildContextMenu ()
{
   QMenu* menu = QEAbstractDynamicWidget::buildContextMenu ();
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
void QETable::contextMenuTriggered (int selectedItemNum)
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
         QEAbstractDynamicWidget::contextMenuTriggered (selectedItemNum);
         break;
   }
}

//------------------------------------------------------------------------------
// Act on a connection change.
// Change how the s looks and change the tool tip
// This is the slot used to recieve connection updates from a QCaObject based class.
//
void QETable::connectionChanged (QCaConnectionInfo& connectionInfo,
                                 const unsigned int& variableIndex)
{
   const int slot = this->slotOf (variableIndex);
   SLOT_CHECK (slot,);

   this->dataSet [slot].isConnected = connectionInfo.isChannelConnected ();
   this->rePopulateData = true;

   this->updateToolTipConnection (dataSet [slot].isConnected, variableIndex);
}

//-----------------------------------------------------------------------------
//
void QETable::dataArrayChanged (const QVector<double>& values,
                                QCaAlarmInfo& alarmInfo,
                                QCaDateTime&,
                                const unsigned int& variableIndex)
{
   const int slot = this->slotOf (variableIndex);
   SLOT_CHECK (slot,);

   this->dataSet [slot].data = QEFloatingArray (values);
   this->dataSet [slot].alarmInfo = alarmInfo;
   this->dataSet [slot].rePopulateData ();
   // this->rePopulateData = true;

   // this->rePopulationRequired = true;

   // Signal a database value change to any Link widgets
   //
   emit this->dbValueChanged (values);

   // Don't invoke common alarm handling processing, as we use a PV specifc alarm
   // dinication per col/row. Update the tool tip to reflect current alarm state.
   //
   this->updateToolTipAlarm (alarmInfo, variableIndex);
}

//---------------------------------------------------------------------------------
//
bool QETable::isVertical () const
{
   return (this->orientation != Qt::Horizontal);
}

//---------------------------------------------------------------------------------
//
void QETable::resizeCoulumns ()
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
// Calculate the required number of cols (or rows when horizontal).
//
int QETable::numberInUse () const
{
   int result = 0;
   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      if (this->dataSet [slot].isInUse()) {
         result++;
      }
   }
   result = MAX (1, result);  // need at least one, even if not in use.
   return result;
}

//------------------------------------------------------------------------------
// Calculate the required number of rows (or cols when horizontal).
//
int QETable::dataSize () const
{
   int result = 1; // need at least one, even if not in use.
   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      if (this->dataSet [slot].isInUse()) {
         result = MAX (result, this->dataSet [slot].data.count ());
      }
   }
   result = MIN (result, this->getDisplayMaximum());
   return result;
}

//------------------------------------------------------------------------------
//
void QETable::timeout ()
{
   if (this->rePopulateAll) {
      this->table->setRowCount (1);
      this->table->setColumnCount (1);
      this->rePopulateAll = false;
      this->rePopulateTitles = true;
   }

   if (this->rePopulateTitles) {

      for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
         this->dataSet [slot].rePopulateTable ();
      }

      // Ensure table just large enough to accomodate all row/cols.
      //
      int number = this->numberInUse ();
      if (this->isVertical ()) {
         table->setColumnCount (number);
      } else {
         table->setRowCount (number);
      }
      this->rePopulateTitles = false;
      this->rePopulateData = true;
   }

   if (this->rePopulateData) {

      // Ensure table just large enough to accomodate all row/cols.
      //
      int number = this->dataSize ();
      if (this->isVertical ()) {
         table->setRowCount (number);
      } else {
         table->setColumnCount (number);
      }

      for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
         this->dataSet [slot].rePopulateData ();
      }
      this->rePopulateData = false;
   }

   this->resizeCoulumns ();
}

//------------------------------------------------------------------------------
//
void QETable::setNewVariableName (QString variableName,
                                  QString substitutions,
                                  unsigned int variableIndex)
{
   int slot = this->slotOf (variableIndex);
   QString pvName;

   SLOT_CHECK (slot,);

   // Note: essentially calls createQcaItem.
   //
   this->setVariableNameAndSubstitutions (variableName, substitutions, variableIndex);

   pvName = this->getSubstitutedVariableName (variableIndex).trimmed ();
   this->dataSet [slot].setPvName (pvName);
   this->rePopulateTitles = true;

   // This prevents infinite looping in the case of cyclic connections.
   //
   this->pvNameSetChangeInhibited = true;
   emit this->pvNameSetChanged (this->getPvNameSet ());
   this->pvNameSetChangeInhibited = false;
}

//---------------------------------------------------------------------------------
// User has clicked on cell or used up/down/left/right key to select cell,
// or we have programtically selected a row/coll.
//
void QETable::gridCellClicked (int row, int column)
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
void QETable::gridCellEntered (int, int)
{
   // place holder
   // qDebug () << __FUNCTION__ << row << column;
}

//------------------------------------------------------------------------------
//
void QETable::setSelection (int selectionIn)
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
int QETable::getSelection () const {
   return this->selection;
}

//------------------------------------------------------------------------------
//
void QETable::setPvNameSet (const QStringList& pvNameSet)
{
   // Guard against circular signal-slot connections.
   //
   if (this->pvNameSetChangeInhibited) return;

   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      QString pvName = pvNameSet.value (slot, "");
      this->setNewVariableName (pvName, "", slot);
   }
}

//------------------------------------------------------------------------------
//
QStringList QETable::getPvNameSet () const
{
   QStringList result;

   result.clear ();
   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      QString pvName = this->getSubstitutedVariableName (slot);
      result.append (pvName);
   }

   return result;
}

//==============================================================================
// Properties
// Update variable name etc.
//
void QETable::setVariableName (const int slot, const QString& pvName)
{
   SLOT_CHECK (slot,);
   this->dataSet [slot].variableNameManager.setVariableNameProperty (pvName);

   // Ensure we always subscribe (activate) irrespective of the profile DontActivateYet state.
   //
   this->establishConnection (slot);
}

//------------------------------------------------------------------------------
//
QString QETable::getVariableName (const int slot) const
{
   SLOT_CHECK (slot, "");
   return this->dataSet [slot].variableNameManager.getVariableNameProperty ();
}

//------------------------------------------------------------------------------
//
void QETable::setSubstitutions (const QString& defaultSubstitutions)
{
   // Use same default subsitutions for all PVs used by this widget.
   //
   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      this->dataSet [slot].variableNameManager.setSubstitutionsProperty (defaultSubstitutions);
   }
}

//------------------------------------------------------------------------------
//
QString QETable::getSubstitutions () const
{
   // Any one of the PV name managers can provide the subsitutions.
   //
   return this->dataSet [0].variableNameManager.getSubstitutionsProperty ();
}

//------------------------------------------------------------------------------
//
void QETable::setTitles (const QStringList& titlesIn)
{
   // Guard against circular signal-slot connections.
   //
   if (this->titlesChangeInhibited) return;

   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      QString title = titlesIn.value (slot, "");
      this->dataSet [slot].title = title;
   }
   this->rePopulateTitles = true;

   this->titlesChangeInhibited = true;
   emit this->titlesChanged (this->getTitles ());
   this->titlesChangeInhibited = false;
}

//------------------------------------------------------------------------------
//
QStringList QETable::getTitles () const
{
   QStringList result;

   result.clear ();
   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      QString title = this->dataSet [slot].title;
      result.append (title);
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QETable::setTitle (const int slot, const QString& title)
{
   SLOT_CHECK (slot,);

   // Guard against circular signal-slot connections.
   //
   if (this->titlesChangeInhibited) return;

   this->dataSet [slot].title = title;
   this->rePopulateTitles = true;

   this->titlesChangeInhibited = true;
   emit this->titlesChanged (this->getTitles ());
   this->titlesChangeInhibited = false;
}

//------------------------------------------------------------------------------
//
void QETable::setTableEntry (const int slot, const QString& pvName, const QString& title)
{
   SLOT_CHECK (slot,);

   this->setVariableName (slot, pvName);
   this->setTitle (slot, title);
}

//------------------------------------------------------------------------------
//
void QETable::setDisplayMaximum (const int displayMaximumIn)
{
   int temp = LIMIT (displayMaximumIn, 1, 0x10000);

   if (this->displayMaximum != temp) {
      this->displayMaximum = temp;
      this->rePopulateData = true;
   }
}

//------------------------------------------------------------------------------
//
int QETable::getDisplayMaximum () const
{
    return this->displayMaximum;
}

//------------------------------------------------------------------------------
//
void QETable::setColumnWidthMinimum (const int minimumColumnWidthIn)
{
   int temp = LIMIT (minimumColumnWidthIn, 20, 320);

   if (this->columnWidthMinimum != temp) {
      this->columnWidthMinimum = temp;
      this->resizeCoulumns ();
   }
}

//------------------------------------------------------------------------------
//
int QETable::getColumnWidthMinimum () const
{
   return this->columnWidthMinimum;
}

//------------------------------------------------------------------------------
//
void QETable::setOrientation (const Qt::Orientation orientationIn)
{
   if (this->orientation != orientationIn) {
      this->orientation = orientationIn;
      if (this->isVertical ()) {
         this->table->setSelectionBehavior (QAbstractItemView::SelectRows);
      } else {
         this->table->setSelectionBehavior (QAbstractItemView::SelectColumns);
      }
      this->rePopulateAll = true;
   }
}

//------------------------------------------------------------------------------
//
Qt::Orientation QETable::getOrientation () const
{
   return this->orientation;
}

//==============================================================================
// Copy / Paste
//
QString QETable::copyVariable ()
{
   QString result;

   // Create a space separated list of PV names.
   //
   result = "";
   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      QString pvName = this->getSubstitutedVariableName (slot);
      if (!pvName.isEmpty()) {
         if (!result.isEmpty()) result.append (" ");
         result.append (pvName);
      }
   }
   return result;
}

//------------------------------------------------------------------------------
//
QVariant QETable::copyData ()
{
   const int fw = 12;   // field width
   DataSets* ds;
   QString result;

   int number;
   number = 1;
   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      ds = &this->dataSet [slot];
      if (ds->isInUse ()) {
         int size = ds->data.count ();
         number = MAX (number, size);
      }
   }
   number = MIN (number, this->displayMaximum);

   result = "\n";
   for (int j = 0; j < number; j++) {
      for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
         ds = &this->dataSet [slot];
         if (ds->isInUse ()) {
            if (j < ds->data.count ()) {
               result.append (QString ("\t%1").arg (ds->data [j], fw));
            } else {
               result.append (QString ("\t%1").arg ("nul", fw));
            }
         }
      }
      result.append (QString ("\n"));
   }

   return QVariant (result);
}


//------------------------------------------------------------------------------
//
int QETable::addPvName (const QString& pvName)
{
   int result = -1;

   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      if (this->dataSet [slot].isInUse() == false) {
         // Found an empty slot.
         //
         this->setVariableName (slot, pvName);
         result = slot;
         break;
      }
   }

   return result;
}

//------------------------------------------------------------------------------
//
void QETable::clearAllPvNames ()
{
   for (int slot = 0; slot < MAXIMUM_NUMBER_OF_VARIABLES; slot++) {
      this->setVariableName (slot, "");
   }
}

//------------------------------------------------------------------------------
//
void QETable::saveConfiguration (PersistanceManager* pm)
{
   const QString formName = this->getPersistantName ();
   PMElement formElement = pm->addNamedConfiguration (formName);

   // Save each active PV.
   //
   PMElement pvListElement = formElement.addElement ("PV_List");

   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      QString pvName = this->getSubstitutedVariableName (slot);

      if (!pvName.isEmpty ()) {
         PMElement pvElement = pvListElement.addElement ("PV");
         pvElement.addAttribute ("id", slot);
         pvElement.addValue ("Name", pvName);
      }
   }
}

//------------------------------------------------------------------------------
//
void QETable::restoreConfiguration (PersistanceManager* pm, restorePhases restorePhase)
{
   if (restorePhase != FRAMEWORK) return;

   const QString formName = this->getPersistantName ();
   PMElement formElement = pm->getNamedConfiguration (formName);

   // Restore each PV.
   //
   PMElement pvListElement = formElement.getElement ("PV_List");

   for (int slot = 0; slot < ARRAY_LENGTH (this->dataSet); slot++) {
      PMElement pvElement = pvListElement.getElement ("PV", "id", slot);
      QString pvName;
      bool status;

      if (pvElement.isNull ()) continue;

      // Attempt to extract a PV name
      //
      status = pvElement.getValue ("Name", pvName);
      if (status) {
         this->setVariableName (slot, pvName);
      }
   }
}

// end
