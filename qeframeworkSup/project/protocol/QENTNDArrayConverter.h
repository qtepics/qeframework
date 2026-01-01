/* QENTNDArrayConverter.h
 *
 * Copied and pruned from Area Detector adCore R3-14
 *
 * Mark Rivers
 * University of Chicago
 * May 10, 2008
 *
 */

#ifndef QE_NTND_ARRAY_CONVERTER_H
#define QE_NTND_ARRAY_CONVERTER_H

#include <QString>
#include <QEFrameworkLibraryGlobal.h>
#include <QEPvaCheck.h>

#ifdef QE_INCLUDE_PV_ACCESS
#include <pv/ntndarray.h>
#endif

/// Copied from NDAttribute.h
//
/** Enumeration of NDArray data types */
enum NDDataType_t
{
   NDInt8,     /**< Signed 8-bit integer */
   NDUInt8,    /**< Unsigned 8-bit integer */
   NDInt16,    /**< Signed 16-bit integer */
   NDUInt16,   /**< Unsigned 16-bit integer */
   NDInt32,    /**< Signed 32-bit integer */
   NDUInt32,   /**< Unsigned 32-bit integer */
   NDInt64,    /**< Signed 64-bit integer */
   NDUInt64,   /**< Unsigned 64-bit integer */
   NDFloat32,  /**< 32-bit float */
   NDFloat64   /**< 64-bit float */
};

/// Copied from NDArray.h
//
/** The maximum number of dimensions in an NDArray */
#define ND_ARRAY_MAX_DIMS 10

/** Enumeration of color modes for NDArray attribute "colorMode" */
enum NDColorMode_t
{
   NDColorModeMono,    /**< Monochromatic image */
   NDColorModeBayer,   /**< Bayer pattern image, 1 value per pixel but with color filter on detector */
   NDColorModeRGB1,    /**< RGB image with pixel color interleave, data array is [3, NX, NY] */
   NDColorModeRGB2,    /**< RGB image with row color interleave, data array is [NX, 3, NY]  */
   NDColorModeRGB3,    /**< RGB image with plane color interleave, data array is [NX, NY, 3]  */
   NDColorModeYUV444,  /**< YUV image, 3 bytes encodes 1 RGB pixel */
   NDColorModeYUV422,  /**< YUV image, 4 bytes encodes 2 RGB pixel */
   NDColorModeYUV411   /**< YUV image, 6 bytes encodes 4 RGB pixels */
};

/// Own QE functions.
//
// Returns the same texts as defined in DataType_RBV records.
//
QString NDColorModeImage (const NDColorMode_t dataType);

// Returns the sabe texts as defined in ColorMode_RBV records.
//
QString NDDataTypeImage (const NDDataType_t colorMode);


/// Copied from ntndArrayConverter.h
// Converted to a class.
//
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT NTNDArrayInfo_t {
public:
   NTNDArrayInfo_t ();
   ~NTNDArrayInfo_t ();
   int ndims;
   size_t dims[ND_ARRAY_MAX_DIMS];
   size_t nElements, totalBytes;
   int bytesPerElement;
   NDColorMode_t colorMode;
   NDDataType_t dataType;
   std::string codec;

   struct {
      int dim;
      size_t size, stride;
   } x, y, color;
};

#ifdef QE_INCLUDE_PV_ACCESS

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT NTNDArrayConverter {
public:
   NTNDArrayConverter (const epics::nt::NTNDArrayPtr array);

   NTNDArrayInfo_t getInfo (void) const;

private:
   const epics::nt::NTNDArrayPtr m_array;

   epics::pvData::ScalarType getValueType (void) const;
   NDColorMode_t getColorMode (void) const;
};

#endif

#endif   // QE_NTND_ARRAY_CONVERTER_H
