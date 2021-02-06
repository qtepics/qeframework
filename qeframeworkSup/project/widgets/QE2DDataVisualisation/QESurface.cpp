/*  QESurface.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2020-2021 Australian Synchrotron
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
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrews@ansto.gov.au
 */

#include "QESurface.h"
#include <QDebug>
#include <QTimer>

// Check that support for Q3DSurface included
// Please see the framework.pro file, approx line 80.
//
#if QE_USE_DATA_VISUALIZATION

#include <QtDataVisualization/QValue3DAxis>
#include <QtDataVisualization/Q3DSurface>
#include <QtDataVisualization/QSurfaceDataProxy>

#define Q3DSURFACE_INUSE
#define BUTTON_TEXT      "Show 3D Surface"

#else

// Declare dummy classes.
//
namespace QtDataVisualization {
   class Q3DSurface { };
   class QSurfaceDataProxy { };
}

#undef  Q3DSURFACE_INUSE
#define BUTTON_TEXT      "Requires Qt5.7"

#endif


#define DEBUG qDebug () << "QESurface" << __LINE__ << __FUNCTION__ << "  "


//------------------------------------------------------------------------------
//
QESurface:: QESurface (QWidget* parent) :
   QEAbstract2DData (parent)
{
   this->commonSetup();
}

//------------------------------------------------------------------------------
//
QESurface::QESurface (const QString& dataVariableName,
                      QWidget* parent):
   QEAbstract2DData (dataVariableName, parent)
{
   this->commonSetup();
}

//------------------------------------------------------------------------------
//
QESurface::QESurface (const QString& dataVariableName,
                      const QString& widthVariableName,
                      QWidget* parent) :
   QEAbstract2DData (dataVariableName, widthVariableName, parent)
{
   this->commonSetup();
}

//------------------------------------------------------------------------------
//
QESurface::~QESurface ()
{
   if (this->surface) {
      delete this->surface;
   }
}

//------------------------------------------------------------------------------
//
void QESurface::commonSetup ()
{
   this->surface = NULL;
   this->surfaceProxy = NULL;

   // Create internal widget.
   //
   this->button = new QPushButton (BUTTON_TEXT, this);
   this->layout = new QHBoxLayout (this);
   this->layout->setMargin (0);    // extact fit.
   this->layout->addWidget (this->button);

   // Set default property values.
   //
   this->setMinimumWidth  (25);
   this->setMinimumHeight (25);

   // Copy actual widget size policy to the containing widget, then ensure
   // internal widget will expand to fill container widget.
   //
   this->setSizePolicy (this->button->sizePolicy ());
   this->button->setSizePolicy (QSizePolicy::Preferred, QSizePolicy::Preferred);

   // Connections
   //
   QObject::connect (this->button, SIGNAL (clicked (bool)),
                     this,         SLOT   (onClick (bool)));

   // Do not create surface at construction time - this stuffs up when used by
   // designer which create and then deletes a single insyance of each class
   // type (to gather meta data?).
   //
   QTimer::singleShot (10, this, SLOT (initialise()));
}

//------------------------------------------------------------------------------
//
void QESurface::initialise ()
{
#ifdef Q3DSURFACE_INUSE
   using namespace QtDataVisualization;

   this->surface = new Q3DSurface (NULL, NULL);

   Qt::WindowFlags flags = surface->flags();
   flags.setFlag (Qt::FramelessWindowHint, false);
   flags.setFlag (Qt::WindowTitleHint, true);
   flags.setFlag (Qt::WindowMinimizeButtonHint, true);
   flags.setFlag (Qt::WindowMaximizeButtonHint, true);
   flags.setFlag (Qt::WindowCloseButtonHint, true);
   surface->setFlags (flags);

   QValue3DAxis* axis;

   axis = new QValue3DAxis (this);
   axis->setSubSegmentCount(5);
   axis->setLabelFormat("%.0f");
   this->surface->setAxisX (axis);

   axis = new QValue3DAxis (this);
   axis->setSubSegmentCount(5);
   axis->setLabelFormat("%.1f");
   this->surface->setAxisY (axis);

   axis = new QValue3DAxis (this);
   axis->setSubSegmentCount(5);
   axis->setLabelFormat("%.0f");
   this->surface->setAxisZ (axis);

   // https://code.qt.io/cgit/qt/qtdatavis3d.git/tree/examples/
   //   datavisualization/surface/surfacegraph.cpp?h=5.15

   // How do we use this?
   //
   this->surfaceProxy = new QSurfaceDataProxy (this);
#else
   DEBUG << "QESurface required Qt5.7";
#endif
}

//------------------------------------------------------------------------------
//
QSize QESurface::sizeHint () const
{
   return QSize (125, 25);
}

//------------------------------------------------------------------------------
//
void QESurface::updateDataVisulation ()
{
#ifdef Q3DSURFACE_INUSE

   using namespace QtDataVisualization;

   if (!this->surface) return;  // sanity check

   const TwoDimensionalData data = this->getData();
// const int number = data.count ();

   double min = this->getMinimum();
// double max = this->getMaximum();


   // Base class worries about image rotation and flipping.
   // Get displayed number of row and cols.
   //
   int numberCols;
   int numberRows;
   this->getNumberRowsAndCols (numberRows, numberCols);

   // We want the data to approx square, so we adjust the rol or col coordinates
   // accordingly to nearest factor of ten. TODO: figure out how to do properly.
   // Note: 3.162 is approx sqrt (10)
   //
   double rowStretch = 1.0;
   double colStretch = 1.0;

   if (numberCols < numberRows) {
      while (numberCols*colStretch*3.162  <numberRows) {
         colStretch *= 10.0;
      }
   } else {
      while (numberRows*rowStretch*3.162  < numberCols) {
         rowStretch *= 10.0;
      }
   }

   QSurface3DSeries* series = new QSurface3DSeries ();

   // Create the new surface data array.
   //
   QSurfaceDataArray* dataArray = new QSurfaceDataArray ();

   for (int row = 0; row < numberRows; row++) {

      QSurfaceDataRow* dataRow = new QSurfaceDataRow ();
      for (int col = 0; col < numberCols; col++) {
         // Define source data row and col.
         //
//         const int srcRow = numberRows - row - 1;
//         const int srcCol = col;

         double value = this->getValue (row, col, min);

         // NOTE: Mitigate the exact rectangular grid "feature" by slightly skewing x and z.
         //       We plot y = f(x, z)
         //
         float skew = 1.0e-6;
         float x = float (row + skew*col);
         float z = float (col + skew*row);
         float y = float (value);

         QSurfaceDataItem dataItem = QSurfaceDataItem (QVector3D (x*rowStretch, y, z*colStretch));
         *dataRow << dataItem;
      }
      *dataArray << dataRow;
   }

   series->dataProxy()->resetArray (dataArray);

   QLinearGradient gradient;
   gradient.setColorAt (0.0, Qt::black);
   gradient.setColorAt (0.1, Qt::blue);
   gradient.setColorAt (0.3, Qt::green);
   gradient.setColorAt (0.6, Qt::yellow);
   gradient.setColorAt (0.9, Qt::red);
   gradient.setColorAt (1.0, Qt::white);

   series->setBaseGradient (gradient);
   series->setColorStyle (Q3DTheme::ColorStyleRangeGradient);

   // Remove the old series - should only be one.
   // Note, the list is a copy, the content is not.
   //
   QList<QSurface3DSeries *> slist = this->surface->seriesList();
   for (int j = 0; j < slist.count(); j++) {
      QSurface3DSeries* s = slist.value (j, NULL);
      this->surface->removeSeries (s);
      delete s;
   }

   // Lastly add the new series to the surface.
   //
   this->surface->addSeries (series);
#endif
}

//------------------------------------------------------------------------------
//
void QESurface::setText (const QString& textIn)
{
   if (this->button) {
      this->button->setText (textIn);
   }
}

//------------------------------------------------------------------------------
//
QString QESurface::getText () const
{
   QString result;
   if (this->button) {
      result = this->button->text ();
   }
   return result;
}

//------------------------------------------------------------------------------
//
void QESurface::onClick (bool)
{
#ifdef Q3DSURFACE_INUSE
   if (!this->surface) return;  // sanity check
   this->surface->show();
#endif
}

// end
