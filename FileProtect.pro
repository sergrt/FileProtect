#-------------------------------------------------
#
# Project created by QtCreator 2016-04-01T14:29:22
#
#-------------------------------------------------

QT       += core gui
QMAKE_CXXFLAGS += -std=c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FileProtect
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    OptionsDialog.cpp \
    Options.cpp \
    WaitDialog.cpp \
    FilesSyncDialog.cpp

HEADERS  += MainWindow.h \
    OptionsDialog.h \
    Options.h \
    WaitDialog.h \
    FilesSyncDialog.h

FORMS    += MainWindow.ui \
    OptionsDialog.ui \
    WaitDialog.ui \
    FilesSyncDialog.ui


LIBS += -L$$PWD/../cryptopp/ -lcryptopp

#INCLUDEPATH += $$PWD/../cryptopp
#DEPENDPATH += $$PWD/../cryptopp

#unix:!macx: PRE_TARGETDEPS += $$PWD/../cryptopp/libcryptopp.a
