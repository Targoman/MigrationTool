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

#include "cmdCheckBC.h"
#include "../Configs.h"

namespace Targoman::Migrate::Commands {

bool IsMigrationFileBackwardCompatible(const stuMigrationFileInfo &_migrationFile);

cmdCheckBC::cmdCheckBC() { ; }

void cmdCheckBC::help() {
}

bool cmdCheckBC::run() {
    ProjectMigrationFileInfoMap ProjectMigrationFiles;
    ExtractMigrationFiles(ProjectMigrationFiles);
//    TargomanDebug(5) << "** All MigrationFiles ******************************";
//    dump(MigrationFiles);

    MigrationHistoryMap MigrationHistories;
    ExtractMigrationHistories(MigrationHistories);
//    TargomanDebug(5) << "** MigrationHistories ******************************";
//    dump(MigrationHistories);

    RemoveAppliedFromList(ProjectMigrationFiles, MigrationHistories);
    if (ProjectMigrationFiles.isEmpty()) {
        TargomanInfo(0).noLabel() << "nothing to check";
        return true;
    }

    //-----------------------------------------
    //remove project name from migrations and make unique
    MigrationFileInfoMap MigrationFiles;
    for (ProjectMigrationFileInfoMap::const_iterator it = ProjectMigrationFiles.constBegin();
         it != ProjectMigrationFiles.constEnd();
         it++
        ) {
        const stuProjectMigrationFileInfo &val = it.value();

        QString MigrationName = val.FileName + ":" + val.Scope;

        if (MigrationFiles.contains(MigrationName))
            continue;

        MigrationFiles.insert(MigrationName, {
                                  MigrationName,
                                  val.FileName,
                                  val.Scope,
                                  val.FullFileName
                              });
    }

    TargomanDebug(5).noquote().nospace()
            << endl
            << "Unapplied migrations:"
            ;
    dump(MigrationFiles);
    TargomanInfo(0).noLabel() << "";

    qint32 RemainCount = 0;

    if (Configs::All.value())
        RemainCount = MigrationFiles.count();
    else {
        while (true) {
            qStdout()
                    << "Which migrations do you want to check?"
                    << " "
                    << reverse("[") << reverse(bold("c")) << reverse("ancel]")
                    << " "
                    ;

            if (MigrationFiles.count() == 1) {
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

            if (value == "a") {
                RemainCount = MigrationFiles.count();
                break;
            } else {
                bool ok = false;
                RemainCount = value.toInt(&ok);

                if (ok) {
                    if ((RemainCount <= 0) || (RemainCount > MigrationFiles.count()))
                        qStdout() << "Input must be between 1 and " << MigrationFiles.count() << endl;
                    else
                        break;
                } else
                    qStdout() << "Invalid input " << value << endl;
            }
        }
    }

    TargomanInfo(0).noLabel().noquote().nospace()
            << "Checking migrations:"
            << endl
            << QString(150, '-')
            ;

    int idx = 1;
    foreach (auto MigrationFile, MigrationFiles) {
        qStdout()
                << QString::number(idx++).rightJustified(4)
                << ") "
                << MigrationFile.FileName
                << " ["
                << MigrationFile.Scope
                << "]"
//                << MigrationFile.FullFileName
                << " : "
                ;

        bool result = IsMigrationFileBackwardCompatible(MigrationFile);
        qStdout() << (result ? "Yes (Backward Compatible)" : "No (Backward Incompatible)") << endl;

        if (result == false) {
            qStdout() << "[STATUS:BREAKED]" << endl;
            return true;
        }

        --RemainCount;
        if (RemainCount <= 0)
            break;
    }

    TargomanInfo(0).noLabel() << "";
    qStdout() << "[STATUS:OK]" << endl;

    return true;
}

bool IsMigrationFileBackwardCompatible(const stuMigrationFileInfo &_migrationFile) {
    if (_migrationFile.Scope == "db") {
        QFile File(_migrationFile.FullFileName);

        if (!File.open(QFile::ReadOnly | QFile::Text))
            throw exTargomanBase("File not found");

        QTextStream Stream(&File);
        QString Qry = Stream.readAll().trimmed();
        File.close();

        if (Qry.isEmpty() == false) {
            QString Delimiter = ";";
            while (Qry.isEmpty() == false) {
                if (Qry.startsWith("delimiter ", Qt::CaseInsensitive)) {
                    Qry.remove(0, QString("delimiter ").length());

                    int idx = Qry.indexOf("\n");
                    if (idx < 0)
                        break; //new delimiter without new line mark: nothing to run

                    if (idx == 0)
                        Qry.remove(0, 1);
                    else {
                        Delimiter = Qry.left(idx).trimmed();
                        Qry.remove(0, idx + 1);

                        if (Delimiter.isEmpty())
                            throw exMigrationTool("delimiter is empty");
                    }
                } else {
                    int idx = Qry.indexOf(Delimiter);

                    QString SmallQry;

                    if (idx >= 0) {
                        SmallQry = Qry.left(idx).trimmed();
                        Qry.remove(0, idx + Delimiter.length());
                    } else {
                        SmallQry = Qry.trimmed();
                        Qry = "";
                    }

                    if (SmallQry.isEmpty() == false) {
//                            SmallQry += ";";
//                        TargomanDebug(5) << "\t\tChecking query" << SmallQry.left(50) << "...";

                        //TODO: parse qry and check command

                        return false;  //not compatible
                    }
                }

                Qry = Qry.trimmed();
            }
        }
    } else { //local
        QFileInfo FileInfo(_migrationFile.FullFileName);

        if (FileInfo.exists() == false)
            throw exTargomanBase("File not found");

        if (FileInfo.size() > 0) {
            if (FileInfo.isExecutable() == false)
                QFile::setPermissions(
                            _migrationFile.FullFileName,
                            FileInfo.permissions() |
                            QFile::ExeUser | QFile::ExeGroup | QFile::ExeOwner | QFile::ExeOther
                            );

            QProcess MigrationProcess;
            MigrationProcess.start(_migrationFile.FullFileName, { "checkbc" });

            if (!MigrationProcess.waitForFinished())
                throw exTargomanBase("Execution failed");

            if (MigrationProcess.exitCode() == 0) //not compatible
                return false;
        }
    }

    return true;
}

} // namespace Targoman::Migrate::Commands
