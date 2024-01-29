/*  QEFormGrid.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2024 Australian Synchrotron
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

#ifndef QE_FORM_GRID_H
#define QE_FORM_GRID_H

#include <QGridLayout>
#include <QString>
#include <QStringList>
#include <QTimer>

#include <QEForm.h>
#include <QEFrame.h>

#include <QCaVariableNamePropertyManager.h>
#include <QEFrameworkLibraryGlobal.h>

/// This class load a grid QEForms.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEFormGrid : public QEFrame {
   Q_OBJECT

public:
   // QEFormGrid specific properties ===============================================
   //
   /// GridOrders specfies how grid elements are layed out.
   ///
   /// Row major (example 10, items, 3 cols):
   ///    0  1  2
   ///    3  4  5
   ///    6  7  8
   ///    9  -  -
   ///
   /// Col major (example 10, items, 3 cols):
   ///    0  4  8
   ///    1  5  9
   ///    2  6  -
   ///    3  7  -
   ///
   enum GridOrders {
      RowMajor = 0,
      ColMajor = 1
   };
   Q_ENUM (GridOrders)


   /// The uiFile loaded into each QEForm element.
   /// Default value: ""
   Q_PROPERTY (QString uiFile          READ getUiFile           WRITE setUiFile )

   Q_PROPERTY (QString variableSubstitutions READ getGridVariableSubstitutions WRITE setGridVariableSubstitutions)

   /// The total number of QEForms. This is restricted to the range 1 to 2000.
   /// Default value: 4.
   Q_PROPERTY (int number              READ getNumber           WRITE setNumber     )


   /// Specifies the number of columns.  This is restricted to the range 1 to 200.
   /// Default value: 1.
   Q_PROPERTY (int columns             READ getColumns          WRITE setColumns    )

   /// Specifies the gridOrder.
   /// Default value: RowMajor
   Q_PROPERTY (GridOrders gridOrder    READ getGridOrder        WRITE setGridOrder )

   /// Margin of the internal QGridLayout object.
   /// Default value: 2.
   Q_PROPERTY (int margin              READ getMargin           WRITE setMargin     )

   /// Spaceing of the internal QGridLayout object.
   /// Default value: 2.
   Q_PROPERTY (int spacing             READ getSpacing          WRITE setSpacing    )


   /// Specified formal slot macro name prefix
   /// Default value: SLOT
   Q_PROPERTY (QString  slotMacroPrefix  READ getSlotMacroPrefix  WRITE setSlotMacroPrefix )

   /// Specifies the SLOT macro number offset.
   /// Default value: 1.
   Q_PROPERTY (int slotNumberOffset    READ getSlotOffset       WRITE setSlotOffset  )

   /// Specifies the SLOT macro width format.
   /// Default value: 2.
   Q_PROPERTY (int slotNumberWidth     READ getSlotNumberWidth  WRITE setSlotNumberWidth  )

   /// Specifies the SLOTNAME macro values applied to each QEFrom grid element.
   /// Slot number allocation is impacted by the gridOrder property.
   /// Indexing of the slotStrings is NOT impacted by the slotNumberOffset.
   /// Default value: ""
   Q_PROPERTY (QStringList slotStrings READ getSlotStrings      WRITE setSlotStrings )

   // Ditto ROW and COL

   /// Specified formal slot macro name prefix
   /// Default value: ROW
   Q_PROPERTY (QString  rowMacroPrefix  READ getRowMacroPrefix      WRITE setRowMacroPrefix )

   /// Specifies the ROW macro number offset.
   /// Default value: 1.
   Q_PROPERTY (int rowNumberOffset     READ getRowOffset        WRITE setRowOffset  )

   /// Specifies the COL macro width format.
   /// Default value: 2.
   Q_PROPERTY (int rowNumberWidth      READ getRowNumberWidth   WRITE setRowNumberWidth  )

   /// Specifies the ROWNAME macro values applied to each QEFrom grid element in a spefic row.
   /// Indexing of the rowStrings is NOT impacted by the rowNumberOffset.
   /// Default value: ""
   Q_PROPERTY (QStringList rowStrings  READ getRowStrings       WRITE setRowStrings )


   /// Specified formal slot macro name prefix
   /// Default value: COL
   Q_PROPERTY (QString  colMacroPrefix  READ getColMacroPrefix      WRITE setColMacroPrefix )

   /// Specifies the COL macro number offset.
   /// Default value: 1.
   Q_PROPERTY (int colNumberOffset     READ getColOffset        WRITE setColOffset  )

   /// Specifies the COL macro width format.
   /// Default value: 2.
   Q_PROPERTY (int colNumberWidth      READ getColNumberWidth   WRITE setColNumberWidth  )

   /// Specifies the COLNAME macro values applied to each QEFrom grid element in a spefic column.
   /// Indexing of the colStrings is NOT impacted by the colNumberOffset.
   /// Default value: ""
   Q_PROPERTY (QStringList colStrings  READ getColStrings       WRITE setColStrings )

   /// variableAsToolTip is added as a non-designable property here only to hide the implementation present in QEFrame
   ///
   Q_PROPERTY (bool variableAsToolTip READ getVariableAsToolTip WRITE setVariableAsToolTip DESIGNABLE false)

   /// allowDrop is added as a non-designable property here only to hide the implementation present in QEFrame
   ///
   Q_PROPERTY (bool allowDrop READ getAllowDrop WRITE setAllowDrop  DESIGNABLE false)

   /// displayAlarmStateOption is added as a non-designable property here only to hide the implementation present in QEFrame
   ///
   Q_PROPERTY (DisplayAlarmStateOptions displayAlarmStateOption
               READ getDisplayAlarmStateOptionProperty
               WRITE setDisplayAlarmStateOptionProperty DESIGNABLE false)


   //
   // End of QEFormGrid specific properties =========================================

public:
   /// Create a grid widget with default parameters.
   ///
   explicit QEFormGrid (QWidget* parent = 0);
   explicit QEFormGrid (const QString& uiFile, const int number, const int cols, QWidget* parent = 0);

   /// Destruction
   virtual ~QEFormGrid ();

   // Property access functions.
   //
   void    setUiFile (QString uiFileName);
   QString getUiFile ();

   void    setGridVariableSubstitutions (QString variableSubstitutions);
   QString getGridVariableSubstitutions ();

   void setNumber (int n);
   int getNumber ();

   void setColumns (int n);
   int getColumns ();

   void setGridOrder (GridOrders go);
   GridOrders getGridOrder ();

   void setMargin (int n);
   int getMargin ();

   void setSpacing (int n);
   int getSpacing ();

   // Define propery access functions for slot, row and col attributes.
   //
#define SET_GET_ATTRIBUTES(Attr, attr)                                                         \
   void set##Attr##MacroPrefix (QString m) {  this->attr##MacroData->setMacroPrefix (m);  }    \
   QString get##Attr##MacroPrefix () { return this->attr##MacroData->getMacroPrefix ();   }    \
                                                                                               \
   void set##Attr##Offset (int n)  {  this->attr##MacroData->setOffset (n);  }                 \
   int  get##Attr##Offset () { return this->attr##MacroData->getOffset ();   }                 \
                                                                                               \
   void set##Attr##NumberWidth (int n)  {  this->attr##MacroData->setNumberWidth (n); }        \
   int  get##Attr##NumberWidth () { return this->attr##MacroData->getNumberWidth ();  }        \
                                                                                               \
   void set##Attr##Strings (QStringList& s)  {  this->attr##MacroData->setStrings (s);   }     \
   QStringList get##Attr##Strings ()   { return this->attr##MacroData->getStrings (); }


   SET_GET_ATTRIBUTES (Row, row)
   SET_GET_ATTRIBUTES (Col, col)
   SET_GET_ATTRIBUTES (Slot, slot)

#undef SET_GET_ATTRIBUTES

   // The number of rows is dertmined from the overall number of items
   // and the number of columns. It cannot be independently set.
   //
   int getRows ();

protected:
   QSize sizeHint () const;

private:
   QGridLayout* layout;
   QList<QEForm*> formsList;              // holds a reference to each of the QEForms

   // Holds the subsituted property values.
   //
   QString uiFile;

   // Note, this is only used to manage the macro substitutions that will be passed
   // down to the grid's QE widgets. The form has no variable names per se. It also
   // handles the property delay mechanism. The inherited VariableNameManager
   // holds the name and manages name substiution.
   //
   QCaVariableNamePropertyManager variableNamePropertyManager;

   int number;
   int columns;
   GridOrders gridOrder;

   // Class holds and manages rol, column and slot macro data.
   //
   class MacroData {
   public:
      explicit MacroData (const QString& prefix, QEFormGrid* formGrid);

      void setMacroPrefix (const QString& prefix);
      QString getMacroPrefix ();

      void setOffset (const int offset);
      int getOffset ();

      void setNumberWidth (const int numberWidth);
      int getNumberWidth ();

      void setStrings (const QStringList& strings);
      QStringList getStrings ();

      // Generate macto sub. string of format
      // <prefix>NAME=strings[n] ,<prefix>=<n+offset>
      //
      QString genSubsitutions (const int n);

   private:
      QString prefix;
      QEFormGrid* formGrid;
      int offset;          // defaults to 1 - typically 0 or 1
      int numberWidth;     // defaults to 2 - typically 1 to 3
      QStringList strings;
   };
   friend class MacroData;

   MacroData* rowMacroData;
   MacroData* colMacroData;
   MacroData* slotMacroData;

   // Converts row and col number to slot number and vice-versa.
   // The conversions are gridOrder dependent.
   // Slots, rows and cols are 0 to N-1
   //
   int slotOf (const int row, const int col);
   void splitSlot (const int slot, int& row, int& col);

   // Local utility functions.
   //
   void commonSetup (const QString& uiFile, const int number, const int cols);
   QString getPrioritySubstitutions (const int slot);
   QEForm* createQEForm (const int slot);

   void addSubForm ();                      // Crates next QEForm instance
   void triggerReCreateAllForms ();         // trigger
   void reCreateAllForms ();                // Re create all forms with new settings

   // Called when new ui file specified.
   //
   void establishConnection (unsigned int variableIndex);

   // Manage property input delayed action similar  QCaVariableNamePropertyManager
   //
   // If set, there is a user typing new propety values.
   // Use timers to wait for typing to finish.
   //
   QTimer* inputTimer;
   bool interactive;

private slots:
   // Called a short time after a user stops typing in 'designer'
   //
   void inputDelayExpired ();

   // Note, in QEFormGrid, the standard variable name mechanism is used for the UI file name.
   //
   void setNewUiFile (QString variableNameIn, QString variableNameSubstitutionsIn, unsigned int variableIndex);
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEFormGrid::GridOrders)
#endif

#endif // QE_FORM_GRID_H
