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

    if (Configs::Mark.value() == false) {
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
               << BAD_FILE_SIGNATURE << endl
               << endl
               << "USE `{{dbprefix}}{{Schema}}`;" << endl
               << endl
               ;
        File.close();

        TargomanInfo(0).noLabel().noquote() << "Empty migration file created successfully. Now gathering diff for selected project.";
    }

    //-- diff --------------------------
    this->diff(
                ProjectIndex,
                FullFileName
              );

    if (Configs::Mark.value() == false) {
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
    }

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

    QStringMap DBServerParameters;
    for (size_t idxDBServers=0; idxDBServers<Configs::DBServers.size(); idxDBServers++) {
        stuDBServer &DBServer = Configs::DBServers[idxDBServers];
        if (DBServer.Name.value() == DBServerName) {
            DBServerParameters.insert("host", DBServer.Host.value());
            DBServerParameters.insert("port", DBServer.Port.value());
            DBServerParameters.insert("user", DBServer.UserName.value());
            DBServerParameters.insert("password", DBServer.Password.value());
        }
    }

    if (DBServerParameters.isEmpty())
        throw exMigrationTool("dbserver not found");

    //---------------------------
    clsDAC DAC(ProjectDBServerName);
    clsDACResult Result = DAC.execQuery("", "SHOW BINARY LOGS");
//    TargomanDebug(5) << Result.toJson(false);
    QVariantList BinaryLogList = Result.toJson(false).toVariant().toList();
    if (BinaryLogList.isEmpty())
        throw exMigrationTool("invalid result of SHOW BINARY LOGS");

    //---------------------------
    QString DBDiffCfgFileName = QString("%1/%2/db/.dbdiff%3.cfg")
                    .arg(BaseFolder.path())
                    .arg(ProjectName)
                    .arg(Configs::DBPrefix.value().isEmpty() ? "" : "." + Configs::DBPrefix.value())
                    ;
    QMap<QString, quint64> DBDiffConfigEntries;
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
            foreach (auto Row, BinaryLogList) {
                QVariantMap Map = Row.toMap();
                QString BinaryLogName = Map["Log_name"].toString();

                //bypass old log files
                if (LineParts[0] == BinaryLogName) {
                    DBDiffConfigEntries[LineParts[0]] = LineParts[1].toULongLong();
                    break;
                }
            }
        };
        File.close();
    }
    QMap<QString, quint64> OriginDBDiffConfigEntries = DBDiffConfigEntries;

    auto SaveDBDiffCfgFile = [&OriginDBDiffConfigEntries, &DBDiffConfigEntries, &DBDiffCfgFileName]() -> bool {
        if (DBDiffConfigEntries.isEmpty())
            return true;

        QFile File(DBDiffCfgFileName);

        if (!File.open(QFile::Append | QFile::Text))
            return false; //throw exTargomanBase("Could not create or append file");

        try {
            QTextStream Stream(&File);

            QString CurrentDateTime = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
            Stream << "# " << CurrentDateTime;
            if (Configs::Mark.value())
                Stream << " : mark only";
            Stream << endl;

            for (QMap<QString, quint64>::const_iterator it = DBDiffConfigEntries.constBegin();
                 it != DBDiffConfigEntries.constEnd();
                 it++
            ) {
                QString Key = it.key();
                quint64 Value = *it;

                if (OriginDBDiffConfigEntries.contains(Key) && (OriginDBDiffConfigEntries[Key] == Value))
                    continue;

                Stream << Key << "=" << Value << endl;
            };

            File.close();
        }  catch (std::exception &_exp) {
            File.close();
            return false;
        }
        return true;
    };

    //---------------------------
    QFile MigOutFile(_fullFileName);
    if (Configs::Mark.value() == false) {
        if (!MigOutFile.open(QFile::Append | QFile::Text))
            throw exTargomanBase("Could not append file");
    }

    try {
        QTextStream MigOutFileStream;
        if (Configs::Mark.value() == false)
            MigOutFileStream.setDevice(&MigOutFile);

        foreach (auto Row, BinaryLogList) {
            QVariantMap Map = Row.toMap();
            QString BinaryLogName = Map["Log_name"].toString();

            QStringList CommandLineParameters;
            QStringList CommandLineParametersNoPsw;

            CommandLineParameters << "--read-from-remote-server";
            CommandLineParametersNoPsw << "--read-from-remote-server";

            for (QStringMap::const_iterator it = DBServerParameters.constBegin();
                 it != DBServerParameters.constEnd();
                 it++
            ) {
                CommandLineParameters << "--" + it.key() + "=" + it.value();
                if (it.key() == "password")
                    CommandLineParametersNoPsw << "--" + it.key() + "=********";
                else
                    CommandLineParametersNoPsw << "--" + it.key() + "=" + it.value();
            }
            CommandLineParameters << "--database=" + Configs::DBPrefix.value() + ProjectName
                                  << "--base64-output=DECODE-ROWS"
                                  ;
            CommandLineParametersNoPsw << "--database=" + Configs::DBPrefix.value() + ProjectName
                                       << "--base64-output=DECODE-ROWS"
                                       ;

            if (DBDiffConfigEntries.contains(BinaryLogName)) {
                CommandLineParameters << "--start-position=" + QString::number(DBDiffConfigEntries[BinaryLogName]);
                CommandLineParametersNoPsw << "--start-position=" + QString::number(DBDiffConfigEntries[BinaryLogName]);
            }

            CommandLineParameters << BinaryLogName;
            CommandLineParametersNoPsw << BinaryLogName;

            TargomanDebug(5).noquote()
                    << "mysqlbinlog"
                    << CommandLineParametersNoPsw.join(" ")
                    ;

            //--------------------------
            if (Configs::Mark.value() == false) {
                MigOutFileStream << "/" << QString(60, '*') << "\\" << endl;
                QString Buffer = BinaryLogName;
                if (DBDiffConfigEntries.contains(BinaryLogName))
                    Buffer += QString(" ") + "--start-position=" + QString::number(DBDiffConfigEntries[BinaryLogName]);
                MigOutFileStream << "| " << Buffer << QString(60 - 2 -  Buffer.length(), ' ') << " |" << endl;
                MigOutFileStream << "\\" << QString(60, '*') << "/" << endl;
                MigOutFileStream << endl;
            }

            //--------------------------
            QProcess Process;
            Process.start("mysqlbinlog", CommandLineParameters);
            if (!Process.waitForFinished())
                throw exTargomanBase("Execution failed");

            QByteArray Output;
            qint64 lastStartPosition = -1;
            qint64 lastEndPosition = -1;

            while (true) {
                QByteArray ProcessLineByteArray = Process.readLine();
                if (ProcessLineByteArray.isNull())
                    break;

                QString ProcessLine = ProcessLineByteArray;

                if (ProcessLine.endsWith("\n"))
                    ProcessLine.chop(1);
                if (ProcessLine.endsWith("\r"))
                    ProcessLine.chop(1);

                if (ProcessLine.startsWith("# at ")) {
                    lastStartPosition = ProcessLine.mid(5).trimmed().toInt();
                }

                int idx;
                if ((idx = ProcessLine.indexOf(" end_log_pos ")) >= 0) {
                    idx += QString(" end_log_pos ").length();
                    int idx2 = ProcessLine.indexOf(" ", idx);
                    lastEndPosition = (idx2 >= 0
                                          ? ProcessLine.mid(idx, idx2 - idx + 1).trimmed().toInt()
                                          : ProcessLine.mid(idx).trimmed().toInt()
                                      );
                } else if (ProcessLine.startsWith("#") and (ProcessLine.startsWith("# at ") == false))
                    continue;

                //DBPrefix
                if (Configs::DBPrefix.value().isEmpty() == false)
                    ProcessLine = ProcessLine.replace(QRegularExpression("(^|[^a-zA-Z])" + Configs::DBPrefix.value()), "\\1{{dbprefix}}");

                //GlobalHistoryTableName
                ProcessLine = ProcessLine.replace(QRegularExpression("(^|[^a-zA-Z])" + Configs::GlobalHistoryTableName.value()), "\\1{{GlobalHistoryTableName}}");

                //DEFINER=`root`@`%`
                ProcessLine = ProcessLine.replace(QRegularExpression("(\\s+)DEFINER(\\s*)=(\\s*)`([^`]*)`@`([^`]*)`(\\s+)"), " ");

                //AUTO_INCREMENT=1936
                ProcessLine = ProcessLine.replace(QRegularExpression("(^|[^a-zA-Z])AUTO_INCREMENT(\\s*)=(\\s*)(\\d+)"), "\\1");

                //------------------
                Output += ProcessLine + "\n";
            }
            Output += "\n";

            TargomanDebug(5).noquote()
                    << "    "
                    << "Result Length:" << Output.length()
                    << "lastPosition:" << lastStartPosition
                    << "lastEndLogPosition:" << lastEndPosition
                    ;

            if (Configs::Mark.value() == false)
                MigOutFileStream << Output;

            if (lastEndPosition > 0)
                DBDiffConfigEntries[BinaryLogName] = lastEndPosition;
        } //foreach (auto Row, BinaryLogList)

        if (Configs::Mark.value() == false)
            MigOutFile.close();

    }  catch (std::exception &_exp) {
        if (Configs::Mark.value() == false)
            MigOutFile.close();

        SaveDBDiffCfgFile();

        throw;
    }

    SaveDBDiffCfgFile();
}

} // namespace Targoman::Migrate::Commands
