/* QENTNDArrayConverter.cpp
 *
 * Copied and pruned from Area Detector: adCore
 * Credit Mark Rivers et al.
 *
 */

#include "QENTNDArrayConverter.h"

#ifdef QE_INCLUDE_PV_ACCESS
using namespace epics::nt;
using namespace epics::pvData;
#endif

#define NUMBER_COLOR_MODES  8
#define NUMBER_DATA_TYPES   10

static const char* ColorModeImages [NUMBER_COLOR_MODES] = {
   "Mono",
   "Bayer",
   "RGB1",
   "RGB2",
   "RGB3",
   "YUV444",
   "YUV422",
   "YUV421"
};

static const char* DataTypeImages [NUMBER_DATA_TYPES] = {
   "Int8",
   "UInt8",
   "Int16",
   "UInt16",
   "Int32",
   "UInt32",
   "Int64",
   "UInt64",
   "Float32",
   "Float64"
};

//------------------------------------------------------------------------------
//
QString NDColorModeImage (const NDColorMode_t colorMode)
{
   if ((colorMode < 0) || (colorMode >= NUMBER_COLOR_MODES)) return "unknown";
   return QString (ColorModeImages [colorMode]);
}

//------------------------------------------------------------------------------
//
QString NDDataTypeImage (const NDDataType_t dataType)
{
   if ((dataType < 0) || (dataType >= NUMBER_DATA_TYPES)) return "unknown";
   return QString (DataTypeImages [dataType]);
}


//==============================================================================
// NTNDArrayInfo_t
//==============================================================================
//
NTNDArrayInfo_t::NTNDArrayInfo_t () : ndims(0) { }

//------------------------------------------------------------------------------
//
NTNDArrayInfo_t::~NTNDArrayInfo_t () { }


#ifdef QE_INCLUDE_PV_ACCESS

//==============================================================================
// NTNDArrayConverter
//==============================================================================
//
NTNDArrayConverter::NTNDArrayConverter (const NTNDArrayPtr array) :
   m_array (array) {}

//------------------------------------------------------------------------------
//
ScalarType NTNDArrayConverter::getValueType (void) const
{
   std::string fieldName(m_array->getValue()->getSelectedFieldName());

   /*
     * Check if union field selected. It happens when the driver is run before
     * the producer. There is a monitor update that is sent on the
     * initialization of a PVRecord with no real data.
     */
   if(fieldName.empty())
      throw std::runtime_error("no union field selected");

   std::string typeName(fieldName.substr(0,fieldName.find("Value")));
   return ScalarTypeFunc::getScalarType(typeName);
}

//------------------------------------------------------------------------------
//
NDColorMode_t NTNDArrayConverter::getColorMode (void) const
{
   NDColorMode_t colorMode = NDColorModeMono;
   PVStructureArray::const_svector attrs(m_array->getAttribute()->view());

   for(PVStructureArray::const_svector::iterator it(attrs.cbegin());
       it != attrs.cend(); ++it)
   {
      PVStringPtr nameFld((*it)->getSubFieldT<PVString>("name"));
      if(nameFld->get() == "ColorMode")
      {
         PVUnionPtr valueUnion((*it)->getSubFieldT<PVUnion>("value"));
         PVScalar::shared_pointer valueFld(valueUnion->get<PVScalar>());
         if(valueFld) {
            int cm = valueFld->getAs<int32>();
            colorMode = (NDColorMode_t) cm;
         } else
            throw std::runtime_error("Error accessing attribute ColorMode");
      }
   }

   return colorMode;
}

//------------------------------------------------------------------------------
//
NTNDArrayInfo_t NTNDArrayConverter::getInfo (void) const
{
   NTNDArrayInfo_t info;

   PVStructureArray::const_svector dims(m_array->getDimension()->view());

   info.ndims     = (int) dims.size();
   info.nElements = 1;

   for(int i = 0; i < info.ndims; ++i)
   {
      info.dims[i]    = (size_t) dims[i]->getSubField<PVInt>("size")->get();
      info.nElements *= info.dims[i];
   }

   PVStructurePtr codec(m_array->getCodec());

   info.codec = codec->getSubField<PVString>("name")->get();

   ScalarType dataType;

   if (info.codec.empty())
      dataType = this->getValueType();
   else {
      // Read uncompressed data type
      PVIntPtr udt(codec->getSubField<PVUnion>("parameters")->get<PVInt>());
      dataType = static_cast<ScalarType>(udt->get());
   }

   NDDataType_t dt;
   int bpe;
   switch(dataType)
   {
      case pvByte:    dt = NDInt8;     bpe = sizeof(epicsInt8);    break;
      case pvUByte:   dt = NDUInt8;    bpe = sizeof(epicsUInt8);   break;
      case pvShort:   dt = NDInt16;    bpe = sizeof(epicsInt16);   break;
      case pvUShort:  dt = NDUInt16;   bpe = sizeof(epicsUInt16);  break;
      case pvInt:     dt = NDInt32;    bpe = sizeof(epicsInt32);   break;
      case pvUInt:    dt = NDUInt32;   bpe = sizeof(epicsUInt32);  break;
      case pvLong:    dt = NDInt64;    bpe = sizeof(epicsInt64);   break;
      case pvULong:   dt = NDUInt64;   bpe = sizeof(epicsUInt64);  break;
      case pvFloat:   dt = NDFloat32;  bpe = sizeof(epicsFloat32); break;
      case pvDouble:  dt = NDFloat64;  bpe = sizeof(epicsFloat64); break;
      case pvBoolean:
      case pvString:
      default:
         throw std::runtime_error("invalid value data type");
         break;
   }

   info.dataType        = dt;
   info.bytesPerElement = bpe;
   info.totalBytes      = info.nElements*info.bytesPerElement;
   info.colorMode       = getColorMode();

   if (info.ndims > 0) {
      info.x.dim    = 0;
      info.x.stride = 1;
      info.x.size   = info.dims[0];
   }

   if (info.ndims > 1) {
      info.y.dim    = 1;
      info.y.stride = 1;
      info.y.size   = info.dims[1];
   }

   if (info.ndims == 3) {
      switch(info.colorMode)
      {
         case NDColorModeRGB1:
            info.x.dim        = 1;
            info.y.dim        = 2;
            info.color.dim    = 0;
            info.x.stride     = info.dims[0];
            info.y.stride     = info.dims[0]*info.dims[1];
            info.color.stride = 1;
            break;

         case NDColorModeRGB2:
            info.x.dim        = 0;
            info.y.dim        = 2;
            info.color.dim    = 1;
            info.x.stride     = 1;
            info.y.stride     = info.dims[0]*info.dims[1];
            info.color.stride = info.dims[0];
            break;

         case NDColorModeRGB3:
            info.x.dim        = 1;
            info.y.dim        = 2;
            info.color.dim    = 0;
            info.x.stride     = info.dims[0];
            info.y.stride     = info.dims[0]*info.dims[1];
            info.color.stride = 1;
            break;

         default:
            info.x.dim        = 0;
            info.y.dim        = 1;
            info.color.dim    = 2;
            info.x.stride     = 1;
            info.y.stride     = info.dims[0];
            info.color.stride = info.dims[0]*info.dims[1];
            break;
      }

      info.x.size     = info.dims[info.x.dim];
      info.y.size     = info.dims[info.y.dim];
      info.color.size = info.dims[info.color.dim];
   }

   return info;
}

#endif

// end
