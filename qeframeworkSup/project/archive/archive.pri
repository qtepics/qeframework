# QEArchive.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#
QT += xml network

_QE_ARCHIVE_LIST = $$(QE_ARCHIVE_LIST)
isEmpty( _QE_ARCHIVE_LIST ) {
    warning( "QE_ARCHIVE_LIST is not defined. That's OK, but if you want to be able to access the PV Channel Archive data, e.g. to backfill" )
    warning( "the QEStripChart widget from you should define this environment variable to be a space delimited list of your archiver URLs." )
    warning( "For example: 'archiver.synchrotron.org.au:80/cgi-bin/ArchiveDataServer1.cgi archiver.synchrotron.org.au:80/cgi-bin/ArchiveDataServer2.cgi'" )
}

OTHER_FILES += \
    archive/archive_status.png \
    archive/drag_icon.png   \
    archive/name_search.png

RESOURCES += \
    archive/archive.qrc

HEADERS += \
    archive/maiaFault.h \
    archive/maiaObject.h \
    archive/maiaXmlRpcClient.h \
    archive/QEArchiveInterface.h \
    archive/QEArchiveManager.h \
    archive/QEArchiveStatus.h  \
    archive/QEArchiveStatusManager.h \
    archive/QEArchiveNameSearch.h  \
    archive/QEArchiveNameSearchManager.h


SOURCES += \
    archive/maiaFault.cpp \
    archive/maiaObject.cpp \
    archive/maiaXmlRpcClient.cpp \
    archive/QEArchiveInterface.cpp \
    archive/QEArchiveManager.cpp \
    archive/QEArchiveStatus.cpp  \
    archive/QEArchiveStatusManager.cpp  \
    archive/QEArchiveNameSearch.cpp  \
    archive/QEArchiveNameSearchManager.cpp

INCLUDEPATH += \
    archive

# end
