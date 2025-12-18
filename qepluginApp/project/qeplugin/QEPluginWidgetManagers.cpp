/*  QEPluginWidgetManagers.cpp
 *
 *  This file is part of the EPICS QT Framework, initially developed at the
 *  Australian Synchrotron.
 *
 *  SPDX-FileCopyrightText: 2017-2025 Australian Synchrotron
 *  SPDX-License-Identifier: LGPL-3.0-only
 *
 *  Author:     Andrew Starritt
 *  Maintainer: Andrew Starritt
 *  Contact:    andrews@ansto.gov.au
 */

#include "QEPluginWidgetManagers.h"

#include <QEDesignerPluginGroupNames.h>
#include <QEAlarmColourSelection.h>
#include <QEAnalogIndicator.h>
#include <QEAnalogProgressBar.h>
#include <QAnalogSlider.h>
#include <QEAnalogSlider.h>
#include <QBitStatus.h>
#include <QEBitStatus.h>
#include <QECalcout.h>
#include <QECheckBox.h>
#include <QEDescriptionLabel.h>
#include <QEPushButton.h>
#include <QERadioButton.h>
#include <QEComboBox.h>
#include <QEConfiguredLayout.h>
#include <QECorrelation.h>
#include <QEDistribution.h>
#include <QEExitButton.h>
#include <QEFileBrowser.h>
#include <QEFileImage.h>
#include <QEForm.h>
#include <QEFormStateChange.h>
#include <QEFormGrid.h>
#include <QEDynamicFormGrid.h>
#include <QEFrame.h>
#include <QEPvFrame.h>
#include <QEGeneralEdit.h>
#include <QEGroupBox.h>
#include <QEFormGroupBox.h>
#include <QEHistogram.h>
#include <QEScalarHistogram.h>
#include <QEWaveformHistogram.h>
#include <QEImage.h>
#include <QELabel.h>
#include <QEDateTime.h>
#include <QELCDNumber.h>
#include <QELineEdit.h>
#include <QELink.h>
#include <QELog.h>
#include <QELogin.h>
#include <QEMenuButton.h>
#include <QNumericEdit.h>
#include <QENumericEdit.h>
#include <QEPeriodic.h>
#include <QEPlot.h>
#include <QEPlotter.h>
#include <QEPvLoadSave.h>
#include <QEPvLoadSaveButton.h>
#include <QEPvProperties.h>
#include <QRadioGroup.h>
#include <QERadioGroup.h>
#include <QERecipe.h>
#include <QEResizeableFrame.h>
#include <QEScratchPad.h>
#include <QEScript.h>
#include <QEShape.h>
#include <QSimpleShape.h>
#include <QESimpleShape.h>
#include <QESlider.h>
#include <QESpinBox.h>
#include <QEStripChart.h>
#include <QESubstitutedLabel.h>
#include <QETable.h>
#include <QENTTable.h>
#include <QESelector.h>
#include <QESpectrogram.h>
#include <QESurface.h>
#include <QEWaterfall.h>
#include <QEArchiveNameSearch.h>
#include <QEArchiveStatus.h>

// Implement the widget plugin managers for all regular widgets.
// Note : QEComment, QEMenuButton and QEPeriodic provide designer extentions
//        and are provided elsewhere.

QE_IMPLEMENT_PLUGIN_MANAGER (QEAlarmColourSelection, QETOOL,  ":/qe/plugin/QEAlarmColourSelection.png", false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEAnalogIndicator,   QETOOL,     ":/qe/plugin/QEAnalogIndicator.png",   false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEAnalogProgressBar, QEMONITOR,  ":/qe/plugin/QEAnalogProgressBar.png", false)
QE_IMPLEMENT_PLUGIN_MANAGER (QAnalogSlider,       QETOOL,     ":/qe/plugin/QAnalogSlider.png",       false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEAnalogSlider,      QECONTROL,  ":/qe/plugin/QEAnalogSlider.png",      false)
QE_IMPLEMENT_PLUGIN_MANAGER (QBitStatus,          QETOOL,     ":/qe/plugin/QBitStatus.png",          false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEBitStatus,         QEMONITOR,  ":/qe/plugin/QEBitStatus.png",         false)
QE_IMPLEMENT_PLUGIN_MANAGER (QECheckBox,          QECONTROL,  ":/qe/plugin/QECheckBox.png",          false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEPushButton,        QECONTROL,  ":/qe/plugin/QEPushButton.png",        false)
QE_IMPLEMENT_PLUGIN_MANAGER (QERadioButton,       QECONTROL,  ":/qe/plugin/QERadioButton.png",       false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEComboBox,          QECONTROL,  ":/qe/plugin/QEComboBox.png",          false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEConfiguredLayout,  QESUPPORT,  ":/qe/plugin/QEConfiguredLayout.png",  false)
QE_IMPLEMENT_PLUGIN_MANAGER (QECorrelation,       QEGRAPHIC,  ":/qe/plugin/QECorrelation.png",       false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEDistribution,      QEGRAPHIC,  ":/qe/plugin/QEDistribution.png",      false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEExitButton,        QESUPPORT,  ":/qe/plugin/QEExitButton.png",        false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEFileBrowser,       QECONTROL,  ":/qe/plugin/QEFileBrowser.png",       false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEFileImage,         QEMONITOR,  ":/qe/plugin/QEFileImage.png",         false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEForm,              QESUPPORT,  ":/qe/plugin/QEForm.png",              false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEFormStateChange,   QESUPPORT,  ":/qe/plugin/QEFormStateChange.png",   false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEFormGrid,          QESUPPORT,  ":/qe/plugin/QEFormGrid.png",          false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEDynamicFormGrid,   QESUPPORT,  ":/qe/plugin/QEDynamicFormGrid.png",   false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEFrame,             QESUPPORT,  ":/qe/plugin/QEFrame.png",             true)
QE_IMPLEMENT_PLUGIN_MANAGER (QEPvFrame,           QEMONITOR,  ":/qe/plugin/QEPvFrame.png",           true)
QE_IMPLEMENT_PLUGIN_MANAGER (QEGeneralEdit,       QECONTROL,  ":/qe/plugin/QEGeneralEdit.png",       false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEGroupBox,          QESUPPORT,  ":/qe/plugin/QEGroupBox.png",          true)
QE_IMPLEMENT_PLUGIN_MANAGER (QEFormGroupBox,      QESUPPORT,  ":/qe/plugin/QEFormGroupBox.png",      false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEHistogram,         QETOOL,     ":/qe/plugin/QEHistogram.png",         false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEScalarHistogram,   QEGRAPHIC,  ":/qe/plugin/QEScalarHistogram.png",   false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEWaveformHistogram, QEGRAPHIC,  ":/qe/plugin/QEWaveformHistogram.png", false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEImage,             QEGRAPHIC,  ":/qe/plugin/QEImage.png",             false)
QE_IMPLEMENT_PLUGIN_MANAGER (QELabel,             QEMONITOR,  ":/qe/plugin/QELabel.png",             false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEDateTime,          QESUPPORT,  ":/qe/plugin/QEDateTime.png",          false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEDescriptionLabel,  QEMONITOR,  ":/qe/plugin/QEDescriptionLabel.png",  false)
QE_IMPLEMENT_PLUGIN_MANAGER (QELCDNumber,         QEMONITOR,  ":/qe/plugin/QELCDNumber.png",         false)
QE_IMPLEMENT_PLUGIN_MANAGER (QELineEdit,          QECONTROL,  ":/qe/plugin/QELineEdit.png",          false)
QE_IMPLEMENT_PLUGIN_MANAGER (QELink,              QESUPPORT,  ":/qe/plugin/QELink.png",              false)
QE_IMPLEMENT_PLUGIN_MANAGER (QECalcout,           QESUPPORT,  ":/qe/plugin/QECalcout.png",           false)
QE_IMPLEMENT_PLUGIN_MANAGER (QELog,               QESUPPORT,  ":/qe/plugin/QELog.png",               false)
QE_IMPLEMENT_PLUGIN_MANAGER (QELogin,             QESUPPORT,  ":/qe/plugin/QELogin.png",             false)
QE_IMPLEMENT_PLUGIN_MANAGER (QNumericEdit,        QETOOL,     ":/qe/plugin/QNumericEdit.png",        false)
QE_IMPLEMENT_PLUGIN_MANAGER (QENumericEdit,       QECONTROL,  ":/qe/plugin/QENumericEdit.png",       false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEPlot,              QEGRAPHIC,  ":/qe/plugin/QEPlot.png",              false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEPlotter,           QEGRAPHIC,  ":/qe/plugin/QEPlotter.png",           false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEPvLoadSave,        QESUPPORT,  ":/qe/plugin/QEPvLoadSave.png",        false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEPvLoadSaveButton,  QESUPPORT,  ":/qe/plugin/QEPvLoadSaveButton.png",  false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEPvProperties,      QETOOL,     ":/qe/plugin/QEPvProperties.png",      false)
QE_IMPLEMENT_PLUGIN_MANAGER (QRadioGroup,         QETOOL,     ":/qe/plugin/QRadioGroup.png",         false)
QE_IMPLEMENT_PLUGIN_MANAGER (QERadioGroup,        QECONTROL,  ":/qe/plugin/QERadioGroup.png",        false)
QE_IMPLEMENT_PLUGIN_MANAGER (QERecipe,            QESUPPORT,  ":/qe/plugin/QERecipe.png",            false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEResizeableFrame,   QETOOL,     ":/qe/plugin/QEResizeableFrame.png",   true)
QE_IMPLEMENT_PLUGIN_MANAGER (QEScratchPad,        QETOOL,     ":/qe/plugin/QEScratchPad.png",        false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEScript,            QESUPPORT,  ":/qe/plugin/QEScript.png",            false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEShape,             QEMONITOR,  ":/qe/plugin/QEShape.png",             false)
QE_IMPLEMENT_PLUGIN_MANAGER (QSimpleShape,        QETOOL,     ":/qe/plugin/QSimpleShape.png",        false)
QE_IMPLEMENT_PLUGIN_MANAGER (QESimpleShape,       QEMONITOR,  ":/qe/plugin/QESimpleShape.png",       false)
QE_IMPLEMENT_PLUGIN_MANAGER (QESlider,            QECONTROL,  ":/qe/plugin/QESlider.png",            false)
QE_IMPLEMENT_PLUGIN_MANAGER (QESpinBox,           QECONTROL,  ":/qe/plugin/QESpinBox.png",           false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEStripChart,        QEGRAPHIC,  ":/qe/plugin/QEStripChart.png",        false)
QE_IMPLEMENT_PLUGIN_MANAGER (QESubstitutedLabel,  QESUPPORT,  ":/qe/plugin/QESubstitutedLabel.png",  false)
QE_IMPLEMENT_PLUGIN_MANAGER (QETable,             QEMONITOR,  ":/qe/plugin/QETable.png",             false)
QE_IMPLEMENT_PLUGIN_MANAGER (QENTTable,           QEMONITOR,  ":/qe/plugin/QENTTable.png",           false)
QE_IMPLEMENT_PLUGIN_MANAGER (QESpectrogram,       QEMONITOR,  ":/qe/plugin/QESpectrogram.png",       false)
QE_IMPLEMENT_PLUGIN_MANAGER (QESelector,          QECONTROL,  ":/qe/plugin/QESelector.png",          false)
QE_IMPLEMENT_PLUGIN_MANAGER (QESurface,           QEMONITOR,  ":/qe/plugin/QESurface.png",           false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEWaterfall,         QEMONITOR,  ":/qe/plugin/QEWaterfall.png",         false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEArchiveNameSearch, QEARCHIVE,  ":/qe/plugin/QEArchiveNameSearch.png", false)
QE_IMPLEMENT_PLUGIN_MANAGER (QEArchiveStatus,     QEARCHIVE,  ":/qe/plugin/QEArchiveStatus.png",     false)

// end
