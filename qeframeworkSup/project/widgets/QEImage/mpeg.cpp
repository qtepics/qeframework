/*  mpeg.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the Australian Synchrotron.
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
 *  Copyright (c) 2014,2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *    Initial code copied by Andrew Rhyder from parts of ffmpegWidget.cpp
 *    (Author anonymous, part of EPICS area detector ffmpegViwer project)
 *
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
 * This class connects to a MJPEG stream and delivers data to the QEImage widget via a QByteArray
 * containing image data in the same format as data delivered over CA, allowing a user to interact
 * with it in the QEImage widget.
 */

#include "mpeg.h"
#include <QDebug>
#include <QMutex>
#include <colourConversion.h>

// Note: the QE_USE_MPEG macro defintion determinted from the QE_FFMPEG environment
// variable when qmake is run.
//
#ifdef QE_USE_MPEG  // =========================================================

//  When QE_USE_MPEG is in use =>
//
// ffmpeg includes
//
extern "C"{

// Ensure uint64_t is available for all compilers
//
#ifdef __cplusplus
#ifdef _STDINT_H
#undef _STDINT_H
#endif
# include <stdint.h>
#endif

#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}

#define DEBUG qDebug() << "mpeg"  << __LINE__ << __FUNCTION__ << "  "


#define MAXWIDTH  4000    // max width of any input image
#define MAXHEIGHT 3000    // max height of any input image
#define NBUFFERS  20      // number of MAXWIDTH*MAXHEIGHT*3 buffers to create
#define MAXTICKS  10      // number of frames to calc fps from


// set this when the ffmpeg lib is initialised
//
static bool ffinit = false;

// need this to protect certain ffmpeg functions
//
static QMutex* ffmutex = NULL;


//==============================================================================
//
class FFBuffer
{
public:
   FFBuffer ();
   ~FFBuffer ();
   void reserve();
   void release();
   QMutex *mutex;
   unsigned char *mem;
   AVFrame *pFrame;

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(51, 42, 0)
   PixelFormat pix_fmt;
#else
   AVPixelFormat pix_fmt;
#endif
   int width;
   int height;
   int refs;
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

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(56, 26, 100)
   this->pFrame = avcodec_alloc_frame();
#else
   this->pFrame = av_frame_alloc();
#endif

   this->mem = (unsigned char *) calloc(MAXWIDTH*MAXHEIGHT*3, sizeof(unsigned char));
}

//------------------------------------------------------------------------------
//
FFBuffer::~FFBuffer()
{
   av_free(this->pFrame);
   free(this->mem);
}

//------------------------------------------------------------------------------
//
void FFBuffer::reserve()
{
   this->mutex->lock();
   this->refs += 1;
   this->mutex->unlock();
}

//------------------------------------------------------------------------------
//
void FFBuffer::release()
{
   this->mutex->lock();
   this->refs -= 1;
   this->mutex->unlock();
}

// List of FFBuffers to use for raw frames
static FFBuffer rawbuffers[NBUFFERS];


//------------------------------------------------------------------------------
// find a free FFBuffer
// TODO: add array size parameter.
//
static FFBuffer* findFreeBuffer(FFBuffer* source)
{
   for (int i = 0; i < NBUFFERS; i++) {
      // if we can lock it and it has a 0 refcount, we can use it!
      if (source[i].mutex->tryLock()) {
         if (source[i].refs == 0) {
            source[i].refs += 1;
            source[i].mutex->unlock();
//          DEBUG << source[i].id << source[i].refs;
            return &source[i];
         } else {
            source[i].mutex->unlock();
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
   // this is the url to read the stream from
   strcpy(this->url, url.toLatin1().data());

   // set this to true to finish
   this->stopping = false;

   // initialise the ffmpeg library once only
   if (!ffinit) {
      ffinit = true;

      // init mutext
      ffmutex = new QMutex();

      // only display errors
      av_log_set_level(AV_LOG_ERROR);

      // Register all formats and codecs
      av_register_all();

      // Allocate buffer ids.
      for (int i = 0; i < NBUFFERS; i++) {
         rawbuffers[i].id = i;
      }
   }
}

//------------------------------------------------------------------------------
// destroy widget
FFThread::~FFThread() {
}

//------------------------------------------------------------------------------
//
void FFThread::stopGracefully() {
   stopping = true;
}

//------------------------------------------------------------------------------
// run the FFThread
void FFThread::run()
{
   AVFormatContext     *pFormatCtx = NULL;
   int                 videoStream;
   AVCodecContext      *pCodecCtx;
   AVCodec             *pCodec;
   AVPacket            packet;
   int                 frameFinished;

   // Open video file
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 2, 0)
   if (av_open_input_file(&pFormatCtx, this->url, NULL, 0, NULL)!=0) {
#else
   if (avformat_open_input(&pFormatCtx, this->url, NULL, NULL)!=0) {
#endif
      DEBUG << QString( "Opening input '%1' failed" ).arg( url );
      return;
   }

   // Find the first video stream
   videoStream=-1;
   for (unsigned int i=0; i<pFormatCtx->nb_streams; i++) {
      if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO) {
         videoStream=i;
         break;
      }
   }
   if( videoStream==-1) {
      DEBUG << QString( "Finding video stream in '%1' failed" ).arg( url );
      return;
   }

   // Get a pointer to the codec context for the video stream
   pCodecCtx=pFormatCtx->streams[videoStream]->codec;

   // Find the decoder for the video stream
   pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
   if(pCodec==NULL) {
      DEBUG << QString( "Could not find decoder for '%1'" ).arg( url );
      return;
   }

   // Open codec
   ffmutex->lock();
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53, 2, 0)
   if(avcodec_open(pCodecCtx, pCodec)<0) {
#else
   if(avcodec_open2(pCodecCtx, pCodec, NULL)<0) {
#endif
      DEBUG << QString( "Could not open codec for '%1'" ).arg( url );
      return;
   }
   ffmutex->unlock();

   // Read frames into the packets.
   //
   // NOTE, most of this thread's time is spent waiting for the next frame, so the 'stopping' flag
   // is most likely to be set while in av_read_frame(), so it is important that the 'stopping' flag
   // is checked after the call to av_read_frame().
   // The 'stopping' flag is, however, also checked after other reasonably CPU expensive steps such as decoding the frame,
   // or steps that wait on resources such as getting a free buffer,
   //
   // NOTE, this thread is stopped by mpegSource::stopStream(). Refer to that function to see how the 'stopping' flag is used.
   //
   while( true )
   {
      // Get the next frame
      if( av_read_frame(pFormatCtx, &packet) < 0 )
      {
         break;
      }

      // If stopping, free resources and leave
      if( stopping )
      {
         av_free_packet(&packet);
         break;
      }

      // Is this a packet from the video stream?
      if (packet.stream_index!=videoStream) {
         // Free the packet if not
         DEBUG << url << "  Non video packet. Shouldn't see this...";
         av_free_packet(&packet);
         continue;
      }

      // If stopping, free resources and leave
      if( stopping )
      {
         av_free_packet(&packet);
         break;
      }

      // grab a buffer to decode into
      FFBuffer* raw = findFreeBuffer(rawbuffers);
      if (raw == NULL) {
         DEBUG << url << "  Couldn't get a free buffer, skipping packet";
         av_free_packet(&packet);
         continue;
      }

      // If stopping, free resources and leave
      if( stopping )
      {
         av_free_packet(&packet);
         break;
      }

      // Decode video frame
      avcodec_decode_video2(pCodecCtx, raw->pFrame, &frameFinished,
                            &packet);
      if (!frameFinished) {
         DEBUG << url << "  Frame not finished. Shouldn't see this...";
         av_free_packet(&packet);
         raw->release();
         continue;
      }

      // If stopping, free resources and leave
      if( stopping )
      {
         av_free_packet(&packet);
         break;
      }

      // Fill in the output buffer
      raw->pix_fmt = pCodecCtx->pix_fmt;
      raw->height = pCodecCtx->height;
      raw->width = pCodecCtx->width;

      // Emit and free
      emit updateSignal(raw);
      av_free_packet(&packet);

      // If stopping, leave
      if( stopping )
      {
         break;
      }
   }

   // tidy up
   ffmutex->lock();
   avcodec_close(pCodecCtx);

#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(54, 20, 4)
   av_close_input_file(pFormatCtx);
#else
   avformat_close_input(&pFormatCtx);
#endif

   pCodecCtx = NULL;
   pFormatCtx = NULL;
   ffmutex->unlock();
}


//==============================================================================
//
MpegSource::MpegSource (QObject* parent) : QObject (parent)
{
   ff = NULL;
}

//------------------------------------------------------------------------------
//
MpegSource::~MpegSource()
{
   // Ensure the thread is dead
   stopStream();
}

//------------------------------------------------------------------------------
//
QString MpegSource::getURL() const
{
   return url;
}

//------------------------------------------------------------------------------
//
void MpegSource::setURL( const QString& urlIn )
{
   // don't do anything if URL is not changing
   if( urlIn == url )
   {
      return;
   }

   url = urlIn;

   startStream();
}

//------------------------------------------------------------------------------
//
void MpegSource::startStream()
{
   // Stop any previous activity
   stopStream();

   // create the ffmpeg thread
   ff = new FFThread( url, this );

   QObject::connect( ff,   SIGNAL(updateSignal(FFBuffer *)),
                     this, SLOT  (updateImage (FFBuffer *)) );
   QObject::connect( this, SIGNAL(aboutToQuit   ()),
                     ff,   SLOT  (stopGracefully()) );

   ff->start();
}

//------------------------------------------------------------------------------
//
void MpegSource::stopStream()
{
   // Tell the ff thread to stop
   if (ff==NULL) return;
   this->aboutToQuit();
   if (!ff->wait(500)) {
      // thread won't stop, kill it
      ff->terminate();
      ff->wait(100);
   }
   delete ff;
   ff = NULL;
}

//------------------------------------------------------------------------------
//
void MpegSource::updateImage(FFBuffer *newbuf)
{
// DEBUG << newbuf->id << newbuf->refs;

   // Ensure an adequate buffer to hold the image data with no line gaps is allocated.
   // (re)allocate if not present of not the right size
   int buffSize = newbuf->width * newbuf->height * 3;   //!!!??? * 3 for color only
   ba.resize( buffSize );

   // Populate buffer with no line gaps
   // (Each horizontal line of pixels in in a larger horizontal line of storage.
   //  Observed example: each line was 1624 pixels stored in 1664 bytes with
   //  trailing 40 bytes of value 128 before start of pixel on next line)
   char* buffPtr = (char*)ba.data();

   // Set up default image information
   unsigned long dataSize = 1;
   unsigned long depth = 8;
   unsigned long elementsPerPixel = 1;
   imageDataFormats::formatOptions format = imageDataFormats::MONO;

   // older version does not have 'format'
   //widgets/QEImage/mpeg.cpp: In member function 'void mpegSource::updateImage(FFBuffer*)':
   //widgets/QEImage/mpeg.cpp:387: error: 'struct AVFrame' has no member named 'format'

   // Format the data in a CA like QByteArray
   switch( newbuf->pix_fmt )
   {
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(51, 42, 0)
      case PIX_FMT_YUVJ420P:
#else
      case AV_PIX_FMT_YUVJ420P:
#endif
         {
            //!!! Since the QEImage widget handles (or should handle) CA image data
            //!!! in all the formats that are expected in this mpeg stream
            //!!! perhaps this formatting here should be simply packaging the data in
            //!!! a QbyteArray and delivering it, rather than perform any conversion.

            // Set up the image information
            dataSize = 1;
            depth = 8;
            elementsPerPixel = 3;
            format = imageDataFormats::RGB1;

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
            format = imageDataFormats::MONO;

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
   emit setDataImage( ba, dataSize, elementsPerPixel, newbuf->width, newbuf->height, format, depth );

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
