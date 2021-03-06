DEFINES *= ACCOUNTBASE_LIBRARY

include(../fmf_plugins.pri)
include(accountbaseplugin_dependencies.pri)

!with-old-account {
  error(AccountBase Old version plugin included but not in the config var of the qmake process)
} else {
  message(Building Old AccountBase plugin)
}

HEADERS += $${PWD}/accountbaseplugin.h \
    $${PWD}/accountbase_exporter.h \
    $${PWD}/accountbase.h \
    $${PWD}/constants.h \
    $${PWD}/bankaccountmodel.h \
    $${PWD}/availablemovementmodel.h \
    $${PWD}/workingplacesmodel.h \
    $${PWD}/insurancemodel.h \
    $${PWD}/accountmodel.h \
    $${PWD}/assetmodel.h \
    $${PWD}/depositmodel.h \
    $${PWD}/movementmodel.h \
    $${PWD}/percentmodel.h \
    $${PWD}/rulesmodel.h \
    $${PWD}/distancerulesmodel.h \
    $${PWD}/medicalproceduremodel.h \
    $${PWD}/accountdata.h \
    $${PWD}/thesaurusmodel.h \
    $${PWD}/actsmodel.h \
    $${PWD}/assetsratesmodel.h \
    $${PWD}/amountmodel.h \
    $${PWD}/datapackmodel.h
    

SOURCES += $${PWD}/accountbaseplugin.cpp \
    $${PWD}/accountbase.cpp \
    $${PWD}/bankaccountmodel.cpp \
    $${PWD}/availablemovementmodel.cpp \
    $${PWD}/workingplacesmodel.cpp \
    $${PWD}/insurancemodel.cpp \
    $${PWD}/accountmodel.cpp \
    $${PWD}/assetmodel.cpp \
    $${PWD}/depositmodel.cpp \
    $${PWD}/movementmodel.cpp \
    $${PWD}/percentmodel.cpp \
    $${PWD}/rulesmodel.cpp \
    $${PWD}/distancerulesmodel.cpp \
    $${PWD}/medicalproceduremodel.cpp \
    $${PWD}/accountdata.cpp \
    $${PWD}/thesaurusmodel.cpp \
    $${PWD}/actsmodel.cpp \
    $${PWD}/assetsratesmodel.cpp \
    $${PWD}/amountmodel.cpp \
    $${PWD}/datapackmodel.cpp

# include translations
TRANSLATION_NAME = accountbase
include($${SOURCES_ROOT_PATH}/buildspecs/translations.pri)
