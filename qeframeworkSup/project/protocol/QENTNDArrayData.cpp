/*  QENTNDArrayData.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2018-2024 Australian Synchrotron
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

#endif

// Move to QECommon
#ifdef MIN
#undef MIN
#endif

#ifdef MAX
#undef MAX
#endif

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
#ifdef QE_INCLUDE_PV_ACCESS

namespace TR1 = std::tr1;
namespace pvd = epics::pvData;

// based on ntndArrayConverter out of areaDetector.
//
template <typename pvAttrType, typename valueType>
static void toAttribute (QENTNDArrayData::AttributeMaps map, pvd::PVStructurePtr src)
{
    const char* name   = src->getSubField<pvd::PVString>("name")->get().c_str();
//  const char* desc   = src->getSubField<pvd::PVString>("descriptor")->get().c_str();
    valueType value    = src->getSubField<pvd::PVUnion>("value")->get<pvAttrType>()->get();
    map.insert (QString (name), QVariant (value));
}

static void toStringAttribute (QENTNDArrayData::AttributeMaps map, pvd::PVStructurePtr src)
{
    const char* name   = src->getSubField<pvd::PVString>("name")->get().c_str();
//  const char* desc   = src->getSubField<pvd::PVString>("descriptor")->get().c_str();
    const char* value  = src->getSubField<pvd::PVUnion>("value")->get<pvd::PVString>()->get().c_str();
    map.insert (QString (name), QVariant (value));
}


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
bool QENTNDArrayData::assignFrom (epics::nt::NTNDArray::const_shared_pointer item)
{
   static bool verbose = true;

   // Local macro function
   //
#define ASSERT(condition, message) {                                             \
      if (!(condition)) {                                                        \
         if (verbose) {                                                          \
            qDebug() << "QENTImageData" << __LINE__ << __FUNCTION__ << message;  \
         }                                                                       \
         return false;                                                           \
      }                                                                          \
   }

   // Ensure we have a value.
   //
   const pvd::PVUnionPtr value = item->getValue();
   ASSERT (value.get(), "Null value");

   const pvd::PVStructurePtr codecPtr = item->getCodec();
   ASSERT (codecPtr.get(), "Null codec");

   const pvd::PVStringPtr codecNamePtr = codecPtr->getSubField <pvd::PVString> ("name");
   ASSERT (codecNamePtr.get(), "Null codec name");

   this->clear();

   // process dimensions - based on ntndArrayConverter out of areaDetector.
   //
   pvd::PVStructureArray::const_svector dimensions (item->getDimension()->view());

   this->numberDimensions = (int) dimensions.size();
   this->numberElements = 1;
   for (int j = 0; j < this->numberDimensions; j++) {
      this->dimensionSizes [j] = (int) dimensions[j]->getSubField<pvd::PVInt>("size")->get();
      this->numberElements *= this->dimensionSizes [j];
   }

   // process the image value data
   //
   const pvd::ScalarType scalarType = this->getValueType (value);

   int bpe;  // bytesPerElement
   switch(scalarType) {
      case pvd::pvByte:    bpe = sizeof(epicsInt8);    break;
      case pvd::pvUByte:   bpe = sizeof(epicsUInt8);   break;
      case pvd::pvShort:   bpe = sizeof(epicsInt16);   break;
      case pvd::pvUShort:  bpe = sizeof(epicsUInt16);  break;
      case pvd::pvInt:     bpe = sizeof(epicsInt32);   break;
      case pvd::pvUInt:    bpe = sizeof(epicsUInt32);  break;
      case pvd::pvFloat:   bpe = sizeof(epicsFloat32); break;
      case pvd::pvDouble:  bpe = sizeof(epicsFloat64); break;

      case pvd::pvBoolean:
      case pvd::pvLong:
      case pvd::pvULong:
      case pvd::pvString:
      default:
         ASSERT (false, "invalid value data type");
         break;
   }

   this->bytesPerPixel = bpe;
   this->totalBytes = this->numberElements * this->bytesPerPixel;
   this->bitDepth = 8 * this->bytesPerPixel;

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

   // Extract the meta data.
   //
   this->codecName = QString::fromStdString (codecNamePtr->getAs <std::string>());

   pvd::PVLongPtr cds = item->getCompressedDataSize();
   this->compressedDataSize = cds->getAs<pvd::int64>();

   pvd::PVLongPtr uds = item->getUncompressedDataSize();
   this->uncompressedDataSize = uds->getAs<pvd::int64>();

   pvd::PVIntPtr id = item->getUniqueId();
   this->uniqueId = id->getAs<pvd::int32>();

   QEPvaData::TimeStamp dts;
   dts.extract (item->getDataTimeStamp());
   this->dtsSecondsPastEpoch = dts.secondsPastEpoch;
   this->dtsNanoseconds = dts.nanoseconds;
   this->dtsUserTag = dts.userTag;

   pvd::PVStringPtr desc = item->getDescriptor();
   this->descriptor = QString::fromStdString (desc->getAs <std::string>());

   // process attributeList - based on ntndArrayConverter out of areaDetector.
   //
   this->attributeMap.clear();
   pvd::PVStructureArray::const_svector attrVec (item->getAttribute()->view());

   for (VectorIter it = attrVec.cbegin(); it != attrVec.cend(); ++it) {
      pvd::PVScalarPtr srcScalar((*it)->getSubField<pvd::PVUnion>("value")->get<pvd::PVScalar>());

      if (srcScalar) {
         switch (srcScalar->getScalar()->getScalarType()) {
            case pvd::pvByte:   toAttribute<pvd::PVByte,   int8_t>  (this->attributeMap, *it); break;
            case pvd::pvUByte:  toAttribute<pvd::PVUByte,  uint8_t> (this->attributeMap, *it); break;
            case pvd::pvShort:  toAttribute<pvd::PVShort,  int16_t> (this->attributeMap, *it); break;
            case pvd::pvUShort: toAttribute<pvd::PVUShort, uint16_t>(this->attributeMap, *it); break;
            case pvd::pvInt:    toAttribute<pvd::PVInt,    int32_t> (this->attributeMap, *it); break;
            case pvd::pvUInt:   toAttribute<pvd::PVUInt,   uint32_t>(this->attributeMap, *it); break;
            case pvd::pvFloat:  toAttribute<pvd::PVFloat,  float>   (this->attributeMap, *it); break;
            case pvd::pvDouble: toAttribute<pvd::PVDouble, double>  (this->attributeMap, *it); break;
            case pvd::pvString: toStringAttribute                   (this->attributeMap, *it); break;
            case pvd::pvBoolean:
            case pvd::pvLong:
            case pvd::pvULong:
            default:
               break;   // ignore invalid types
         }
      }
   }

   // Do a special for the ColorMode attribute.
   //
   this->format = this->getImageFormat (attrVec);

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


//------------------------------------------------------------------------------
//
QE::ImageFormatOptions QENTNDArrayData::getImageFormat
   (pvd::PVStructureArray::const_svector attrs) const
{
   QE::ImageFormatOptions result = QE::Mono;

   for (VectorIter it (attrs.cbegin()); it != attrs.cend (); ++it) {
       const std::string name = (*it)->getSubField<pvd::PVString>("name")->get();
       if (name == "ColorMode") {
           pvd::PVUnionPtr field ((*it)->getSubField<pvd::PVUnion>("value"));
           int cm = TR1::static_pointer_cast<pvd::PVInt> (field->get())->get();
           if ((cm >= 0) && (cm < QE::numberOfImageFormats)) {
              // Is casting ok - maybe we need a look up table.
              //
              result = QE::ImageFormatOptions (cm);
           }
           break;
       }
   }
   return result;
}

#endif  // QE_INCLUDE_PV_ACCESS

//------------------------------------------------------------------------------
//
void QENTNDArrayData::clear ()
{
   this->isDecompressed = false;
   this->data.clear();
   this->attributeMap.clear();
   this->codecName = "";
   this->format = QE::Mono;
   this->bitDepth = 8;

   this->compressedDataSize = 0;
   this->uncompressedDataSize = 0;
   this->dtsSecondsPastEpoch = 0;
   this->dtsNanoseconds = 0;
   this->dtsUserTag = 0;
   this->uniqueId = 0;
   this->descriptor = "";

   this->numberElements = 0;
   this->numberDimensions = 0;
   for (int j = 0; j < ARRAY_LENGTH (this->dimensionSizes); j++) {
      this->dimensionSizes[j] = 0;
   }
   this->bytesPerPixel = 0;
   this->totalBytes = 0;
}

//------------------------------------------------------------------------------
//
QByteArray QENTNDArrayData::getData () const
{
   return this->data;
}

//------------------------------------------------------------------------------
//
QString QENTNDArrayData::getCodecName () const
{
   return this->codecName;
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
QE::ImageFormatOptions QENTNDArrayData::getFormat() const
{
   return this->format;
}

//------------------------------------------------------------------------------
//
int QENTNDArrayData::getBytesPerPixel () const
{
    return this->bytesPerPixel;
}

//------------------------------------------------------------------------------
//
int QENTNDArrayData::getWidth () const
{
   int result;
   switch (this->numberDimensions) {
      case 2:  result = this->dimensionSizes [0]; break;
      case 3:  result = this->dimensionSizes [1]; break;
      default: result = 0; break;
   }
   return result;
}

//------------------------------------------------------------------------------
//
int QENTNDArrayData::getHeight () const
{
   int result;
   switch (this->numberDimensions) {
      case 2:  result = this->dimensionSizes [1]; break;
      case 3:  result = this->dimensionSizes [2]; break;
      default: result = 0; break;
   }
   return result;
}

//------------------------------------------------------------------------------
//
int QENTNDArrayData::getBitDepth () const
{
   return this->bitDepth;
}

//------------------------------------------------------------------------------
//
QVariant QENTNDArrayData::getAttibute (const QString& name) const
{
   QVariant result (QVariant::Invalid);
   result = this->attributeMap.value(name, result);
   return result;
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
bool QENTNDArrayData::assignFromVariant (const QVariant & item)
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
bool QENTNDArrayData::decompressData ()
{
   if (this->isDecompressed) {
      return true;   // Already decompressed - do nothing
   }

   // Not decompressed yet.
   //
   if (this->codecName == "" || this->codecName == "none") {
      // Do nothing - already decompressed.
      //
      this->isDecompressed = true;
      return true;
   }

#ifdef QE_AD_SUPPORT

   bool result;

   if (this->codecName == "jpeg") {
      result = this->isDecompressed = this->decompressJpeg();

   } else if (this->codecName == "blosc") {
      result = this->isDecompressed = this->decompressBlosc ();

   } else if (this->codecName == "lz4") {
      result = this->isDecompressed = this->decompressLz4 ();

   } else if (this->codecName == "bslz4") {
      result = this->isDecompressed = this->decompressBslz4 ();

   } else {
      DEBUG << "Codec " + this->codecName + " not handled/unexpected";
      result = false;                                                           \
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
bool QENTNDArrayData::decompressJpeg ()
{
#ifdef QE_AD_SUPPORT

   if (this->codecName != "jpeg") {
      DEBUG << "Unexpected codec:" << this->codecName;
      return false;
   }

   bool result = true;    // hypothesize successful.
   jpeg_decompress_struct jpegInfo;
   jpeg_error_mgr jpegErr;

   jpeg_create_decompress (&jpegInfo);
   jpegInfo.err = jpeg_std_error (&jpegErr);

   // Copy source (by referance).
   //
   QByteArray input = this->data;
   unsigned char* inbuffer = (unsigned char*) (input.data());

   QByteArray output = QByteArray (int (this->uncompressedDataSize), 0);

   jpeg_mem_src (&jpegInfo, inbuffer, this->compressedDataSize);

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
bool QENTNDArrayData::decompressBlosc ()
{
#ifdef QE_AD_SUPPORT

   if (this->codecName != "blosc") {
      DEBUG << "Unexpected codec:" << this->codecName;
      return false;
   }

   bool result = true;    // hypothesize successful.
   int status;

   // Copy source (by referance).
   //
   QByteArray input = this->data;
   QByteArray output = QByteArray (int (this->uncompressedDataSize), 0);

   const size_t destSize = this->uncompressedDataSize;

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
bool QENTNDArrayData::decompressLz4 ()
{
#ifdef QE_AD_SUPPORT

   if (this->codecName != "lz4") {
      DEBUG << "Unexpected codec:" << this->codecName;
      return false;
   }

   bool result = true;    // hypothesize successful.
   int status;

   // Copy source (by referance).
   //
   QByteArray input = this->data;
   QByteArray output = QByteArray (int (this->uncompressedDataSize), 0);

   int originalSize = this->uncompressedDataSize;

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
bool QENTNDArrayData::decompressBslz4 ()
{
#ifdef QE_AD_SUPPORT

   if (this->codecName != "bslz4") {
      DEBUG << "Unexpected codec:" << this->codecName;
      return false;
   }

   bool result = true;    // hypothesize successful.
   int status;

   // Copy source (by referance).
   //
   QByteArray input = this->data;
   QByteArray output = QByteArray (int (this->uncompressedDataSize), 0);

   const size_t numberOfElements = this->uncompressedDataSize;
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
// static
bool QENTNDArrayData::registerMetaType ()
{
   qRegisterMetaType < QENTNDArrayData > ();
   return true;
}

// Elaborate on start up.
//
static const bool _elaborate = QENTNDArrayData::registerMetaType ();

//------------------------------------------------------------------------------
//
QDebug operator<< (QDebug dbg, const QENTNDArrayData& arrayData )
{
   dbg << "arrayData:" << "\n";
   const int nd = arrayData.getNumberDimensions();
   dbg << "number dimensions:" << nd << "\n";
   for (int j = 0; j < nd; j++) {
      dbg << "  dimension" << j << ":" << arrayData.getDimensionSize (j) << "\n";
   }
   return dbg.maybeSpace ();
}

// end
