# QEPvLoadSave.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
#
# SPDX-FileCopyrightText: 2017-2025 Australian Synchrotron
# SPDX-License-Identifier: LGPL-3.0-only
#
# Author:     Andrew Starritt
# Maintainer: Andrew Starritt
# Contact:    andrews@ansto.gov.au
#

QEPVLOADSAVE = widgets/QEPvLoadSave

INCLUDEPATH += $$QEPVLOADSAVE

HEADERS += $$QEPVLOADSAVE/QEPVLoadSaveNameSelectDialog.h
SOURCES += $$QEPVLOADSAVE/QEPVLoadSaveNameSelectDialog.cpp
FORMS   += $$QEPVLOADSAVE/QEPVLoadSaveNameSelectDialog.ui

HEADERS += $$QEPVLOADSAVE/QEPvLoadSave.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSave.cpp

HEADERS += $$QEPVLOADSAVE/QEPvLoadSaveAccessFail.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSaveAccessFail.cpp
FORMS   += $$QEPVLOADSAVE/QEPvLoadSaveAccessFail.ui

HEADERS += $$QEPVLOADSAVE/QEPvLoadSaveCommon.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSaveCommon.cpp

HEADERS += $$QEPVLOADSAVE/QEPvLoadSaveCompare.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSaveCompare.cpp
FORMS   += $$QEPVLOADSAVE/QEPvLoadSaveCompare.ui

HEADERS += $$QEPVLOADSAVE/QEPvLoadSaveGroupNameDialog.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSaveGroupNameDialog.cpp
FORMS   += $$QEPVLOADSAVE/QEPvLoadSaveGroupNameDialog.ui

HEADERS += $$QEPVLOADSAVE/QEPvLoadSaveItem.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSaveItem.cpp

HEADERS += $$QEPVLOADSAVE/QEPvLoadSaveModel.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSaveModel.cpp

HEADERS += $$QEPVLOADSAVE/QEPvLoadSaveTimeDialog.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSaveTimeDialog.cpp
FORMS   += $$QEPVLOADSAVE/QEPvLoadSaveTimeDialog.ui

HEADERS += $$QEPVLOADSAVE/QEPvLoadSaveUtilities.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSaveUtilities.cpp

HEADERS += $$QEPVLOADSAVE/QEPvLoadSaveValueEditDialog.h
SOURCES += $$QEPVLOADSAVE/QEPvLoadSaveValueEditDialog.cpp
FORMS   += $$QEPVLOADSAVE/QEPvLoadSaveValueEditDialog.ui

OTHER_FILES += $$QEPVLOADSAVE/archive_time.png
OTHER_FILES += $$QEPVLOADSAVE/copy_all.png
OTHER_FILES += $$QEPVLOADSAVE/copy_subset.png
OTHER_FILES += $$QEPVLOADSAVE/read_all.png
OTHER_FILES += $$QEPVLOADSAVE/read_subset.png
OTHER_FILES += $$QEPVLOADSAVE/write_all.png
OTHER_FILES += $$QEPVLOADSAVE/write_subset.png
OTHER_FILES += $$QEPVLOADSAVE/ypoc_all.png
OTHER_FILES += $$QEPVLOADSAVE/ypoc_subset.png

RESOURCES += $$QEPVLOADSAVE/QEPvLoadSave.qrc

# end
