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
 *  Copyright (c) 2012 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

/*
 This class manages the simple line profile plotting performed within the QEImage widget
 A user can select horizontal or vertical slice at a given point in the image, or an arbitrary
 line through the image and a plot of the pixel values on the line will be shown beside the image.
 */

#ifndef PROFILEPLOT_H
#define PROFILEPLOT_H

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <imageContextMenu.h>

class profilePlot : public QwtPlot
{
    Q_OBJECT
public:

    enum plotDirections{ PROFILEPLOT_LR, PROFILEPLOT_RL, PROFILEPLOT_TB, PROFILEPLOT_BT };
    profilePlot( plotDirections plotDirectionIn );
    ~profilePlot();

    void setProfile( QVector<QPointF>* profile, double minX, double maxX, double minY, double maxY, QString title, QPoint start, QPoint end, unsigned int thicknessIn );  // Set the profile data
    void clearProfile();  // Clear the profile data

protected:

private slots:
    void showProfileContextMenu( const QPoint& pos );

private:
    void copy();    // Copy plot data to clipboard
    void updateProfile( QVector<QPointF>* profile, double minX, double maxX, double minY, double maxY );

    int scale;
    int cursor;

     plotDirections plotDirection;

    QwtPlotCurve* curve;
    QVector<QPointF>* data; // Pointer to original data (NULL if not set up)
    QString title;
    QPoint start;
    QPoint end;
    unsigned int thickness;

};

#endif // PROFILEPLOT_H
