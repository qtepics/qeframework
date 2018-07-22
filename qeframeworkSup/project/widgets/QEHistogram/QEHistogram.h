/*  QEHistogram.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2014-2018 Australian Synchrotron
 *
 *  The EPICS QT Framework is free software: you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public License as published
 *  by the Free Software Foundation, either version 3 of the License, or
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
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_HISTOGRAM_H
#define QE_HISTOGRAM_H

#include <QColor>
#include <QEvent>
#include <QFrame>
#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QScrollBar>
#include <QBoxLayout>
#include <QWidget>
#include <QEAxisPainter.h>

#include <QEFrameworkLibraryGlobal.h>

/// The QEHistogram class is a non-EPICS aware histogram widget.
/// The value of, i.e. the length of each bar, and colour may be set indepedently.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEHistogram : public QFrame {
   Q_OBJECT

public:
   Q_PROPERTY (bool   autoBarGapWidths READ getAutoBarGapWidths WRITE setAutoBarGapWidths)
   Q_PROPERTY (int    barWidth         READ getBarWidth         WRITE setBarWidth)
   Q_PROPERTY (int    gap              READ getGap              WRITE setGap)
   Q_PROPERTY (int    margin           READ getMargin           WRITE setMargin)
   Q_PROPERTY (bool   autoScale        READ getAutoScale        WRITE setAutoScale)
   Q_PROPERTY (double minimum          READ getMinimum          WRITE setMinimum)
   Q_PROPERTY (double maximum          READ getMaximum          WRITE setMaximum)
   Q_PROPERTY (double baseLine         READ getBaseLine         WRITE setBaseLine)
   Q_PROPERTY (bool   drawAxies        READ getDrawAxies        WRITE setDrawAxies)
   Q_PROPERTY (bool   showScale        READ getShowScale        WRITE setShowScale)
   Q_PROPERTY (int    precision        READ getPrecision        WRITE setPrecision)
   /// This is the value grid selection
   Q_PROPERTY (bool   showGrid         READ getShowGrid         WRITE setShowGrid)
   Q_PROPERTY (bool   logScale         READ getLogScale         WRITE setLogScale)
   /// Where possible I spell colour properly.
   Q_PROPERTY (QColor backgroundColour READ getBackgroundColour WRITE setBackgroundColour)
   Q_PROPERTY (QColor secondBgColour   READ getSecondBgColour   WRITE setSecondBgColour)
   Q_PROPERTY (int    secondBgSize     READ getSecondBgSize     WRITE setSecondBgSize)
   Q_PROPERTY (bool   showSecondBg     READ getShowSecondBg     WRITE setShowSecondBg)
   Q_PROPERTY (QColor barColour        READ getBarColour        WRITE setBarColour)
   Q_PROPERTY (bool   drawBorder       READ getDrawBorder       WRITE setDrawBorder)

   /// orientation horizontal (default) or vertical. Horizontal means each element
   /// displayed horzontally from left to right with the bar represting the value increasing
   /// vertically from bottom to top.
   Q_PROPERTY (Qt::Orientation orientation READ getOrientation  WRITE setOrientation)

   // Test - used for previewing/testing - may be removed.
   //
   Q_PROPERTY (int    testSize         READ getTestSize         WRITE setTestSize)

public:
   typedef QVector <double> DataArray;

   // Constructor
   //
   explicit QEHistogram (QWidget* parent = 0);
   virtual ~QEHistogram () {}

   virtual QSize sizeHint () const;

   // Property functions
   // Standard propery access macro.
   //
#define PROPERTY_ACCESS(type, name)        \
   void set##name (const type value);      \
   type get##name () const;


   PROPERTY_ACCESS (int,    BarWidth)
   PROPERTY_ACCESS (int,    Gap)
   PROPERTY_ACCESS (int,    Margin)
   PROPERTY_ACCESS (double, Minimum)
   PROPERTY_ACCESS (double, Maximum)
   PROPERTY_ACCESS (double, BaseLine)
   PROPERTY_ACCESS (bool,   AutoScale)
   PROPERTY_ACCESS (bool,   AutoBarGapWidths)
   PROPERTY_ACCESS (bool,   ShowScale)
   PROPERTY_ACCESS (int,    Precision)
   PROPERTY_ACCESS (bool,   ShowGrid)
   PROPERTY_ACCESS (bool,   LogScale)
   PROPERTY_ACCESS (bool,   DrawAxies)
   PROPERTY_ACCESS (bool,   DrawBorder)
   PROPERTY_ACCESS (QColor, BackgroundColour)
   PROPERTY_ACCESS (QColor, SecondBgColour)
   PROPERTY_ACCESS (int,    SecondBgSize)
   PROPERTY_ACCESS (bool,   ShowSecondBg)
   PROPERTY_ACCESS (QColor, BarColour)
   PROPERTY_ACCESS (Qt::Orientation, Orientation)
   //
   PROPERTY_ACCESS (int,    TestSize)
#undef PROPERTY_ACCESS

   int count () const;

   // Returns associated data index of specified position, or -1.
   // data position includes max value/full draw area, not just currently
   // occupied draw area.
   //
   int indexOfPosition (const int x, const int y) const;
   int indexOfPosition (const QPoint& p) const;  // overloaded function

   // Return location of index'th element with respect to the QEHistogram widget,
   // and w.r.t. the internal histogramArea widget. The function takes account
   // first displayed offset.
   //
   QRect positionOfIndex (const int index) const;

   void clearValue (const int index);
   void clearColour (const int index);
   void clear ();   // clear all data

   double value (const int index) const;
   DataArray values () const;

signals:
   // signals element index (0 .. N-1) of histogram which mouse has entered
   // or -1 if/when no longer over the element's bar.
   //
   void mouseIndexChanged (const int index);
   void mouseIndexPressed (const int index, const Qt::MouseButton button);

public slots:
   void setColour (const int index, const QColor& value);
   void setValue (const int index, const double value);
   void setValues (const DataArray& values);

protected:
   bool eventFilter (QObject* obj, QEvent* event);

private:
   int indexOfHistogramAreaPosition (const int x, const int y) const;
   int indexOfHistogramAreaPosition (const QPoint& p) const;

   int firstBarTopLeft () const;
   QRect fullBarRect (const int position) const;  // within paintArea
   QRect backgroundAreaRect (const int groupIndex) const;  // within paintArea

   QString coordinateText (const double value) const;
   int maxPaintTextWidth (QPainter& painter) const;
   void paintGrid (QPainter& painter, const QColor& penColour) const;

   void paintSecondaryBackground (QPainter& painter) const;

   // Returns true if item position is in the paintArea
   bool paintItem (QPainter& painter, const int position, const int index) const;

   void paintAllItems ();

   int scrollMaximum () const;

   // Detrmines the color to paint. If slot has a specific colour, that colour
   // is used, otherwise the default bar colour is returned.
   //
   QColor getPaintColour (const int index) const;

   // Usefull for preview mode and testing.
   //
   void createTestData ();

   // Private types
   //
   typedef QVector <QColor> ColourArray;

   // Private data
   //
   // Internal widgets
   //
   QBoxLayout* layoutA;   // manages histogramAxisPlusArea + scrollbar
   QBoxLayout* layoutB;  // manages axisPainter + histogramArea

   // histogramAxisPlusArea/histogramArea do nothing per se other than be size
   // managed by the layouts and provides a paint area for the historgram proper.
   //
   QWidget* histogramAxisPlusArea;
   QWidget* histogramArea;
   QScrollBar* scrollbar;
   QEAxisPainter* axisPainter;
   QRect paintArea;             // defines actual bar draw area - subset of histogramArea

   DataArray dataArray;
   ColourArray colourArray;

   // class member variable names associated with a property start with 'm'
   // so as not to clash with the propery names - this is more for qtcreator
   // user's benefit as opposed to the moc and/or c++ compiler.
   //
   QColor mBarColour;
   QColor mBackgroundColour;
   QColor mSecondBgColour;

   double mMinimum;
   double mMaximum;
   double mBaseLine;
   int mBarWidth;
   int mGap;
   int mMargin;
   bool mAutoBarGapWidths;
   bool mAutoScale;
   bool mDrawAxies;
   bool mDrawBorder;
   bool mShowScale;
   bool mShowGrid;    // value grid
   bool mShowSecondBg;
   int mSecondBgSize;
   bool mLogScale;
   Qt::Orientation mOrientation;
   int mTestSize;

   int lastEmittedIndex;   // allows filtering when mouse moves within single bar
   int firstDisplayed;
   int numberDisplayed;
   double drawMinimum;
   double drawMaximum;
   double drawMajor;
   double useGap;
   double useBarWidth;     // or height if/when vertical

private slots:
   void scrollBarValueChanged (int value);
};

#endif    // QE_HISTOGRAM_H
