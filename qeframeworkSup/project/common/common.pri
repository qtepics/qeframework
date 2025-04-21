# common.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
#
# Copyright (c) 2017-2023 Australian Synchrotron
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
#   andrews@ansto.gov.au
#

INCLUDEPATH += $$PWD

# Explicity add common, and hence QEFrameworkVersion.h, to the dependacy path
# So that changes to the version/release numbers force relevent recompilations.
#
DEPENDPATH += common

HEADERS += $$PWD/PasswordDialog.h
SOURCES += $$PWD/PasswordDialog.cpp
FORMS   += $$PWD/PasswordDialog.ui

HEADERS += $$PWD/QEActionRequests.h
SOURCES += $$PWD/QEActionRequests.cpp

HEADERS += $$PWD/QEAxisIterator.h
SOURCES += $$PWD/QEAxisIterator.cpp

HEADERS += $$PWD/QEAxisPainter.h
SOURCES += $$PWD/QEAxisPainter.cpp

HEADERS += $$PWD/QEColourBandList.h
SOURCES += $$PWD/QEColourBandList.cpp

HEADERS += $$PWD/QECommon.h
SOURCES += $$PWD/QECommon.cpp

HEADERS += $$PWD/QEDelayedText.h
SOURCES += $$PWD/QEDelayedText.cpp

HEADERS += $$PWD/QEDialog.h
SOURCES += $$PWD/QEDialog.cpp

HEADERS += $$PWD/QEDisplayRanges.h
SOURCES += $$PWD/QEDisplayRanges.cpp

HEADERS += $$PWD/QEEnums.h

HEADERS += $$PWD/QEExpressionEvaluation.h
SOURCES += $$PWD/QEExpressionEvaluation.cpp

HEADERS += $$PWD/QEFileMonitor.h
SOURCES += $$PWD/QEFileMonitor.cpp

HEADERS += $$PWD/QEFixedPointRadix.h
SOURCES += $$PWD/QEFixedPointRadix.cpp

HEADERS += $$PWD/QEFormMapper.h
SOURCES += $$PWD/QEFormMapper.cpp

HEADERS += $$PWD/QEFrameworkVersion.h
SOURCES += $$PWD/QEFrameworkVersion.cpp

HEADERS += $$PWD/QEGraphic.h
SOURCES += $$PWD/QEGraphic.cpp

HEADERS += $$PWD/QEGraphicMarkup.h
SOURCES += $$PWD/QEGraphicMarkup.cpp

HEADERS += $$PWD/QEGraphicNames.h
SOURCES += $$PWD/QEGraphicNames.cpp

HEADERS += $$PWD/QEOneToOne.h

HEADERS += $$PWD/QEPVNameSelectDialog.h
SOURCES += $$PWD/QEPVNameSelectDialog.cpp
FORMS   += $$PWD/QEPVNameSelectDialog.ui

HEADERS += $$PWD/QEPlatform.h
SOURCES += $$PWD/QEPlatform.cpp

HEADERS += $$PWD/QEPvNameSearch.h
SOURCES += $$PWD/QEPvNameSearch.cpp

HEADERS += $$PWD/QEPvWriteOnce.h
SOURCES += $$PWD/QEPvWriteOnce.cpp

HEADERS += $$PWD/QEQuickSort.h
SOURCES += $$PWD/QEQuickSort.cpp

HEADERS += $$PWD/QERecordFieldName.h
SOURCES += $$PWD/QERecordFieldName.cpp

HEADERS += $$PWD/QEScaling.h
SOURCES += $$PWD/QEScaling.cpp

HEADERS += $$PWD/QEScanTimers.h
SOURCES += $$PWD/QEScanTimers.cpp

HEADERS += $$PWD/QEThreadSafeQueue.h

HEADERS += $$PWD/QETwinScaleSelectDialog.h
SOURCES += $$PWD/QETwinScaleSelectDialog.cpp
FORMS   += $$PWD/QETwinScaleSelectDialog.ui

HEADERS += $$PWD/macroSubstitution.h
SOURCES += $$PWD/macroSubstitution.cpp

HEADERS += $$PWD/windowCustomisation.h
SOURCES += $$PWD/windowCustomisation.cpp

# end
