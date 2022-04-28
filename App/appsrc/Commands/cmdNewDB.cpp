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

#include "cmdNewDB.h"
#include "../Configs.h"
#include <signal.h>
#include <unistd.h>

namespace Targoman::Migrate::Commands {

cmdNewDB::cmdNewDB() { ; }

void cmdNewDB::help() {
}

bool cmdNewDB::run() {
    QString FileName;
    QString FullFileName;
    quint32 ProjectIndex;

    if (ChooseCreateMigrationProperties(
                enuChooseCreateMigrationScope::db,
                FileName,
                FullFileName,
                ProjectIndex
                ) == false)
        return true;

    QFile File(FullFileName);
    if (File.open(QFile::WriteOnly | QFile::Text) == false) {
        qInfo() << "Could not create new migration file.";
        return true;
    }

    QFileInfo info(FullFileName);
    if (info.isSymLink())
        FullFileName = info.symLinkTarget();

    qInfo().noquote().nospace() << "Creating new migration file: " << FullFileName;

    QTextStream writer(&File);
    writer << "/* Migration File: " << FileName << " */" << endl
           << "/* CAUTION: don't forget to use {{dbprefix}} for schemas */" << endl
           << endl;
    File.close();

    qInfo().noquote() << "Empty migration file created successfully.";

    qint64 PID;

    if (QProcess::startDetached(Configs::DefaultEditor.value(),
                                QStringList() << FullFileName,
                                {},
                                &PID) == false)
        throw exTargomanBase("Execution of default editor failed");

    while (kill(PID, 0) == 0) { usleep(1); }

    //----
    if (Configs::AutoGitAdd.value()) {
        QProcess MigrationProcess;
        MigrationProcess.start(QString("git add %1").arg(FullFileName));
        if (MigrationProcess.waitForFinished())
            qInfo().noquote() << "File added to git";
        else
            qInfo().noquote() << "Could not add file to git";
    }

    return true;
}

} // namespace Targoman::Migrate::Commands
