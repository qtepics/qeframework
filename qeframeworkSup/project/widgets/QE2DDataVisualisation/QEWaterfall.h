/*  QEWaterfall.h
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

#ifndef QE_WATERFALL_H
#define QE_WATERFALL_H

#include <QColor>
#include <QHBoxLayout>
#include <QList>
#include <QString>
#include <QWidget>
#include <QEAbstract2DData.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QEAxisPainter.h>

class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEWaterfall :
   public QEAbstract2DData
{
   Q_OBJECT

   typedef QEAbstract2DData ParentWidgetClass;

public:
   /// Offset display angle from vertical in degrees.
   /// Allowed: 0 .. 90
   /// Default: 30
   ///
   Q_PROPERTY (int angle          READ getAngle        WRITE setAngle)

   /// Trace gap/separation, expressed in pixels.
   /// Allowed: 1 .. 40      -- max is arbitrary but sufficient
   /// Default: 5
   ///
   Q_PROPERTY (int traceGap       READ getTraceGap     WRITE setTraceGap)

   /// Pen width - 0 best auto guess.
   /// Allowed: 0 .. 10     -- max is arbitrary but sufficient
   /// Default: 1
   ///
   Q_PROPERTY (int traceWidth     READ getTraceWidth   WRITE setTraceWidth)

   /// Default: white
   ///
   Q_PROPERTY (QColor backgroundColour READ getBackgroundColour  WRITE setBackgroundColour)

   /// Default: dark blue
   ///
   Q_PROPERTY (QColor traceColour READ getTraceColour   WRITE setTraceColour)

   /// When true, the hue of each trace is increased by 12 modulo 360.
   /// Default: false
   ///
   Q_PROPERTY (bool mutableHue    READ getMutableHue   WRITE setMutableHue)

   /// Margin 0 to 40 - default is 4
   /// Margin area shows the alarm colour, provided not inhibited by
   /// the value of the displayAlarmStateOption property.
   ///
   Q_PROPERTY (int margin                READ getMargin                WRITE setMargin)

public:
   /// Create without a variable.
   /// Use setDataPvName () and setVariableNameSubstitutions() to define a
   /// variable and, optionally, macro substitutions later.
   ///
   explicit QEWaterfall (QWidget* parent = 0);

   /// Create with a variable (s).
   /// A connection is automatically established.
   /// If macro substitutions are required, create without a variable and
   /// set the variable and macro substitutions after creation.
   ///
   explicit QEWaterfall (const QString& dataVariableName,
                         QWidget* parent = 0);

   explicit QEWaterfall (const QString& dataVariableName,
                         const QString& widthVariableName,
                         QWidget* parent = 0);

   /// Destruction
   virtual ~QEWaterfall ();

   // Property functions
   //
public slots:
   void setAngle (const int angle);
   void setTraceGap (const int traceGap);
   void setTraceWidth (const int traceWidth);
   void setTraceColour (const QColor& traceColour);
   void setMutableHue (const bool mutableHue);
   void setBackgroundColour (const QColor& traceColour);
   void setMargin  (const int margin);

public:
   int getAngle () const;
   int getTraceGap () const;
   int getTraceWidth () const;
   QColor getTraceColour () const;
   bool getMutableHue () const;
   QColor getBackgroundColour () const;
   int getMargin () const;

protected:
   // Override parent virtual functions.
   //
   QSize sizeHint () const;
   bool eventFilter (QObject* watched, QEvent* event);
   void updateDataVisulation ();   // hook function

private:
   void commonSetup ();
   void waterfallMouseMove (const QPoint& pos);
   void paintWaterfall ();

   // The display area is split into 20 x 40 grid into which each display point
   // is allocated. See PosToSrcMap for more detail.
   //
   enum Constants {
      NumberListRows = 20,
      NumberListCols = 40
   };

   // Provides a mapping from pixel postions to data source.
   // The plot area is divided up into a 20 by 40 grids, and each displayed
   // element is allocated to one, sometimes 2 or 3, lists associated with each
   // grid. This allows for a more rapid search by reducing the number of 
   // items to be considered.
   //
   class PosToSrcMap {
   public:
      explicit PosToSrcMap ();
      explicit PosToSrcMap (const int posX, const int posY,
                            const int dataRow, const int dataCol);
      ~PosToSrcMap ();

      static void clear (QEWaterfall* owner);    // clear the mapArrays

      // Insert (copy) into appropriate mapArrays(s).
      // Can be inserted into more than one, if close to a boundary.
      //
      void insert (QEWaterfall* owner) const;

      // Search the mapArrays for nearest point - if any.
      //
      static bool findNearest (QEWaterfall* owner,
                               const int px, const int py,
                               int& dataRow, int& dataCol);
   private:
      int posX;     // pixel x position
      int posY;     // pixel y position
      int dataRow;
      int dataCol;
   };

   // Provides a 800 way quazi-hash lookup table.
   //
   typedef QList <PosToSrcMap> PosToSrcMapLists;
   PosToSrcMapLists mapArrays [NumberListRows][NumberListCols];

   QWidget* plotArea;      // internal widget on which we paint.
   QHBoxLayout* layout;    // holds the widget - any layout type will do
   QEAxisPainter* xAxis;
   QEAxisPainter* yAxis;

   // Property members
   //
   int mAngle;
   int mTraceGap;
   QColor mTraceColour;
   QColor mBackgroundColour;
   bool mMutableHue;
   int mTraceWidth;
   int mMargin;
};

#endif  // QE_WATERFALL_H
