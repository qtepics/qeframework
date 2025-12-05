/*  mpeg.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2014-2025 Australian Synchrotron
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation, version 3.
 *
 *  The EPICS QT Framework is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Author:     Andrew Rhyder
 *    Initial code copied by Andrew Rhyder from parts of ffmpegWidget.h
 *    (Author anonymous, part of EPICS area detector ffmpegViwer project)
 *
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/*
 * This class connects to a MJPEG stream and delivers data to the QEImage widget
 * via a QByteArray containing image data in a similar format as data delivered
 * over CA, allowing a user to interact with it in the QEImage widget.
 */

#include "mpeg.h"

#include <QDebug>
#include <QMutex>
#include <QMutexLocker>
#include <colourConversion.h>
#include <QEEnums.h>

#define DEBUG qDebug() << "mpeg"  << __LINE__ << __FUNCTION__ << "  "

// Note: the QE_USE_MPEG macro defintion determined from the QE_FFMPEG environment
// variable when qmake is run.
//
#ifdef QE_USE_MPEG  // =========================================================

//  When QE_USE_MPEG is in use =>
//
// ffmpeg includes
//
extern "C" {

// Ensure uint64_t is available for all compilers
//
#ifdef __cplusplus
#ifdef _STDINT_H
#undef _STDINT_H
#endif
# include <stdint.h>
#endif

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
}

#define NBUFFERS  40    // number of buffer objects to create

// set this when the ffmpeg lib is initialised
//
static bool ffinit = false;

// need this to protect certain ffmpeg functions
//
static QMutex* ffmpegCodecMutex = new QMutex();

//==============================================================================
//
class FFBuffer
{
public:
   FFBuffer ();
   ~FFBuffer ();

   static FFBuffer* findFreeBuffer ();
   void release();

   QMutex *mutex;
   AVFrame* pFrame;
   AVPixelFormat pix_fmt;
   int width;
   int height;
   int refs;  // used as inUse bool
   int id;    // for debug messages
};


//------------------------------------------------------------------------------
// An FFBuffer contains an AVFrame, a mutex for access and some data
//
FFBuffer::FFBuffer()
{
   this->mutex = new QMutex();
   this->refs = 0;
   this->id = -1;

   this->pFrame = av_frame_alloc ();
}

//------------------------------------------------------------------------------
//
FFBuffer::~FFBuffer()
{
   av_frame_free(&this->pFrame);
}

//------------------------------------------------------------------------------
//
void FFBuffer::release()
{
   QMutexLocker locker (this->mutex);
   this->refs -= 1;
}

// List of FFBuffers to use for raw frames
static FFBuffer rawbuffers[NBUFFERS];

//------------------------------------------------------------------------------
// find a free FFBuffer
// TODO: add array size parameter.
// static
FFBuffer* FFBuffer::findFreeBuffer ()
{
   for (int i = 0; i < NBUFFERS; i++) {
      // if we can lock it and it has a 0 refcount, we can use it!
      if (rawbuffers[i].mutex->tryLock()) {
         if (rawbuffers[i].refs == 0) {
            rawbuffers[i].refs += 1;
            rawbuffers[i].mutex->unlock();
            return &rawbuffers[i];
         } else {
            rawbuffers[i].mutex->unlock();
         }
      }
   }
   return NULL;
}

//==============================================================================
// thread that decodes frames from video stream and emits updateSignal when
// each new frame is available
//
// TODO - update to match suggested Qt thread paradigm
//
FFThread::FFThread (const QString &url, QObject* parent)
   : QThread (parent)
{
   // this is the url to read the stream from.
   // copy using snprintf, this ensures a/ not too big and b/ ends with null char.
   //
   snprintf (this->url, sizeof (this->url), "%s", url.toLatin1().data());

   // set this to true to finish.
   this->stopping = false;

   // initialise the ffmpeg library once only.
   if (!ffinit) {
      ffinit = true;

      // only display errors.
      //
      av_log_set_level (AV_LOG_ERROR);

      // Allocate buffer ids - diagnostic only.
      //
      for (int i = 0; i < NBUFFERS; i++) {
         rawbuffers[i].id = i;
      }
   }
}

//------------------------------------------------------------------------------
// destroy widget
FFThread::~FFThread() { }

//------------------------------------------------------------------------------
//
void FFThread::stopGracefully()
{
   this->stopping = true;
}

//------------------------------------------------------------------------------
// run the FFThread
void FFThread::run()
{
   AVFormatContext* pFormatContext = NULL;

   // Open video file
   //
   int status = -1;
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 2, 0)
   status = av_open_input_file (&pFormatCtx, this->url, NULL, 0, NULL)
#else
   status = avformat_open_input (&pFormatContext, this->url, NULL, NULL);
#endif

   if (status == 0) {
      this->processStream (pFormatContext);

      // And close file.
      //
      avformat_close_input (&pFormatContext);

   } else {
      DEBUG << QString ("Opening input '%1' failed: %2").arg (url).arg (status);
   }

   // ends thread
}

//------------------------------------------------------------------------------
//
void FFThread::processStream (AVFormatContext* pFormatContext)
{
   int status;
   AVCodecParameters* pCodecParameters = NULL;
   AVCodecContext*    pCodecContext = NULL;
   const AVCodec*     pCodec = NULL;

   // Find the first video stream
   //
   int videoStream = -1;
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(59, 0, 0)
   for (unsigned int i = 0; i < pFormatContext->nb_streams; i++) {
      if (pFormatContext->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
         videoStream = i;
         break;
      }
   }   
#else
   videoStream = av_find_best_stream (pFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &pCodec, 0);
#endif   

   if (videoStream==-1) {
      DEBUG << QString( "Finding video stream in '%1' failed" ).arg( url );
      return;
   }

   // Get a pointer to the codec context for the video stream.
   //
   pCodecParameters = pFormatContext->streams[videoStream]->codecpar;

   // Find the decoder for the video stream
   pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
   if (pCodec == NULL) {
      DEBUG << QString( "Could not find decoder for '%1'" ).arg( url );
      return;
   }

   pCodecContext = avcodec_alloc_context3 (pCodec);
   if (!pCodecContext) {
      DEBUG << QString( "Allocate an AVCodecContext '%1' failed" ).arg( url );
      return;
   }

   status = avcodec_parameters_to_context (pCodecContext, pCodecParameters);
   if (status != 0) {
      DEBUG << QString( "avcodec_parameters_to_context '%1' failed" ).arg( url );
      return;
   }

   // Open codec
   {
      QMutexLocker locker (ffmpegCodecMutex);
      status = avcodec_open2 (pCodecContext, pCodec, NULL);
      if (status < 0) {
         DEBUG << QString( "Could not open codec %1 for '%2'" ).arg (status).arg( url );
         return;
      }
   }

   // Read frames into the packets.
   //
   // NOTE, most of this thread's time is spent waiting for the next frame, so the 'stopping' flag
   // is most likely to be set while in av_read_frame(), so it is important that the 'stopping' flag
   // is checked after the call to av_read_frame().
   // The 'stopping' flag is, however, also checked after other reasonably CPU expensive steps such as decoding the frame,
   // or steps that wait on resources such as getting a free buffer,
   //
   // NOTE, this thread is stopped by mpegSource::stopStream().
   // Refer to that function to see how the 'stopping' flag is used.
   //
   while (!this->stopping) {

      // Get the next frame.
      //
      AVPacket packet;

      status = av_read_frame (pFormatContext, &packet);
      if (status >= 0) {
         this->processFrame (pCodecContext, packet, videoStream);
         av_packet_unref (&packet);

      } else {
         DEBUG << "av_read_frame failed" << status;
         break;
      }
   }

   // tidy up
   {
      QMutexLocker locker (ffmpegCodecMutex);
      avcodec_close (pCodecContext);
   }
}

//------------------------------------------------------------------------------
//
void FFThread::processFrame (AVCodecContext* pCodecContext,
                             AVPacket& packet,
                             int videoStream)
{
   // If stopping, free resources and leave.
   //
   if (this->stopping) {
      return;
   }

   // Is this a packet from the video stream?
   //
   if (packet.stream_index != videoStream) {
      // Free the packet if not
      DEBUG << url << "  Non video packet. Shouldn't see this...";
      return;
   }

   // If stopping, free resources and leave.
   //
   if (this->stopping) {
      return;
   }

   // grab a buffer to decode into.
   //
   FFBuffer* raw = FFBuffer::findFreeBuffer ();
   if (raw == NULL) {
      DEBUG << url << "  Couldn't get a free buffer, skipping packet";
      return;
   }

   // If stopping, free resources and leave.
   //
   if (this->stopping) {
      return;
   }

   // Decode video frame.
   //
   int status = avcodec_send_packet (pCodecContext, &packet);
   if (status < 0) {
      DEBUG << url << "  avcodec_send_packet:" << status << ". Shouldn't see this...";
      raw->release();
      return;
   }

   status = avcodec_receive_frame (pCodecContext, raw->pFrame);
   if (status < 0) {
      DEBUG << url << "  Frame not finished:" << status << ". Shouldn't see this...";
      raw->release();
      return;
   }

   // If stopping, free resources and leave.
   //
   if (this->stopping) {
      return;
   }

   // Fill in the output buffer.
   //
   raw->pix_fmt = pCodecContext->pix_fmt;
   raw->height = pCodecContext->height;
   raw->width = pCodecContext->width;

   // Emit. The d=raw buffer is freed by updateImage out of MpegSource,
   //
   emit updateSignal (raw);
}

//==============================================================================
//
MpegSource::MpegSource (QObject* parent) : QObject (parent)
{
   this->ffThread = NULL;
}

//------------------------------------------------------------------------------
//
MpegSource::~MpegSource()
{
   // Ensure the thread is dead
   this->stopStream();
}

//------------------------------------------------------------------------------
//
QString MpegSource::getURL() const
{
   return this->url;
}

//------------------------------------------------------------------------------
//
void MpegSource::setURL( const QString& urlIn )
{
   // don't do anything if URL is not changing
   if (urlIn == this->url) {
      return;
   }
   DEBUG << urlIn;
   this->url = urlIn;
   this->startStream();
}

//------------------------------------------------------------------------------
//
void MpegSource::startStream()
{
   // Stop any previous activity
   this->stopStream();

   // create the ffmpeg thread
   this->ffThread = new FFThread (this->url, this);

   QObject::connect (this->ffThread, SIGNAL(updateSignal(FFBuffer *)),
                     this,     SLOT  (updateImage (FFBuffer *)));
   QObject::connect (this,     SIGNAL(aboutToQuit   ()),
                     this->ffThread, SLOT  (stopGracefully()));

   this->ffThread->start();
}

//------------------------------------------------------------------------------
//
void MpegSource::stopStream()
{
   // Tell the ff thread to stop
   if (this->ffThread == NULL) return;
   this->aboutToQuit();
   if (!this->ffThread->wait(500)) {
      // thread won't stop, kill it
      this->ffThread->terminate();
      this->ffThread->wait(100);
   }
   delete this->ffThread;
   this->ffThread = NULL;
}

//------------------------------------------------------------------------------
// slot [updateSignal  from the FFThread]
//
void MpegSource::updateImage (FFBuffer* newbuf) {
//  DEBUG << newbuf->id << newbuf->refs;

   // Ensure an adequate buffer to hold the image data with no line gaps is allocated.
   // (re)allocate if not present of not the right size.
   //
   int buffSize = newbuf->width * newbuf->height * 3;   //!!!??? * 3 for color only
   imageData.resize( buffSize );

   // Populate buffer with no line gaps
   // (Each horizontal line of pixels in in a larger horizontal line of storage.
   //  Observed example: each line was 1624 pixels stored in 1664 bytes with
   //  trailing 40 bytes of value 128 before start of pixel on next line).
   //
   char* buffPtr = (char*)imageData.data();

   // Set up default image information
   unsigned long dataSize = 1;
   unsigned long depth = 8;
   unsigned long elementsPerPixel = 1;
   QE::ImageFormatOptions format = QE::Mono;

   // older version does not have 'format'
   //widgets/QEImage/mpeg.cpp: In member function 'void mpegSource::updateImage(FFBuffer*)':
   //widgets/QEImage/mpeg.cpp:387: error: 'struct AVFrame' has no member named 'format'

   // Format the data in a CA like QByteArray.
   //
   switch( newbuf->pix_fmt )
   {
      case AV_PIX_FMT_YUVJ420P:
         {
            //!!! Since the QEImage widget handles (or should handle) CA image data
            //!!! in all the formats that are expected in this mpeg stream
            //!!! perhaps this formatting here should be simply packaging the data in
            //!!! a QbyteArray and delivering it, rather than perform any conversion.

            // Set up the image information
            dataSize = 1;
            depth = 8;
            elementsPerPixel = 3;
            format = QE::rgb1;

            const unsigned char* linePtrY = (const unsigned char*)(newbuf->pFrame->data[0]);
            const unsigned char* linePtrU = (const unsigned char*)(newbuf->pFrame->data[1]);
            const unsigned char* linePtrV = (const unsigned char*)(newbuf->pFrame->data[2]);

            // For each row...
            //            qint64 start = QDateTime::currentMSecsSinceEpoch();
            for( int i = 0; i < newbuf->height; i++ )
            {
               // For each pixel...
               for( int j = 0; j < newbuf->width; j++ )
               {
                  unsigned char y,u,v;
                  unsigned char r,g,b;

                  // Use U and V values for every pair of pixels
                  int uv = j/2;

                  // Get YUV values
                  y = linePtrY[j];
                  u = linePtrU[uv];
                  v = linePtrV[uv];

                  // Convert to RGB
                  r = YUVJ2R(y, u, v);
                  g = YUVJ2G(y, u, v);
                  b = YUVJ2B(y, u, v);

                  // Save RGB result
                  *buffPtr++ = r;
                  *buffPtr++ = g;
                  *buffPtr++ = b;
               }

               // Step on to new Y data for every line
               linePtrY += newbuf->pFrame->linesize[0];

               // Step onto new U and V data every two lines
               if( i & 1 )
               {
                  linePtrU += newbuf->pFrame->linesize[1];
                  linePtrV += newbuf->pFrame->linesize[2];
               }
            }
            //            qint64 end = QDateTime::currentMSecsSinceEpoch();
            //            DEBUG <<"decode mS:" << end-start;
         }
         break;

      default:
         {
            // Set up the image information
            dataSize = 1;
            depth = 8;
            elementsPerPixel = 1;
            format = QE::Mono;

            // Package the data in a CA like QByteArray
            const char* linePtr = (const char*)(newbuf->pFrame->data[0]);
            for( int i = 0; i < newbuf->height; i++ )
            {
               memcpy( buffPtr, linePtr, newbuf->width );
               buffPtr += newbuf->width;
               linePtr += newbuf->pFrame->linesize[0];
            }
         }
         break;
   }

   // Deliver image update
   emit setDataImage (this->imageData, dataSize, elementsPerPixel,
                      newbuf->width, newbuf->height, format, depth);

   // Unlock buffer
   newbuf->release();
// DEBUG << newbuf->id << newbuf->refs;
}

#else  // ======================================================================

// When QE_USE_MPEG not in use =>
// Create stubb class functions.
//
FFThread ::FFThread (const QString &, QObject* parent) : QThread (parent) {}

FFThread::~FFThread () {}

void FFThread::run () {}

void FFThread::stopGracefully () {}

void processStream (AVFormatContext*) {}

void processFrame (AVCodecContext*, AVPacket&, int) {}


//------------------------------------------------------------------------------
//
MpegSource::MpegSource (QObject* parent) : QObject (parent) {}

MpegSource::~MpegSource () {}

QString MpegSource::getURL () const
{
   return "---MPEG not enabled in this build---";
}

void MpegSource::setURL (const QString&) {}

void MpegSource::stopStream () {}

void MpegSource::startStream () {}

void MpegSource::updateImage (FFBuffer*) {}

#endif // QE_USE_MPEG  =========================================================

// end
