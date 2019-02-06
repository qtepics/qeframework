/*  QENTTable.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2014,2016,2018 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_NT_TABLE_H
#define QE_NT_TABLE_H

#include <QHBoxLayout>
#include <QList>
#include <QMenu>
#include <QString>
#include <QStringList>
#include <QSize>
#include <QTableWidget>
#include <QTimer>
#include <QVector>

#include <QECommon.h>
#include <QEAbstractWidget.h>
#include <QCaObject.h>
#include <QEWidget.h>
#include <QCaVariableNamePropertyManager.h>
#include <QESingleVariableMethods.h>
#include <QEFrameworkLibraryGlobal.h>

/*!
   This class provides an EPICS aware table widget which is capable of displaying
   a PV Access Normative Type Table in tabular form. When in the default vertical
   orientation each column displays a consecutive element from an array EPICS variable.

   When in horizonal mode, the table and functionality is transposed.

   It is tighly integrated, via the QEAbstractWidget, with the base class QEWidget
   class which provides generic support such as macro substitutions, drag/drop,
   and standard properties. QEAbstractWidget provides all standard properties.
 */

class QENTTableData;  //  differed

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QENTTable :
      public QEAbstractWidget,
      public QESingleVariableMethods
{
   Q_OBJECT

   // Note, a property macro in the form 'Q_PROPERTY(QString variableName READ ...' doesn't work.
   // A property name ending with 'Name' results in some sort of string a variable being displayed,
   // but will only accept alphanumeric and won't generate callbacks on change.
public:
   /// EPICS variable name (PVA PV)
   ///
   Q_PROPERTY (QString variable READ getVariableNameProperty WRITE setVariableNameProperty)

   /// Macro substitutions. The default is no substitutions. The format is NAME1=VALUE1[,] NAME2=VALUE2...
   /// Values may be quoted strings. For example, 'PUMP=PMP3, NAME = "My Pump"'
   /// These substitutions are applied to variable names for all QE widgets.
   /// In some widgets are are also used for other purposes.
   ///
   Q_PROPERTY (QString variableSubstitutions READ getVariableNameSubstitutionsProperty WRITE setVariableNameSubstitutionsProperty)

   /// Specified the minimum allow column width. The widget will shrink/expand the width
   /// of each column to as to exactly fit the with of the widget. However, columns will
   /// not shrink to less than the value provided by this property. Defaults to 80.
   ///
   Q_PROPERTY (int colWidthMinimum    READ getColumnWidthMinimum   WRITE setColumnWidthMinimum)

   /// The maximum number of array elements that will be displayed irrespective of the
   /// number of elements that the EPICS variable contains. Defaults to 4096.
   ///
   Q_PROPERTY (int displayMaximum     READ getDisplayMaximum       WRITE setDisplayMaximum)

   /// Determines if the variable values are displayed in rows (orientation is horizontal)
   /// or in columns (orientation is vertical). The default is vertical.
   ///
   Q_PROPERTY (Qt::Orientation orientation READ getOrientation WRITE setOrientation)

   /// Controls if table grid is displayed. Default to true.
   ///
   Q_PROPERTY (bool showGrid          READ showGrid            WRITE setShowGrid)

   /// Sets table grid style. Defaults to SolidLine.
   ///
   Q_PROPERTY (Qt::PenStyle gridStyle READ gridStyle           WRITE setGridStyle)
   //
   // End of QENTTable specific properties =========================================

public:
   /// Create without a variable.
   /// Use setVariableName functions.
   //
   explicit QENTTable (QWidget* parent = 0);

   /// Create with a variable.
   //
   explicit QENTTable (const QString& variableName, QWidget* parent = 0);

   /// Destruction
   virtual ~QENTTable ();

   void setColumnWidthMinimum (const int columnWidthMinimum);
   int getColumnWidthMinimum () const;

   void setDisplayMaximum (const int displayMaximum);
   int getDisplayMaximum () const;

   void setOrientation (const Qt::Orientation orientation);
   Qt::Orientation getOrientation () const;

   // Expose access to the internal table widget's set/get functions.
   //
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (table, bool,         showGrid,  setShowGrid)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (table, Qt::PenStyle, gridStyle, setGridStyle)

public slots:
   // Selects row/col depending on orientation vertical/horizontal.
   //
   void setSelection (int value);

public:
   QStringList getTitles () const;
   int getSelection () const;

signals:
   void selectionChanged (int value);
   void pvNameSetChanged (const QStringList& pvNameSet);
   void titlesChanged (const QStringList& pvNameSet);

   // Note, the following signals are common to many QE widgets,
   // if changing the doxygen comments, ensure relevent changes are migrated to all instances
   /// Sent when the widget is updated following a data change
   /// Can be used to pass on EPICS data (as presented in this widget) to other widgets.
   /// For example a QList widget could log updates from this widget.
   /// Note: this widget emits the numeric enumeration value as opposed to the associated text.
   ///
   void dbValueChanged (const QENTTableData& out);
   void dbValueChanged (const QVariant& out);

protected:
   QSize sizeHint () const;
   bool eventFilter (QObject* watched, QEvent* event);
   void resizeEvent (QResizeEvent* event);

   // Override QEWidget functions.
   //
   void establishConnection (unsigned int variableIndex);
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);
   void activated ();

   // Context menu
   //
   enum OwnContextMenuOptions { CM_HORIZONTAL_TABLE = CM_SPECIFIC_WIDGETS_START_HERE ,
                                CM_VERTICAL_TABLE };

   QMenu* buildContextMenu ();                        // Build the QENTTable specific context menu
   void contextMenuTriggered (int selectedItemNum);   // An action was selected from the context menu

   // Drag and Drop
   //
   void mousePressEvent (QMouseEvent *event)    { qcaMousePressEvent (event); }
   void dragEnterEvent (QDragEnterEvent *event) { qcaDragEnterEvent (event, false); }
   void dropEvent (QDropEvent *event)           { qcaDropEvent (event, true); }

   // This widget uses the setDrop/getDrop defined in QEWidget.

   // Copy paste
   //
   QString copyVariable ();
   QVariant copyData ();

private:
   void commonConstruct ();   //
   bool isVertical () const;  // True iff the orientation is Qt::Vertical
   void resizeCoulumns ();    // Resizes colums to fit available space

   void populateTable ();             //
   void populateVerticalTable ();     //
   void populateHorizontalTable ();   //

   // Provides consistant interpretation of variableIndex.
   // Must be consistent with variableIndex allocation in the contructor.
   //
   int slotOf  (const unsigned int vi) { return int (vi); }

   QENTTableData* tableData;
   QTableWidget* table;         // internal widget
   QHBoxLayout* layout;         // holds the internal widget - any layout type will do
   QTimer* rePopulateTimer;
   int displayMaximum;
   Qt::Orientation orientation;
   int selection;
   int columnWidthMinimum;
   bool selectionChangeInhibited;
   bool rePopulateData;
   bool isFirstUpdate;
   bool isConnected;

private slots:
   void setNewVariableName (QString variableNameIn,
                            QString variableNameSubstitutionsIn,
                            unsigned int variableIndex);

   void connectionChanged (QCaConnectionInfo& connectionInfo,
                           const unsigned int &variableIndex);

   void tableDataChanged (const QVariant& value,
                          QCaAlarmInfo& alarmInfo,
                          QCaDateTime& timeStamp,
                          const unsigned int& variableIndex);

   void gridCellClicked (int row, int column);
   void gridCellEntered (int row, int column);

   void timeout ();
};

#endif // QE_TABLE_H
