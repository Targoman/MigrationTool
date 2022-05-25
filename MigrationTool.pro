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
    .gitignore \
    conf/* \
    migrationTool-bash-completion.bash \
    install-bash-completion.sh \
    runMigrationTool-dev.sh \
    runMigrationTool.sh \
    migrations/MigrationTool/db/* \
    migrations/MigrationTool/db/.dbdiff* \
    migrations/MigrationTool/local/* \
    migrations/MigrationTool/local/.migrations \
    migrations/ttt1/db/* \
    migrations/ttt1/db/.dbdiff* \
    migrations/ttt1/local/* \
    migrations/ttt1/local/.migrations \
    migrations/ttt2/db/* \
    migrations/ttt2/db/.dbdiff* \
    migrations/ttt2/local/* \
    migrations/ttt2/local/.migrations \
    migrations/ttt3/db/* \
    migrations/ttt3/db/.dbdiff* \
    migrations/ttt3/local/* \
    migrations/ttt3/local/.migrations \
