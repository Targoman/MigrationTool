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

#include "cmdNewDBDiff.h"
#include "../Configs.h"
#include <signal.h>
#include <unistd.h>

namespace Targoman::Migrate::Commands {

cmdNewDBDiff::cmdNewDBDiff() { ; }

void cmdNewDBDiff::help() {
}

bool cmdNewDBDiff::run() {
    if (Configs::RunningParameters.ProjectAllowedDBServers.isEmpty()) {
        TargomanInfo(0).noLabel() << "No DB Servers found.";
        return true;
    }

    QString FileName;
    QString FullFileName;
    quint32 ProjectIndex;

    if (ChooseCreateMigrationProperties(
                enuChooseCreateMigrationScope::dbdiff,
                FileName,
                FullFileName,
                ProjectIndex
            ) == false)
        return true;

    QFile File(FullFileName);
    if (File.open(QFile::WriteOnly | QFile::Text) == false) {
        TargomanInfo(0).noLabel() << "Could not create new migration file.";
        return true;
    }

    FullFileName = GetSymlinkTarget(FullFileName);

    TargomanInfo(0).noLabel().noquote().nospace() << "Creating new migration file: " << FullFileName;

    QTextStream writer(&File);
    writer << "/* Migration File: " << FileName << " */" << endl
           << "/* CAUTION: don't forget to use {{dbprefix}} for schemas */" << endl
           << endl
           << "/* The next line is to prevent this file from being committed. When done, delete this and next line: */" << endl
           << "ERROR(\"THIS MIGRATION FILE IS NOT READY FOR EXECUTE.\")" << endl
           << endl;
    File.close();

    TargomanInfo(0).noLabel().noquote() << "Empty migration file created successfully. Now gathering diff for selected project.";

    //-- diff --------------------------
    this->diff(
                ProjectIndex,
                FullFileName
              );

    //-- vim --------------------------
    qint64 PID;

    if (QProcess::startDetached(Configs::DefaultEditor.value(),
                                QStringList() << FullFileName,
                                {},
                                &PID) == false)
        throw exTargomanBase("Execution of default editor failed");

    while (kill(PID, 0) == 0) { usleep(1); }

    //-- git --------------------------
    TargomanInfo(0).noLabel().noquote() << "Do not forgot to add new generated file to git if needed." << endl;

    return true;
}

void cmdNewDBDiff::diff(
    quint32 _projectIndex,
    const QString &_fullFileName
) {
    QDir BaseFolder(Configs::MigrationsFolderName.value());
    ///TODO: check if dir exists
    BaseFolder.makeAbsolute();

    stuProject &Project = Configs::Projects[_projectIndex];
    QString ProjectName = Project.Name.value();
    QString DBServerName = Configs::RunningParameters.ProjectAllowedDBServers[ProjectName].first();
    QString ProjectDBServerName = ProjectName + "@" + DBServerName;

    QStringList BaseCommandLineParameters;
    for (size_t idxDBServers=0; idxDBServers<Configs::DBServers.size(); idxDBServers++) {
        stuDBServer &DBServer = Configs::DBServers[idxDBServers];
        if (DBServer.Name.value() == DBServerName) {
            BaseCommandLineParameters << "--read-from-remote-server"
                                  << "--host=" + DBServer.Host.value()
                                  << "--port=" + DBServer.Port.value()
                                  << "--user=" + DBServer.UserName.value()
                                  << "--password=" + DBServer.Password.value()
                                  ;
        }
    }

    if (BaseCommandLineParameters.isEmpty())
        throw exMigrationTool("dbserver not found");

    BaseCommandLineParameters << "--database=" + Configs::DBPrefix.value() + ProjectName;

    QString DBDiffCfgFileName = QString("%1/%2/db/.dbdiff%3.cfg")
                    .arg(BaseFolder.path())
                    .arg(ProjectName)
                    .arg(Configs::DBPrefix.value().isEmpty() ? "" : "." + Configs::DBPrefix.value())
                    ;
    QStringMap DBDiffConfigEntries;
    if (QFile::exists(DBDiffCfgFileName)) {

        QFile File(DBDiffCfgFileName);

        if (!File.open(QFile::ReadOnly | QFile::Text))
            throw exTargomanBase("File not found");

        QTextStream Stream(&File);
        while (true) {
            QString Buffer = Stream.readLine();
            if (Buffer.isNull())
                break;

            if (Buffer.isEmpty() || Buffer.startsWith("#"))
                continue;

            QStringList LineParts = Buffer.split("=", QString::SkipEmptyParts);
            DBDiffConfigEntries[LineParts[0]] = LineParts[1];
        };
        File.close();
    }

    clsDAC DAC(ProjectDBServerName);
    clsDACResult Result = DAC.execQuery("", "SHOW BINARY LOGS");
//    TargomanDebug(5) << Result.toJson(false);
    QVariantList BinaryLogList = Result.toJson(false).toVariant().toList();
    if (BinaryLogList.isEmpty())
        throw exMigrationTool("invalid result of SHOW BINARY LOGS");

    foreach (auto Row, BinaryLogList) {
        QVariantMap Map = Row.toMap();
        QString BinaryLogName = Map["Log_name"].toString();

        QStringList CommandLineParameters = BaseCommandLineParameters;

        if (DBDiffConfigEntries.contains(BinaryLogName))
            CommandLineParameters << "--start-position=" + QString::number(atol(DBDiffConfigEntries[BinaryLogName].toStdString().c_str()) + 1);

        CommandLineParameters << BinaryLogName;

        TargomanDebug(5).noquote()
                << "mysqlbinlog"
                << CommandLineParameters.join(" ")
                ;

        QProcess Process;
        Process.start("mysqlbinlog", CommandLineParameters);
        if (!Process.waitForFinished())
            throw exTargomanBase("Execution failed");

        QByteArray ProcessResult = Process.readAll();

        TargomanDebug(5).noquote()
                << "Result Length:"
                << ProcessResult.length()
                ;

        if (ProcessResult.isEmpty() == false) {
//            DBDiffConfigEntries[BinaryLogName] = "3";
        }

    }

}

} // namespace Targoman::Migrate::Commands
