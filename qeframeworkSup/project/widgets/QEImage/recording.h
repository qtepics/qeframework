/*  recording.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2014-2026 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/*
 This class manages image recording and playback for the QEImage widget
 */

#ifndef QE_IMAGE_RECORDING_H
#define QE_IMAGE_RECORDING_H

#include <QWidget>
#include <QList>
#include <QTimer>

#include <QByteArray>
#include <QCaAlarmInfo.h>
#include <QCaDateTime.h>

// Class used to hold a record of a single image
// Used when building a list of recorded images
class historicImage
{
public:
    historicImage( const QByteArray& image,
                   unsigned long dataSize,
                   const QCaAlarmInfo& alarmInfo,
                   const QCaDateTime& time );
    ~historicImage(){}

    QByteArray image;
    unsigned long dataSize;
    QCaAlarmInfo alarmInfo;
    QCaDateTime time;
};

namespace Ui {
    class recording;
}

class recording;

// Playback timer used to timer replay rate of historic images
class playbackTimer : public QTimer
{
    Q_OBJECT
public:
    playbackTimer( recording* recorderIn ){ recorder = recorderIn; }
    recording* recorder;
    void timerEvent( QTimerEvent * event );
};

// Class used to manage recording for QEImage including playback controls.
class recording : public QWidget
{
    Q_OBJECT

public:
    explicit recording( QWidget *parent = 0 );
    ~recording();

    bool isRecording();             // Determine if in playback or record mode
    void recordImage( const QByteArray& image,
                      unsigned long dataSize,
                      const QCaAlarmInfo& alarmInfo,
                      const QCaDateTime& time );  // Save a new image

    void nextFrameDue();             // Present the next frame due when playing back (public so accessible by playback timer class)

private:
    void reset();                   // Initialise controls
    void startPlaying();            // Start playing back recorded images
    void stopPlaying();             // Stop playback (still in playback mode)
    void showRecordedFrame( int currentFrame );

    playbackTimer* timer;           // Playback timer
    Ui::recording *ui;              // Recording and playback controls
    QList<historicImage> history;   // Saved images

    // Icons
    QIcon* pauseIcon;
    QIcon* playIcon;
    QIcon* recordIcon;
    QIcon* stopIcon;


signals:
  void byteArrayChanged( const QByteArray& value, unsigned long dataSize,
                         const QCaAlarmInfo& alarmInfo, const QCaDateTime& timeStamp,
                         const unsigned int& variableIndex );
  void playingBack( bool playing );

private slots:
    void on_pushButtonPlay_toggled(bool checked);
    void on_pushButtonRecord_toggled(bool checked);
    void on_pushButtonClear_clicked();

    void on_pushButtonLastImage_clicked();
    void on_pushButtonNextImage_clicked();
    void on_pushButtonFirstImage_clicked();
    void on_pushButtonPreviousImage_clicked();
    void on_horizontalSliderPosition_valueChanged(int value);
    void on_radioButtonLive_toggled(bool checked);
};

#endif // QE_IMAGE_RECORDING_H
