/*  QEDesignerPlugin.cpp
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
 *  Copyright (c) 2009, 2010 Australian Synchrotron
 *
 *  Author:
 *    Andrew Rhyder
 *  Contact details:
 *    andrew.rhyder@synchrotron.org.au
 */

#include <QELabelManager.h>
#include <QESubstitutedLabelManager.h>
#include <QELineEditManager.h>
#include <QNumericEditManager.h>
#include <QENumericEditManager.h>
#include <QEPushButtonManager.h>
#include <QERadioButtonManager.h>
#include <QRadioGroupManager.h>
#include <QERadioGroupManager.h>
#include <QECheckBoxManager.h>
#include <QEShapeManager.h>
#include <QESliderManager.h>
#include <QESpinBoxManager.h>
#include <QEComboBoxManager.h>
#include <QELinkManager.h>
#include <QEPlotManager.h>
#include <QEPlotterManager.h>
#include <QEPeriodicManager.h>
#include <QEAnalogProgressBarManager.h>
#include <QEAnalogIndicatorManager.h>
#include <QEAnalogSliderManager.h>
#include <QAnalogSliderManager.h>
#include <QEBitStatusManager.h>
#include <QBitStatusManager.h>
#include <QEImageManager.h>
#include <QEPvLoadSaveManager.h>
#include <QEPvPropertiesManager.h>
#include <QEStripChartManager.h>
#include <QEConfiguredLayoutManager.h>
#include <QEFileBrowserManager.h>
#include <QERecipeManager.h>
#include <QELoginManager.h>
#include <QELogManager.h>
#include <QEScriptManager.h>
#include <QEScratchPadManager.h>
#include <QEFormManager.h>
#include <QEFormGridManager.h>
#include <QEFrameManager.h>
#include <QEPvFrameManager.h>
#include <QEGeneralEditManager.h>
#include <QEGroupBoxManager.h>
#include <QEFileImageManager.h>
#include <QSimpleShapeManager.h>
#include <QESimpleShapeManager.h>
#include <QEArchiveNameSearchManager.h>
#include <QEArchiveStatusManager.h>
#include <QEHistogramManager.h>
#include <QEScalarHistogramManager.h>
#include <QEWaveformHistogramManager.h>
#include <QETableManager.h>
#include <QEResizeableFrameManager.h>
#include <QEMenuButtonManager.h>

#include <QEDesignerPlugin.h>

/*
    Add all plugins that will be provided in QT Designer.
*/
QEWidgets::QEWidgets(QObject *parent) : QObject(parent) {
    // note: maintain in alphabetical order.
    widgets.append(new QEAnalogProgressBarManager(this));
    widgets.append(new QEAnalogSliderManager(this));
    widgets.append(new QEBitStatusManager(this));
    widgets.append(new QECheckBoxManager(this));
    widgets.append(new QEComboBoxManager(this));
    widgets.append(new QEConfiguredLayoutManager(this));
    widgets.append(new QEFileBrowserManager(this));
    widgets.append(new QEFileImageManager(this));
    widgets.append(new QEFormManager(this));
    widgets.append(new QEFormGridManager(this));
    widgets.append(new QEGeneralEditManager(this));
    widgets.append(new QEImageManager(this));
    widgets.append(new QELabelManager(this));
    widgets.append(new QELineEditManager(this));
    widgets.append(new QELogManager(this));
    widgets.append(new QELoginManager(this));
    widgets.append(new QENumericEditManager(this));
    widgets.append(new QEPeriodicManager(this));
    widgets.append(new QEPlotManager(this));
    widgets.append(new QEPlotterManager(this));
    widgets.append(new QEPushButtonManager(this));
    widgets.append(new QEPvLoadSaveManager(this));
    widgets.append(new QEPvFrameManager(this));
    widgets.append(new QEPvPropertiesManager(this));
    widgets.append(new QERadioButtonManager(this));
    widgets.append(new QERadioGroupManager(this));
    widgets.append(new QERecipeManager(this));
    widgets.append(new QEScalarHistogramManager(this));
    widgets.append(new QEScratchPadManager(this));
    widgets.append(new QEScriptManager(this));
    widgets.append(new QEShapeManager(this));
    widgets.append(new QESimpleShapeManager(this));
    widgets.append(new QESliderManager(this));
    widgets.append(new QESpinBoxManager(this));
    widgets.append(new QEStripChartManager(this));
    widgets.append(new QESubstitutedLabelManager(this));
    widgets.append(new QETableManager(this));
    widgets.append(new QEWaveformHistogramManager(this));

    // Non-EPICS aware framework widgets.
    //
    widgets.append(new QEAnalogIndicatorManager(this));
    widgets.append(new QAnalogSliderManager(this));
    widgets.append(new QEArchiveNameSearchManager(this));
    widgets.append(new QEArchiveStatusManager(this));
    widgets.append(new QBitStatusManager(this));
    widgets.append(new QEFrameManager(this));
    widgets.append(new QEGroupBoxManager(this));
    widgets.append(new QEHistogramManager(this));
    widgets.append(new QELinkManager(this));
    widgets.append(new QEMenuButtonManager(this));
    widgets.append(new QNumericEditManager(this));
    widgets.append(new QRadioGroupManager(this));
    widgets.append(new QEResizeableFrameManager(this));
    widgets.append(new QSimpleShapeManager(this));
}

/*
    Multiplugin interface.
*/
QList<QDesignerCustomWidgetInterface*> QEWidgets::customWidgets() const {
    return widgets;
}

#if (QT_VERSION < QT_VERSION_CHECK(5, 0, 0))
// Publish plugins through to QT designer.
Q_EXPORT_PLUGIN2(QEWidgetsPlugin, QEWidgets)
#endif

// end
