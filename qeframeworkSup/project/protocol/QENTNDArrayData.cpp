/*  QENTNDArrayData.cpp
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

#include "QENTNDArrayData.h"
#include <QEPvaData.h>
#include <QString>

// Do we need support for decpmpression?
//
#ifdef QE_AD_SUPPORT

#if defined (Q_OS_LINUX)
#include <os/Linux/jpeglib.h>

#elif defined (Q_OS_WIN)
#include <os/WIN32/jpeglib.h>

#elif defined (Q_OS_DARWIN)
#include <os/Darwin/jpeglib.h>

#else
#error Unexpected OS - not Q_OS_LINUX, Q_OS_WIN nor Q_OS_DARWIN
#endif

#include <blosc.h>
#include <lz4.h>
#include <bitshuffle.h>

#endif   // QE_AD_SUPPORT

// Move to QECommon
#ifdef MIN
#undef MIN
#endif

#ifdef MAX
#undef MAX
#endif

#include <QENTNDArrayConverter.h>
#include <QECommon.h>

#define DEBUG qDebug() << "QENTNDArrayData" << __LINE__ << __FUNCTION__ << "  "

//------------------------------------------------------------------------------
//
QENTNDArrayData::QENTNDArrayData ()
{
   // needed for types to be registrered as meta type.
   //
   this->clear();
}

//------------------------------------------------------------------------------
//
QENTNDArrayData::QENTNDArrayData (const QENTNDArrayData & other)
{
   // needed for types to be registrered as meta type.
   //
   this->assignOther (other);
   *this = other;
}

//------------------------------------------------------------------------------
//
QENTNDArrayData::~QENTNDArrayData ()
{
   // needed for types to be registrered as meta type.
}

//------------------------------------------------------------------------------
//
QENTNDArrayData& QENTNDArrayData::operator=(const QENTNDArrayData& other)
{
   this->assignOther (other);
   return *this;
}

//------------------------------------------------------------------------------
//
void QENTNDArrayData::assignOther (const QENTNDArrayData& other)
{
   this->data = other.data;

   this->colourMode = other.colourMode;
   this->dataType = other.dataType;

   this->numberDimensions = other.numberDimensions;

   for (int dsi = 0; dsi < ARRAY_LENGTH (dimensionSizes); dsi++) {
      this->dimensionSizes [dsi] = other.dimensionSizes[dsi];
   }

   this->bytesPerPixel = other.bytesPerPixel;

   this->uniqueId = other.uniqueId;
   this->descriptor = other.descriptor;
}

//------------------------------------------------------------------------------
//
#ifdef QE_INCLUDE_PV_ACCESS

namespace TR1 = std::tr1;
namespace pvd = epics::pvData;


// Format/structure
// epics:nt/NTNDArray:1.0
//     union value
//         boolean[] booleanValue
//         byte[] byteValue
//         short[] shortValue
//         int[] intValue
//         long[] longValue
//         ubyte[] ubyteValue
//         ushort[] ushortValue
//         uint[] uintValue
//         ulong[] ulongValue
//         float[] floatValue
//         double[] doubleValue
//     codec_t codec
//         string name
//         any parameters
//     long compressedSize
//     long uncompressedSize
//     dimension_t[] dimension
//         dimension_t
//             int size
//             int offset
//             int fullSize
//             int binning
//             boolean reverse
//     int uniqueId
//     time_t dataTimeStamp
//         long secondsPastEpoch
//         int nanoseconds
//         int userTag
//     epics:nt/NTAttribute:1.0[] attribute
//         epics:nt/NTAttribute:1.0
//             string name
//             any value
//             string descriptor
//             int sourceType
//             string source
//     string descriptor
//     alarm_t alarm ...
//     time_t timeStamp ...
//     display_t display ...
//
bool QENTNDArrayData::assignFrom (epics::nt::NTNDArrayPtr item)
{
   static bool verbose = true;

   // Local macro function
   //
#define ASSERT(condition, message) {           \
      if (!(condition)) {                      \
         if (verbose) {                        \
            DEBUG << message;                  \
         }                                     \
         return false;                         \
      }                                        \
   }

   ASSERT (item, "Null item");

   // Ensure we have a value.
   //
   const pvd::PVUnionPtr value = item->getValue();
   ASSERT (value.get(), "Null value");

   const pvd::PVStructurePtr codecPtr = item->getCodec();
   ASSERT (codecPtr.get(), "Null codec");

   const pvd::PVStringPtr codecNamePtr = codecPtr->getSubField <pvd::PVString> ("name");
   ASSERT (codecNamePtr.get(), "Null codec name");

   this->clear();

   // Extract the image information, excluding the actual data array.
   // NTNDArrayConverter cribbed from adCore out of Area Detector.
   //
   NTNDArrayInfo_t info;
   try {
      NTNDArrayConverter converter (item);
      info = converter.getInfo();
   } catch (std::exception& e) {
      DEBUG << "exception from NTNDArrayConverter: " << e.what();
      return false;
   }

   // Copy basic meta data
   //
   this->colourMode = NDColorModeImage (info.colorMode);
   this->dataType = NDDataTypeImage (info.dataType);

   this->numberDimensions = info.ndims;
   for (int d = 0; d < info.ndims; d++) {
      this->dimensionSizes [d] = info.dims[d];
   }
   this->bytesPerPixel = info.bytesPerElement;  //

   const pvd::PVIntPtr id = item->getUniqueId();
   this->uniqueId = id->getAs<pvd::int32>();

   const pvd::PVStringPtr desc = item->getDescriptor();
   this->descriptor = QString::fromStdString (desc->getAs <std::string>());

   // Now extract and process the image value data itself
   //
   const pvd::ScalarType scalarType = this->getValueType (value);
   switch (scalarType) {
      case pvd::pvByte:    this->toValue<pvd::PVByteArray>  (value); break;
      case pvd::pvUByte:   this->toValue<pvd::PVUByteArray> (value); break;
      case pvd::pvShort:   this->toValue<pvd::PVShortArray> (value); break;
      case pvd::pvUShort:  this->toValue<pvd::PVUShortArray>(value); break;
      case pvd::pvInt:     this->toValue<pvd::PVIntArray>   (value); break;
      case pvd::pvUInt:    this->toValue<pvd::PVUIntArray>  (value); break;
      case pvd::pvFloat:   this->toValue<pvd::PVFloatArray> (value); break;
      case pvd::pvDouble:  this->toValue<pvd::PVDoubleArray>(value); break;

      case pvd::pvBoolean:
      case pvd::pvLong:
      case pvd::pvULong:
      case pvd::pvString:
      default:
         ASSERT (false, "invalid value data type");
         break;
   }

   // Extract the size meta data.
   //
   pvd::PVLongPtr cds = item->getCompressedDataSize();
   pvd::PVLongPtr uds = item->getUncompressedDataSize();

   Compression compression;
   compression.codecName = QString::fromStdString (info.codec);
   compression.compressedDataSize = cds->getAs<pvd::int64>();
   compression.uncompressedDataSize = uds->getAs<pvd::int64>();

   // Decompress if needs be.
   //
   this->decompressData (compression);

   return true;

#undef ASSERT
}

//------------------------------------------------------------------------------
//
pvd::ScalarType QENTNDArrayData::getValueType (pvd::PVUnionPtr value) const
{
   pvd::ScalarType result = pvd::pvString;  // an unexpected type

   QString fieldName = QString::fromStdString (value->getSelectedFieldName());

   if (!fieldName.isEmpty()) {
      // Format is, e.g. ubyteValue, so drop the "Value" part
      //
      fieldName.chop (5);
      result = pvd::ScalarTypeFunc::getScalarType (fieldName.toStdString());
   }

   return result;
}

//------------------------------------------------------------------------------
// writes to QByteArray data
template <typename arrayType>
void QENTNDArrayData::toValue (pvd::PVUnionPtr value)
{
   typedef typename arrayType::value_type arrayValType;
   typedef typename arrayType::const_svector arrayVecType;

   arrayVecType srcVec (value->get<arrayType>()->view());

   const int length = (int) srcVec.size() * (int) sizeof(arrayValType);
   this->data.clear();
   this->data.append ((const char *)srcVec.data(), length);
}

#endif  // QE_INCLUDE_PV_ACCESS

//------------------------------------------------------------------------------
//
void QENTNDArrayData::clear ()
{
   this->data.clear();
   this->colourMode = NDColorModeImage (NDColorModeMono);
   this->dataType = NDDataTypeImage (NDUInt8);
   this->numberDimensions = 0;
   for (int j = 0; j < ARRAY_LENGTH (this->dimensionSizes); j++) {
      this->dimensionSizes[j] = 0;
   }
   this->bytesPerPixel = 1;
   this->descriptor = "";
   this->uniqueId = 0;
}

//------------------------------------------------------------------------------
//
QByteArray QENTNDArrayData::getData () const
{
   return this->data;
}

//------------------------------------------------------------------------------
//
QString QENTNDArrayData::getColourMode() const
{
    return this->colourMode;
}

//------------------------------------------------------------------------------
//
QString QENTNDArrayData::getDataType() const
{
   return this->dataType;
}

//------------------------------------------------------------------------------
//
int QENTNDArrayData::getNumberDimensions () const
{
   return this->numberDimensions;
}

//------------------------------------------------------------------------------
//
int QENTNDArrayData::getDimensionSize (const int d) const
{
   return ((d >= 0) && d < (this->numberDimensions)) ? this->dimensionSizes [d] : 0;
}

//------------------------------------------------------------------------------
//
int QENTNDArrayData::getBytesPerPixel () const
{
    return this->bytesPerPixel;
}

//------------------------------------------------------------------------------
//
QString QENTNDArrayData::getDescription () const
{
   return this->descriptor;
}

//------------------------------------------------------------------------------
//
int QENTNDArrayData::getUniqueId () const
{
   return this->uniqueId;
}

//------------------------------------------------------------------------------
//
QVariant QENTNDArrayData::toVariant () const
{
   QVariant result;
   result.setValue (*this);
   return result;
}

//------------------------------------------------------------------------------
//
bool QENTNDArrayData::assignFromVariant (const QVariant& item)
{
   bool result = QENTNDArrayData::isAssignableVariant (item);
   if (result) {
      this->clear ();
      *this = item.value < QENTNDArrayData > ();
   }
   return result;
}

//------------------------------------------------------------------------------
//
bool QENTNDArrayData::decompressData (const Compression& compression)
{
   if (compression.codecName == "" || compression.codecName == "none") {
      // Do nothing - already decompressed.
      //
      return true;
   }

#ifdef QE_AD_SUPPORT

   bool result;

   if (compression.codecName == "jpeg") {
      result = this->decompressJpeg(compression);

   } else if (compression.codecName == "blosc") {
      result = this->decompressBlosc (compression);

   } else if (compression.codecName == "lz4") {
      result = this->decompressLz4 (compression);

   } else if (compression.codecName == "bslz4") {
      result = this->decompressBslz4 (compression);

   } else {
      DEBUG << "Codec " + compression.codecName + " not handled/unexpected";
      result = false;
   }

   return result;

#else
   DEBUG << "NTNDArray decompression not supported";
   return false;
#endif
}

//------------------------------------------------------------------------------
// Cribbed from the various decompress functions out of NDPluginCodec.cpp (R3-8)
//
bool QENTNDArrayData::decompressJpeg (const Compression& compression)
{
#ifdef QE_AD_SUPPORT

   if (compression.codecName != "jpeg") {   // sanity check
      DEBUG << "Unexpected codec:" << compression.codecName;
      return false;
   }

   bool result = true;    // hypothesize successful.
   jpeg_decompress_struct jpegInfo;
   jpeg_error_mgr jpegErr;

   jpeg_create_decompress (&jpegInfo);
   jpegInfo.err = jpeg_std_error (&jpegErr);

   // Copy source (by referance).
   //
   const QByteArray input = this->data;
   const unsigned char* inbuffer = (unsigned char*) (input.data());

   QByteArray output = QByteArray (compression.uncompressedDataSize, 0);

   jpeg_mem_src (&jpegInfo, inbuffer, compression.compressedDataSize);

   jpeg_read_header (&jpegInfo, TRUE);
   jpeg_start_decompress (&jpegInfo);

   unsigned char* dest = (unsigned char*)(output.data());

   while (jpegInfo.output_scanline < jpegInfo.output_height) {
      unsigned char* row_pointer[1] = { dest };

      if (jpeg_read_scanlines (&jpegInfo, row_pointer, 1) != 1) {
         DEBUG << "Error decoding JPEG";
         result = false;
         break;
      }

      dest += jpegInfo.output_width*jpegInfo.output_components;
   }

   jpeg_finish_decompress (&jpegInfo);

   // Copy output back to data (by referance).
   //
   this->data = output;

   return result;

#else
   DEBUG << "Jpeg decompression not supported";
   return false;
#endif
}

//------------------------------------------------------------------------------
//
bool QENTNDArrayData::decompressBlosc (const Compression& compression)
{
#ifdef QE_AD_SUPPORT

   if (compression.codecName != "blosc") {   // sanity check
      DEBUG << "Unexpected codec:" << compression.codecName;
      return false;
   }

   bool result = true;    // hypothesize successful.
   int status;

   // Copy source (by referance).
   //
   const QByteArray input = this->data;
   QByteArray output = QByteArray (compression.uncompressedDataSize, 0);

   const size_t destSize = compression.uncompressedDataSize;

   status = blosc_decompress_ctx (input.data(), output.data(), destSize, 1);
   result = (status >= 0);

   // Copy output back to data (by ref with copy-on-write).
   //
   this->data = output;

   return result;

#else
   DEBUG << "Blosc decompression not supported";
   return false;
#endif
}

//------------------------------------------------------------------------------
//
bool QENTNDArrayData::decompressLz4 (const Compression& compression)
{
#ifdef QE_AD_SUPPORT

   if (compression.codecName != "lz4") {   // sanity check
      DEBUG << "Unexpected codec:" << compression.codecName;
      return false;
   }

   bool result = true;    // hypothesize successful.
   int status;

   // Copy source (by referance).
   //
   const QByteArray input = this->data;
   QByteArray output = QByteArray (compression.uncompressedDataSize, 0);

   const int originalSize = compression.uncompressedDataSize;

   status = LZ4_decompress_fast (input.data (), output.data (), originalSize);
   result = (status >= 0);

   // Copy output back to data (by ref with copy-on-write).
   //
   this->data = output;

   return result;

#else
   DEBUG << "Bsloc decompression not supported";
   return false;
#endif
}

//------------------------------------------------------------------------------
//
bool QENTNDArrayData::decompressBslz4 (const Compression& compression)
{
#ifdef QE_AD_SUPPORT

   if (compression.codecName != "bslz4") {   // sanity check
      DEBUG << "Unexpected codec:" << compression.codecName;
      return false;
   }

   bool result = true;    // hypothesize successful.
   int status;

   // Copy source (by referance).
   //
   const QByteArray input = this->data;
   QByteArray output = QByteArray (compression.uncompressedDataSize, 0);

   const size_t numberOfElements = compression.uncompressedDataSize;
   const size_t elementSize = 1;  /// ONLY works for mono 8 bit

   size_t blockSize = 0;
   status = bshuf_decompress_lz4 (input.data(), output.data(),
                                  numberOfElements, elementSize,
                                  blockSize);
   result = (status >= 0);

   // Copy output back to data (by ref with copy-on-write).
   //
   this->data = output;

   return result;

#else
   DEBUG << "Bslz4 decompression not supported";
   return false;
#endif
}

//------------------------------------------------------------------------------
// static
bool QENTNDArrayData::isAssignableVariant (const QVariant & item)
{
   return item.canConvert < QENTNDArrayData > ();
}

//------------------------------------------------------------------------------
//
static bool registerMetaType ()
{
   qRegisterMetaType < QENTNDArrayData > ();
   return true;
}

// Elaborate on start up.
//
static const bool _elaborate = registerMetaType ();

//------------------------------------------------------------------------------
//
QDebug operator<< (QDebug dbg, const QENTNDArrayData& arrayData)
{
   dbg << "(Num Bytes  :"  << arrayData.getData().size() << ",\n";
   dbg << " Colour Mode:" << arrayData.getColourMode() << ",\n";
   dbg << " Data Type  :" << arrayData.getDataType() << ",\n";
   const int nd = arrayData.getNumberDimensions();
   dbg << " Number dimensions: " << nd << ",\n";
   dbg << " Dimensions: ";
   for (int j = 0; j < nd; j++) {
      dbg << arrayData.getDimensionSize (j);
      if (j < nd -1 ) dbg << ", ";
   }
   dbg << "\n)";
   return dbg.maybeSpace ();
}

// end
