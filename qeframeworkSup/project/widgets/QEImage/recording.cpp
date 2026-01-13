/*  recording.cpp
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
 This class manages image recording and playback for the QEImage widget.

 This class emits a signal 'byteArrayChanged' to stream saved image history
 back to the QEImage class. Saved images from this signal are presented
 exactly the same way immages are processed from the QEImage's QCa image
 source or MPEG source.

 This class emits a signal 'playingBack to indicate when this class in in
 playback mode. When in playback mode the QEImage widget ensures it is not
 displaying live images.

 QEImage class can determine if this class is currently recording images by calling isRecording()
 When recording, the QEImage class can deliver new images to record by calling recordImage()

*/

#include "recording.h"
#include "ui_recording.h"

// Construction
recording::recording( QWidget* parent ) :
    QWidget(parent),
    ui(new Ui::recording)
{
    // Present controls
    ui->setupUi(this);

    // Prepare playback timer
    timer = new playbackTimer( this );

    // Create icons
    pauseIcon = new QIcon( ":/qe/image/pause.png" );
    playIcon = new QIcon( ":/qe/image/play.png" );
    recordIcon = new QIcon( ":/qe/image/record.png" );
    stopIcon = new QIcon( ":/qe/image/stop.png" );

    // Initialise
    reset();
}

// Destruction
recording::~recording()
{
    delete ui;

    delete pauseIcon;
    delete playIcon;
    delete recordIcon;
    delete stopIcon;
}

// Set default control values
void recording::reset()
{
    ui->doubleSpinBoxPlaybackRate->setValue( 1.0 );
    ui->doubleSpinBoxPlaybackRate->setMaximum( 10.0 );
    ui->doubleSpinBoxPlaybackRate->setMinimum( 0.02 );
    ui->horizontalSliderPosition->setValue( 0 );
    ui->spinBoxMaxImages->setValue( 20 );
    ui->groupBoxPlayback->setVisible( false );
}

// Return if recording is in progress
// Used by QEImage to stop displaying live images
bool recording::isRecording()
{
    return ui->pushButtonRecord->isChecked();
}

// Record an image.
// Used by QEImage to record a new image.
void recording::recordImage( const QByteArray& image,
                             unsigned long dataSize,
                             const QCaAlarmInfo& alarmInfo,
                             const QCaDateTime& time )
{
    // Determine behaviour
    bool stopAtLimit = ui->radioButtonStopAtLimit->isChecked();
    int limit = ui->spinBoxMaxImages->value();

    // Discard images if limit has been reached and not stopping when limit is reached
    if( history.count() >= limit && !stopAtLimit )
    {
        history.removeFirst();
    }

    // If not at limit, add new image
    if( history.count() < limit )
    {
        history.append( historicImage( image, dataSize, alarmInfo, time ) );

        ui->labelImageCountRecord->setText( QString( "%1" ).arg( history.count() ) );
    }

    // If limit has been reached, and stopping when limit is reached, then stop recording
    if( history.count() >= limit && stopAtLimit )
    {
        ui->pushButtonRecord->setChecked( false );
    }

    // At least one image is present, enable 'clear' and 'playback mode' buttons
    ui->pushButtonClear->setEnabled( true );
    ui->radioButtonPlayback->setEnabled( true );
}

// Start playing back recorded images
void recording::startPlaying()
{
    if( ui->horizontalSliderPosition->value() == ui->horizontalSliderPosition->maximum() )
    {
        ui->horizontalSliderPosition->setValue( 0 );
    }
    timer->start( 0 );

}

// Stop playback (still in playback mode)
void recording::stopPlaying()
{
    ui->pushButtonPlay->setChecked( false );
    timer->stop();
}

// Show a specified frame in the QEImage class
void recording::showRecordedFrame( int currentFrame )
{
    if( currentFrame<0) //check for currentFrame <0 because it could be set to -1 by invalid slider position.
        return;
    // Get and display the frame
    if( currentFrame < history.count() )
    {
        ui->labelImageCountPlayback->setText( QString( "%1/%2" ).arg( currentFrame+1 ).arg( ui->horizontalSliderPosition->maximum()+1 ) );
        historicImage frame = history.at( currentFrame );
        emit byteArrayChanged( frame.image, frame.dataSize, frame.alarmInfo, frame.time, 0 );
    }
}

// ================================================
// Control slots

void recording::on_pushButtonRecord_toggled( bool checked )
{
    // Set icon
    if( checked )
    {
        ui->pushButtonRecord->setIcon( *stopIcon );
    }
    else
    {
        ui->pushButtonRecord->setIcon( *recordIcon );
    }
}

void recording::on_pushButtonPlay_toggled(bool checked)
{
    // Set icon and stop or start playback
    if( checked )
    {
        ui->pushButtonPlay->setIcon( *pauseIcon );
        startPlaying();
    }
    else
    {
        ui->pushButtonPlay->setIcon( *playIcon );
        stopPlaying();
    }
}

void recording::on_pushButtonClear_clicked()
{
    history.clear();
    ui->labelImageCountRecord->setText( "0" );
    ui->radioButtonPlayback->setEnabled( false );
}


void recording::on_pushButtonPreviousImage_clicked()
{
    int currentFrame = ui->horizontalSliderPosition->value();
    if( currentFrame > 0 )
    {
        currentFrame--;
        ui->horizontalSliderPosition->setValue( currentFrame );
        showRecordedFrame( currentFrame );
    }
}

void recording::on_pushButtonFirstImage_clicked()
{
    ui->horizontalSliderPosition->setValue( 0 );
    showRecordedFrame( 0 );
}

void recording::on_pushButtonNextImage_clicked()
{
    int currentFrame = ui->horizontalSliderPosition->value();
    if( currentFrame < ui->horizontalSliderPosition->maximum() )
    {
        currentFrame++;
        ui->horizontalSliderPosition->setValue( currentFrame );
        showRecordedFrame( currentFrame );
    }
}

void recording::on_pushButtonLastImage_clicked()
{
    int currentFrame = ui->horizontalSliderPosition->maximum();
    ui->horizontalSliderPosition->setValue( currentFrame );
    showRecordedFrame( currentFrame );
}

void recording::on_horizontalSliderPosition_valueChanged(int value)
{
    showRecordedFrame( value );
}

void recording::on_radioButtonLive_toggled( bool checked )
{
    // If going to live mode, ensure no longer playing
    if( checked )
    {
        if( ui->pushButtonPlay->isChecked() )
        {
            stopPlaying();
        }
    }

    // If going to playback mode, ensure no longer recording
    else
    {
        if( ui->pushButtonRecord->isChecked() )
        {
            ui->pushButtonRecord->setChecked( false );
        }
        ui->horizontalSliderPosition->setMaximum( history.count()-1 );

        on_pushButtonFirstImage_clicked();
    }

    // Enable appropriate controls (playback or record)
    ui->groupBoxLive->setVisible( ui->radioButtonLive->isChecked() );
    ui->groupBoxPlayback->setVisible( !ui->radioButtonLive->isChecked() );

    // Signal to the QEImage that recorder is in playback or record mode
    emit playingBack( !checked );
}


// ================================================
// Playback timer class
void playbackTimer::timerEvent( QTimerEvent* )
{
    recorder->nextFrameDue();
}
// ================================================

// Present the next frame due when playing back.
// Used by the playback timer class.
void recording::nextFrameDue()
{
    int currentFrame = ui->horizontalSliderPosition->value();
    showRecordedFrame( currentFrame );

    // If done all frames, loop if looping, otherwise stop
    if( ui->horizontalSliderPosition->value() == ui->horizontalSliderPosition->maximum() )
    {
        if( ui->checkBoxLoop->isChecked() )
        {
            ui->horizontalSliderPosition->setValue( 0 );
        }
        else
        {
            stopPlaying();
        }
    }

    // If not done all frames, step on to the next
    else
    {
        ui->horizontalSliderPosition->setValue( currentFrame+1 );
    }

    // Set the due time for the next frame
    timer->setInterval( ui->doubleSpinBoxPlaybackRate->value() * 1000 );
}
