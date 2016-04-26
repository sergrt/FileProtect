#-------------------------------------------------
#
# Project created by QtCreator 2016-04-01T14:29:22
#
#-------------------------------------------------

QT       += core gui
QMAKE_CXXFLAGS += -std=c++11

VERSION = 0.8.9
DEFINES += VERSION=\\\"$$VERSION\\\"

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FileProtect
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    OptionsDialog.cpp \
    Options.cpp \
    WaitDialog.cpp \
    FileOperation.cpp \
    FilesSyncDialog.cpp \
    CryptoppUtils.cpp \
    ViewFileDialog.cpp \
    InputKeyDialog.cpp \
    AboutDialog.cpp

HEADERS  += MainWindow.h \
    OptionsDialog.h \
    Options.h \
    WaitDialog.h \
    FileOperation.h \
    FilesSyncDialog.h \
    CryptoppUtils.h \
    ViewFileDialog.h \
    InputKeyDialog.h \
    AboutDialog.h


FORMS    += MainWindow.ui \
    OptionsDialog.ui \
    WaitDialog.ui \
    FilesSyncDialog.ui \
    ViewFileDialog.ui \
    InputKeyDialog.ui \
    AboutDialog.ui

unix: LIBS += -L$$PWD/../cryptopp/ -lcryptopp
win32:debug: LIBS += -L$$PWD/../cryptopp/Win32/Output/Debug -lcryptlib
#win32:release: LIBS += -L$$PWD/../cryptopp/Win32/Output/Release -lcryptlib
win32:release: LIBS += -L$$PWD/../cryptopp/x64/Output/Release -lcryptlib

#INCLUDEPATH += $$PWD/../cryptopp
#DEPENDPATH += $$PWD/../cryptopp

#unix:!macx: PRE_TARGETDEPS += $$PWD/../cryptopp/libcryptopp.a
win32: RC_ICONS = MainIcon.ico
