/******************************************************************************
#   MigrationTool
#
#   Copyright 2014-2020 by Targoman Intelligent Processing <http://tip.co.ir>
#
#   MigrationTool is free software: you can redistribute it and/or modify
#   it under the terms of the GNU AFFERO GENERAL PUBLIC LICENSE as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   MigrationTool is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU AFFERO GENERAL PUBLIC LICENSE for more details.
#
#   You should have received a copy of the GNU AFFERO GENERAL PUBLIC LICENSE
#   along with Targoman. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************/
/**
 * @author S.Mehran M.Ziabary <ziabary@targoman.com>
 * @author Kambiz Zandi <kambizzandi@gmail.com>
 */

#include "Configs.h"
#include "libTargomanCommon/Configuration/Validators.hpp"

namespace Targoman::Migrate {

using namespace Targoman::Common;
using namespace Targoman::Common::Configuration;

tmplConfigurable<enuAppCommand::Type> Configs::Command(
    Configs::makeConfig("Command"),
    R"(Application command:
                    showconf    : Show Migrations Config
                    newdb       : Creating new global migration (store in /migrations/.../db/)
                    newlocal    : Creating new local migration (store in /migrations/.../local/)
                    list        : List of unapplied migrations
                    history     : List of applied migrations
                    commit      : Apply unapplied migrations
                    mark        : Set migrations as applied to the specified point (not actually run migrations)
)",
//newdbdiff      : Creating new global database migration and fills by libTargomanDBCompare (store in /migrations/db/)
//rollback          : Rollback applied migrations
    enuAppCommand::list,
    ReturnTrueCrossValidator(),
    "",
    "COMMAND",
    "command",
    enuConfigSource::Arg
);

tmplConfigurable<QString> Configs::MigrationsFolderName(
    Configs::makeConfig("MigrationsFolderName"),
    "Relative folder name for creating migration files",
    "migrations",
    ReturnTrueCrossValidator(),
    "",
    "PATH",
    "migrations-folder",
    enuConfigSource::Arg | enuConfigSource::File
);

tmplConfigurable<QString> Configs::GlobalHistoryTableName(
    Configs::makeConfig("GlobalHistoryTableName"),
    "Table name for storing global migration history",
    "tblMigrations",
    ReturnTrueCrossValidator(),
    "",
    "TABLENAME",
    "global-history-table",
    enuConfigSource::Arg | enuConfigSource::File
);

tmplConfigurable<QString> Configs::LocalHistoryFileName(
    Configs::makeConfig("LocalHistoryFileName"),
    "File name for storing local migration history",
    ".migrations",
    ReturnTrueCrossValidator(),
    "",
    "FILE",
    "local-history-file",
    enuConfigSource::Arg | enuConfigSource::File
);

tmplConfigurable<QString> Configs::DBPrefix(
    Configs::makeConfig("DBPrefix"),
    "Prefix to prepend to all database names",
    "",
    ReturnTrueCrossValidator(),
    "",
    "PREFIX",
    "dbprefix",
    enuConfigSource::Arg | enuConfigSource::File
);

tmplConfigurable<QString> Configs::NewDBCollation(
    Configs::makeConfig("NewDBCollation"),
    "New DB collation",
    "utf8mb4_general_ci",
    ReturnTrueCrossValidator(),
    "",
    "COLLATION",
    "new-db-collation",
    enuConfigSource::Arg | enuConfigSource::File
);

tmplConfigurableArray<stuDBServer> Configs::DBServers(
    Configs::makeConfig("DBServers"),
    "DB Servers",
    1
);

tmplConfigurableArray<stuRunningMode> Configs::RunningModes(
    Configs::makeConfig("RunningModes"),
    "Running modes",
    1
);

tmplConfigurable<QString> Configs::ActiveRunningMode(
    Configs::makeConfig("ActiveRunningMode"),
    "Active running mode",
    "",
    ReturnTrueCrossValidator(),
    "r",
    "MODE",
    "active-running-mode",
    enuConfigSource::Arg
);

tmplConfigurableArray<stuProject> Configs::Projects(
    Configs::makeConfig("Projects"),
    "Projects",
    1
);

//tmplConfigurable<QString> Configs::ApplyToAllSourceName(
//    Configs::makeConfig("ApplyToAllSourceName"),
//    "Source name for migrations set that must applied to the all other sources",
//    "MigrationTool",
//    ReturnTrueCrossValidator(),
//    "",
//    "NAME",
//    "apply-to-all-source-name",
//    enuConfigSource::Arg | enuConfigSource::File
//);

tmplConfigurable<QString> Configs::Project(
    Configs::makeConfig("Project"),
    "Project to focus",
    "",
    ReturnTrueCrossValidator(),
    "",
    "PROJECT",
    "project",
    enuConfigSource::Arg | enuConfigSource::File
);

tmplConfigurable<bool> Configs::DBOnly(
    Configs::makeConfig("DBOnly"),
    "Only db migrations",
    false,
    ReturnTrueCrossValidator(),
    "",
    "",
    "db-only",
    enuConfigSource::Arg
);

tmplConfigurable<bool> Configs::LocalOnly(
    Configs::makeConfig("LocalOnly"),
    "Only local migrations",
    false,
    ReturnTrueCrossValidator(),
    "",
    "",
    "local-only",
    enuConfigSource::Arg
);

tmplConfigurable<bool> Configs::All(
    Configs::makeConfig("All"),
    "Turn All switch to on for (commit/mark/checkbc) commands",
    false,
    ReturnTrueCrossValidator(),
    "",
    "",
    "all",
    enuConfigSource::Arg
);

tmplConfigurable<QString> Configs::DefaultEditor(
    Configs::makeConfig("DefaultEditor"),
    "Default text editor",
    "vim",
    ReturnTrueCrossValidator(),
    "",
    "EDITOR",
    "default-editor",
    enuConfigSource::Arg | enuConfigSource::File
);

tmplConfigurable<bool> Configs::AutoGitAdd(
    Configs::makeConfig("AutoGitAdd"),
    "Auto add new files to the git",
    false,
    ReturnTrueCrossValidator(),
    "",
    "",
    "git-add",
    enuConfigSource::Arg | enuConfigSource::File
);

Configs::stuRunningParameters Configs::RunningParameters;

void Configs::FillRunningParameters() {
    //1: find RunningMode:
    if (Configs::ActiveRunningMode.value().isEmpty())
        throw exTargomanBase("Active Running Mode not defined");

    int RunningModeIndex = -1;

    for (size_t idxRunningModes=0; idxRunningModes<Configs::RunningModes.size(); idxRunningModes++) {
        stuRunningMode &RunningMode = Configs::RunningModes[idxRunningModes];

        if (RunningMode.Name.value() == Configs::ActiveRunningMode.value()) {
            RunningModeIndex = idxRunningModes;
            break;
        }
    }

    if (RunningModeIndex < 0)
        throw exTargomanBase("Running Mode not found");

    Configs::RunningParameters.RunningModeDBServers = Configs::RunningModes[RunningModeIndex].DBServers.value();

    if (Configs::RunningParameters.RunningModeDBServers.isEmpty() == false) {
        //2: find DBServers:
        quint32 dbIdx = 0;
        for (size_t idxDBServers=0; idxDBServers<Configs::DBServers.size(); idxDBServers++) {
            stuDBServer &DBServer = Configs::DBServers[idxDBServers];

            foreach (QString DBServerName, Configs::RunningParameters.RunningModeDBServers) {
                if (DBServerName == DBServer.Name.value()) {
                    bool DBServerHasProjects = false;

                    //projects
                    for (size_t idxProjects=0; idxProjects<Configs::Projects.size(); idxProjects++) {
                        stuProject &Project = Configs::Projects[idxProjects];

                        if ((Configs::Project.value().isEmpty() == false)
                                && (Project.Name.value() != Configs::Project.value())
                                && (Project.ApplyToTags.value().isEmpty())
                            )
                            continue;

//                        qDebug() << "lookup" << DBServerName << "in" << Project.DBDestinations.value(); //.join("|");

                        if (Project.AllowDB.value()
                                && (Project.DBDestinations.value().isEmpty() == false)
                                && Project.DBDestinations.value().contains(DBServerName)
                            ) {
//                            qDebug() << "found";
                            DBServerHasProjects = true;

                            //---------------------------
                            QStringList ProjectAllowedDBServers = Configs::RunningParameters.ProjectAllowedDBServers[Project.Name.value()];
                            ProjectAllowedDBServers.append(DBServerName);
                            Configs::RunningParameters.ProjectAllowedDBServers[Project.Name.value()] = ProjectAllowedDBServers;

                            //---------------------------
                            QString ConnStringWithSchema = QString("HOST=%1;PORT=%2;USER=%3;PASSWORD=%4;SCHEMA=%5%6;")
                                                           .arg(DBServer.Host.value())
                                                           .arg(DBServer.Port.value())
                                                           .arg(DBServer.UserName.value())
                                                           .arg(DBServer.Password.value())
                                                           .arg(Configs::DBPrefix.value())
                                                           .arg(Project.Name.value());

                            QString ProjectDestinationKey = QString("%1@%2")
//                                                            .arg(Configs::DBPrefix.value())
                                                            .arg(Project.Name.value())
                                                            .arg(DBServerName);

                            Configs::RunningParameters.ProjectDBConnectionStrings[ProjectDestinationKey] = ConnStringWithSchema;
                        }
                    }

                    //add default connection string for dbserver
                    if (DBServerHasProjects) {
                        Configs::RunningParameters.DBServersDefaultConnectionString[DBServerName] = QString("HOST=%1;PORT=%2;USER=%3;PASSWORD=%4;")
                                                                                              .arg(DBServer.Host.value())
                                                                                              .arg(DBServer.Port.value())
                                                                                              .arg(DBServer.UserName.value())
                                                                                              .arg(DBServer.Password.value())
                                                                                              ;
                    }

                    break;
                }
            }
            ++dbIdx;
        }
    }

    //projects by tag
    for (size_t idxProjects=0; idxProjects<Configs::Projects.size(); idxProjects++) {
        stuProject &Project = Configs::Projects[idxProjects];

        if (Project.Tags.value().isEmpty())
            continue;

        QStringList Tags = Project.Tags.value().split(",", QString::SkipEmptyParts);
        foreach (QString Tag, Tags) {
            Tag = Tag.trimmed(); //.toLower();
            if (Tag.isEmpty())
                continue;

            if (Configs::RunningParameters.ProjectsByTag.contains(Tag))
                Configs::RunningParameters.ProjectsByTag[Tag].append(Project.Name.value());
            else
                Configs::RunningParameters.ProjectsByTag.insert(Tag, { Project.Name.value() });
        }
    }
}

} //namespace Targoman::Migrate

ENUM_CONFIGURABLE_IMPL(Targoman::Migrate::enuAppCommand);
