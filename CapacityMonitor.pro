#-------------------------------------------------
#
# Project created by QtCreator 2020-09-01T13:54:44
#
#-------------------------------------------------

QT       += core network sql

TARGET = CapacityMonitor
CONFIG   += console qt

TEMPLATE = app

SOURCES += main.cpp \
    databasehelper.cpp \
    configini.cpp \
    capacitymonitor.cpp \
    CapacityMonitorService.cpp \
    CapacityMonitor_types.cpp \
    CapacityMonitor_constants.cpp \
    rtdbapp.cpp \
    LeastSquareSurface.cpp \
    LeastSquaresCurve.cpp

MOC_DIR     = temp/moc
RCC_DIR     = temp/rcc
UI_DIR      = temp/ui
OBJECTS_DIR = temp/obj

include(E:/opensouce/qt-solutions-master/qtservice/src/qtservice.pri)

HEADERS += \
    databasehelper.h \
    configini.h \
    capacitymonitor.h \
    CapacityMonitorService.h \
    CapacityMonitor_types.h \
    CapacityMonitor_constants.h \
    rtdbapp.h \
    LeastSquareSurface.h \
    LeastSquaresCurve.h


win32: LIBS += -L$$PWD/../xdb/lib/ -lRTDB

INCLUDEPATH += $$PWD/../xdb/include
DEPENDPATH += $$PWD/../xdb/include

win32: PRE_TARGETDEPS += $$PWD/../xdb/lib/RTDB.lib

win32: LIBS += -L$$PWD/../thrift/lib/ -llibthrift

INCLUDEPATH += $$PWD/../thrift/include
DEPENDPATH += $$PWD/../thrift/include

LIBS += -L$$PWD/../boost_1_55_0/lib -llibboost_thread-vc100-mt-gd-1_55

INCLUDEPATH += $$PWD/../boost_1_55_0

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../OpenSSL-Win32/lib/ -llibeay32
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../OpenSSL-Win32/lib/ -llibeay32

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../OpenSSL-Win32/lib/ -lssleay32
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../OpenSSL-Win32/lib/ -lssleay32

INCLUDEPATH += $$PWD/../OpenSSL-Win32/include


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/../opencv/lib/ -lopencv_world320
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/../opencv/lib/ -lopencv_world320d

INCLUDEPATH += $$PWD/../opencv/include
DEPENDPATH += $$PWD/../opencv/include
