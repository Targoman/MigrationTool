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

#ifndef MIGRATION_TOOL_COMMON_H
#define MIGRATION_TOOL_COMMON_H

#include "../Defs.h"
#include "../Configs.h"
using namespace Targoman::Common;

#include "libTargomanDBM/clsDAC.h"
using namespace Targoman::DBManager;

#include <iostream>
using namespace std;

namespace Targoman::Migrate::Commands {

typedef QMap<QString, QString> QStringMap;

enum class enuChooseCreateMigrationScope
{
    db,
    dbdiff,
    local
};

inline bool ChooseCreateMigrationProperties(
    enuChooseCreateMigrationScope _chooseScope,
    QString &_fileName,
    QString &_fullFileName,
    quint32 &_projectIndex
) {
    QDir BaseFolder(Configs::MigrationsFolderName.value());
    ///TODO: check if dir exists
    BaseFolder.makeAbsolute();

    qInfo().noquote()
            << "    "
            << "Project"
            ;
    qInfo() << LINE_SPLITTER;

    struct stuProjectInfo {
        QString Name;
        qint32 Index;

        stuProjectInfo(
                QString _name,
                qint32 _index = -1
            ) :
            Name(_name),
            Index(_index)
        { ; }
    };

    QList<stuProjectInfo> Projects;
    for (size_t idxProject=0; idxProject<Configs::Projects.size(); idxProject++) {
        stuProject &Project = Configs::Projects[idxProject];

        if ((Configs::Project.value().isEmpty() == false)
                && (Project.Name.value() != Configs::Project.value())
                && Project.ApplyToTags.value().isEmpty()
            )
            continue;

        QString Name = Project.Name.value();
        QStringList TagProjectNames;
        if ((Project.ApplyToTags.value().isEmpty() == false) && (Project.ApplyToTags.value() != "*")) {
            foreach (QString Tag, Project.ApplyToTags.value().split(",", QString::SkipEmptyParts)) {
                foreach (QString PrjName, Configs::RunningParameters.ProjectsByTag[Tag.trimmed()]) {
                    QString FullPrjName;
                    if ((_chooseScope == enuChooseCreateMigrationScope::local && Project.AllowLocal.value())
                            || (_chooseScope != enuChooseCreateMigrationScope::local && Project.AllowDB.value())) {
                        if (_chooseScope == enuChooseCreateMigrationScope::local)
                            FullPrjName = PrjName;
                        else
                            FullPrjName = Configs::DBPrefix.value() + PrjName;
                    }

                    if (FullPrjName.isEmpty())
                        continue;

                    if (TagProjectNames.contains(FullPrjName))
                        continue;
                    TagProjectNames.append(FullPrjName);
                }
            }
        }

        if (_chooseScope == enuChooseCreateMigrationScope::local) {
            if (Project.AllowLocal.value()) {
                Projects.append(stuProjectInfo(Name, idxProject));

                qInfo().noquote()
                        << QString::number(Projects.length()).rightJustified(4)
                        << Name
                        << (Project.ApplyToTags.value().isEmpty() ? ""
                            : "[" + (Project.ApplyToTags.value() == "*" ? "Apply to all" : TagProjectNames.join(", ")) + "]")
                        ;
            }
        } else if (Project.AllowDB.value()
             && (Project.ApplyToTags.value().isEmpty() == false
                 || Configs::RunningParameters.ProjectAllowedDBServers.contains(Name))
        ) {
            if ((_chooseScope != enuChooseCreateMigrationScope::dbdiff) || (Project.ApplyToTags.value() == false)) {
                Projects.append(stuProjectInfo(Name, idxProject));

                qInfo().noquote()
                        << QString::number(Projects.length()).rightJustified(4)
                        << (Project.ApplyToTags.value().isEmpty() ? Configs::DBPrefix.value() : "") + Name
                        << (Project.ApplyToTags.value().isEmpty() ? ""
                            : "[" + (Project.ApplyToTags.value() == "*" ? "Apply to all" : TagProjectNames.join(", ")) + "]")
                        ;
            }
        }
    }

    qInfo() << "";

    qint32 ProjectID;
    while (true) {
        qStdout()
                << "For which project do you want to create a new "
                << (_chooseScope == enuChooseCreateMigrationScope::local ? "local" : "db")
                << " migration file?"
                << " "
                << reverse("[") << reverse(bold("c")) << reverse("ancel]")
                << " "
                << reverse("[") << reverse(bold("1")) << reverse("]")
                ;

        if (Projects.length() > 1) {
            qStdout()
                    << reverse(" to ")
                    << reverse("[") << reverse(bold(QString::number(Projects.length()))) << reverse("]")
                    ;
        }
        qStdout() << " ";
        qStdout().flush();

        QString value = qStdIn().readLine().trimmed();

        if (value.isEmpty())
            continue;

        if (value == "c")
            return false;

        bool ok = false;
        ProjectID = value.toInt(&ok);

        if (ok) {
            if ((ProjectID <= 0) || (ProjectID > Projects.length()))
                qStdout() << "Input must be between 1 and " << Projects.length() << endl;
            else {
                qInfo().noquote().nospace()
                        << "Your choose: ["
                        << ProjectID
                        << "- "
                        << (_chooseScope == enuChooseCreateMigrationScope::local ? "" : Configs::DBPrefix.value())
                        << Projects[ProjectID - 1].Name
                        << "]"
                ;
                break;
            }
        } else
            qStdout() << "Invalid input " << value << endl;
    }
    qInfo() << "";

    QString MigrationLabel;
    while (true) {
        qStdout()
                << "Enter label of new migration file"
                << " "
                << reverse("[only ") << reverse(bold("a-z A-Z 0-9")) << reverse(" and ") << reverse(bold("_")) << reverse(" allowed, maximum of 64 characters]")
                << " or "
                << reverse("[") << reverse(bold("empty")) << reverse(" for cancel]")
                << " : "
                ;
        qStdout().flush();

        MigrationLabel = qStdIn().readLine().trimmed();

        if (MigrationLabel.isEmpty())
            return false;

        if (QRegExp("[a-zA-Z0-9_]+").exactMatch(MigrationLabel) == false) {
            MigrationLabel = "";
            qStdout() << "Invalid characters in source label." << endl;
            continue;
        }

        if (MigrationLabel.length() > 64) {
            MigrationLabel = "";
            qStdout() << "Source label length must be <= 64 characters" << endl;
            continue;
        }

        //ok:
        break;
    }

//    qDebug() << "***************"
//             << SourceID
//             << Sources[SourceID - 1].SourceName
//             << Sources[SourceID - 1].SourceIndex
//             << Sources[SourceID - 1].DBIndex
//             ;

    _projectIndex = Projects[ProjectID - 1].Index;
//    stuProject &SelectedProject = Configs::Projects[_projectIndex];

    _fileName = QString("m%1_%2_%3.%4")
                .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"))
                .arg(Projects[ProjectID - 1].Name)
                .arg(MigrationLabel)
                .arg(_chooseScope == enuChooseCreateMigrationScope::local ? "sh" : "sql")
                ;

    _fullFileName = QString("%1/%2/%3")
                    .arg(BaseFolder.path())
                    .arg(Projects[ProjectID - 1].Name)
                    .arg(_chooseScope == enuChooseCreateMigrationScope::local ? "local" : "db")
                    ;

    QDir().mkpath(_fullFileName);

    _fullFileName = _fullFileName + "/" + _fileName;

    return true;
}

struct stuMigrationFileInfo {
    QString MigrationName;
    QString FileName;
    QString Scope; //db, local
    QString FullFileName;

    stuMigrationFileInfo(const stuMigrationFileInfo &_other) :
        MigrationName(_other.MigrationName),
        FileName(_other.FileName),
        Scope(_other.Scope),
        FullFileName(_other.FullFileName)
    { ; }

    stuMigrationFileInfo(
            const QString &_migrationName,
            const QString &_fileName,
            const QString &_scope,
            const QString &_fullFileName
        ) :
        MigrationName(_migrationName),
        FileName(_fileName),
        Scope(_scope),
        FullFileName(_fullFileName)
    { ; }
};
//key: MigrationName
typedef QMap<QString, stuMigrationFileInfo> MigrationFileInfoMap;

inline void dump(MigrationFileInfoMap &_var) {
    int maxWidth_Name = QString("File Name").length();
    foreach (auto val, _var) {
        maxWidth_Name = max(maxWidth_Name, val.FileName.length());
    }

    qDebug().noquote() //.nospace()
            << "     "
//#ifdef QT_DEBUG
//            << QString("key").leftJustified(60)
//#endif
            << QString("File Name").leftJustified(maxWidth_Name)
            << QString("Scope").leftJustified(8)
            << "Full File Name"
            ;
    qDebug().noquote()
            << QString(150, '-');

    QString PWD = QDir::currentPath();
    auto RelativeFileNameToPWD = [&PWD](QString _fileName) -> QString {
        if ((PWD.isEmpty() == false) && (_fileName.indexOf(PWD) == 0))
            _fileName = "." + _fileName.mid(PWD.length());
        return _fileName;
    };

    int idx = 1;
    for (MigrationFileInfoMap::const_iterator it = _var.constBegin();
        it != _var.constEnd();
        it++
    ) {
        QString key = it.key();
        const stuMigrationFileInfo &val = it.value();

        qDebug().noquote() //.nospace()
                << QString::number(idx).rightJustified(4) + ")"
//#ifdef QT_DEBUG
//                << key.leftJustified(60)
//#endif
                << val.FileName.leftJustified(maxWidth_Name)
                << val.Scope.leftJustified(8)
                << RelativeFileNameToPWD(val.FullFileName)
                ;

        ++idx;
    }
}

struct stuProjectMigrationFileInfo {
    QString MigrationName;
    QString FileName;
    QString Scope; //db, local
    QString Project;
    QString FullFileName;
    bool IsFromApplyToAll;

    stuProjectMigrationFileInfo(const stuProjectMigrationFileInfo &_other) :
        MigrationName(_other.MigrationName),
        FileName(_other.FileName),
        Scope(_other.Scope),
        Project(_other.Project),
        FullFileName(_other.FullFileName),
        IsFromApplyToAll(_other.IsFromApplyToAll)
    { ; }

    stuProjectMigrationFileInfo(
        const QString &_migrationName,
        const QString &_fileName,
        const QString &_scope,
        const QString &_project,
        const QString &_fullFileName,
        bool _isFromApplyToAll = false
    ) :
        MigrationName(_migrationName),
        FileName(_fileName),
        Scope(_scope),
        Project(_project),
        FullFileName(_fullFileName),
        IsFromApplyToAll(_isFromApplyToAll)
    { ; }
};
//key: MigrationName
typedef QMap<QString, stuProjectMigrationFileInfo> ProjectMigrationFileInfoMap;

//inline void dumpHeaders(QStringList &_headers, QList<quint32> &_maxWidth)
//{
//}

inline void dump(ProjectMigrationFileInfoMap &_var, bool _renderForAppliedHistoryItem = false) {
//    QStringList Headers = QStringList()
//        << "File Name"
//        << "Scope"
//        << "Project"
//        << (_renderForAppliedHistoryItem ? "Applied At" : "Full File Name")
//    ;

    int maxWidth_Name = QString("File Name").length(),
        maxWidth_Project = QString("Project").length();

    foreach (auto val, _var) {
        maxWidth_Name = max(maxWidth_Name, val.FileName.length());
        maxWidth_Project = max(maxWidth_Project, ((val.Scope == "local" ? "" : Configs::DBPrefix.value()) + val.Project).length() + 2);
    }

    qDebug().noquote() //.nospace()
            << "     "
//#ifdef QT_DEBUG
//            << QString("key").leftJustified(60)
//#endif
            << QString("File Name").leftJustified(maxWidth_Name)
            << QString("Scope").leftJustified(8)
            << QString("Project").leftJustified(maxWidth_Project)
            << (_renderForAppliedHistoryItem ? "Applied At" : "Full File Name")
            ;
    qDebug().noquote()
            << QString(150, '-');

    QString PWD = QDir::currentPath();
    auto RelativeFileNameToPWD = [&PWD](QString _fileName) -> QString {
        if ((PWD.isEmpty() == false) && (_fileName.indexOf(PWD) == 0))
            _fileName = "." + _fileName.mid(PWD.length());
        return _fileName;
    };

    int idx = 1;
    for (ProjectMigrationFileInfoMap::const_iterator it = _var.constBegin();
        it != _var.constEnd();
        it++
    ) {
        QString key = it.key();
        const stuProjectMigrationFileInfo &val = it.value();

        //-----------------------------------
        QString ProjectName = "";
        if (val.IsFromApplyToAll)
            ProjectName += "[";
        if (val.Scope != "local")
            ProjectName += Configs::DBPrefix.value();
        ProjectName += val.Project;
        if (val.IsFromApplyToAll)
            ProjectName += "]";

        //-----------------------------------
        qDebug().noquote() //.nospace()
                << QString::number(idx).rightJustified(4) + ")"
//#ifdef QT_DEBUG
//                << key.leftJustified(60)
//#endif
                << val.FileName.leftJustified(maxWidth_Name)
                << val.Scope.leftJustified(8)
                << ProjectName.leftJustified(maxWidth_Project)
                << RelativeFileNameToPWD(val.FullFileName)
                ;

        ++idx;
    }
}

struct stuHistoryAppliedItem {
    QString MigrationName;
//    QString FullFileName;
    QDateTime AppliedAt;

    stuHistoryAppliedItem(const stuHistoryAppliedItem &_other) :
        MigrationName(_other.MigrationName),
//        FullFileName(_other.FullFileName),
        AppliedAt(_other.AppliedAt)
    { ; }

    stuHistoryAppliedItem(
        const QString &_migrationName,
//        const QString &_fullFileName,
        const QDateTime &_appliedAt
    ) :
        MigrationName(_migrationName),
//        FullFileName(_fullFileName),
        AppliedAt(_appliedAt)
    { ; }
};

struct stuMigrationHistory {
    QString Project; //App, App.CommonFuncs
    QString Scope; //db, local
    QString HistoryFileOrTableName;
    QMap<QString, stuHistoryAppliedItem> AppliedItems;
    bool Exists;

    stuMigrationHistory(const stuMigrationHistory &_other) :
        Project(_other.Project),
        Scope(_other.Scope),
        HistoryFileOrTableName(_other.HistoryFileOrTableName),
        AppliedItems(_other.AppliedItems),
        Exists(_other.Exists)
    { ; }

    stuMigrationHistory(
        const QString &_project,
        const QString &_scope,
        const QString &_historyFileOrTableName,
        const QMap<QString, stuHistoryAppliedItem> &_appliedItems
    ) :
        Project(_project),
        Scope(_scope),
        HistoryFileOrTableName(_historyFileOrTableName),
        AppliedItems(_appliedItems),
        Exists(false)
    { ; }
};
//key: Source + Scope
typedef QMap<QString, stuMigrationHistory> MigrationHistoryMap;

inline void dump(const QMap<QString, stuHistoryAppliedItem> &_var) {
    qDebug().noquote() //.nospace()
            << "    "
            << QString("MigrationName").leftJustified(50)
            << QString("AppliedAt").leftJustified(22)
//            << QString("FullFileName")
            ;
    qDebug().noquote()
            << "    "
            << QString(100, '-');

//    QString PWD = QDir::currentPath();
//    auto RelativeFileNameToPWD = [&PWD](QString _fileName) -> QString {
//        if ((PWD.isEmpty() == false) && (_fileName.indexOf(PWD) == 0))
//            _fileName = "." + _fileName.mid(PWD.length());
//        return _fileName;
//    };

    for (QMap<QString, stuHistoryAppliedItem>::const_iterator it = _var.constBegin();
        it != _var.constEnd();
        it++
    ) {
        QString key = it.key();
        const stuHistoryAppliedItem &val = it.value();

        qDebug().noquote() //.nospace()
                << "    "
                << val.MigrationName.leftJustified(50)
                << val.AppliedAt.toString("yyyy-MM-dd hh:mm:ss a").leftJustified(22)
//                << RelativeFileNameToPWD(val.FullFileName)
                ;
    }
}

inline void dump(MigrationHistoryMap &_var) {
    qDebug().noquote() //.nospace()
            << QString("Project").leftJustified(20)
            << QString("Scope").leftJustified(5)
            << QString("Count").leftJustified(6)
            << QString("Exists").leftJustified(6)
            << QString("HistoryFileOrTableName")
            ;
    qDebug().noquote()
            << QString(150, '-');

    for (MigrationHistoryMap::const_iterator it = _var.constBegin();
        it != _var.constEnd();
        it++
    ) {
        QString key = it.key();
        const stuMigrationHistory &val = it.value();

        qDebug().noquote() //.nospace()
                << ((val.Scope == "local" ? "" : Configs::DBPrefix.value()) + val.Project).leftJustified(20)
                << val.Scope.leftJustified(5)
                << QString("%1").arg(val.AppliedItems.count()).leftJustified(6)
                << QString(val.Exists ? "Yes" : "No").leftJustified(6)
                << val.HistoryFileOrTableName
                ;

        if (val.AppliedItems.isEmpty() == false) {
//            qDebug() << "    applied items:";
            dump(val.AppliedItems);
        }
    }
}

inline void dump(QStringMap &_var) {
    for (QStringMap::const_iterator it = _var.constBegin();
        it != _var.constEnd();
        it++
    ) {
        QString key = it.key();
        QString val = it.value();

        qDebug().noquote().nospace()
                << key.leftJustified(65)
                << val;
    }
}

inline void ExtractMigrationFiles(ProjectMigrationFileInfoMap &_migrationFiles) {
    QDir BaseFolder(Configs::MigrationsFolderName.value());
    ///TODO: check if dir exists

    BaseFolder.makeAbsolute();
//    qDebug() << BaseFolder;

    //Tag => (FileName => FullFileName)
    QMap<QString, QStringMap> ApplyToAllMigrationFiles;

    //get file list of ApplyToTags
    //  db    -> ApplyToAllMigrationFiles
    //  local -> MigrationFiles
    for (size_t idxProject=0; idxProject<Configs::Projects.size(); idxProject++) {
        stuProject &Project = Configs::Projects[idxProject];

        QString ApplyToTags = Project.ApplyToTags.value().trimmed();

        if (ApplyToTags.isEmpty())
            continue;

        if (BaseFolder.cd(Project.Name.value())) {
            //db
            if ((Configs::LocalOnly.value() == false)
//                && (Configs::Projects.size() > 0)
                && (Configs::RunningParameters.ProjectAllowedDBServers.isEmpty() == false)
                && BaseFolder.cd("db")
            ) {
                QDirIterator itdb(BaseFolder.path(), QDir::Files);
                while (itdb.hasNext()) {
                    QString FullFileName = itdb.next();
                    QString FileName = BaseFolder.relativeFilePath(FullFileName);
                    if (QRegExp(REGEX_PATTERN_MIGRATION_FILENAME).exactMatch(FileName) == false) {
                        if (QRegExp(REGEX_PATTERN_MIGRATION_LOG_FILENAME).exactMatch(FileName) == false)
                            qDebug() << "invalid file name:" << FileName << "in" << BaseFolder.path();
                        continue;
                    }

//                    QString MigrationName = QString("%1:db/*").arg(FileName);

//                    qDebug() << "[*]" << FullFileName << "\t\t" << FileName;

                    if (ApplyToTags == "*") {
                        if (ApplyToAllMigrationFiles.contains("*"))
                            ApplyToAllMigrationFiles["*"].insert(FileName, FullFileName);
                        else
                            ApplyToAllMigrationFiles.insert("*", QStringMap({ { FileName, FullFileName } }));
                    }
                    else {
                        QStringList Tags = ApplyToTags.split(",", QString::SkipEmptyParts);

                        foreach (QString Tag, Tags) {
                            Tag = Tag.trimmed(); //.toLower();
                            if (Tag.isEmpty())
                                continue;

                            if (ApplyToAllMigrationFiles.contains(Tag))
                                ApplyToAllMigrationFiles[Tag].insert(FileName, FullFileName);
                            else
                                ApplyToAllMigrationFiles.insert(Tag, QStringMap({ { FileName, FullFileName } }));
                        }
                    }
                }
                BaseFolder.cdUp();
            }

            //local
            if ((Configs::DBOnly.value() == false) && BaseFolder.cd("local")) {
                QDirIterator itlocal(BaseFolder.path(), QDir::Files | QDir::Hidden);
                while (itlocal.hasNext()) {
                    QString FullFileName = itlocal.next();
                    QString FileName = BaseFolder.relativeFilePath(FullFileName);
                    if (QRegExp(REGEX_PATTERN_MIGRATION_FILENAME).exactMatch(FileName) == false) {
                        if (FileName != Configs::LocalHistoryFileName.value())
                            if (QRegExp(REGEX_PATTERN_MIGRATION_LOG_FILENAME).exactMatch(FileName) == false)
                                qDebug() << "invalid file name:" << FileName << "in" << BaseFolder.path();

                        continue;
                    }

                    QString MigrationName = QString("%1:local/%2").arg(FileName).arg(Project.Name.value());

//                    qDebug() << "[*]" << FullFileName << "\t\t" << MigrationName;

                    _migrationFiles.insert(MigrationName,
                                           stuProjectMigrationFileInfo(
                                               MigrationName,
                                               FileName,
                                               "local",
                                               Project.Name.value(),
                                               FullFileName,
                                               true
                                           ));
                }
                BaseFolder.cdUp();
            }

            //
            BaseFolder.cdUp();
        }
    }

    //get list of exists sources
    //  extract file list of every source
//    if (Configs::Projects.size() == 0)
    if (Configs::RunningParameters.ProjectAllowedDBServers.isEmpty())
        return;

    for (size_t idxProject=0; idxProject<Configs::Projects.size(); idxProject++) {
        stuProject &Project = Configs::Projects[idxProject];

        if ((Configs::Project.value().isEmpty() == false)
            && (Project.Name.value() != Configs::Project.value())
            && (Project.ApplyToTags.value().isEmpty())
        )
            continue;

        QStringList LookupScopes;

        if ((Configs::LocalOnly.value() == false) && Project.AllowDB.value()) {
            if (ApplyToAllMigrationFiles.isEmpty() == false) {
                for (QMap<QString, QStringMap>::const_iterator it1 = ApplyToAllMigrationFiles.constBegin();
                    it1 != ApplyToAllMigrationFiles.constEnd();
                    it1++
                ) {
                    QString TagName = it1.key();

                    if ((TagName == "*")
                        || ((Project.Tags.value().isEmpty() == false)
                            && (("," + Project.Tags.value() + ",").indexOf("," + TagName + ",") >= 0)
                        )
                    ) {
                        for (QStringMap::const_iterator it = it1->constBegin();
                            it != it1->constEnd();
                            it++
                        ) {
                            QString FileName = it.key();
                            QString FullFileName = it.value();

                            foreach (QString AllowedDBServer, Configs::RunningParameters.ProjectAllowedDBServers[Project.Name.value()]) {
                                QString MigrationName = QString("%1:db/%2@%3")
                                                        .arg(FileName)
        //                                                .arg(Configs::DBPrefix.value())
                                                        .arg(Project.Name.value())
                                                        .arg(AllowedDBServer)
                                                        ;

        //                        qDebug() << "[" << Project.Name.value() << "]" << FullFileName << "\t\t" << MigrationName;

                                _migrationFiles.insert(MigrationName,
                                                       stuProjectMigrationFileInfo(
                                                           MigrationName,
                                                           FileName,
                                                           "db",
                                                           Project.Name.value() + "@" + AllowedDBServer,
                                                           FullFileName,
                                                           true
                                                      ));
                            }
                        }
                    }
                }
            } //ApplyToAllMigrationFiles.isEmpty() == false

            LookupScopes.append("db");
        } //Project.AllowDB

        if ((Configs::DBOnly.value() == false) && Project.AllowLocal.value())
            LookupScopes.append("local");

        if ((LookupScopes.isEmpty() == false) && BaseFolder.cd(Project.Name.value())) {
            foreach (QString Scope, LookupScopes) {
                if (BaseFolder.cd(Scope)) {
                    QDirIterator itFile(BaseFolder.path(), QDir::Files | QDir::Hidden);
                    while (itFile.hasNext()) {
                        QString FullFileName = itFile.next();
                        QString FileName = BaseFolder.relativeFilePath(FullFileName);
                        if (QRegExp(REGEX_PATTERN_MIGRATION_FILENAME).exactMatch(FileName) == false) {
                            if (FileName != Configs::LocalHistoryFileName.value())
                                if (QRegExp(REGEX_PATTERN_MIGRATION_LOG_FILENAME).exactMatch(FileName) == false)
                                    qDebug() << "invalid file name:" << FileName << "in" << BaseFolder.path();

                            continue;
                        }

                        if (Scope == "db") {
                            foreach (QString AllowedDBServer, Configs::RunningParameters.ProjectAllowedDBServers[Project.Name.value()]) {
                                QString MigrationName = QString("%1:db/%2@%3")
                                                        .arg(FileName)
//                                                        .arg(Configs::DBPrefix.value())
                                                        .arg(Project.Name.value())
                                                        .arg(AllowedDBServer)
                                                        ;

        //                        qDebug() << "[" << Project.Name.value() << "]" << FullFileName << "\t\t" << MigrationName;

                                _migrationFiles.insert(MigrationName,
                                                      stuProjectMigrationFileInfo(
                                                          MigrationName,
                                                          FileName,
                                                          "db",
                                                          Project.Name.value() + "@" + AllowedDBServer,
                                                          FullFileName
                                                      ));
                            }
                        } else { //not db
                            QString MigrationName = QString("%1:local/%2")
                                                    .arg(FileName)
                                                    .arg(Project.Name.value());

    //                        qDebug() << "[" << Source.Name.value() << "]" << FullFileName << "\t\t" << MigrationName;

                            _migrationFiles.insert(MigrationName,
                                                  stuProjectMigrationFileInfo(
                                                      MigrationName,
                                                      FileName,
                                                      "local",
                                                      Project.Name.value(),
                                                      FullFileName
                                                  ));
                        }
                    }
                    BaseFolder.cdUp();
                }

            } //foreach (QString Scope, LookupScopes)

            BaseFolder.cdUp();
        } //LookupScopes.isEmpty() == false
    } //for Projects
}

inline void ExtractMigrationHistories(MigrationHistoryMap &_migrationHistories) {
    QDir BaseFolder(Configs::MigrationsFolderName.value());
    ///TODO: check if dir exists

    BaseFolder.makeAbsolute();

    //extract db histories from config::db::schema
    auto fnCheckDBSource = [&](const QString &_projectName, const QString &_dbServerName) {
        QString ProjectDBServerName = _projectName + "@" + _dbServerName;

        stuMigrationHistory MigrationHistory(
                    ProjectDBServerName,
                    "db",
                    Configs::GlobalHistoryTableName.value(),
                    {}
                    );

        try {
            if (Configs::RunningParameters.NonExistsProjectDBConnectionStrings.contains(ProjectDBServerName) == false) {
                //check tblMigrations
                clsDAC DAC(ProjectDBServerName);
                QString Qry = R"(
                    SELECT TABLE_NAME
                      FROM information_schema.TABLES
                     WHERE TABLE_SCHEMA=?
                       AND TABLE_NAME=?
                )";
                clsDACResult ResultTable = DAC.execQuery("", Qry, {
                                                             Configs::DBPrefix.value() + _projectName,
                                                             Configs::GlobalHistoryTableName.value(),
                                                         });

                if (ResultTable.toJson(true).object().isEmpty()) {
                    MigrationHistory.Exists = false;
                } else {
                    MigrationHistory.Exists = true;

                    //check status field
                    QString Qry = R"(
                        SELECT COLUMN_NAME
                          FROM information_schema.COLUMNS
                         WHERE TABLE_SCHEMA=?
                           AND TABLE_NAME=?
                           AND COLUMN_NAME='migStatus'
                    )";
                    clsDACResult ResultColumn = DAC.execQuery("", Qry, {
                                                                 Configs::DBPrefix.value() + _projectName,
                                                                 Configs::GlobalHistoryTableName.value(),
                                                             });

                    if (ResultColumn.toJson(true).object().isEmpty())
                        Qry = QString("SELECT migName, migAppliedAt FROM %1")
                              .arg(Configs::GlobalHistoryTableName.value());
                    else
                        Qry = QString("SELECT migName, migAppliedAt FROM %1 WHERE migStatus!='R'")
                              .arg(Configs::GlobalHistoryTableName.value());

                    //------------------------
                    clsDACResult Result = DAC.execQuery("", Qry);
                    QJsonDocument ResultRows = Result.toJson(false);
                    QVariantList List = ResultRows.toVariant().toList();
                    if (List.isEmpty() == false) {
                        ///TODO: complete this
                        foreach (auto Row, List) {
                            QVariantMap Map = Row.toMap();
                            QString migName = Map.value("migName").toString();
                            QDateTime migAppliedAt = QDateTime::fromString(Map.value("migAppliedAt").toString(), Qt::ISODate);

                            stuHistoryAppliedItem HistoryItem(
                                        migName,
//                                        MigrationFullFileName,
                                        migAppliedAt
                                    );

                            MigrationHistory.AppliedItems.insert(
                                        migName,
                                        HistoryItem);
                        }
                    }
                }
            }
        } catch (std::exception &exp) {
//            qDebug() << exp.what();
        }

        _migrationHistories.insert(
                    ProjectDBServerName,
                    MigrationHistory
                    );
    };

    if ((Configs::LocalOnly.value() == false) && (Configs::RunningParameters.ProjectAllowedDBServers.isEmpty() == false)) {
        for (QMap<QString, QStringList>::const_iterator it = Configs::RunningParameters.ProjectAllowedDBServers.constBegin();
             it != Configs::RunningParameters.ProjectAllowedDBServers.constEnd();
             it++
            ) {
            QString ProjectName = it.key();
            QStringList DBServerNames = it.value();

            foreach (QString DBServerName, DBServerNames) {
//                qDebug() << Source.Name.value() << "DB" << Source.DB.size();
                fnCheckDBSource(ProjectName, DBServerName);
            }
        }
    }

    //extract local histories from config::sources
    auto fnCheckLocalSource = [&](const QString &_projectName) {
        QString HistoryFullFileName = QString("%1/%2/local/%3")
                                      .arg(BaseFolder.path())
                                      .arg(_projectName)
                                      .arg(Configs::LocalHistoryFileName.value());

//        qDebug() << HistoryFullFileName;

        stuMigrationHistory MigrationHistory(
                    _projectName,
                    "local",
                    HistoryFullFileName,
                    {}
                    );
        MigrationHistory.Exists = false;

        QFile File(HistoryFullFileName);
        if (File.exists() && File.open(QIODevice::ReadOnly | QIODevice::Text)) {
            MigrationHistory.Exists = true;

            while (File.atEnd() == false) {
                QByteArray Line = File.readLine().trimmed();

                //#Migration Name   Applied At
                if (Line.isEmpty() || Line.startsWith('#'))
                    continue;

                QList<QByteArray> Parts = Line.split('\t');

//                QString MigrationFullFileName = QString("%1/%2/local/%3")
//                                                .arg(BaseFolder.path())
//                                                .arg(_sourceName)
//                                                .arg(QString(Parts[0]));

                stuHistoryAppliedItem HistoryItem(
                            Parts[0],
//                            MigrationFullFileName,
                            QDateTime::fromString(Parts[1], Qt::ISODate) //"yyyy/MM/dd HH:mm:ss")
                        );

                MigrationHistory.AppliedItems.insert(
                            Parts[0],
                            HistoryItem);
            }

            File.close();
        }

        _migrationHistories.insert(
                    _projectName,
                    MigrationHistory
                    );
    };

    if (Configs::DBOnly.value() == false) {
        if (Configs::Projects.size() > 0) {
            for (size_t idx=0; idx<Configs::Projects.size(); idx++) {
                stuProject &Project = Configs::Projects[idx];

                if (Project.AllowLocal.value() == false)
                    continue;

                fnCheckLocalSource(Project.Name.value());
            }
        }
    }
}

inline void RemoveAppliedFromList(
    ProjectMigrationFileInfoMap &_migrationFiles,
    const MigrationHistoryMap &_migrationHistories
) {
    if (_migrationFiles.isEmpty() || _migrationHistories.isEmpty())
        return;

    for (MigrationHistoryMap::const_iterator itHistoryMap = _migrationHistories.constBegin();
         itHistoryMap != _migrationHistories.constEnd();
         itHistoryMap++
        ) {
        const stuMigrationHistory &MigrationHistory = itHistoryMap.value();

        if (MigrationHistory.AppliedItems.isEmpty())
            continue;

        for (QMap<QString, stuHistoryAppliedItem>::const_iterator itHistoryItem = MigrationHistory.AppliedItems.constBegin();
             itHistoryItem != MigrationHistory.AppliedItems.constEnd();
             itHistoryItem++
            ) {
            QString key = itHistoryItem.key();
            const stuHistoryAppliedItem &HistoryAppliedItem = itHistoryItem.value();

            ProjectMigrationFileInfoMap::iterator itMigrationFile = _migrationFiles.begin();
            while (itMigrationFile != _migrationFiles.end()) {
                stuProjectMigrationFileInfo &ProjectMigrationFileInfo = itMigrationFile.value();

//                qDebug() << ">>>>>>>>>>>>>>>>> checking:" << endl
//                         << "\t[" << ProjectMigrationFileInfo.FileName << "<=>" << HistoryAppliedItem.MigrationName << "]" << endl
//                         << "\t[" << ProjectMigrationFileInfo.Scope << "<=>" << MigrationHistory.Scope << "]" << endl
//                         << "\t[" << //(ProjectMigrationFileInfo.Scope == "local" ? "" : Configs::DBPrefix.value()) +
//                            ProjectMigrationFileInfo.Project << "<=>" << MigrationHistory.Project << "]"
//                         ;

                if ((ProjectMigrationFileInfo.FileName == HistoryAppliedItem.MigrationName)
                        && (ProjectMigrationFileInfo.Scope == MigrationHistory.Scope)
                        && (/*(ProjectMigrationFileInfo.Scope == "local" ? "" : Configs::DBPrefix.value()) + */
                            ProjectMigrationFileInfo.Project == MigrationHistory.Project)
                    ) {
//                    qDebug() << "FOUND";

                    itMigrationFile = _migrationFiles.erase(itMigrationFile);
                    break;
                } else
                    ++itMigrationFile;
            }
        }
    }
}

inline void CreateDBIfNotExists(const QString &_projectName, QString &_dbServerName) {
    qDebug() << "CreateDBIfNotExists" << Configs::DBPrefix.value() << _projectName << _dbServerName;
    qDebug() << Configs::RunningParameters.NonExistsProjectDBConnectionStrings.keys();

    QString ProjectDestinationKey = _projectName + "@" + _dbServerName;

    if (Configs::RunningParameters.NonExistsProjectDBConnectionStrings.contains(ProjectDestinationKey) == false)
        return;

    qDebug() << "creating database"
             << Configs::DBPrefix.value() + _projectName
             << "because this is not exist in"
             << _dbServerName;

    clsDAC DAC(_dbServerName);
    QString Qry = QString(R"(
        CREATE DATABASE `%1`
                COLLATE '%2'
        )")
        .arg(Configs::DBPrefix.value() + _projectName)
        .arg(Configs::NewDBCollation.value())
    ;
    clsDACResult Result = DAC.execQuery("", Qry);

    qDebug() << Result.toJson(true);
    Configs::RunningParameters.NonExistsProjectDBConnectionStrings.remove(ProjectDestinationKey);

    QString ConnStringWithSchema = Configs::RunningParameters.ProjectDBConnectionStrings[ProjectDestinationKey];

    qDebug() << "setConnectionString" << ProjectDestinationKey << "=" << ConnStringWithSchema;
    clsDAC::setConnectionString(ConnStringWithSchema, ProjectDestinationKey);
}

inline bool IsMigrationFileApplied(const stuProjectMigrationFileInfo &_migrationFile) {
    if (_migrationFile.Scope == "db") {
        QStringList Parts = _migrationFile.Project.split('@');
        QString Schema = Parts[0];

        CreateDBIfNotExists(Schema, Parts[1]);

        clsDAC DAC(_migrationFile.Project);

        //check table field
        QString Qry = R"(
            SELECT TABLE_NAME
              FROM information_schema.TABLES
             WHERE TABLE_SCHEMA=?
               AND TABLE_NAME=?
        )";
        clsDACResult ResultTable = DAC.execQuery("", Qry, {
                                                     Configs::DBPrefix.value() + Schema,
                                                     Configs::GlobalHistoryTableName.value(),
                                                 });

        if (ResultTable.toJson(true).object().isEmpty())
            return false;

        //check status field
        Qry = R"(
            SELECT COLUMN_NAME
              FROM information_schema.COLUMNS
             WHERE TABLE_SCHEMA=?
               AND TABLE_NAME=?
               AND COLUMN_NAME='migStatus'
        )";
        clsDACResult ResultColumn = DAC.execQuery("", Qry, {
                                                     Configs::DBPrefix.value() + Schema,
                                                     Configs::GlobalHistoryTableName.value(),
                                                 });

        if (ResultColumn.toJson(true).object().isEmpty())
            Qry = QString("SELECT * FROM %1%2.%3 WHERE migName=?")
                  .arg(Configs::DBPrefix.value())
                  .arg(Schema)
                  .arg(Configs::GlobalHistoryTableName.value());
        else
            Qry = QString("SELECT * FROM %1%2.%3 WHERE migName=? AND migStatus!='R'")
                  .arg(Configs::DBPrefix.value())
                  .arg(Schema)
                  .arg(Configs::GlobalHistoryTableName.value());

        clsDACResult Result = DAC.execQuery("", Qry, {
                                                _migrationFile.FileName
                                            });

        return (Result.toJson(true).object().isEmpty() == false);

    } else { //local
        QFileInfo FileInfo(_migrationFile.FullFileName);

        if (FileInfo.exists() == false)
//            throw exTargomanBase("File not found");
            return false;

        QString MigrationHistoryFileName = QString("%1/%2")
                                           .arg(FileInfo.path())
                                           .arg(Configs::LocalHistoryFileName.value());

        QFile File(MigrationHistoryFileName);

        if (File.open(QFile::ReadWrite | QFile::Text) == false)
//            throw exTargomanBase("Could not create or open history file");
            return false;

        QTextStream stream(&File);

        bool Found = false;

        while (stream.atEnd() == false) {
            QString Line = stream.readLine();
            if (Line.startsWith(_migrationFile.FileName)) {
                Found = true;
                break;
            }
        }

        File.close();

        return Found;
    }
}

inline void MarkMigrationFile(const stuProjectMigrationFileInfo &_migrationFile) {
    if (_migrationFile.Scope == "db") {
        QStringList Parts = _migrationFile.Project.split('@');
        QString Schema = Parts[0];

        CreateDBIfNotExists(Schema, Parts[1]);

        clsDAC DAC(_migrationFile.Project);

        QString Qry = QString(R"(
            INSERT INTO %1%2.%3
               SET migName=?
                 , migAppliedAt=NOW()
            )")
            .arg(Configs::DBPrefix.value())
            .arg(Schema)
            .arg(Configs::GlobalHistoryTableName.value())
        ;

        clsDACResult Result = DAC.execQuery("", Qry, {
                                                _migrationFile.FileName
                                            });
    } else { //local
        QFileInfo FileInfo(_migrationFile.FullFileName);

        if (FileInfo.exists() == false)
            throw exTargomanBase("File not found");

        QString MigrationHistoryFileName = QString("%1/%2")
                                           .arg(FileInfo.path())
                                           .arg(Configs::LocalHistoryFileName.value());

        QFile File(MigrationHistoryFileName);

        if (File.open(QFile::ReadWrite | QFile::Text) == false)
            throw exTargomanBase("Could not create or open history file");

        QTextStream stream(&File);

        QString HistoryItem = "";

        if (File.size() > 0) {
            if (File.size() > 1) {
                stream.seek(File.size() - 1);
                QString b = stream.read(1);
                if (b != "\n")
                    HistoryItem = "\n";
            }
            stream.seek(File.size());
        } else
            stream << "#Migration Name   Applied At" << endl;

        HistoryItem += QString("%1\t%2").arg(_migrationFile.FileName).arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"));
        stream << HistoryItem << endl;

        File.close();
    }
}

inline void RunMigrationFile(const stuProjectMigrationFileInfo &_migrationFile, bool _run = true) {
    //1: check migration not applied or marked before
    if (IsMigrationFileApplied(_migrationFile))
        return;

    //2: run migration
    if (_migrationFile.Scope == "db") {
        QStringList Parts = _migrationFile.Project.split('@');
        QString Schema = Parts[0];

        CreateDBIfNotExists(Schema, Parts[1]);

        if (_run) {
            clsDAC DAC(_migrationFile.Project);

            QFile File(_migrationFile.FullFileName);

            if (!File.open(QFile::ReadOnly | QFile::Text))
                throw exTargomanBase("File not found");

            QTextStream Stream(&File);
            QString Qry = Stream.readAll().trimmed();
            File.close();

            if (Qry.isEmpty() == false) {
                Qry = Qry
                    .replace("{{dbprefix}}", Configs::DBPrefix.value())
                    .replace("{{Schema}}", Schema)
                    .replace("{{GlobalHistoryTableName}}", Configs::GlobalHistoryTableName.value())
                ;
//                clsDACResult MainResult = DAC.execQuery("", Qry);

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

//                            QString SmallQry = QString("DELIMITER %1").arg(Delimiter);
//                            qDebug() << "Executing query" << SmallQry;
//                            clsDACResult MainResult = DAC.execQuery("", SmallQry);
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
                            qDebug() << "\t\tExecuting query" << SmallQry.left(50) << "...";
                            clsDACResult MainResult = DAC.execQuery("", SmallQry);
                        }
                    }

                    Qry = Qry.trimmed();
                }
            }
        }
    } else { //local
        //1: run migration
        if (_run)  {
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

                QStringList Aruments;
                Aruments << "--active-running-mode" << Configs::ActiveRunningMode.value();

                if (Configs::DBPrefix.value().isEmpty() == false)
                    Aruments << "--dbprefix" << Configs::DBPrefix.value();

                QProcess MigrationProcess;
                MigrationProcess.start(_migrationFile.FullFileName, Aruments);

                if (!MigrationProcess.waitForFinished())
                    throw exTargomanBase("Execution failed");

                QByteArray ExResult = MigrationProcess.readAll();
                if (ExResult.isEmpty() == false) {
                    QFile LogFile(QString("%1.log").arg(_migrationFile.FullFileName));
                    if (LogFile.open(QFile::WriteOnly | QFile::Text)) {
                        QTextStream out(&LogFile);
                        out << ExResult;
                        LogFile.close();
                    }
                }
            }
        }
    }

    //3: add to history
    MarkMigrationFile(_migrationFile);
}

inline QString GetSymlinkTarget(QString _path) {

    _path = _path.trimmed();
    if (_path.right(1) == "/")
        _path = _path.left(_path.length() - 1);

    QStringList Parts = _path.split("/");

    QString Result = "";

    foreach (QString Part, Parts) {
        QString Item = QFile::symLinkTarget((Result.isEmpty() ? "" : Result + "/") + Part);
        if (Item.isEmpty())
            Result = Result + "/" + Part;
        else
            Result = Item;
    }

    return Result;
}

} // namespace Targoman::Migrate::Commands

#endif // MIGRATION_TOOL_COMMON_H
