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

#include "cmdHistory.h"
#include "../Configs.h"

namespace Targoman::Migrate::Commands {

cmdHistory::cmdHistory()
{
}

void cmdHistory::help()
{
    qInfo() << "List of applied migrations";
    //        qInfo() << _line_splitter;
    //        qInfo() << "./MigrationTool" << "History     : showing the first 10 applied migrations";
    //        qInfo() << "./MigrationTool" << "History 5   : showing the first 5 applied migrations";
    //        qInfo() << "./MigrationTool" << "History all : showing all applied migrations";
}

bool cmdHistory::run()
{
    MigrationHistoryMap MigrationHistories;
    ExtractMigrationHistories(MigrationHistories);
//    qDebug() << "** MigrationHistories ******************************";
//    dump(MigrationHistories);

    ProjectMigrationFileInfoMap MigrationFiles;
    for (MigrationHistoryMap::const_iterator it = MigrationHistories.constBegin();
         it != MigrationHistories.constEnd();
         it++)
    {
        QString key = it.key();
        const stuMigrationHistory &MigrationHistoryItem = it.value();

        if (MigrationHistoryItem.AppliedItems.isEmpty() == false)
        {
            for (QMap<QString, stuHistoryAppliedItem>::const_iterator it = MigrationHistoryItem.AppliedItems.constBegin();
                 it != MigrationHistoryItem.AppliedItems.constEnd();
                 it++)
            {
                QString key = it.key();
                const stuHistoryAppliedItem &MigrationHistoryAppliedItem = it.value();

                QString MigrationName = QString("%1:%2/%3")
                                        .arg(MigrationHistoryAppliedItem.MigrationName)
                                        .arg(MigrationHistoryItem.Scope)
                                        .arg(MigrationHistoryItem.Project);

                stuProjectMigrationFileInfo ProjectMigrationFileInfo(
                    MigrationName,
                    MigrationHistoryAppliedItem.MigrationName,
                    MigrationHistoryItem.Scope,
                    MigrationHistoryItem.Project,
                    MigrationHistoryAppliedItem.AppliedAt.toString("yyyy-MM-dd hh:mm:ss a")
                );

//                qDebug() << "*" << MigrationName;
                MigrationFiles.insert(MigrationName, ProjectMigrationFileInfo);
            }
        }
    }

    qInfo() << "Applied migrations:";
    qInfo() << LINE_SPLITTER;
    dump(MigrationFiles, true);

    return true;
}

} // namespace Targoman::Migrate::Commands
