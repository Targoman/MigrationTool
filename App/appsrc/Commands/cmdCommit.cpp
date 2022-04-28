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

cmdCommit::cmdCommit() { ; }

void cmdCommit::help() {
}

bool cmdCommit::run() {
    ProjectMigrationFileInfoMap ProjectMigrationFiles;
    ExtractMigrationFiles(ProjectMigrationFiles);
//    qDebug() << "** All MigrationFiles ******************************";
//    dump(MigrationFiles);

    MigrationHistoryMap MigrationHistories;
    ExtractMigrationHistories(MigrationHistories);
//    qDebug() << "** MigrationHistories ******************************";
//    dump(MigrationHistories);

    RemoveAppliedFromList(ProjectMigrationFiles, MigrationHistories);

    if (ProjectMigrationFiles.isEmpty()) {
        qInfo() << "nothing to commit";
        return true;
    }

    qDebug() << "** Unapplied MigrationFiles ******************************";
    dump(ProjectMigrationFiles);
    qInfo() << "";

    qint32 RemainCount = 0;

    if (Configs::All.value())
        RemainCount = ProjectMigrationFiles.count();
    else {
        while (true) {
            qStdout()
                    << "Which migrations do you want to run?"
                    << " "
                    << reverse("[") << reverse(bold("c")) << reverse("ancel]")
                    << " "
                    ;

            if (ProjectMigrationFiles.count() == 1) {
                qStdout()
                        << reverse("[") << reverse(bold("a")) << reverse("ll]")
                        << reverse(" = ")
                        << reverse("[") << reverse(bold("1")) << reverse("]")
                        ;
            } else {
                qStdout()
                        << reverse("[") << reverse(bold("a")) << reverse("ll]")
                        << " "
                        << reverse("1 to [") << reverse(bold("1")) << reverse("]")
                        << reverse(" ... ")
                        << reverse("[") << reverse(bold(QString::number(ProjectMigrationFiles.count()))) << reverse("]")
                        ;
            }
            qStdout() << " ";
            qStdout().flush();

            QString value = qStdIn().readLine().trimmed();

            if (value.isEmpty())
                continue;

            if (value == "c")
                return true;

            if (value == "a") {
                RemainCount = ProjectMigrationFiles.count();
                break;
            } else {
                bool ok = false;
                RemainCount = value.toInt(&ok);

                if (ok) {
                    if ((RemainCount <= 0) || (RemainCount > ProjectMigrationFiles.count()))
                        qStdout() << "Input must be between 1 and " << ProjectMigrationFiles.count() << endl;
                    else
                        break;
                } else
                    qStdout() << "Invalid input " << value << endl;
            }
        }
    }

    qInfo() << "Applying migrations:";
    qInfo() << LINE_SPLITTER;

    int idx = 1;
    foreach (auto ProjectMigrationFile, ProjectMigrationFiles) {
        qStdout()
                << QString::number(idx++).rightJustified(4)
                << ") "
                << ProjectMigrationFile.FileName
                << " ["
                << ProjectMigrationFile.Scope
                << "/"
                << (ProjectMigrationFile.Scope == "local" ? "" : Configs::DBPrefix.value())
                << ProjectMigrationFile.Project
                << "]"
//                << MigrationFile.FullFileName
                << " : "
                ;

        RunMigrationFile(ProjectMigrationFile);

        qStdout() << "OK" << endl;

        --RemainCount;
        if (RemainCount <= 0)
            break;
    }

    qInfo() << "";

    return true;
}

} // namespace Targoman::Migrate::Commands
