/*  QEDynamicFormGrid.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2019-2024 Australian Synchrotron
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

#ifndef QE_DYNAMIC_FORM_GRID_H
#define QE_DYNAMIC_FORM_GRID_H

#include <QList>
#include <QGridLayout>
#include <QString>
#include <QStringList>
#include <QHBoxLayout>
#include <QMenu>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>

#include <QEEnums.h>
#include <QEForm.h>
#include <QEFrame.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEFrameworkLibraryGlobal.h>
#include <persistanceManager.h>

namespace Ui {
   class QEDynamicFormGrid;
}


/// This class loosely based on QEFormGrid
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEDynamicFormGrid : public QEFrame {
   Q_OBJECT

public:
   // QEDynamicFormGrid specific properties ====================================
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

   /// The uiFile loaded into each QEForm element.
   /// Default value: "".
   ///
   Q_PROPERTY (QString uiFile
               READ getUiFile                 WRITE setUiFile)

   /// The title of the QEDynamicFormGrid.
   /// Default value: "".
   ///
   Q_PROPERTY (QString title
               READ getTitle                  WRITE setTitle)

   /// (Default) name substitutions applied to uiFile and title.
   /// Called variableSubstitutions for consistancy.
   /// Default value: "".
   ///
   Q_PROPERTY (QString variableSubstitutions
               READ getNameSubstitutions      WRITE setNameSubstitutions)

   /// The formal macro name used when loading uiFile
   /// Default value: "DEVICE".
   ///
   Q_PROPERTY (QString formalArg
               READ getFormalArg              WRITE setFormalArg)

   /// The set of actual macro values available at run time.
   /// Default value: empty list.
   ///
   Q_PROPERTY (QStringList actualArgList
               READ getActualArgList          WRITE setActualArgList)

   /// Specifies the number of columns. This is restricted to the range 1 to 20.
   /// Default value: 1.
   ///
   Q_PROPERTY (int columns
               READ getColumns                WRITE setColumns)

   /// Specifies the gridOrder: RowMajor or ColMajor.
   /// Default value: RowMajor.
   ///
   Q_PROPERTY (QE::GridOrders gridOrder
               READ getGridOrder              WRITE setGridOrder)

   /// Margin applied to/extracted from the internal QGridLayout object.
   /// Default value: 2.
   ///
   Q_PROPERTY (int margin
               READ getMargin                 WRITE setMargin )

   /// Spaceing applied to/extracted from the internal QGridLayout object.
   /// Default value: 2.
   ///
   Q_PROPERTY (int spacing
               READ getSpacing                WRITE setSpacing)

   /// variableAsToolTip is added as a non-designable property here only to hide the implementation present in QEFrame
   ///
   Q_PROPERTY (bool variableAsToolTip
               READ getVariableAsToolTip      WRITE setVariableAsToolTip
               DESIGNABLE false)

   /// allowDrop is added as a non-designable property here only to hide the implementation present in QEFrame
   ///
   Q_PROPERTY (bool allowDrop
               READ getAllowDrop              WRITE setAllowDrop
               DESIGNABLE false)

   /// displayAlarmStateOption is added as a non-designable property here only
   /// to hide the implementation present in QEFrame
   ///
   Q_PROPERTY (QE::DisplayAlarmStateOptions displayAlarmStateOption
               READ getDisplayAlarmStateOption
               WRITE setDisplayAlarmStateOption
               DESIGNABLE false)
   //
   // End of QEDynamicFormGrid specific properties =============================

public:
   /// Create a grid widget with default parameters.
   ///
   explicit QEDynamicFormGrid (QWidget* parent = 0);
   explicit QEDynamicFormGrid (const QString& uiFileName, QWidget* parent = 0);

   /// Destruction
   virtual ~QEDynamicFormGrid ();

   // Property access functions.
   //
   void    setUiFile (const QString& uiFileName);
   QString getUiFile () const;

   void    setTitle (const QString& title);
   QString getTitle () const;

   void    setNameSubstitutions (const QString& variableSubstitutions);
   QString getNameSubstitutions () const;

   void setFormalArg (const QString& formalArg);
   QString getFormalArg () const;

   void setActualArgList (const QStringList& actualArgList);
   QStringList getActualArgList  () const;

   void setColumns (const int n);
   int getColumns () const;

   void setGridOrder (const QE::GridOrders go);
   QE::GridOrders getGridOrder () const;

   void setMargin (const int n);
   int getMargin () const;

   void setSpacing (const int n);
   int getSpacing () const;

protected:
   QSize sizeHint () const;

   // Called when new ui file/title is specified.
   //
   void establishConnection (unsigned int variableIndex);
   void saveConfiguration (PersistanceManager* pm);
   void restoreConfiguration (PersistanceManager* pm, restorePhases restorePhase);

private:
   class SingleItemContainer : public QWidget {
   public:
      explicit SingleItemContainer (const QString& actualArg,
                                    QEDynamicFormGrid* owner);
      ~SingleItemContainer();

      QString getActualArg () const;
      static SingleItemContainer* containerOf (QObject* theSender);

   private:
      QEDynamicFormGrid* owner;
      QString actualArg;
      QHBoxLayout* horizontalLayout;
      QEForm* qeform;
      QWidget* controlContainer;
      QVBoxLayout* verticalLayout;
      QPushButton* closeButton;
      QSpacerItem* verticalSpacer;
   };

   typedef QList<SingleItemContainer*> SingleItemList;

   void commonSetup (const QString& uiFileIn);
   void constructItem (const QString& actualArg);
   void reorderItems ();

   // The number of rows is determined from the overall number of items
   // and the number of allowed columns. It cannot be independently set.
   //
   int getRows () const;

   // Note, this is only used to manage the macro substitutions that will be
   // use to manage the uiFileName and the Title.
   //
   enum {
      UIFILE_NAME_VARIABLE = 0,
      TITLE_VARIABLE,
      NUMBER_OF_VARIABLES
   };

   Ui::QEDynamicFormGrid* ui;

   QMenu* selectionMenu;
   SingleItemList formList;   // SingleItemContainer are lazily constructed.
   QGridLayout* scrollFrameLayout;
   QWidget* rowScrollSpacer;
   QWidget* colScrollSpacer;

   // Property values.
   //
   QCaVariableNamePropertyManager vnpm [NUMBER_OF_VARIABLES];
   int columns;
   QE::GridOrders gridOrder;
   QString formalArg;
   QStringList actualArgList;
   int margin;
   int spacing;

private slots:
   void setNewName (QString name, QString substitutions, unsigned int index);
   void onSelectionMenuTriggered (QAction *);
   void onCloseButtonClick (bool);
   void resetScrollFrameSize ();
};

#endif // QE_DYNAMIC_FORM_GRID_H
