/*  fullScreenWindow.h
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

/*
  This class is a light wrapper around a QMainwindow to allow resize events to be caught and emitted as a resize signal
 */

#ifndef FULLSCREENWINDOW_H
#define FULLSCREENWINDOW_H

#include <QMainWindow>

class fullScreenWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit fullScreenWindow(QWidget *parent = 0);

protected:
    void resizeEvent( QResizeEvent * event );

signals:
    void fullScreenResize();

public slots:

};

#endif // FULLSCREENWINDOW_H
