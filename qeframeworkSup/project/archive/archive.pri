# QEArchive.pri
#
# This file is part of the EPICS QT Framework, initially developed at
# the Australian Synchrotron. This file is included into and as part
# of the overall framework.pro project file.
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
# Copyright (c) 2017 Australian Synchrotron
#
# Author:
#   Andrew Starritt
# Contact details:
#   andrew.starritt@synchrotron.org.au
#

QT += xml network

_QE_ARCHIVE_LIST = $$(QE_ARCHIVE_LIST)
isEmpty( _QE_ARCHIVE_LIST ) {
    warning( "QE_ARCHIVE_LIST is not defined. That's OK, but if you want to be able to access the PV Channel Archive data, e.g. to backfill" )
    warning( "the QEStripChart widget from you should define this environment variable to be a space delimited list of your archiver URLs." )
    warning( "For example: 'archiver.synchrotron.org.au:80/cgi-bin/ArchiveDataServer1.cgi archiver.synchrotron.org.au:80/cgi-bin/ArchiveDataServer2.cgi'" )
}

OTHER_FILES += \
    archive/drag_icon.png

RESOURCES += \
    archive/archive.qrc

HEADERS += \
    archive/maiaFault.h \
    archive/maiaObject.h \
    archive/maiaXmlRpcClient.h \
    archive/QEArchiveAccess.h \
    archive/QEArchiveInterface.h \
    archive/QEArchiveManager.h \
    archive/QEArchiveStatus.h  \
    archive/QEArchiveNameSearch.h


SOURCES += \
    archive/maiaFault.cpp \
    archive/maiaObject.cpp \
    archive/maiaXmlRpcClient.cpp \
    archive/QEArchiveAccess.cpp \
    archive/QEArchiveInterface.cpp \
    archive/QEArchiveManager.cpp \
    archive/QEArchiveStatus.cpp  \
    archive/QEArchiveNameSearch.cpp

INCLUDEPATH += \
    archive

# end
