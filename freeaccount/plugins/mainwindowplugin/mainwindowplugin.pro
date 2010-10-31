TEMPLATE = lib
TARGET = MainWindow
PACKAGE_VERSION = 0.0.2

DEFINES += FACCOUNT_MAINWIN_LIBRARY

BUILD_PATH_POSTFIXE = FreeAccount

include(../../../plugins/fmf_plugins.pri)
include(mainwindowplugin_dependencies.pri)


HEADERS = mainwindowplugin.h \
    mainwindow_exporter.h \
    mainwindow.h \
    mainwindowpreferences.h

SOURCES = mainwindowplugin.cpp \
    mainwindow.cpp \
    mainwindowpreferences.cpp

FORMS = mainwindow.ui \
    mainwindowpreferenceswidget.ui

# Translators
TRANSLATIONS += $${SOURCES_TRANSLATIONS}/faccountmainwindowplugin_fr.ts \
                $${SOURCES_TRANSLATIONS}/faccountmainwindowplugin_de.ts \
                $${SOURCES_TRANSLATIONS}/faccountmainwindowplugin_es.ts

OTHER_FILES = MainWindow.pluginspec

