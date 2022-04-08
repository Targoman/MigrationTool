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

namespace Targoman::Migrate::Commands {

cmdNewDBDiff::cmdNewDBDiff()
{ ; }

void cmdNewDBDiff::help()
{
}

bool cmdNewDBDiff::run()
{
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

//    qDebug() << "===================="
//             << SourceIndex
//             << DBIndex
//             ;

    stuProject &SelectedProject = Configs::Projects[ProjectIndex];

    qDebug() << SelectedProject.Name.value();




//    libTargomanCompare


    qInfo().noquote().nospace() << "Creating new migration file: " << FullFileName;



//    QFile File(FullFileName);

//    if (File.open(QFile::WriteOnly | QFile::Text) == false)
//    {
//        qInfo() << "Could not create new migration file.";
//        return true;
//    }

//    QTextStream writer(&File);
//    writer
//        << "/* Migration File: "
//        << FileName
//        << " */"
//        << endl
//        << endl
//        ;
//    File.close();

//    qInfo().noquote() << "Migration file by diff created successfully.";

    return true;
}

} // namespace Targoman::Migrate::Commands
