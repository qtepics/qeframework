/*  fullScreenWindow.cpp
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

#include "fullScreenWindow.h"

fullScreenWindow::fullScreenWindow(QWidget *parent) :
    QMainWindow(parent,Qt::Window)
{
}

// The mouse has been pressed over the image
void fullScreenWindow::resizeEvent( QResizeEvent* /*event*/ )
{
    emit fullScreenResize();
}
