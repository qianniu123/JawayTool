QT       += core gui serialport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#need distinguish 32bit or 64bit gcc
#LIBS += -L$$PWD/../lib/32bit -lyaml-cpp  #mingw32
LIBS += -L$$PWD/../lib/64bit -lyaml-cpp #mingw64

INCLUDEPATH += $$PWD/../yaml-cpp/inc \
               $$PWD/inc

SOURCES += \
    main.cpp \
    src/controller.cpp \
    src/protocol.cpp \
    widget.cpp

HEADERS += \
    inc/controller.h \
    inc/protocol.h \
    widget.h

FORMS += \
    widget.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    Res/Res.qrc
