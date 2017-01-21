# adaptation_parameters.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#

AP=adaptation_parameters

INCLUDEPATH += \
   $$AP

HEADERS += \
   $$AP/QEEnvironmentVariables.h \
   $$AP/QEOptions.h  \
   $$AP/QESettings.h  \
   $$AP/QEAdaptationParameters.h

SOURCES += \
   $$AP/QEEnvironmentVariables.cpp \
   $$AP/QEOptions.cpp  \
   $$AP/QESettings.cpp  \
   $$AP/QEAdaptationParameters.cpp

# end
