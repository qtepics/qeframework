# QEPvLoadSave.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
#
# SPDX-FileCopyrightText: 2017-2026 Australian Synchrotron
# SPDX-License-Identifier: LGPL-3.0-only
#
# Author:     Andrew Starritt
# Maintainer: Andrew Starritt
# Contact:    andrews@ansto.gov.au
#

INCLUDEPATH += $$PWD

HEADERS += $$PWD/QEPVLoadSaveNameSelectDialog.h
SOURCES += $$PWD/QEPVLoadSaveNameSelectDialog.cpp
FORMS   += $$PWD/QEPVLoadSaveNameSelectDialog.ui

HEADERS += $$PWD/QEPvLoadSave.h
SOURCES += $$PWD/QEPvLoadSave.cpp

HEADERS += $$PWD/QEPvLoadSaveAccessFail.h
SOURCES += $$PWD/QEPvLoadSaveAccessFail.cpp
FORMS   += $$PWD/QEPvLoadSaveAccessFail.ui

HEADERS += $$PWD/QEPvLoadSaveCommon.h
SOURCES += $$PWD/QEPvLoadSaveCommon.cpp

HEADERS += $$PWD/QEPvLoadSaveCompare.h
SOURCES += $$PWD/QEPvLoadSaveCompare.cpp
FORMS   += $$PWD/QEPvLoadSaveCompare.ui

HEADERS += $$PWD/QEPvLoadSaveDelayEditDialog.h
SOURCES += $$PWD/QEPvLoadSaveDelayEditDialog.cpp
FORMS   += $$PWD/QEPvLoadSaveDelayEditDialog.ui

HEADERS += $$PWD/QEPvLoadSaveGroupNameDialog.h
SOURCES += $$PWD/QEPvLoadSaveGroupNameDialog.cpp
FORMS   += $$PWD/QEPvLoadSaveGroupNameDialog.ui

FORMS   += $$PWD/QEPvLoadSaveHalf.ui

HEADERS += $$PWD/QEPvLoadSaveItem.h
SOURCES += $$PWD/QEPvLoadSaveItem.cpp

HEADERS += $$PWD/QEPvLoadSaveModel.h
SOURCES += $$PWD/QEPvLoadSaveModel.cpp

HEADERS += $$PWD/QEPvLoadSaveTimeDialog.h
SOURCES += $$PWD/QEPvLoadSaveTimeDialog.cpp
FORMS   += $$PWD/QEPvLoadSaveTimeDialog.ui

HEADERS += $$PWD/QEPvLoadSaveUtilities.h
SOURCES += $$PWD/QEPvLoadSaveUtilities.cpp

HEADERS += $$PWD/QEPvLoadSaveValueEditDialog.h
SOURCES += $$PWD/QEPvLoadSaveValueEditDialog.cpp
FORMS   += $$PWD/QEPvLoadSaveValueEditDialog.ui

OTHER_FILES += $$PWD/archive_time.png
OTHER_FILES += $$PWD/copy_all.png
OTHER_FILES += $$PWD/copy_subset.png
OTHER_FILES += $$PWD/read_all.png
OTHER_FILES += $$PWD/read_subset.png
OTHER_FILES += $$PWD/write_all.png
OTHER_FILES += $$PWD/write_subset.png
OTHER_FILES += $$PWD/ypoc_all.png
OTHER_FILES += $$PWD/ypoc_subset.png

RESOURCES += $$PWD/QEPvLoadSave.qrc

# end
