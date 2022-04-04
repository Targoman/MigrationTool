################################################################################
#   QBuildSystem
#
#   Copyright(c) 2021 by Targoman Intelligent Processing <http://tip.co.ir>
#
#   Redistribution and use in source and binary forms are allowed under the
#   terms of BSD License 2.0.
################################################################################
include($$QBUILD_PATH/templates/projectConfigs.pri)

addSubdirs(App)
#addSubdirs(unitTest)

# +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-#
OTHER_FILES += \
    README.md \
    INSTALL \
    LICENSE \
    migrationTool-bash-completion.bash \
    migrations/MigrationTool/db/* \
    migrations/MigrationTool/local/* \
    migrations/MigrationTool/local/.migrations \
    conf/* \
    install-bash-completion.sh \
    runMigrationTool-dev.sh \
    runMigrationTool.sh \
