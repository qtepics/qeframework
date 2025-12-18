/*  profilePlot.cpp
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

#include "profilePlot.h"
#include <QClipboard>
#include <QApplication>

//------------------------------------------------------------------------------
// Construction
profilePlot::profilePlot (plotDirections plotDirectionIn) :
   QwtPlot (NULL)
{
   this->data = NULL;

   this->thickness = 1;

   this->plotDirection = plotDirectionIn;

   this->profileAxisEnabled = false;   // should be consistant with QEImageOptionsDialog initial check box state
   this->enableAxis (xBottom, this->profileAxisEnabled);
   this->enableAxis (yLeft, this->profileAxisEnabled);

   this->curve = new QwtPlotCurve();
   this->curve->setRenderHint( QwtPlotItem::RenderAntialiased );
   this->curve->attach(this);

   // Set up context sensitive menu (right click menu)
   //
   this->setContextMenuPolicy (Qt::CustomContextMenu);
   QObject::connect( this, SIGNAL (customContextMenuRequested (const QPoint&)),
                     this, SLOT   (showProfileContextMenu     (const QPoint&)));

   int w = 200; // Initialised to avoid compiler warning
   int h = 100; // Initialised to avoid compiler warning

   switch (this->plotDirection)
   {
      case PROFILEPLOT_LR:
      case PROFILEPLOT_RL:
         w = 200;
         h = 100;
         break;

      case PROFILEPLOT_TB:
      case PROFILEPLOT_BT:
         w = 100;
         h = 200;
         break;
   }

   this->setGeometry( 0, 0, w, h );
   this->setMinimumWidth( w );
   this->setMinimumHeight( h );
}

//------------------------------------------------------------------------------
// Desctuction
profilePlot::~profilePlot()
{
   delete this->curve;
}

//------------------------------------------------------------------------------
//
void profilePlot::enableProfileAxes (const bool enable)
{
   this->profileAxisEnabled = enable;
   this->enableAxis (xBottom, this->profileAxisEnabled);
   this->enableAxis (yLeft, this->profileAxisEnabled);
}

//------------------------------------------------------------------------------
// Set the profile data
void profilePlot::setProfile (QVector<QPointF>* profile,
                              double minX, double maxX,
                              double minY, double maxY, QString titleIn,
                              QPoint startIn, QPoint endIn, unsigned int thicknessIn)
{
   // Save a reference to the data for copying if required
   //
   this->data = profile;
   this->title = titleIn;
   this->start = startIn;
   this->end = endIn;
   this->thickness = thicknessIn;

   // Update the plot
   //
   this->updateProfile (profile, minX, maxX, minY, maxY);
}

//------------------------------------------------------------------------------
// Clear the profile data
void profilePlot::clearProfile()
{
   // Invalidate reference to the data (used for copying)
   this->data = NULL;

   // Update the plot with 'nothing'
   QVector<QPointF> empty;
   this->updateProfile (&empty, 0.0, 1.0, 0.0, 1.0);
}

//------------------------------------------------------------------------------
// Update (set of clear) the profile data
void profilePlot::updateProfile (QVector<QPointF>* profile,
                                 double minX, double maxX,
                                 double minY, double maxY)
{
   // Set the curve data
#if QWT_VERSION >= 0x060000
   this->curve->setSamples( *profile );
#else
   this->curve->setData( *profile );
#endif
   this->setAxisScale (xBottom, minX, maxX);
   this->setAxisScale (yLeft,   minY, maxY);

   // Update the plot
   this->replot();
}

//------------------------------------------------------------------------------
// Show the profile plot context menu.
//
// This method currently populates a imageContextMenu with one 'copy plot data' option.
// Refer to  QEImage::showImageContextMenu() to see how imageContextMenu can be
// populated with checkable, and non checkable items, and sub menus.
//
void profilePlot::showProfileContextMenu( const QPoint& pos )
{
   // Get the overall position on the display
   QPoint globalPos = mapToGlobal( pos );

   imageContextMenu menu;

   //                      Title                            checkable  checked                 option
   menu.addMenuItem(       "Copy Plot Data",                false,     false,                  imageContextMenu::ICM_COPY_PLOT_DATA             );

   // Present the menu
   imageContextMenu::imageContextMenuOptions option;
   bool checked;
   menu.getContextMenuOption( globalPos, &option, &checked );

   // Act on the menu selection
   switch( option )
   {
      default:
      case imageContextMenu::ICM_NONE:
         break;

      case imageContextMenu::ICM_COPY_PLOT_DATA:
         this->copy();
         break;
   }
}

//------------------------------------------------------------------------------
// Copy plot data to clipboard
void profilePlot::copy()
{
   // If no data, do nothing
   if( !this->data )
      return;

   QClipboard *cb = QApplication::clipboard();
   QString text;
   text.append( title ).append( QString( " - Start: %1,%2  End: %3,%4  Thickness: %5\n" ).arg( start.x() ).arg( start.y() ).arg( end.x() ).arg( end.y() ).arg( thickness ) );
   int size = this->data->size();
   switch( this->plotDirection )
   {
      case PROFILEPLOT_LR:
         for (int i = 0; i < size; i++)    text.append( QString( "%1\n" ).arg((*data)[i].y()));
         break;
      case PROFILEPLOT_RL:
         for (int i = size-1; i >= 0; i--) text.append( QString( "%1\n" ).arg((*data)[i].y()));
         break;
      case PROFILEPLOT_TB:
         for (int i = 0; i < size; i++)    text.append( QString( "%1\n" ).arg((*data)[i].x()));
         break;
      case PROFILEPLOT_BT:
         for (int i = size-1; i >= 0; i--) text.append( QString( "%1\n" ).arg((*data)[i].x()));
         break;
   }

   cb->setText( text );
}

// end
