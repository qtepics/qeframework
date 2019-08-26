/*  QEGraphic.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  Copyright (c) 2013-2019 Australian Synchrotron.
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
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_GRAPHIC_H
#define QE_GRAPHIC_H

#include <QEvent>
#include <QHBoxLayout>
#include <QList>
#include <QMap>
#include <QObject>
#include <QTimer>
#include <QWidget>
#include <QVariant>
#include <QVector>
#include <QPen>
#include <QBrush>
#include <QFont>

#include <qwt_plot.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_grid.h>

#include <QEGraphicNames.h>
#include <QEFrameworkLibraryGlobal.h>
#include <QEDisplayRanges.h>
#include <persistanceManager.h>

class QEGraphicMarkup;  // differed declaration

/// Provides a basic wrapper around QwtPlot, which:
///
/// a) Allocates and attaches curves and grids, and releases these
///    on delete, and releases curves on request;
///
/// b) Interprets mouse events with real world co-ordinates;
///
/// c) Provides a log scale mode (X and/or Y);
///
/// d) Provides a consistant means to round down/up min/max values and the
///    selection of a major interval value, e.g.:
///    2.1 .. 7.83 (user min/max) =>  2.0 .. 8.0, 1.0  (display min/max, major);
///
/// e) Standardised mouse and wheel zoomimg;
///
/// f) Smart axis re-scaling;
///
/// g) Provides markups; and
///
/// h) Provides wrapper functions to hide QWT version API changes.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEGraphic :
      public QWidget
{
   Q_OBJECT
public:
   // By default, there are no markups set as in use.
   //
   explicit QEGraphic (QWidget* parent = 0);
   explicit QEGraphic (const QString &title, QWidget* parent = 0);
   ~QEGraphic ();

   // Call before any replotting, releases all curves from previous plot.
   //
   void releaseCurves ();

   // User defined curve attached to the internal QwtPlot object.
   // Will be released by releaseCurves.
   //
   void attchOwnCurve (QwtPlotCurve* curve);

   void setBackgroundColour (const QColor colour);

   void setGridPen (const QPen& pen);

   // Set/Get the set of in use, i.e. permitted, markups.
   //
   void setAvailableMarkups (const QEGraphicNames::MarkupFlags graphicMarkupsSet);
   QEGraphicNames::MarkupFlags getAvailableMarkups () const;

   // Set/Get the markup visible.
   //
   void setMarkupVisible (const QEGraphicNames::Markups markup, const bool isVisible);
   bool getMarkupVisible (const QEGraphicNames::Markups markup) const;

   // Set/Get the markup enabled.
   //
   void setMarkupEnabled (const QEGraphicNames::Markups markup, const bool isEnabled);
   bool getMarkupEnabled (const QEGraphicNames::Markups markup) const;

   // Set/Get the markup selected.
   //
   void setMarkupSelected (const QEGraphicNames::Markups markup, const bool selected);
   bool getMarkupIsSelected (const QEGraphicNames::Markups markup) const;

   // When a mark has only an x or y postion, the y or x value is ignored.
   //
   void setMarkupPosition (const QEGraphicNames::Markups markup, const QPointF& position);
   QPointF getMarkupPosition (const QEGraphicNames::Markups markup) const;

   // Set/Get the markup text.
   //
   void setMarkupData (const QEGraphicNames::Markups markup, const QVariant& text);
   QVariant getMarkupData (const QEGraphicNames::Markups markup) const;

   // NOTE: Depricated - use setMarkupPosition/setMarkupVisible instead.
   //
   Q_DECL_DEPRECATED
   void setCrosshairsVisible (const bool isVisible);                            ///< Depricated

   Q_DECL_DEPRECATED
   void setCrosshairsVisible (const bool isVisible, const QPointF& position);   ///< Depricated

   Q_DECL_DEPRECATED
   bool getCrosshairsVisible () const;                                          ///< Depricated

   /**
    * Allocates a curve, sets current curve attibutes and attaches to plot.
    *
    * @param xData Vector of X axis values
    * @param yData Vector of Y axis values
    * @param yAxis Which Y axis should the yData be plotted against. If not
    * provided left axis is used by default
    */
   void plotCurveData (const QEGraphicNames::DoubleVector& xData,
                       const QEGraphicNames::DoubleVector& yData,
                       const QwtPlot::Axis yAxis = QwtPlot::yLeft);

   /**
    * Draw text at position specifying centre of bottom left corner.
    * Position may be real world coordinates or pixel coordates.
    *
    * @param posn  The postion to draw text
    * @param text  The text to draw
    * @param option  Specifies if position is in cavas pixels or real world co-ordinates
    * @param pointSize Defines text point size. Honors QEScaling parameters.
    * @param isCentred  When true text is centered about point, otherwise point
    *                   defines bottom left corner of the generated text
    */
   void drawText (const QPointF& posn,
                  const QString& text,
                  const QEGraphicNames::TextPositions option,
                  bool isCentred = true);

   void drawText (const QPoint& posn,
                  const QString& text,
                  const QEGraphicNames::TextPositions option,
                  bool isCentred = true);


   void setXRange (const double min, const double max,
                   const QEGraphicNames::AxisMajorIntervalModes mode,
                   const double value,
                   const bool immediate);

   /**
    * Returns the range of X axis
    *
    * @param min Minimum X value
    * @param max Maximum X value
    */
   void getXRange (double& min, double& max) const;

   void setYRange (const double min, const double max,
                   const QEGraphicNames::AxisMajorIntervalModes mode,
                   const double value,
                   const bool immediate,
                   const QwtPlot::Axis selectedYAxis = QwtPlot::yLeft);

   /**
    * Returns the rance of either left or right Y axis
    *
    * @param min              Reference to variable to be used to store minimum value
    * @param max              Reference to variable to be used to store maximumSize() value
    * @param selectedYAxis    Range for which Y axis is required. If not provided left
    *                         axis is used by default
    */
   void getYRange (double& min, double& max, const QwtPlot::Axis selectedYAxis = QwtPlot::yLeft) const;

   // Last call - renders all curves defined since call to releaseCurves.
   // Calls inner QwtPlot replot.
   //
   void replot ();

   Q_DECL_DEPRECATED
   bool rightButtonPressed () const;    // to allow inhibition of context menu.

   QPointF getRealMousePosition () const;   // current mouse position in real world coords

   // Returns true if the given global position of over the plot cancas.
   //
   bool globalPosIsOverCanvas (const QPoint& golbalPos) const;

   // Is the QEGraphicNames::Line visible. If yes the slope argument is set to
   // the slope of the line.
   //
   bool getSlopeIsDefined (QPointF& slope) const;

   // Set and get axis attribute functions
   //
   void setAxisEnableX (const bool enable);
   bool getAxisEnableX () const;

   /**
    * Enables or disables left or right Y axis
    *
    * @param enable        Enable/disable Y axis
    * @param selectedYAxis Left/right Y axis. If not provided left axis is used by default
    */
   void setAxisEnableY (const bool enable, const QwtPlot::Axis selectedYAxis = QwtPlot::yLeft);

   /**
    * Is left/right Y axis enabled/disabled
    *
    * @param selectedYAxis Left/right Y axis. If not provided left axis is used by default
    * @return              Boolean signaling if axis is enabled/disabled
    */
   bool getAxisEnableY (const QwtPlot::Axis selectedYAxis = QwtPlot::yLeft) const;

   // Scale and offset scale the x and y data before ploting, i.e. allows different
   // axis and data coordinates. For example, with X scale set to 1/60, data could be
   // expressed in seconds, but (more conviently) have time axis is minutes.
   //
   void setXScale (const double scale);
   double getXScale () const;

   void setXOffset (const double offset);
   double getXOffset () const;

   void setXLogarithmic (const bool isLog);
   bool getXLogarithmic () const;

   /**
    * Sets scale on either left or right Y axis
    *
    * @param scale         Scale to be set
    * @param selectedYAxis Left/right Y axis. If not provided left axis is used by default
    */
   void setYScale (const double scale, const QwtPlot::Axis selectedYAxis = QwtPlot::yLeft);

   /**
    * Returns the scale set on left/right Y axis.
    *
    * @param selectedYAxis Left/right Y axis. If not provided left axis is used by default.
    * @return              Scale set
    */
   double getYScale (const QwtPlot::Axis selectedYAxis = QwtPlot::yLeft) const;

   /**
    * Sets offset on either left or right Y axis
    *
    * @param offset        Offset to be set
    * @param selectedYAxis Left/right Y axis. If not provided left axis is used by default
    */
   void setYOffset (const double offset, const QwtPlot::Axis selectedYAxis = QwtPlot::yLeft);

   /**
    * Returns the offset set on left/right Y axis.
    *
    * @param selectedYAxis Left/right Y axis. If not provided left axis is used by default.
    * @return              Offset set
    */
   double getYOffset (const QwtPlot::Axis selectedYAxis = QwtPlot::yLeft) const;

   /**
    * Sets the scale on Y axis to be logarithmic or not
    *
    * @param isLog         Boolean flag defining if the scale is to be logarithmic
    * @param selectedYAxis Left/right Y axis. If not provided left axis is used by default
    */
   void setYLogarithmic (const bool isLog, const QwtPlot::Axis selectedYAxis = QwtPlot::yLeft);

   /**
    * Is the scale set to be logarithmic or not on left/right axis
    *
    * @param selectedYAxis Left/right Y axis. If not provided left axis is used by default
    * @return              Boolean flag defining if the scale set is logarithmic
    */
   bool getYLogarithmic (const QwtPlot::Axis selectedYAxis = QwtPlot::yLeft) const;

   /**
    * Sets the color of left/right Y axis
    *
    * @param color         Color to be set
    * @param selectedYAxis Left/right Y axis. If not provided left axis is used by default
    */
   void setYColor (const QColor color, const QwtPlot::Axis selectedYAxis = QwtPlot::yLeft);

   // Set and get current curve attributes.
   // These are used for internally allocated curves.
   //
   void setCurvePen (const QPen& pen);
   QPen getCurvePen () const;

   void setCurveBrush (const QBrush& brush);
   QBrush getCurveBrush () const;

   // Set and get current text font attributes.
   // These are used for internally allocated curves.
   //
   void setTextFont (const QFont& font);
   QFont getTextFont () const;

   // Point size of current text font.
   //
   void setTextPointSize (const int pointSize);
   int getTextPointSize () const;

   // The RenderAntialiased hint is off by default.
   //
   void setCurveRenderHint (const QwtPlotItem::RenderHint hint, const bool on = true);
   QwtPlotItem::RenderHint getCurveRenderHint () const;
   bool getCurveRenderHintOn () const;

   void setCurveStyle (const QwtPlotCurve::CurveStyle style);
   QwtPlotCurve::CurveStyle getCurveStyle ();

   // Utility functions.
   //
   // Returns the pixel distance between two real points.
   //
   QPoint pixelDistance (const QPointF& from, const QPointF& to) const;

   // Returns the real delta offset for a nominated pixel delta/offset.
   // Not suitable for logarithic scaling.
   //
   QPointF realOffset (const QPoint&  offset, const QwtPlot::Axis selectedYAxis = QwtPlot::yLeft) const;
   QPointF realOffset (const QPointF& offset, const QwtPlot::Axis selectedYAxis = QwtPlot::yLeft) const;  // overloaded form

   // Converts between pixel coords to real world coords taking into
   // account any scaling and/or logarithic scaling. This can be done for either
   // left or right Y axis
   //
   QPointF pointToReal (const QPoint& pos,  const QwtPlot::Axis selectedYAxis = QwtPlot::yLeft) const;
   QPointF pointToReal (const QPointF& pos, const QwtPlot::Axis selectedYAxis = QwtPlot::yLeft) const;   // overloaded form
   QPoint  realToPoint (const QPointF& pos, const QwtPlot::Axis selectedYAxis = QwtPlot::yLeft) const;


   // Embedded qwt plot manipulation functions.
   //
   void enableAxis (int axisId, bool tf = true);
   void setAxisScale (int axisId, double min, double max, double step = 0);
   void installCanvasEventFilter (QObject* filter);
   bool isCanvasObject (QObject* obj) const;
   QRect getEmbeddedCanvasGeometry () const;

   // Use with care - allows direct manipulation of the plot object.
   // for functions not covered above.
   //
   QwtPlot* getEmbeddedQwtPlot () const;

   // Save/restore all markup configuration
   //
   void saveConfiguration (PMElement & parentElement);
   void restoreConfiguration (PMElement & parentElement);
   
signals:
   void mouseMove     (const QPointF& posn);
   void wheelRotate   (const QPointF& posn, const int delta);

   // For middle and right buttons respectively, provides down (from) mouse
   // position and current (to) mouse position in user coordinates.
   //
   void areaDefinition (const QPointF& from, const QPointF& to);
   void lineDefinition (const QPointF& from, const QPointF& to);
   void crosshairsMove (const QPointF& posn);
   void markupMove     (const QEGraphicNames::Markups markup, const QPointF& position);

protected:
   void canvasMousePress (QMouseEvent* mouseEvent);
   void canvasMouseRelease (QMouseEvent* mouseEvent);
   void canvasMouseMove (QMouseEvent* mouseEvent, const bool isButtonAction);
   bool eventFilter (QObject *obj, QEvent *event);

private:
   class OwnPlot;   // private and differed.

   // Handle each axis in own class.
   //
   class Axis {
   public:
      explicit Axis (QwtPlot* plot, const int axisId);
      ~Axis ();

      void setRange (const double min, const double max,
                     const QEGraphicNames::AxisMajorIntervalModes mode,
                     const double value,
                     const bool immediate);
      void getRange (double& min, double& max);
      bool doDynamicRescaling ();
      void determineAxisScale ();

      // Converts between pixel coords to real world coords taking into
      // account any scaling and/or logarithic scaling.
      //
      double pointToReal (const int pos) const;
      int realToPoint (const double pos) const;

      double scaleValue (const double coordinate) const;

      void setAxisEnable (const bool axisEnable);
      bool getAxisEnable () const;

      void setScale (const double scale);
      double getScale () const;

      void setAxisColor (const QColor axisColor) const;

      void setOffset (const double offset);
      double getOffset () const;

      void setLogarithmic (const bool isLogarithmic);
      bool getLogarithmic () const;

   private:
      QwtPlot* plot;
      int axisId;
      QEDisplayRanges source;    // where we started from
      QEDisplayRanges target;    // wehere we are going
      QEDisplayRanges current;   // where we are now
      int transitionCount;
      QEGraphicNames::AxisMajorIntervalModes intervalMode;
      double intervalValue;
      bool axisEnabled;

      bool isLogarithmic;   // vs. Linear

      // Data scaling x' = mx + c. This is applied before any log10 scaling.
      // Allows axis scale to be different units to plot scale, e.g. minutes vs. seconds
      double scale;     // m
      double offset;    // c

      double useMin;
      double useMax;
      double useStep;
   };

   void construct ();

   // Steps go from NUMBER_TRANISTION_STEPS (at start) down to 0 (at finish).
   //
   static QEDisplayRanges calcTransitionPoint (const QEDisplayRanges& start,
                                               const QEDisplayRanges& finish,
                                               const int step);

   // Progresses any on-going dynamic axis rescaling for left/right Y axis.
   // Return true when this is in progress.
   //
   bool doDynamicRescaling (const QwtPlot::Axis selectedYAxis = QwtPlot::yLeft);

   // Uses cursor position to find closest, if any markup calls
   // each markup's plot functions which call plotMarkupCurveData.
   QEGraphicMarkup* mouseIsOverMarkup ();
   void plotMarkups ();

   void plotMarkupCurveData (const QEGraphicNames::DoubleVector& xData,
                             const QEGraphicNames::DoubleVector& yData);

   // Creates curve data using left/right Y axis
   QwtPlotCurve* createCurveData (const QEGraphicNames::DoubleVector& xData,
                                  const QEGraphicNames::DoubleVector& yData,
                                  const QwtPlot::Axis selectedYAxis = QwtPlot::yLeft);

   // Relases and replots markups, then calls QwtPlot replot
   void graphicReplot ();

   void drawTexts (QPainter* painter);    // called from OwnPlot

   //Axis position to actual QEGraphic::Axis conversion
   Axis* axisFromPosition (const QwtPlot::Axis axis) const;

   Axis* xAxis;       // bottom
   Axis* yAxisLeft;
   Axis* yAxisRight;

   QEGraphicNames::QEGraphicMarkupsSets* graphicMarkupsSet;  // a set of available markups.

   QHBoxLayout* layout;                         // controls plot layout within QEGraphic
   OwnPlot* plot;                               // Essentially QwtPlot
   QwtPlotGrid* plotGrid;
   QTimer* tickTimer;

   // Keep a list of allocated curves so that we can track and delete them.
   //
   typedef QList<QwtPlotCurve*> CurveLists;
   CurveLists userCurveList;                    // for user curves
   CurveLists markupCurveList;                  // for internal markup curves
   void releaseCurveList (CurveLists& list);

   // Keep a list of drawn texts.
   //
   struct TextItems {
      QPointF position;   // stored in real world coordinates.
      QString text;
      bool isCentred;     // when true text is centred about the given position
      QFont font;
      QPen pen;
   };

   typedef QList <TextItems> TextItemLists;
   TextItemLists textItemList;
   void releaseTextItemList (TextItemLists& list);

   // Curve/text attributes.
   //
   QPen pen;
   QBrush brush;
   QFont textFont;

   QwtPlotItem::RenderHint hint;
   bool hintOn;
   QwtPlotCurve::CurveStyle style;
   QPointF realMousePosition;

   bool rightButtonIsPressed;

private slots:
   void tickTimeout ();

   // The price we pay for separate classes is we have to befriend them all.
   //
   friend class OwnPlot;
   friend class QEGraphicMarkup;
   friend class QEGraphicAreaMarkup;
   friend class QEGraphicLineMarkup;
   friend class QEGraphicCrosshairsMarkup;
   friend class QEGraphicHorizontalMarkup;
   friend class QEGraphicVerticalMarkup;
};

# endif  // QE_GRAPHIC_H
