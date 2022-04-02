# common.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
#
# Copyright (c) 2017-2022 Australian Synchrotron
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

HEADERS += \
   common/QEActionRequests.h \
   common/QEAxisIterator.h \
   common/QEAxisPainter.h \
   common/QEColourBandList.h \
   common/QECommon.h \
   common/QEDialog.h \
   common/QEDelayedText.h \
   common/QEDisplayRanges.h \
   common/QEExpressionEvaluation.h  \
   common/QEFileMonitor.h  \
   common/QEFixedPointRadix.h \
   common/QEFormMapper.h \
   common/QEFrameworkVersion.h \
   common/QEPvNameSearch.h \
   common/QEPVNameSelectDialog.h \
   common/QEGraphic.h \
   common/QEGraphicMarkup.h \
   common/QEGraphicNames.h \
   common/QEOneToOne.h \
   common/QEPlatform.h \
   common/QEQuickSort.h \
   common/QERecordFieldName.h \
   common/QEScaling.h \
   common/QEScanTimers.h \
   common/QETwinScaleSelectDialog.h \
   common/PasswordDialog.h \
   common/windowCustomisation.h \
   common/macroSubstitution.h

SOURCES += \
   common/QEActionRequests.cpp \
   common/QEAxisIterator.cpp \
   common/QEAxisPainter.cpp \
   common/QEColourBandList.cpp \
   common/QECommon.cpp \
   common/QEDialog.cpp \
   common/QEDelayedText.cpp \
   common/QEDisplayRanges.cpp \
   common/QEExpressionEvaluation.cpp  \
   common/QEFileMonitor.cpp  \
   common/QEFixedPointRadix.cpp \
   common/QEFormMapper.cpp \
   common/QEFrameworkVersion.cpp \
   common/QEPvNameSearch.cpp \
   common/QEPVNameSelectDialog.cpp \
   common/QEGraphic.cpp \
   common/QEGraphicMarkup.cpp \
   common/QEGraphicNames.cpp \
   common/QEPlatform.cpp \
   common/QEQuickSort.cpp \
   common/QERecordFieldName.cpp \
   common/QEScaling.cpp \
   common/QEScanTimers.cpp \
   common/QETwinScaleSelectDialog.cpp \
   common/PasswordDialog.cpp \
   common/windowCustomisation.cpp \
   common/macroSubstitution.cpp

INCLUDEPATH += \
   common

# Explicity add common, and hence QEFrameworkVersion.h, to the dependacy path
# So that changes to the version/release numbers force relevent recompilations.
#
DEPENDPATH += common

FORMS += \
   common/QEPVNameSelectDialog.ui \
   common/QETwinScaleSelectDialog.ui \
   common/PasswordDialog.ui

# end
