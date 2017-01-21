# QEWidgets.pri
#
# This file is part of the EPICS QT Framework, and included into
# and as part of the overall framework plugins.pro project file.
#

INCLUDEPATH += \
    widgets/QEWidget

HEADERS += \
    widgets/QEWidget/VariableManager.h \
    widgets/QEWidget/VariableNameManager.h \
    widgets/QEWidget/UserMessage.h \
    widgets/QEWidget/contextMenu.h \
    widgets/QEWidget/QEWidget.h \
    widgets/QEWidget/QEWidgetProperties.h \
    widgets/QEWidget/managePixmaps.h \
    widgets/QEWidget/QEDragDrop.h \
    widgets/QEWidget/styleManager.h \
    widgets/QEWidget/standardProperties.h \
    widgets/QEWidget/QESingleVariableMethods.h \
    widgets/QEWidget/QEStringFormattingMethods.h \
    widgets/QEWidget/QEToolTip.h \
    widgets/QEWidget/QEDesignerPlugin.h \
    widgets/QEWidget/QEDesignerPluginCommon.h \
    widgets/QEWidget/ContainerProfile.h \
    widgets/QEWidget/QEPluginLibrary_global.h \
    widgets/QEWidget/persistanceManager.h \
    widgets/QEWidget/applicationLauncher.h \
    widgets/QEWidget/QEEmitter.h

SOURCES += \
    widgets/QEWidget/VariableManager.cpp \
    widgets/QEWidget/VariableNameManager.cpp \
    widgets/QEWidget/UserMessage.cpp \
    widgets/QEWidget/contextMenu.cpp \
    widgets/QEWidget/QEWidget.cpp \
    widgets/QEWidget/QEWidgetProperties.cpp \
    widgets/QEWidget/managePixmaps.cpp \
    widgets/QEWidget/QEDragDrop.cpp \
    widgets/QEWidget/styleManager.cpp \
    widgets/QEWidget/standardProperties.cpp \
    widgets/QEWidget/QESingleVariableMethods.cpp \
    widgets/QEWidget/QEStringFormattingMethods.cpp \
    widgets/QEWidget/QEToolTip.cpp \
    widgets/QEWidget/QEDesignerPlugin.cpp \
    widgets/QEWidget/ContainerProfile.cpp \
    widgets/QEWidget/persistanceManager.cpp \
    widgets/QEWidget/applicationLauncher.cpp \
    widgets/QEWidget/QEEmitter.cpp

OTHER_FILES += \
    widgets/QEWidget/qeplugin.json

# end
