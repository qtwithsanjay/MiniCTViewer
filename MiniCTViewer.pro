QT       += core gui widgets

TARGET   = MiniCTViewer
TEMPLATE = app

CONFIG   += c++17 warn_on

INCLUDEPATH += $$PWD/src

SOURCES += \
    $$PWD/src/main.cpp \
    $$PWD/src/MainWindow.cpp \
    $$PWD/src/CTVolumeModel.cpp \
    $$PWD/src/CTSliceView.cpp

HEADERS += \
    $$PWD/src/MainWindow.h \
    $$PWD/src/CTVolumeModel.h \
    $$PWD/src/CTSliceView.h

DESTDIR     = bin
OBJECTS_DIR = build/.obj
MOC_DIR     = build/.moc
RCC_DIR     = build/.rcc
UI_DIR      = build/.ui
