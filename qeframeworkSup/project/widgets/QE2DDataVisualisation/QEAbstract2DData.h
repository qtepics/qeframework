/*  QEAbstract2DData.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2020 Australian Synchrotron
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
 *    andrews@ansto.gov.au
 */

#ifndef QE_ABSTRACT_2D_DATA_H
#define QE_ABSTRACT_2D_DATA_H

#include <QList>
#include <QString>
#include <QSize>
#include <QRect>
#include <QWidget>
#include <QEFloating.h>
#include <QEFloatingArray.h>
#include <QEFloatingFormatting.h>
#include <QEInteger.h>
#include <QEIntegerFormatting.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QEFrame.h>

/// \brief The QEAbstract2DData class.
/// This is the base class for the QESpectogram, QEWaterfall and QESurface
/// widgets. It provides data management functionaly for each sub class, and
/// is not intended to be used as a concrete class.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEAbstract2DData : public QEFrame
{
   Q_OBJECT
public:
   // Some QEFrame properties are really not applicable to this widget hierarchy.
   // These are re-declared as DESIGNABLE false.
   // Consider spliting QEFrame into QEFrame (basic) and QEPixmapFrame - keep that for 3.8
   //
   Q_PROPERTY (bool scaledContents READ getScaledContents  WRITE setScaledContents  DESIGNABLE false)
   Q_PROPERTY (QPixmap pixmap0     READ getPixmap0Property WRITE setPixmap0Property DESIGNABLE false)
   Q_PROPERTY (QPixmap pixmap1     READ getPixmap1Property WRITE setPixmap1Property DESIGNABLE false)
   Q_PROPERTY (QPixmap pixmap2     READ getPixmap2Property WRITE setPixmap2Property DESIGNABLE false)
   Q_PROPERTY (QPixmap pixmap3     READ getPixmap3Property WRITE setPixmap3Property DESIGNABLE false)
   Q_PROPERTY (QPixmap pixmap4     READ getPixmap4Property WRITE setPixmap4Property DESIGNABLE false)
   Q_PROPERTY (QPixmap pixmap5     READ getPixmap5Property WRITE setPixmap5Property DESIGNABLE false)
   Q_PROPERTY (QPixmap pixmap6     READ getPixmap6Property WRITE setPixmap6Property DESIGNABLE false)
   Q_PROPERTY (QPixmap pixmap7     READ getPixmap7Property WRITE setPixmap7Property DESIGNABLE false)

   /// EPICS main data variable name.
   ///
   Q_PROPERTY (QString dataVariable  READ getDataPvName     WRITE setDataPvName)

   /// Data width variable name - needed when data format is 2D, otherwise ignored.
   ///
   Q_PROPERTY (QString widthVariable READ getWidthPvName    WRITE setWidthPvName)

   /// Default macro substitutions. The default is no substitutions.
   /// The format is NAME1=VALUE1[,] NAME2=VALUE2...
   /// Values may be quoted strings. For example, 'PUMP=PMP3, NAME = "My Pump"'
   /// These substitutions are applied to variable names for all QE widgets.
   /// In some widgets are are also used for other purposes.
   ///
   Q_PROPERTY (QString variableSubstitutions
               READ getVariableNameSubstitutions
               WRITE setVariableNameSubstitutions)

   /// If a width variable is NOT specified, the data width may be set as a property.
   /// If defined, the widthVariable always overrides this value.
   /// Constrained to be >= 1. Default 100
   ///
   Q_PROPERTY (int dataWidth         READ getDataWidth      WRITE setDataWidth)

   /// Defines whether the source data is treated as a 1D array oe a 2D array.
   /// When source data is defined as 1D, the widget accumulates data on a FIFO
   /// basis, similar to the compress record, upto a maximum of numberOfSets
   /// "rows" of data. The accumulated data is then treated as a 2D array.
   /// Default: array2D
   ///
   enum DataFormats {
      array1D,
      array2D
   };
   Q_ENUMS(DataFormats)

   Q_PROPERTY (DataFormats dataFormat   READ getDataFormat  WRITE setDataFormat)

   /// Specifies the number of data sets to accumulate when the dataFormat
   /// property is set to array1D, otherwise it is ignored.
   /// Allowed range: 1 .. 1024
   /// Default: 40
   ///
   Q_PROPERTY (int numberOfSets      READ getNumberOfSets   WRITE setNumberOfSets)

   Q_PROPERTY (bool autoScale        READ getAutoScale      WRITE setAutoScale)
   Q_PROPERTY (double minimum        READ getMinimum        WRITE setMinimum)
   Q_PROPERTY (double maximum        READ getMaximum        WRITE setMaximum)

public:
   enum Constants {
      DATA_PV_INDEX = 0,
      WIDTH_PV_INDEX = 1
   };

   /// Create without a variable.
   /// Use setDataPvName () and setVariableNameSubstitutions() to define a
   /// variable and, optionally, macro substitutions later.
   ///
   explicit QEAbstract2DData (QWidget* parent = 0);

   /// Create with a variable (s).
   /// A connection is automatically established.
   /// If macro substitutions are required, create without a variable and
   /// set the variable and macro substitutions after creation.
   ///
   explicit QEAbstract2DData (const QString& dataVariableName,
                              QWidget* parent = 0);

   explicit QEAbstract2DData (const QString& dataVariableName,
                              const QString& widthVariableName,
                              QWidget* parent = 0);

   /// Destruction
   virtual ~QEAbstract2DData ();

   int getEffectiveDataWidth () const;

   // Property access functions.
   //
   void setDataPvName (const QString dataVariable);
   QString getDataPvName () const;

   void setWidthPvName (const QString widthVariable);
   QString getWidthPvName () const;

   void setVariableNameSubstitutions (const QString variableSubstitutions);
   QString getVariableNameSubstitutions () const;

   void setDataWidth (const int dataWidth);
   int getDataWidth () const;

   void setDataFormat (const DataFormats dataFormat);
   DataFormats getDataFormat () const;

   void setNumberOfSets (const int numberOfSets);
   int getNumberOfSets () const;

   void setAutoScale (const bool autoScale);
   bool getAutoScale () const;

   void setMinimum (const double minimum);
   double getMinimum () const;

   void setMaximum (const double maximum);
   double getMaximum () const;

signals:
   // This signal is emitted using the QEEmitter::emitDbConnectionChanged function.
   /// Sent when the widget state updated following a channel connection change
   /// Applies to every varible.
   ///
   void dbConnectionChanged (const bool& isConnected);

   /// Sent when the widget is updated following a data change
   /// Can be used to pass on data update 'event' to other widgets.
   ///
   void dbValueChanged ();

protected:
   // Override parent virtual functions.
   //
   // Drag and Drop
   //
   void dragEnterEvent (QDragEnterEvent* event) { this->qcaDragEnterEvent (event);  }
   void dropEvent (QDropEvent* event)           { this->qcaDropEvent (event);       }
   void mousePressEvent (QMouseEvent* event)    { this->qcaMousePressEvent (event); }

   // Use default getDrop/setDrop

   // Copy paste
   //
   QString copyVariable ();
   QVariant copyData ();
   void paste (QVariant s);

   // override QEWidget functions.
   //
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);
   void establishConnection (unsigned int variableIndex);

   // Data is held as a list of vectors.
   // For 2D data, there is only one vector in the list.
   //
   typedef QList<QEFloatingArray> TwoDimensionalData;

   TwoDimensionalData getData () const;

   // If srcRow or srcCol out of range of the available data, this function
   // returns the specified defaultValue.
   //
   double getValue (const int srcRow, const int srcCol,
                    const double defaultValue) const;

   int getUpdateCount () const;

   // Determines and updates min and max values if data available, otherwise
   // min and max left "as is", i.e. unchanged. Therefore caller should supply
   // sensible default.
   //
   void getDataMinMaxValues (double& min, double& max);

   // When accumulate is false, numberRows is just the number of rows and the
   // potential parameter is essentially ignored.
   //
   // When accumulate is true, the potential parameter controls whether the
   // returned numberRows is the actual number of rows/data sets accumulated
   // so far (potential = false) or the maximum number that can be accumulated
   // (potential = true). Of course once the widget has been running for a
   // while these number will be the same.
   //
   void getNumberRowsAndCols (const bool potential,
                              int& numberRows, int& numberCols);

   // Get the data engineering units and precision.
   //
   QString getUnits() const;
   int getPrecision() const;

   void setReadOut (const QString& text);
   void setElementReadout (const int row, const int col);

   virtual void updateDataVisulation ();   // hook function

private:
   void commonSetup ();

   QCaVariableNamePropertyManager dnpm;   // data name
   QCaVariableNamePropertyManager wnpm;   // width name

   QEFloatingFormatting floatingFormatting;
   QEIntegerFormatting integerFormatting;

   // Property members
   //
   int mDataWidth;
   bool mAutoScale;
   double mMinimum;
   double mMaximum;
   DataFormats mDataFormat;
   int mNumberOfSets;

   // When mDataFormat is array2D, this is limited to one QEFloatingArray
   // When mDataFormat is array1D, is limited to mNumberOfSets QEFloatingArrays.
   //
   TwoDimensionalData data;
   int updateCount;

   bool pvDataWidthAvailable;
   int  pvDataWidth;

private slots:
   void setVariableNameProperty (QString variableName,
                                 QString variableNameSubstitutions,
                                 unsigned int variableIndex);

   void connectionChanged (QCaConnectionInfo& connectionInfo,
                           const unsigned int& variableIndex);

   void onDataArrayUpdate (const QVector<double>& values, QCaAlarmInfo&,
                           QCaDateTime&, const unsigned int&);

   void onWidthUpdate (const long value, QCaAlarmInfo&,
                       QCaDateTime&, const unsigned int&);
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEAbstract2DData::DataFormats)
#endif

#endif // QE_ABSTRACT_2D_DATA_H
