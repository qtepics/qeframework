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
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
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
