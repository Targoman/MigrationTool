/******************************************************************************
#   TargomanMigrate
#
#   Copyright 2014-2020 by Targoman Intelligent Processing <http://tip.co.ir>
#
#   TargomanMigrate is free software: you can redistribute it and/or modify
#   it under the terms of the GNU AFFERO GENERAL PUBLIC LICENSE as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   TargomanMigrate is distributed in the hope that it will be useful,
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

cmdList::cmdList()
{
}

void cmdList::help()
{
    qInfo() << "List of unapplied migrations";
    //        qInfo() << _line_splitter;
    //        qInfo() << "./targomanMigrate" << "List     : showing the first 10 new migrations";
    //        qInfo() << "./targomanMigrate" << "List 5   : showing the first 5 new migrations";
    //        qInfo() << "./targomanMigrate" << "List all : showing all new migrations";
}

bool cmdList::run()
{
    qInfo() << "Unapplied migrations:";
    qInfo() << LINE_SPLITTER;

    ProjectMigrationFileInfoMap MigrationFiles;
    ExtractMigrationFiles(MigrationFiles);
//    qDebug() << "** All MigrationFiles ******************************";
//    dump(MigrationFiles);

    MigrationHistoryMap MigrationHistories;
    ExtractMigrationHistories(MigrationHistories);
//    qDebug() << "** MigrationHistories ******************************";
//    dump(MigrationHistories);

    RemoveAppliedFromList(MigrationFiles, MigrationHistories);
//    qDebug() << "** Unapplied MigrationFiles ******************************";
    dump(MigrationFiles);

    qInfo() << "";

    return true;
}

} // namespace Targoman::Migrate::Commands
