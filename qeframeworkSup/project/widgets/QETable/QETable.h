/*  QETable.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2014-2024 Australian Synchrotron
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

#ifndef QE_TABLE_H
#define QE_TABLE_H

#include <QHBoxLayout>
#include <QList>
#include <QMenu>
#include <QString>
#include <QStringList>
#include <QSize>
#include <QTableWidget>
#include <QTimer>
#include <QVector>

#include <QEEnums.h>
#include <QECommon.h>
#include <QEAbstractDynamicWidget.h>
#include <QEStringFormatting.h>
#include <QEStringFormattingMethods.h>
#include <QEFloatingArray.h>
#include <QEFloatingFormatting.h>
#include <persistanceManager.h>
#include <QCaObject.h>
#include <QEWidget.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEFrameworkLibraryGlobal.h>

/*!
   This class provides an EPICS aware table widget which is capable of displaying
   up to 20 array PVs in tabular form. When in the default vertical orientation
   each column displays a consecutive element from an array EPICS variable.

   When in horizonal mode, the table and functionality is transposed.

   It is tighly integrated, via the QEAbstractWidget, with the base class QEWidget
   class which provides generic support such as macro substitutions, drag/drop,
   and standard properties. QEAbstractWidget provides all standard properties.
 */
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QETable :
      public QEAbstractDynamicWidget,
      public QEStringFormattingMethods
{
   Q_OBJECT

public:
   // Must be consistant with the number of variable name properties below and
   // calls to PROPERTY_ACCESS below.
   //
   enum Constants {
      MAXIMUM_NUMBER_OF_VARIABLES = 20
   };

   // QETable specific properties ===============================================
   /// EPICS variable names (CA PV)
   //
   // Note, a property macro in the form 'Q_PROPERTY(QString variableName READ ...' doesn't work.
   // A property name ending with 'Name' results in some sort of string a variable being displayed,
   // but will only accept alphanumeric and won't generate callbacks on change.
   //
   Q_PROPERTY (QString variableName1  READ getVariableName1    WRITE setVariableName1)
   Q_PROPERTY (QString variableName2  READ getVariableName2    WRITE setVariableName2)
   Q_PROPERTY (QString variableName3  READ getVariableName3    WRITE setVariableName3)
   Q_PROPERTY (QString variableName4  READ getVariableName4    WRITE setVariableName4)
   Q_PROPERTY (QString variableName5  READ getVariableName5    WRITE setVariableName5)
   Q_PROPERTY (QString variableName6  READ getVariableName6    WRITE setVariableName6)
   Q_PROPERTY (QString variableName7  READ getVariableName7    WRITE setVariableName7)
   Q_PROPERTY (QString variableName8  READ getVariableName8    WRITE setVariableName8)
   Q_PROPERTY (QString variableName9  READ getVariableName9    WRITE setVariableName9)
   Q_PROPERTY (QString variableName10 READ getVariableName10   WRITE setVariableName10)
   Q_PROPERTY (QString variableName11 READ getVariableName11   WRITE setVariableName11)
   Q_PROPERTY (QString variableName12 READ getVariableName12   WRITE setVariableName12)
   Q_PROPERTY (QString variableName13 READ getVariableName13   WRITE setVariableName13)
   Q_PROPERTY (QString variableName14 READ getVariableName14   WRITE setVariableName14)
   Q_PROPERTY (QString variableName15 READ getVariableName15   WRITE setVariableName15)
   Q_PROPERTY (QString variableName16 READ getVariableName16   WRITE setVariableName16)
   Q_PROPERTY (QString variableName17 READ getVariableName17   WRITE setVariableName17)
   Q_PROPERTY (QString variableName18 READ getVariableName18   WRITE setVariableName18)
   Q_PROPERTY (QString variableName19 READ getVariableName19   WRITE setVariableName19)
   Q_PROPERTY (QString variableName20 READ getVariableName20   WRITE setVariableName20)

   /// Macro substitutions. The default is no substitutions. The format is NAME1=VALUE1[,] NAME2=VALUE2...
   /// Values may be quoted strings. For example, 'PUMP=PMP3, NAME = "My Pump"'
   /// These substitutions are applied to variable names for all QE widgets.
   /// In some widgets are are also used for other purposes.
   ///
   Q_PROPERTY (QString variableSubstitutions READ getSubstitutions WRITE setSubstitutions)

   /// Allows specification of tables titles. If blank, the default, then out-of-the-box
   /// QTableWidget heading are used, i.e. 1, 2, etc.  If "<>" is specified, then this is
   /// replaced by the PV name. This is particulary useful when PV na,es are specifed
   /// dynamically or by substitution.
   ///
   Q_PROPERTY (QStringList titles            READ getTitles        WRITE setTitles)

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

   // The following is a sub-set of the standard string formatting properties.
   //
   /// If true (default), add engineering units supplied with the data.
   ///
   Q_PROPERTY (bool addUnits READ getAddUnits WRITE setAddUnits)

   /// If false (default), no "+" sign, when true always add a sign.
   ///
   Q_PROPERTY (bool forceSign READ getForceSign WRITE setForceSign)

   /// Format to apply to data. Default is 'Default' in which case the data type
   /// supplied with the data determines how the data is formatted.
   /// For all other options, an attempt is made to format the data as requested
   /// (whatever its native form).
   ///
   Q_PROPERTY (QE::Formats format READ getFormatProperty WRITE setFormatProperty)

   /// Seperators used for interger and fixed point formatting. Default is None.
   ///
   Q_PROPERTY(QE::Separators separator READ getSeparatorProperty WRITE setSeparatorProperty)

   /// Notation used for numerical formatting. Default is fixed.
   ///
   Q_PROPERTY(QE::Notations notation READ getNotationProperty WRITE setNotationProperty)

   // End of QETable specific properties =========================================

public:
   /// Create without a variable(s).
   /// Use setVariableName functions.
   //
   explicit QETable (QWidget* parent = 0);

   /// Destruction
   virtual ~QETable ();

   // Single function for all set/get PV properties.
   //
public slots:
   void    setVariableName (const int, const QString&);
public:
   QString getVariableName (const int) const;

   void setSubstitutions (const QString& substitutions);
   QString getSubstitutions () const;

   void setColumnWidthMinimum (const int columnWidthMinimum);
   int getColumnWidthMinimum () const;

   void setDisplayMaximum (const int displayMaximum);
   int getDisplayMaximum () const;

   void setOrientation (const Qt::Orientation orientation);
   Qt::Orientation getOrientation () const;

   void setFormatProperty (const QE::Formats format);
   QE::Formats getFormatProperty () const;

   void setSeparatorProperty (const QE::Separators separator);
   QE::Separators getSeparatorProperty () const;

   void setNotationProperty (const QE::Notations notation);
   QE::Notations getNotationProperty () const;

   int addPvName (const QString& pvName);
   void clearAllPvNames ();

   // Property access READ and WRITE functions. We can define the access functions
   // using a macro.  Alas, due to SDK limitation, we cannot embedded the property
   // definitions in a macro.
   //
   #define PROPERTY_ACCESS(symbol, slot)                                                                 \
      void    setVariableName##symbol (const QString& name)  { this->setVariableName (slot, name); }     \
      QString getVariableName##symbol () const { return this->getVariableName (slot); }


   PROPERTY_ACCESS  (1,  0)
   PROPERTY_ACCESS  (2,  1)
   PROPERTY_ACCESS  (3,  2)
   PROPERTY_ACCESS  (4,  3)
   PROPERTY_ACCESS  (5,  4)
   PROPERTY_ACCESS  (6,  5)
   PROPERTY_ACCESS  (7,  6)
   PROPERTY_ACCESS  (8,  7)
   PROPERTY_ACCESS  (9,  8)
   PROPERTY_ACCESS  (10, 9)
   PROPERTY_ACCESS  (11, 10)
   PROPERTY_ACCESS  (12, 11)
   PROPERTY_ACCESS  (13, 12)
   PROPERTY_ACCESS  (14, 13)
   PROPERTY_ACCESS  (15, 14)
   PROPERTY_ACCESS  (16, 15)
   PROPERTY_ACCESS  (17, 16)
   PROPERTY_ACCESS  (18, 17)
   PROPERTY_ACCESS  (19, 18)
   PROPERTY_ACCESS  (20, 19)

   #undef PROPERTY_ACCESS

   // Expose access to the internal table widget's set/get functions.
   //
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (table, bool,         showGrid,  setShowGrid)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (table, Qt::PenStyle, gridStyle, setGridStyle)

public slots:
   // Sets PV col/row titles.
   //
   void setTitles (const QStringList& titles);
   void setTitle (const int slot, const QString& title);

   // Selects row/col depending on orientation vertical/horizontal.
   //
   void setSelection (int value);

   // Set the set of PV names.
   //
   void setPvNameSet (const QStringList& pvNameSet);

   // Set PV and title.
   //
   void setTableEntry (const int slot, const QString& pvName, const QString& title);

public:
   QStringList getTitles () const;
   int getSelection () const;
   QStringList getPvNameSet () const;

   // Allow third party plugins and display managers finer formatting control.
   // Note: (Re)setting common/property foratting setting will override the any
   // individual slot setting values.
   //
   void setSlotAddUnits  (const int slot, const bool addUnits);
   void setSlotForceSign (const int slot, const bool forceSign);
   void setSlotFormat    (const int slot, const QE::Formats format);
   void setSlotSeparator (const int slot, const QE::Separators separator);
   void setSlotNotation  (const int slot, const QE::Notations notation);

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
   void dbValueChanged (const QVector<double>& out);

protected:
   QSize sizeHint () const;
   bool eventFilter (QObject* watched, QEvent* event);
   void resizeEvent (QResizeEvent* event);

   // Override QEWidget functions.
   //
   void establishConnection (unsigned int variableIndex);
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);
   void activated ();
   void stringFormattingChange ();

   // Context menu
   //
   enum OwnContextMenuOptions {
      CM_HORIZONTAL_TABLE = ADWCM_SUB_CLASS_WIDGETS_START_HERE,
      CM_VERTICAL_TABLE
   };

   QMenu* buildContextMenu ();                        // Build the QETable specific context menu
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

   void saveConfiguration (PersistanceManager* pm);
   void restoreConfiguration (PersistanceManager* pm, restorePhases restorePhase);

private:
   bool isVertical () const;  // True iff the orientation is Qt::Vertical
   void resizeCoulumns ();    // Resizes colums to fit available space
   int numberInUse () const;  // Calculate the required number of cols (or rows when horizontal).
   int dataSize () const;     // Calculate the required number of rows (or cols when horizontal).

   // Provides consistant interpretation of variableIndex.
   // Must be consistent with variableIndex allocation in the contructor.
   //
   int slotOf  (const unsigned int vi) { return int (vi); }

   QTableWidget* table;         // internal widget
   QHBoxLayout* layout;         // holds the internal widget - any layout type will do
   QTimer* rePopulateTimer;
   int displayMaximum;
   Qt::Orientation orientation;
   QEFloatingFormatting floatingFormatting;
   int selection;
   int columnWidthMinimum;
   bool selectionChangeInhibited;
   bool pvNameSetChangeInhibited;
   bool titlesChangeInhibited;
   bool rePopulateAll;
   bool rePopulateTitles;
   bool rePopulateData;

   // Per PV data.
   //
   class DataSets {
   public:
      explicit DataSets ();
      ~DataSets ();
      void setContext (QETable* owner, const int slot);
      void clear ();

      void setPvName (const QString& pvName);
      QString getPvName () const;

      bool isInUse () const;
      void rePopulateTable ();
      void rePopulateData ();

      QEFloatingArray data;
      QCaAlarmInfo alarmInfo;

      QString title;
      QCaVariableNamePropertyManager variableNameManager;
      bool isConnected;
      QEStringFormatting stringFormatting;  // each data set gets own copy

   private:
      QETable* owner;
      int slot;
      QString pvName;
      int index;  // <= slot . Is < if unused slots
   };

   DataSets dataSet [MAXIMUM_NUMBER_OF_VARIABLES];

private slots:
   void setNewVariableName (QString variableNameIn,
                            QString variableNameSubstitutionsIn,
                            unsigned int variableIndex);

   void connectionChanged (QCaConnectionInfo& connectionInfo,
                           const unsigned int &variableIndex);

   void dataArrayChanged (const QVector<double>& values,
                          QCaAlarmInfo& alarmInfo,
                          QCaDateTime& timeStamp,
                          const unsigned int& variableIndex);

   void gridCellClicked (int row, int column);
   void gridCellEntered (int row, int column);

   void timeout ();
};

#endif // QE_TABLE_H
