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

#include "cmdShowConf.h"
#include "../Configs.h"

namespace Targoman::Migrate::Commands {

cmdShowConf::cmdShowConf()
{ ; }

void cmdShowConf::help()
{
}

bool cmdShowConf::run()
{
//    if (Configs::Sources.size() == 0)
    {
        qInfo() << "nothing to show";
        return true;
    }

//    qInfo().noquote()
//            << QString("Source").leftJustified(20)
//            << "Databases"
//            << endl
//            << QString(100, '-')
//            ;

//    for (size_t idxSource=0; idxSource<Configs::Sources.size(); idxSource++)
//    {
//        stuMigrationSource &Source = Configs::Sources[idxSource];

//        QStringList DBs;
//        if (Source.DB.size() > 0)
//        {
//            for (size_t idxDB=0; idxDB<Source.DB.size(); idxDB++)
//            {
//                stuMigrationDB &DB = Source.DB[idxDB];

//                DBs.append(DB.Schema.value());
//            }
//        }

//        qInfo().noquote()
//                << Source.Name.value().leftJustified(20)
//                << DBs.join(", ")
//                ;
//    }

//    qInfo() << "";

    return true;
}

} // namespace Targoman::Migrate::Commands
