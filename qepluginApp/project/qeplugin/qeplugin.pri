# QEWidgets.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework qeplugins.pro project file.
#
# Copyright (c) 2017-2018 Australian Synchrotron
#
# The EPICS QT Framework is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# The EPICS QT Framework is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with the EPICS QT Framework.  If not, see <http://www.gnu.org/licenses/>.
#
# Author:
#   Andrew Starritt
# Contact details:
#   andrew.starritt@synchrotron.org.au
#

INCLUDEPATH += qeplugin
OTHER_FILES += qeplugin/qeplugin.json

HEADERS += qeplugin/QEPluginLibrary_global.h
HEADERS += qeplugin/QEDesignerPluginGroupNames.h

HEADERS += qeplugin/QEDesignerPlugin.h
SOURCES += qeplugin/QEDesignerPlugin.cpp

HEADERS += qeplugin/QEPluginWidgetManagers.h
SOURCES += qeplugin/QEPluginWidgetManagers.cpp

RESOURCES   += qeplugin/qeplugin.qrc

OTHER_FILES += qeplugin/QAnalogSlider.png
OTHER_FILES += qeplugin/QBitStatus.png
OTHER_FILES += qeplugin/QEAnalogIndicator.png
OTHER_FILES += qeplugin/QEAnalogProgressBar.png
OTHER_FILES += qeplugin/QEAnalogSlider.png
OTHER_FILES += qeplugin/QEArchiveNameSearch.png
OTHER_FILES += qeplugin/QEArchiveStatus.png
OTHER_FILES += qeplugin/QEBitStatus.png
OTHER_FILES += qeplugin/QECalcout.png
OTHER_FILES += qeplugin/QECheckBox.png
OTHER_FILES += qeplugin/QEComboBox.png
OTHER_FILES += qeplugin/QEConfiguredLayout.png
OTHER_FILES += qeplugin/QEDescriptionLabel.png
OTHER_FILES += qeplugin/QEFileBrowser.png
OTHER_FILES += qeplugin/QEFileImage.png
OTHER_FILES += qeplugin/QEForm.png
OTHER_FILES += qeplugin/QEFormGrid.png
OTHER_FILES += qeplugin/QEFrame.png
OTHER_FILES += qeplugin/QEGeneralEdit.png
OTHER_FILES += qeplugin/QEGroupBox.png
OTHER_FILES += qeplugin/QEHistogram.png
OTHER_FILES += qeplugin/QEImage.png
OTHER_FILES += qeplugin/QELCDNumber.png
OTHER_FILES += qeplugin/QEMenuButton.png
OTHER_FILES += qeplugin/QENumericEdit.png
OTHER_FILES += qeplugin/QEPeriodic.png
OTHER_FILES += qeplugin/QEPlot.png
OTHER_FILES += qeplugin/QEPlotter.png
OTHER_FILES += qeplugin/QEPushButton.png
OTHER_FILES += qeplugin/QEPvFrame.png
OTHER_FILES += qeplugin/QEPvLoadSave.png
OTHER_FILES += qeplugin/QEPvProperties.png
OTHER_FILES += qeplugin/QERadioButton.png
OTHER_FILES += qeplugin/QERadioGroup.png
OTHER_FILES += qeplugin/QERecipe.png
OTHER_FILES += qeplugin/QEResizeableFrame.png
OTHER_FILES += qeplugin/QEScalarHistogram.png
OTHER_FILES += qeplugin/QEScratchPad.png
OTHER_FILES += qeplugin/QEScript.png
OTHER_FILES += qeplugin/QEShape.png
OTHER_FILES += qeplugin/QESimpleShape.png
OTHER_FILES += qeplugin/QESlider.png
OTHER_FILES += qeplugin/QESpinBox.png
OTHER_FILES += qeplugin/QEStripChart.png
OTHER_FILES += qeplugin/QESubstitutedLabel.png
OTHER_FILES += qeplugin/QETable.png
OTHER_FILES += qeplugin/QEWaveformHistogram.png
OTHER_FILES += qeplugin/QNumericEdit.png
OTHER_FILES += qeplugin/QRadioGroup.png
OTHER_FILES += qeplugin/QSimpleShape.png
OTHER_FILES += qeplugin/QELabel.png
OTHER_FILES += qeplugin/QELineEdit.png
OTHER_FILES += qeplugin/QELog.png
OTHER_FILES += qeplugin/QELogin.png
OTHER_FILES += qeplugin/QELink.png

# end
