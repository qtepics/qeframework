# api.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#

INCLUDEPATH += \
    api

HEADERS += \
    api/Generic.h \
    api/CaRecord.h \
    api/CaRef.h \
    api/CaObject.h \
    api/CaConnection.h \
    api/CaObjectPrivate.h

SOURCES += \
    api/Generic.cpp \
    api/CaRecord.cpp \
    api/CaObject.cpp \
    api/CaConnection.cpp \
    api/CaRef.cpp

# end
