################################################################################
#   QBuildSystem
#
#   Copyright(c) 2021 by Targoman Intelligent Processing <http://tip.co.ir>
#
#   Redistribution and use in source and binary forms are allowed under the
#   terms of BSD License 2.0.
################################################################################
APP_NAME=migrationTool
include($$QBUILD_PATH/templates/innerAppConfigs.pri)

# +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-#
HEADERS += \
    appsrc/Commands/cmdCheckBC.h \
    appsrc/Commands/cmdNewDB.h \
    appsrc/Commands/cmdNewDBDiff.h \
    appsrc/Commands/cmdNewLocal.h \
    appsrc/Defs.h \
    appsrc/Configs.h \
    appsrc/CommandManager.h \
    appsrc/Commands/Common.h \
    appsrc/Commands/intfCommand.h \
    appsrc/Commands/cmdCommit.h \
    appsrc/Commands/cmdHistory.h \
    appsrc/Commands/cmdList.h \
    appsrc/Commands/cmdMark.h \
    appsrc/Commands/cmdShowConf.h

# +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-#
SOURCES += \
    appsrc/Commands/cmdCheckBC.cpp \
    appsrc/Commands/cmdNewDB.cpp \
    appsrc/Commands/cmdNewDBDiff.cpp \
    appsrc/Commands/cmdNewLocal.cpp \
    appsrc/Configs.cpp \
    appsrc/CommandManager.cpp \
    appsrc/Commands/cmdCommit.cpp \
    appsrc/Commands/cmdHistory.cpp \
    appsrc/Commands/cmdList.cpp \
    appsrc/Commands/cmdMark.cpp \
    appsrc/Commands/cmdShowConf.cpp \
    appsrc/main.cpp

OTHER_FILES +=

# +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-#
