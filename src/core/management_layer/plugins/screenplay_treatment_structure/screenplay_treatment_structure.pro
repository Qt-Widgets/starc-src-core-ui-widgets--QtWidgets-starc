TEMPLATE = lib

CONFIG += plugin c++1z
CONFIG += force_debug_info
CONFIG += separate_debug_info
QT += widgets

TARGET = screenplaytreatmentstructureplugin

DEFINES += MANAGER_PLUGIN
DEFINES += QT_DEPRECATED_WARNINGS

mac {
    DESTDIR = ../../../../_build/starcapp.app/Contents/PlugIns
    CORELIBDIR = ../../../../_build/starcapp.app/Contents/Frameworks
} else {
    DESTDIR = ../../../../_build/plugins
    CORELIBDIR = ../../../../_build
}

INCLUDEPATH += $$PWD/../../../..

#
# Подключаем библиотеку corelib
#

LIBS += -L$$CORELIBDIR/ -lcorelib
INCLUDEPATH += $$PWD/../../../../corelib
DEPENDPATH += $$PWD/../../../../corelib

#

HEADERS += \
    business_layer/screenplay_treatment_structure_model.h \
    screenplay_treatment_structure_manager.h \
    ui/screenplay_treatment_structure_delegate.h \
    ui/screenplay_treatment_structure_view.h

SOURCES += \
    business_layer/screenplay_treatment_structure_model.cpp \
    screenplay_treatment_structure_manager.cpp \
    ui/screenplay_treatment_structure_delegate.cpp \
    ui/screenplay_treatment_structure_view.cpp

mac {
    load(resolve_target)
    QMAKE_POST_LINK += install_name_tool -change libcorelib.1.dylib @executable_path/../Frameworks/libcorelib.dylib $$QMAKE_RESOLVED_TARGET
}
