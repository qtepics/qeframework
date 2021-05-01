/*  QESurface.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2017-2021 Australian Synchrotron.
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

#ifndef QE_SURFACE_H
#define QE_SURFACE_H

#include <QEvent>
#include <QWheelEvent>
#include <QVector>
#include <QString>
#include <QSize>
#include <QColor>
#include <QFrame>
#include <QPoint>
#include <QPointF>
#include <QRect>
#include <QEAbstract2DData.h>
#include <QEFrameworkLibraryGlobal.h>

/// The QESurface class is a non-EPICS aware surface plot widget.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QESurface :
   public QEAbstract2DData
{
   Q_OBJECT
   typedef QEAbstract2DData ParentWidgetClass;

   Q_PROPERTY (bool showGrid                READ getShowGrid      WRITE setShowGrid)
   Q_PROPERTY (Qt::PenStyle gridStyle       READ getGridStyle     WRITE setGridStyle)
   Q_PROPERTY (QColor gridColour            READ getGridColour    WRITE setGridColour)

   Q_PROPERTY (bool showSurface             READ getShowSurface   WRITE setShowSurface)
   Q_PROPERTY (Qt::BrushStyle surfaceStyle  READ getSurfaceStyle  WRITE setSurfaceStyle)

   Q_PROPERTY (QColor axisColour            READ getAxisColour    WRITE setAxisColour)

   Q_PROPERTY (double theta                 READ getTheta         WRITE setTheta)
   Q_PROPERTY (double phi                   READ getPhi           WRITE setPhi)
   Q_PROPERTY (double zoom                  READ getZoom          WRITE setZoom)

   Q_PROPERTY (double xScale                READ getXScale        WRITE setXScale)
   Q_PROPERTY (double yScale                READ getYScale        WRITE setYScale)
   Q_PROPERTY (double zScale                READ getZScale        WRITE setZScale)

   Q_PROPERTY (bool   clampData             READ getClampData     WRITE setClampData)
   Q_PROPERTY (bool   showScaling           READ getShowScaling   WRITE setShowScaling)

   Q_PROPERTY (bool testDataEnabled         READ getTestData      WRITE setTestData)

public:
   enum Markers {
      mkNone = -1,
      mkX = 0,
      mkY,
      mkZ,
      NUMBER_OF_MARKERS   // must be last
   };

   enum Constants {
      maxNumberOfRows = 256,
      maxNumberOfCols = 256,
   };

   // QESurface context menu values
   //
   enum OwnContextMenuOptions {
      QESURF_NONE = A2DDCM_SUB_CLASS_WIDGETS_START_HERE,
      QESURF_SHOW_GRID_FLIP,
      QESURF_SHOW_SURFACE_FLIP,
      QESURF_CLAMP_DATA_FLIP,
      QESURF_SHOW_SCALING_FLIP,
      QESURF_SUB_CLASS_WIDGETS_START_HERE
   };

   /// Construction
   /// Create without a variable.
   /// Use setDataPvName () and setVariableNameSubstitutions() to define a
   /// variable and, optionally, macro substitutions later.
   ///
   explicit QESurface (QWidget* parent = 0);

   /// Create with a variable (s).
   /// A connection is automatically established.
   /// If macro substitutions are required, create without a variable and
   /// set the variable and macro substitutions after creation.
   ///
   explicit QESurface (const QString& dataVariableName,
                       QWidget* parent = 0);

   explicit QESurface (const QString& dataVariableName,
                       const QString& widthVariableName,
                       QWidget* parent = 0);

   /// Destruction
   virtual ~QESurface ();

   void setGridStyle (const Qt::PenStyle gridStyle);
   Qt::PenStyle getGridStyle () const;

   void setGridColour (const QColor& gridColour);
   QColor getGridColour () const;

   void setAxisColour (const QColor& axisColour);
   QColor getAxisColour () const;

   void setSurfaceStyle (const Qt::BrushStyle surfaceStyle);
   Qt::BrushStyle getSurfaceStyle () const;

   void setClampData (const bool clampData);
   bool getClampData () const;

   void setShowScaling (const bool showScaling);
   bool getShowScaling () const;

   void setTestData (const bool testDataEnabled);
   bool getTestData () const;

   bool getShowGrid () const;
   bool getShowSurface () const;
   double getTheta () const;
   double getPhi () const;
   double getZoom () const;
   double getXScale () const;
   double getYScale () const;
   double getZScale () const;

public slots:
   void setShowGrid (const bool showGrid);
   void setShowSurface (const bool showSurface);
   void setTheta (const double theta);
   void setPhi (const double phi);
   void setZoom (const double zoom);
   void setXScale (const double xScale);
   void setYScale (const double yScale);
   void setZScale (const double zScale);

protected:
   QSize sizeHint () const;
   void paintEvent (QPaintEvent* event);
   void wheelEvent (QWheelEvent* event);
   void mousePressEvent (QMouseEvent* event);
   void mouseMoveEvent (QMouseEvent* event);
   void mouseReleaseEvent (QMouseEvent* event);
   void updateDataVisulation ();   // hook function
   QMenu* buildContextMenu ();                        // Build the specific context menu
   void contextMenuTriggered (int selectedItemNum);   // An action was selected from the context menu

private:
   void commonSetup ();
   void paintAxis ();  // Like QEAxisPainter, but any arbitary orientation

   // iteration control functions.
   //
   bool firstPoint (int& row, int& col);
   bool nextPoint  (int& row, int& col);
   double normaliseAngle (const double angle);

   // iteration context values.
   //
   int iterationCount;
   bool iterationRowMajor;
   int iterationRowFirst;    // 0 or nr - 2   (posts and panels)
   int iterationColFirst;    // 0 or nc - 2

   Markers activeMarker;   // none, x, y, z
   QRect   markerBoxes   [NUMBER_OF_MARKERS];
   QPointF markerCorners [NUMBER_OF_MARKERS];
   QPointF coMarker;

   // These three set on mouse down iff over a scale marker.
   //
   QPointF scalePositionA;
   QPointF scalePositionB;
   double referenceDistance;
   double referenceScale;

   bool mouseIsDown;
   QPoint mouseDownPoint;

   // class member variable names associated with a property start with 'm'
   // so as not to clash with the propery names - this is more for qtcreator
   // user's benefit as opposed to the moc and/or C++ compiler.
   //
   double mTheta;
   double mPhi;
   double mZoom;
   double mXScale;
   double mYScale;
   double mZScale;
   QColor mAxisColour;
   QColor mGridColour;
   Qt::PenStyle mGridStyle;
   Qt::BrushStyle mSurfaceStyle;
   bool mShowGrid;
   bool mShowSurface;
   bool mClampData;
   bool mShowScaling;
   bool mTestDataEnabled;

   int numberCols;    // <= maxNumberOfRows
   int numberRows;    // <= maxNumberOfCols
   double  surfaceData [maxNumberOfRows][maxNumberOfCols];
   QPointF transformed [maxNumberOfRows][maxNumberOfCols];
   double  zinfo       [maxNumberOfRows][maxNumberOfCols];

   QPointF transformedCorners [2][2][2];
   double zinfoCorners [2][2][2];

   typedef double Vector [3];
   typedef double Matrix [3][3];

   // C++ does not allow function that returns Matrix/Vector type
   //
   static void mmult (Matrix result, const Matrix a, const Matrix b);   // result = a*b
   static void vmult (Vector result, const Matrix a, const Vector b);   // result = a*b

   // p1 and p2 define a line.
   // p3 is an arbitary point
   // result is point of intersect between line and perpendicular line through p3
   //
   static QPointF calcPerpIntersect (const QPointF p1, const QPointF p2,
                                     const QPointF p3);
};

#endif // QE_SURFACE_H
