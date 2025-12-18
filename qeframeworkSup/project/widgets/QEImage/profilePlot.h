/*  profilePlot.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2012-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Rhyder
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

/*
 This class manages the simple line profile plotting performed within the QEImage widget
 A user can select horizontal or vertical slice at a given point in the image, or an arbitrary
 line through the image and a plot of the pixel values on the line will be shown beside the image.
 */

#ifndef QE_IMAGE_PROFILE_PLOT_H
#define QE_IMAGE_PROFILE_PLOT_H

#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <imageContextMenu.h>

class profilePlot : public QwtPlot
{
   Q_OBJECT
public:
   enum plotDirections {
      PROFILEPLOT_LR,
      PROFILEPLOT_RL,
      PROFILEPLOT_TB,
      PROFILEPLOT_BT
   };

   explicit profilePlot (plotDirections plotDirection);
   ~profilePlot();

   void enableProfileAxes (const bool enable);  // Turn axis on/off.

   void setProfile (QVector<QPointF>* profile,
                    double minX, double maxX,
                    double minY, double maxY,
                    QString title, QPoint start, QPoint end,
                    unsigned int thicknessIn);  // Set the profile data

   void clearProfile();  // Clear the profile data

protected:

private slots:
   void showProfileContextMenu (const QPoint& pos);

private:
   void copy();    // Copy plot data to clipboard
   void updateProfile (QVector<QPointF>* profile,
                       double minX, double maxX,
                       double minY, double maxY);

   bool profileAxisEnabled;
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

#endif // QE_IMAGE_PROFILE_PLOT_H
