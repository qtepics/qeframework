/*  QENTNDArrayData.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2018-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_NT_NDARRAY_DATA_H
#define QE_NT_NDARRAY_DATA_H

#include <QByteArray>
#include <QDebug>
#include <QList>
#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QVariant>
#include <QEEnums.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QEPvaCheck.h>

#ifdef QE_INCLUDE_PV_ACCESS
#include <pv/ntndarray.h>
#include <pv/pvData.h>
#endif

/// Defines an image class type, specifically to support the epics:nt/NTNDArray type.
/// This type is registered as a QVariant type, and can be set/get like this:
///
///   QENTNDArrayData arrayData;
///   QVariant variant;
///
///   variant.setValue (arrayData);                         or
///   variant = arrayData.toVariant();                      -- defined in this module
///
///   arrayData = variant.value<QENTNDArrayData>();         or
///   arrayData = qvariant_cast<QENTNDArrayData>(variant);  or
///   arrayData.assignFromVariant (variant);                -- defined in this module
///
/// Much of this class was based on ntndArrayConverter out of areaDetector
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QENTNDArrayData {
public:
   // explicit is a no-no here.
   QENTNDArrayData ();
   QENTNDArrayData (const QENTNDArrayData& other);
   ~QENTNDArrayData ();

   QENTNDArrayData& operator=(const QENTNDArrayData& other);

#ifdef QE_INCLUDE_PV_ACCESS
   // Note: we can only read, as opposed to write, NTNDArray types for now.
   //
   bool assignFrom (epics::nt::NTNDArrayPtr item);
#endif

   // Access image data attributes.
   //
   QByteArray getData () const;

   QString getColourMode() const;
   QString getDataType() const;

   int getNumberDimensions () const;
   int getDimensionSize (const int dimension) const;  // returns 0 if dimension is out of range

   int getBytesPerPixel () const;

   QString getDescription () const;
   int getUniqueId () const;

   // Clear all data.
   //
   void clear ();

   // QVariant conversion related functions.
   //
   // Conversion to QVariant
   //
   QVariant toVariant () const;

   // Returns true if item is a QENTNDArrayData variant, i.e. can be used
   // as a parameter to the assignFromVariant function.
   //
   static bool isAssignableVariant (const QVariant& item);

   // Returns true if the assignment from item is successful.
   //
   bool assignFromVariant (const QVariant& item);

private:
   struct Compression {
      QString codecName;
      qlonglong compressedDataSize;
      qlonglong uncompressedDataSize;
   };

   // Will decompress the image data if needs be.
   // Returns true if decompression is successfull.
   // If ADSupport not incuded, will always return false.
   //
   bool decompressData (const Compression& compression);

#ifdef QE_INCLUDE_PV_ACCESS
   // array iterator
   //
   typedef epics::pvData::PVStructureArray::const_svector::const_iterator VectorIter;

   epics::pvData::ScalarType getValueType (epics::pvData::PVUnionPtr value) const;

   // writes to the QByteArray data member
   //
   template <typename arrayType>
   void toValue (epics::pvData::PVUnionPtr value);

#endif

   void assignOther (const QENTNDArrayData& other);

   bool decompressJpeg (const Compression& compression);
   bool decompressBlosc (const Compression& compression);
   bool decompressLz4 (const Compression& compression);
   bool decompressBslz4 (const Compression& compression);

   QByteArray data;           // basic image data
   QString colourMode;        // text as is from ColorMode_RBV
   QString dataType;          // text as is from DataType_RBV
   int numberDimensions;
   int dimensionSizes [10];   // we expect only 2 or 3. area detector NDArray allows upto 10
   int bytesPerPixel;              // bytes per pixel

   // Not needed for QEImage per se..
   QString descriptor;
   int uniqueId;
};

// allows qDebug() << QENTNDArrayData object.
//
QDebug QE_FRAMEWORK_LIBRARY_SHARED_EXPORT
operator<<(QDebug dbg, const QENTNDArrayData &arrayData);

Q_DECLARE_METATYPE (QENTNDArrayData)

#endif  // QE_NT_NDARRAY_DATA_H
