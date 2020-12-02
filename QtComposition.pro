QT -= gui

TEMPLATE = lib
DEFINES += QTCOMPOSITION_LIBRARY

CONFIG += c++14

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

include(../config.pri)

SOURCES += \
    metaobjectbuilder.cpp \
    qcomponentcontainer.cpp \
    qcomponentfactoryinterface.cpp \
    qcomponentregistry.cpp \
    qexport.cpp \
    qimport.cpp \
    qlazy.cpp \
    qpart.cpp \
    qpartmetaobject.cpp \
    qtestcomposition.cpp

HEADERS += \
    QtComposition_global.h \
    metaobjectbuilder.h \
    qcomponentcontainer.h \
    qcomponentfactoryinterface.h \
    qcomponentregistry.h \
    qexport.h \
    qimport.h \
    qlazy.h \
    qlazy.hpp \
    qpart.h \
    qpartmetaobject.h \
    qtestcomposition.h

includes.files = $$PWD/*.h $$PWD/*.hpp
win32 {
    includes.path = $$[QT_INSTALL_HEADERS]/QtComposition
    target.path = $$[QT_INSTALL_LIBS]
}
INSTALLS += includes

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target
