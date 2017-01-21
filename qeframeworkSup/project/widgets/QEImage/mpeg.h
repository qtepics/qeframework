/*
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
 *  Copyright (c) 2014 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *    Initial code copied by Andrew Rhyder from parts of ffmpegWidget.h (Author anonymous, part of EPICS area detector ffmpegViwer project)
 *
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef MPEG_H
#define MPEG_H

#include <QThread>
#include <QWidget>
#include <QMutex>

/* ffmpeg includes */
extern "C"{

// Ensure uint64_t is available for all compilers
#ifdef __cplusplus
 #ifdef _STDINT_H
  #undef _STDINT_H
 #endif
 # include <stdint.h>
#endif

#include "libavformat/avformat.h"
#include "libavutil/avutil.h"
}


#include "imageDataFormats.h"

// max width of any input image
#define MAXWIDTH 4000
// max height of any input image
#define MAXHEIGHT 3000
// number of MAXWIDTH*MAXHEIGHT*3 buffers to create
#define NBUFFERS 20
// number of frames to calc fps from
#define MAXTICKS 10
// size of URL string
#define MAXSTRING 1024

class FFBuffer
{
public:
    FFBuffer ();
    ~FFBuffer ();
	void reserve();
	void release();
	bool grabFree();
    QMutex *mutex;
    unsigned char *mem;
    AVFrame *pFrame;
    PixelFormat pix_fmt;
    int width;
    int height;
    int refs;
};

class FFThread : public QThread
{
    Q_OBJECT

public:
    FFThread (const QString &url, QObject* parent);
    ~FFThread ();
    void run();

public slots:
    void stopGracefully() { stopping = 1; }

signals:
    void updateSignal(FFBuffer * buf);

private:
    char url[MAXSTRING];
    int stopping;
};

class mpegSource;

class mpegSourceObject : public QObject
{
    Q_OBJECT
public:
    mpegSourceObject( mpegSource* msIn );
    ~mpegSourceObject();

    void sentAboutToQuit();

public slots:
    void updateImage(FFBuffer *buf);

signals:
    void aboutToQuit();

private:
    mpegSource* ms;
};

class mpegSource
{
public:
    mpegSource();
    ~mpegSource();

    void updateImage(FFBuffer *buf);

protected:
    QString getURL();
    void setURL( QString urlIn );
    void stopStream();
    void startStream();

private:
    mpegSourceObject* mso;
    QString url;
    FFThread* ff;
    virtual void setImage( const QByteArray& imageIn,
                           unsigned long dataSize,
                           unsigned long elements,
                           unsigned long width,
                           unsigned long height,
                           imageDataFormats::formatOptions format,
                           unsigned int depth ) = 0;    // Function to consume image formatted like a CA update

    QByteArray ba;  // Byte array used to deliver images

};

#endif // MPEG_H
