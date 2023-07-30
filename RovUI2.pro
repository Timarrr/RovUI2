QT       += core gui network multimedia multimediawidgets charts websockets svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20 console vulkan

QMAKE_CXXFLAGS += -Wno-c++20-extensions -Wall

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0



SOURCES += \
    cameralabel.cpp \
    camerasettings.cpp \
    instrumentcluster.cpp \
    joystickhandler.cpp \
    joysticksetupdialog.cpp \
    ledindicator.cpp \
    main.cpp \
    mainwindow.cpp \
    rovcameracapture.cpp \
    rovcommunication.cpp \
    rovdataparser.cpp \
    rovdatasplines.cpp \
    thrustersetupdialog.cpp \
    rovcameracommunication.cpp \
    client.cpp

HEADERS += \
    cameralabel.h \
    camerasettings.h \
    helpers.h \
    instrumentcluster.h \
    joystick.h \
    joystickhandler.h \
    joysticksetupdialog.h \
    ledindicator.h \
    mainwindow.h \
    rovcameracapture.h \
    rovcommunication.h \
    rovdataparser.h \
    rovdatasplines.h \
    rovdatatypes.h \
    thrustersetupdialog.h \
    rovcameracommunication.h \
    client.h

FORMS += \
    camerasettings.ui \
    joysticksetupdialog.ui \
    mainwindow2.ui \
    rovdatasplines.ui \
    dataparser.ui

RESOURCES += \
    resources.qrc


unix{
    CONFIG += link_pkgconfig
    PKGCONFIG += sdl2
}

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    Doxyfile
