/*  QEPluginWidgetManagers.h
 *
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
 *  Copyright (c) 2017 Australian Synchrotron
 *
 *  Author:
 *    Andrew Starritt
 *  Contact details:
 *    andrew.starritt@synchrotron.org.au
*/

#ifndef QE_PLUGIN_WIDGET_MANAGERS_H
#define QE_PLUGIN_WIDGET_MANAGERS_H

#include <QObject>
#include <QEDesignerPluginCommon.h>
#include <QEPluginLibrary_global.h>

/// This module provides the manager class associated with each regular QE widget
/// provided by the QE Plugin library apart from QEMenyButton and QEPeriodoc.
//
//------------------------------------------------------------------------------
// While the bulk of the class defintion can be provided by the declare
// manager macro, Qt artefacts need to be explicit to keep moc happy.
//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEAnalogIndicatorManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEAnalogIndicator)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEAnalogProgressBarManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEAnalogProgressBar)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QAnalogSliderManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QAnalogSlider)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEAnalogSliderManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEAnalogSlider)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QBitStatusManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QBitStatus)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEBitStatusManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEBitStatus)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QECheckBoxManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QECheckBox)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEPushButtonManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEPushButton)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QERadioButtonManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QERadioButton)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEComboBoxManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEComboBox)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEConfiguredLayoutManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEConfiguredLayout)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEFileBrowserManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEFileBrowser)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEFileImageManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEFileImage)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEFormManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEForm)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEFormGridManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEFormGrid)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEFrameManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEFrame)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEPvFrameManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEPvFrame)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEGeneralEditManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEGeneralEdit)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEGroupBoxManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEGroupBox)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEHistogramManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEHistogram)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEScalarHistogramManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEScalarHistogram)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEWaveformHistogramManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEWaveformHistogram)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEImageManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEImage)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QELabelManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QELabel)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QELineEditManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QELineEdit)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QELinkManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QELink)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QELogManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES(QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QELog)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QELoginManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES(QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QELogin)
};

//------------------------------------------------------------------------------
// QEMenuButton - place holder

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QNumericEditManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES(QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QNumericEdit)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QENumericEditManager
      : public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QENumericEdit)
};

//------------------------------------------------------------------------------
// QEPeriodic - place holder

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEPlotManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEPlot)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEPlotterManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEPlotter)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEPvLoadSaveManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEPvLoadSave)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEPvPropertiesManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEPvProperties)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QRadioGroupManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QRadioGroup)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QERadioGroupManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QERadioGroup)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QERecipeManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QERecipe)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEResizeableFrameManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEResizeableFrame)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEScratchPadManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES(QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEScratchPad)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEScriptManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES(QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEScript)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEShapeManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES(QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEShape)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QSimpleShapeManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES(QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QSimpleShape)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QESimpleShapeManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES(QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QESimpleShape)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QESliderManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES(QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QESlider)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QESpinBoxManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QESpinBox)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEStripChartManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEStripChart)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QESubstitutedLabelManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QESubstitutedLabel)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QETableManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QETable)
};


//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEArchiveNameSearchManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEArchiveNameSearch)
};

//------------------------------------------------------------------------------
//
class QEPLUGINLIBRARYSHARED_EXPORT QEArchiveStatusManager :
      public QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES (QDesignerCustomWidgetInterface)
   QE_DECLARE_PLUGIN_MANAGER (QEArchiveStatus)
};

#endif // QE_PLUGIN_WIDGET_MANAGERS_H
