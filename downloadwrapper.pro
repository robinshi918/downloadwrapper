QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

ICON = ym.icns

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    downloadstate.cpp \
    renamedialog.cpp \
    settingmanager.cpp \
    settingsdialog.cpp

HEADERS += \
    mainwindow.h \
    downloadstate.h \
    renamedialog.h \
    settingmanager.h \
    settingsdialog.h

FORMS += \
    mainwindow.ui \
    renamedialog.ui \
    settingsdialog.ui

TRANSLATIONS += \
    downloadwrapper_en_NL.ts

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
