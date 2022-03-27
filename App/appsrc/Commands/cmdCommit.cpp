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

#include "cmdCommit.h"
#include "../Configs.h"

namespace Targoman::Migrate::Commands {

cmdCommit::cmdCommit()
{
}

void cmdCommit::help()
{
}

bool cmdCommit::run()
{
    ProjectMigrationFileInfoMap MigrationFiles;
    ExtractMigrationFiles(MigrationFiles);
//    qDebug() << "** All MigrationFiles ******************************";
//    dump(MigrationFiles);

    MigrationHistoryMap MigrationHistories;
    ExtractMigrationHistories(MigrationHistories);
//    qDebug() << "** MigrationHistories ******************************";
//    dump(MigrationHistories);

    RemoveAppliedFromList(MigrationFiles, MigrationHistories);

    if (MigrationFiles.isEmpty())
    {
        qInfo() << "nothing to commit";
        return true;
    }

    qDebug() << "** Unapplied MigrationFiles ******************************";
    dump(MigrationFiles);
    qInfo() << "";

    qint32 RemainCount = 0;

    if (Configs::All.value())
    {
        RemainCount = MigrationFiles.count();
    }
    else
    {
        while (true)
        {
            qStdout()
                    << "Which migrations do you want to run?"
                    << " "
                    << reverse("[") << reverse(bold("c")) << reverse("ancel]")
                    << " "
                    ;

            if (MigrationFiles.count() == 1)
            {
                qStdout()
                        << reverse("[") << reverse(bold("a")) << reverse("ll]")
                        << reverse(" = ")
                        << reverse("[") << reverse(bold("1")) << reverse("]")
                        ;
            }
            else
            {
                qStdout()
                        << reverse("[") << reverse(bold("a")) << reverse("ll]")
                        << " "
                        << reverse("1 to [") << reverse(bold("1")) << reverse("]")
                        << reverse(" ... ")
                        << reverse("[") << reverse(bold(QString::number(MigrationFiles.count()))) << reverse("]")
                        ;
            }
            qStdout() << " ";
            qStdout().flush();

            QString value = qStdIn().readLine().trimmed();

            if (value.isEmpty())
                continue;

            if (value == "c")
                return true;

            if (value == "a")
            {
                RemainCount = MigrationFiles.count();
                break;
            }
            else
            {
                bool ok = false;
                RemainCount = value.toInt(&ok);

                if (ok)
                {
                    if ((RemainCount <= 0) || (RemainCount > MigrationFiles.count()))
                        qStdout() << "Input must be between 1 and " << MigrationFiles.count() << endl;
                    else
                        break;
                }
                else
                    qStdout() << "Invalid input " << value << endl;
            }
        }
    }

    qInfo() << "Applying migrations:";
    qInfo() << LINE_SPLITTER;

    int idx = 1;
    foreach (auto MigrationFile, MigrationFiles)
    {
        qStdout()
                << "    "
                << QString::number(idx++).rightJustified(5)
                << " "
                << MigrationFile.FileName
                << " ["
                << MigrationFile.Scope
                << "/"
                << (MigrationFile.Scope == "local" ? "" : Configs::DBPrefix.value())
                << MigrationFile.Project
                << "]"
//                << MigrationFile.FullFileName
                << " : "
                ;

        RunMigrationFile(MigrationFile);

        qStdout() << "OK" << endl;

        --RemainCount;
        if (RemainCount <= 0)
            break;
    }

    qInfo() << "";

    return true;
}

} // namespace Targoman::Migrate::Commands
