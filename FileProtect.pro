#-------------------------------------------------
#
# Project created by QtCreator 2016-04-01T14:29:22
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FileProtect
TEMPLATE = app


SOURCES += main.cpp\
        MainWindow.cpp \
    OptionsDialog.cpp \
    Options.cpp \
    WaitDialog.cpp

HEADERS  += MainWindow.h \
    OptionsDialog.h \
    Options.h \
    WaitDialog.h

FORMS    += MainWindow.ui \
    OptionsDialog.ui \
    WaitDialog.ui


LIBS += -L$$PWD/../cryptopp/ -lcryptopp

#INCLUDEPATH += $$PWD/../cryptopp
#DEPENDPATH += $$PWD/../cryptopp

#unix:!macx: PRE_TARGETDEPS += $$PWD/../cryptopp/libcryptopp.a
