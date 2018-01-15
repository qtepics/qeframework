/*  mpeg.h
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
 *    Initial code copied by Andrew Rhyder from parts of ffmpegWidget.h
 *    (Author anonymous, part of EPICS area detector ffmpegViwer project)
 *
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#ifndef QE_IMAGE_MPEG_H
#define QE_IMAGE_MPEG_H

#include <QObject>
#include <QString>
#include <QThread>
#include <QWidget>
#include <QByteArray>
#include "imageDataFormats.h"


#define MAXSTRING 1024    // size of URL string

class FFBuffer;    // differed
class MpegSource;  // differed

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
   void updateSignal (FFBuffer* buf);

private:
   explicit FFThread (const QString& url, QObject* parent);
   ~FFThread ();

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
   MpegSource (QObject* parent);
   ~MpegSource();

   QString getURL() const;
   void setURL( const QString& urlIn );
   void stopStream();
   void startStream();

signals:
   void aboutToQuit ();

   // Signal to send image formatted like a CA update
   void setDataImage( const QByteArray& imageIn,
                      unsigned long dataSize,
                      unsigned long elements,
                      unsigned long width,
                      unsigned long height,
                      imageDataFormats::formatOptions format,
                      unsigned int depth );

private:
   QString url;
   FFThread* ff;

   QByteArray ba;  // Byte array used to deliver images

private slots:
   void updateImage(FFBuffer* buf);
};

#endif // QE_IMAGE_MPEG_H
