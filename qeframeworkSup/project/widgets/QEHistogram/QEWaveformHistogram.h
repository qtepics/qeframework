/*  QEWaveformHistogram.h
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
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
 *  Copyright (c) 2014,2016 Australian Synchrotron.
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
 */

#ifndef QE_WAVEFORM_HISTOGRAM_H
#define QE_WAVEFORM_HISTOGRAM_H

#include <QLayout>
#include <QString>
#include <QVector>

#include <QECommon.h>
#include <QEHistogram.h>
#include <QEFrame.h>
#include <QEWidget.h>
#include <QEFloating.h>
#include <QEFloatingFormatting.h>
#include <QCaVariableNamePropertyManager.h>
#include <QEFrameworkLibraryGlobal.h>

/// The QEWaveformHistogram class is a EPICS aware histogram widget.
/// The value of, i.e. the length of each bar of the histogram is controlled by
/// the corresponding element of an array, e.g. waveform, process variable.
/// When the variable is defined (connected), the bar lengths are updated, and
/// optionally the bar colours set to reflect the variable's severity status.
/// The bar is 'grayed-out' when the variable is disconnected (although the bars
/// retains their last known values/lengths).
/// The histogram nature of the this widget is provided by a QEHistogram widget.
/// The QEWaveformHistogram widget is tighly integrated with the base class QEWidget,
/// via QEFrame, which provides generic support such as macro substitutions,
/// drag/drop, and standard properties.
///
class QE_FRAMEWORK_LIBRARY_SHARED_EXPORT QEWaveformHistogram:public QEFrame {
   Q_OBJECT

   // All standard properties are inherited from QEFrame.
   //
   Q_PROPERTY (QString variable READ getVariableNameProperty WRITE setVariableNameProperty)
   Q_PROPERTY (QString variableSubstitutions READ getPvNameSubstitutions WRITE
               setPvNameSubstitutions)

public:
   /// \enum ScaleModes
   enum ScaleModes {
      Manual,              ///< Use property minimum/maximum to scale histogram
      Auto,                ///< Dynamically scale based on minimum/maximum displayed value
      OperationalRange     ///< Use process variable operational range (LOPR/HOPR).
   };

   Q_ENUMS (ScaleModes)

   // Histogram properties
   //
   Q_PROPERTY (bool   autoBarGapWidths READ getAutoBarGapWidths WRITE setAutoBarGapWidths)
   Q_PROPERTY (int    barWidth         READ getBarWidth         WRITE setBarWidth)
   Q_PROPERTY (int    gap              READ getGap              WRITE setGap)
   Q_PROPERTY (int    margin           READ getMargin           WRITE setMargin)
   Q_PROPERTY (ScaleModes scaleMode    READ getScaleMode        WRITE setScaleMode)
   Q_PROPERTY (double minimum          READ getMinimum          WRITE setMinimum)
   Q_PROPERTY (double maximum          READ getMaximum          WRITE setMaximum)
   Q_PROPERTY (double baseLine         READ getBaseLine         WRITE setBaseLine)
   Q_PROPERTY (bool   drawAxies        READ getDrawAxies        WRITE setDrawAxies)
   Q_PROPERTY (bool   showScale        READ getShowScale        WRITE setShowScale)
   Q_PROPERTY (bool   showGrid         READ getShowGrid         WRITE setShowGrid)
   Q_PROPERTY (bool   logScale         READ getLogScale         WRITE setLogScale)
   // When dislayAlarmState set ture, this property value effectively ignored.
   Q_PROPERTY (QColor backgroundColour READ getBackgroundColour WRITE setBackgroundColour)
   Q_PROPERTY (QColor barColour        READ getBarColour        WRITE setBarColour)
   Q_PROPERTY (bool   drawBorder       READ getDrawBorder       WRITE setDrawBorder)
   Q_PROPERTY (Qt::Orientation orientation READ getOrientation  WRITE setOrientation)

public:
   explicit QEWaveformHistogram (QWidget* parent = 0);
   ~QEWaveformHistogram () { }

   void setScaleMode (const ScaleModes scaleMode);
   ScaleModes getScaleMode () const;

   // Expose access to the internal widget's set/get functions.
   //
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (histogram, double, getMinimum,    setMinimum)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (histogram, double, getMaximum,    setMaximum)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (histogram, double, getBaseLine,   setBaseLine)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (histogram, int,    getGap,        setGap)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (histogram, int,    getBarWidth,   setBarWidth)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (histogram, int,    getMargin,      setMargin)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (histogram, bool,   getAutoBarGapWidths, setAutoBarGapWidths)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (histogram, bool,   getShowScale,  setShowScale)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (histogram, bool,   getShowGrid,   setShowGrid)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (histogram, bool,   getLogScale,   setLogScale)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (histogram, bool,   getDrawAxies,  setDrawAxies)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (histogram, bool,   getDrawBorder, setDrawBorder)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (histogram, QColor, getBackgroundColour, setBackgroundColour)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (histogram, QColor, getBarColour,  setBarColour)
   QE_EXPOSE_INTERNAL_OBJECT_FUNCTIONS (histogram, Qt::Orientation, getOrientation, setOrientation)

signals:
   // signals element index (0 .. N-1) of histogram which mouse has entered
   // or -1 if/when no longer over the element's bar.
   //
   void mouseIndexChanged (const int index);
   void mouseIndexPressed (const int index, const Qt::MouseButton button);

protected:
   qcaobject::QCaObject* createQcaItem (unsigned int variableIndex);
   void establishConnection (unsigned int variableIndex);

   // Drag and Drop
   void dragEnterEvent (QDragEnterEvent* event) { this->qcaDragEnterEvent (event);  }
   void dropEvent (QDropEvent* event)           { this->qcaDropEvent (event);       }
   void mousePressEvent (QMouseEvent* event)    { this->qcaMousePressEvent (event); }
   // This widget uses the setDrop/getDrop defined in QEWidget which is copy/paste.

   // Copy paste
   QString copyVariable ();
   QVariant copyData ();
   void paste (QVariant v);

   // Sets (or replaces) the pvName and establish the connection.
   //
   void setPvName (const QString& pvName);

private:
   void setVariableNameProperty (const QString& variableName);
   QString getVariableNameProperty ();
   void setPvNameSubstitutions (const QString& subs);
   QString getPvNameSubstitutions () const;
   void updateHistogramScale ();
   void setReadOut (const QString& text);
   void genReadOut (const int index);

   QEHistogram* histogram;
   QHBoxLayout* layout;         // holds the histogram - any layout type will do
   QCaVariableNamePropertyManager vnpm;
   QEFloatingFormatting floatingFormatting;
   int selectedChannel;         //
   ScaleModes mScaleMode;
   bool isFirstUpdate;

private slots:
   void newVariableNameProperty (QString pvName, QString subs, unsigned int variableIndex);

   void connectionChanged (QCaConnectionInfo& connectionInfo, const unsigned int&variableIndex);

   void setChannelArrayValue (const QVector <double>& value, QCaAlarmInfo&,
                              QCaDateTime&, const unsigned int&);

   void mouseIndexChangedSlot (const int index);
   void mouseIndexPressedSlot (const int index, const Qt::MouseButton button);
};

#ifdef QE_DECLARE_METATYPE_IS_REQUIRED
Q_DECLARE_METATYPE (QEWaveformHistogram::ScaleModes)
#endif

#endif // QE_WAVEFORM_HISTOGRAM_H
