/*  QENTNDArrayData.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (C) 2018-2024 Australian Synchrotron
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
///   QVariant var;
///
///   var.setValue <QENTNDArrayData> (arrayData);       or
///   var = arrayData.toVariant();
///
///   arrayData = var.value<QENTNDArrayData>();
///   arrayData = qvariant_cast<QENTNDArrayData>(var);  or
///   arrayData.assignFromVariant (var);
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
   bool assignFrom (epics::nt::NTNDArray::const_shared_pointer item);
#endif

   // Clear all data.
   //
   void clear ();

   // Will decompress the image data if needs be.
   // This is an itempotent function.
   // Returns true if decompression is (or has been) successfull.
   //
   bool decompressData ();

   QByteArray getData () const;

   // Returns the name of the codec used to compress image.
   // If image was not compressed, this function returns an empty string.
   //
   QString getCodecName () const;

   int getNumberDimensions () const;
   int getDimensionSize (const int dimension) const;  // returns 0 if dimension is out of range

   QE::ImageFormatOptions getFormat() const;
   int getBytesPerPixel () const;
   int getWidth () const;
   int getHeight () const;
   int getBitDepth () const;

   // Returns QVariant type Invalid is the attribute is not defined.
   //
   QVariant getAttibute (const QString& name) const;


   // QVariant conversion related functions.
   //
   // Converstion to QVariant
   //
   QVariant toVariant () const;

   // Returns true if item is a QENTNDArrayData variant, i.e. can be used
   // as a parameter to the assignFromVariant function.
   //
   static bool isAssignableVariant (const QVariant& item);

   // Returns true if the assignment from item is successful.
   //
   bool assignFromVariant (const QVariant& item);

   // Register the QENTNDArrayData meta type.
   // Note: This function is public for conveniance only, and is invoked by
   // the module itself during program elaboration.
   //
   static bool registerMetaType ();

   typedef QMap<QString, QVariant> AttributeMaps;

private:

#ifdef QE_INCLUDE_PV_ACCESS
   // array iterator
   //
   typedef epics::pvData::PVStructureArray::const_svector::const_iterator VectorIter;

   epics::pvData::ScalarType getValueType (epics::pvData::PVUnionPtr value) const;

   // writes to the QByteArray data member
   //
   template <typename arrayType>
   void toValue (epics::pvData::PVUnionPtr value);

   QE::ImageFormatOptions getImageFormat
      (epics::pvData::PVStructureArray::const_svector attrVec) const;

#endif

   void assignOther (const QENTNDArrayData& other);

   bool decompressJpeg ();
   bool decompressBlosc ();
   bool decompressLz4 ();
   bool decompressBslz4 ();

   int numberDimensions;
   int dimensionSizes [10];   // we expect only 2 or 3. area detector NDArray allows upto 10
   int bytesPerPixel;
   size_t numberElements;
   size_t totalBytes;

   qlonglong compressedDataSize;
   qlonglong uncompressedDataSize;

   // data time stamp
   qlonglong dtsSecondsPastEpoch;
   int dtsNanoseconds;
   int dtsUserTag;

   int uniqueId;
   QString descriptor;

   AttributeMaps attributeMap;

   QByteArray data;                 // basic image data
   QString codecName;               // the codec nam
   QE::ImageFormatOptions format;   // derived from the ColorMode attribute
   int bitDepth;
   bool isDecompressed;
};

// allows qDebug() << QENTNDArrayData object.
//
QDebug operator<<(QDebug dbg, const QENTNDArrayData &arrayData);

Q_DECLARE_METATYPE (QENTNDArrayData)

#endif  // QE_NT_NDARRAY_DATA_H
