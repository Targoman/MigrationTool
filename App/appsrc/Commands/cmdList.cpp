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

#include "cmdList.h"
#include "../Configs.h"

namespace Targoman::Migrate::Commands {

cmdList::cmdList() { ; }

void cmdList::help() {
    TargomanInfo(0).noLabel() << "List of unapplied migrations";
    //        TargomanInfo(0).noLabel() << _line_splitter;
    //        TargomanInfo(0).noLabel() << "./MigrationTool" << "List     : showing the first 10 new migrations";
    //        TargomanInfo(0).noLabel() << "./MigrationTool" << "List 5   : showing the first 5 new migrations";
    //        TargomanInfo(0).noLabel() << "./MigrationTool" << "List all : showing all new migrations";
}

bool cmdList::run() {
    TargomanInfo(0).noLabel() << "Unapplied migrations:";
    TargomanInfo(0).noLabel() << LINE_SPLITTER;

    ProjectMigrationFileInfoMap ProjectMigrationFiles;
    ExtractMigrationFiles(ProjectMigrationFiles);
//    TargomanDebug(5) << "** All MigrationFiles ******************************";
//    dump(MigrationFiles);

    MigrationHistoryMap MigrationHistories;
    ExtractMigrationHistories(MigrationHistories);
//    TargomanDebug(5) << "** MigrationHistories ******************************";
//    dump(MigrationHistories);

    RemoveAppliedFromList(ProjectMigrationFiles, MigrationHistories);
//    TargomanDebug(5) << "** Unapplied MigrationFiles ******************************";
    dump(ProjectMigrationFiles);

    TargomanInfo(0).noLabel() << "";

    return true;
}

} // namespace Targoman::Migrate::Commands
