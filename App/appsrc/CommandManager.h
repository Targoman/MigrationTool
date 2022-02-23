/******************************************************************************
#   TargomanMigrate
#
#   Copyright 2014-2020 by Targoman Intelligent Processing <http://tip.co.ir>
#
#   TargomanMigrate is free software: you can redistribute it and/or modify
#   it under the terms of the GNU AFFERO GENERAL PUBLIC LICENSE as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   TargomanMigrate is distributed in the hope that it will be useful,
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

#ifndef TARGOMAN_MIGRATE_COMMANDMANAGER_H
#define TARGOMAN_MIGRATE_COMMANDMANAGER_H

#include "Defs.h"
#include <QObject>

namespace Targoman::Migrate {

class CommandManager : public QObject
{
    Q_OBJECT

public:
    explicit CommandManager(QObject *parent = nullptr);

public slots:
    void slotExecute();
};

} //namespace Targoman::Migrate

#endif // TARGOMAN_MIGRATE_COMMANDMANAGER_H

/*

DBServers/0/Name = devdb1
DBServers/0/Host = 127.0.0.1
DBServers/0/Password = targoman123
DBServers/0/Port = 3306
DBServers/1/Name = proddb1
DBServers/1/Host = 127.0.0.2
DBServers/1/Password = targoman123
DBServers/1/Port = 3306
DBServers/2/Name = proddb2
DBServers/2/Host = 127.0.0.3
DBServers/2/Password = targoman123
DBServers/2/Port = 3306

RunningModes/0/Name = dev
RunningModes/0/DBServers = devdb1
RunningModes/1/Name = prod
RunningModes/1/DBServers = proddb1,proddb2

ActiveRunnigMode = dev

Projects/0/Name = App
Projects/0/AllowDB = false
Projects/0/AllowLocal = true
Projects/0/DBDestinations =
Projects/1/Name = CommonFuncs
Projects/1/AllowDB = true
Projects/1/AllowLocal = false
Projects/1/DBDestinations = devdb1,proddb1,proddb2
Projects/2/Name = I18N
Projects/2/AllowDB = true
Projects/2/AllowLocal = false
Projects/2/DBDestinations = devdb1,proddb1,proddb2
Projects/3/Name = Account
Projects/3/AllowDB = false
Projects/3/AllowLocal = true
Projects/3/DBDestinations =
Projects/4/Name = AAA
Projects/4/AllowDB = true
Projects/4/AllowLocal = false
Projects/4/DBDestinations = devdb1,proddb1
Projects/5/Name = Advert
Projects/5/AllowDB = true
Projects/5/AllowLocal = true
Projects/5/DBDestinations = devdb1,proddb2

*/
