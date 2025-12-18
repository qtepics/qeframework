/*  mpeg.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2014-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#ifndef QE_IMAGE_MPEG_H
#define QE_IMAGE_MPEG_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QWidget>
#include <QByteArray>
#include <QEEnums.h>

#define MAXSTRING 1024    // size of URL string

// differed
class FFBuffer;
class MpegSource;
struct AVFormatContext;
struct AVPacket;
struct AVCodecContext;

//------------------------------------------------------------------------------
// Essentially a private class, but must be declared in header to allow moc to
// do its stuff.
class FFThread : public QThread
{
   Q_OBJECT
public slots:
   void stopGracefully ();

protected:
   void run();

signals:
   void updateSignal (FFBuffer*);

private:
   explicit FFThread (const QString& url, QObject* parent);
   ~FFThread ();

   void processStream (AVFormatContext* pFormatContext);
   void processFrame (AVCodecContext* pCodecContext,
                      AVPacket& packet,
                      int videoStream);

   char url[MAXSTRING];
   bool stopping;

   friend class MpegSource;
};

//------------------------------------------------------------------------------
//
class MpegSource : public QObject
{
   Q_OBJECT
public:
   explicit MpegSource (QObject* parent);
   ~MpegSource();

   QString getURL() const;
   void setURL (const QString& urlIn);
   void stopStream();
   void startStream();

signals:
   void aboutToQuit ();

   // Signal to send image formatted like a CA update
   //
   void setDataImage (const QByteArray& image,
                      unsigned long dataSize,  // element/pixel size
                      unsigned long elements,
                      unsigned long width,
                      unsigned long height,
                      QE::ImageFormatOptions format,
                      unsigned int depth);

private:
   QString url;
   FFThread* ffThread;

   QByteArray imageData;  // Byte array used to deliver images

private slots:
   void updateImage (FFBuffer*);
};

#endif // QE_IMAGE_MPEG_H
