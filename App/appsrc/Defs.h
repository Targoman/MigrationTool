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

#ifndef MIGRATION_TOOL_DEFS_H
#define MIGRATION_TOOL_DEFS_H

#include <QHostAddress>
#include <QVariantMap>
#include "libTargomanCommon/Macros.h"
#include "libTargomanCommon/Configuration/tmplConfigurable.h"
#include "libTargomanCommon/Configuration/tmplConfigurableArray.hpp"
using namespace Targoman::Common::Configuration;

namespace Targoman::Migrate {

#define instanceGetterPtr(_class) static _class* instancePtr() { \
    static _class* Instance = nullptr; return (Q_LIKELY(Instance) ? Instance : (Instance = new _class)); \
}

TARGOMAN_DEFINE_ENHANCED_ENUM(enuAppCommand,
                              showconf,
                              newdb,
                              newdbdiff,
                              newlocal,
                              list,
                              history,
                              commit,
                              checkbc,
//                              rollback,
//                              redo,
//                              fresh,
                              mark
                              );

constexpr char LINE_SPLITTER[] = "------------------------------------------------------------------------";
constexpr char REGEX_PATTERN_MIGRATION_FILENAME[]      = "m[0-9]{8}_[0-9]{6}_[a-zA-Z0-9-_]*.(sh|sql)";
constexpr char REGEX_PATTERN_MIGRATION_LOG_FILENAME[]  = "m[0-9]{8}_[0-9]{6}_[a-zA-Z0-9-_]*.(sh|sql).log";
constexpr char CREATE_DB_MIGRATION_HISTORY_FILE_NAME[] = "m00000000_000000_MigrationTool_migration_init.sql";

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wweak-vtables"
TARGOMAN_ADD_EXCEPTION_HANDLER(exMigrationTool, Targoman::Common::exTargomanBase);
#pragma clang diagnostic pop

struct stuDBServer
{
    QString BasePath;
    tmplConfigurable<QString>  Name; //devdb1|devdb2|db1|db2|db3
    tmplConfigurable<QString>  Host;
    tmplConfigurable<QString>  Port;
    tmplConfigurable<QString>  UserName;
    tmplConfigurable<QString>  Password;

    stuDBServer(const QString &_basePath) :
        BasePath(_basePath),
        Name(_basePath + "Name",
             "",
             ""
             ),
        Host(_basePath + "Host",
             "",
             ""
             ),
        Port(_basePath + "Port",
             "",
             ""
             ),
        UserName(_basePath + "UserName",
                 "",
                 ""
                 ),
        Password(_basePath + "Password",
                 "",
                 ""
                 )
    { ; }

    void setFromVariant(const QVariant &_value)
    {
        qDebug() << __FUNCTION__ << __LINE__ << _value;

        QVariantMap map = _value.toMap();

        this->Name.setFromVariant(map.value(BasePath + "Name"));
        this->Host.setFromVariant(map.value(BasePath + "Host"));
        this->Port.setFromVariant(map.value(BasePath + "Port"));
        this->UserName.setFromVariant(map.value(BasePath + "UserName"));
        this->Password.setFromVariant(map.value(BasePath + "Password"));
    }

    QVariant toVariant() const
    {
        qDebug() << __FUNCTION__ << __LINE__;

        QVariantMap map;

        map.insert(BasePath + "Name", this->Name.toVariant());
        map.insert(BasePath + "Host", this->Host.toVariant());
        map.insert(BasePath + "Port", this->Port.toVariant());
        map.insert(BasePath + "UserName", this->UserName.toVariant());
        map.insert(BasePath + "Password", this->Password.toVariant());

        qDebug() << map;

        return map;
    }
};

struct stuRunningMode
{
    QString BasePath;
    tmplConfigurable<QString>       Name; //dev|prod
    tmplConfigurable<QStringList>   DBServers; //devdb1,devdb2 | db1,db2,db3

    stuRunningMode(const QString &_basePath) :
        BasePath(_basePath),
        Name(_basePath + "Name",
             "Name of the running mode"
             ),
        DBServers(_basePath + "DBServers",
                  "Database servers"
                  )
    { ; }

    void setFromVariant(const QVariant &_value)
    {
        qDebug() << __FUNCTION__ << __LINE__ << _value;

        QVariantMap map = _value.toMap();

        this->Name.setFromVariant(map.value(BasePath + "Name"));
        this->DBServers.setFromVariant(map.value(BasePath + "DBServers"));
    }

    QVariant toVariant() const
    {
        qDebug() << __FUNCTION__ << __LINE__;

        QVariantMap map;

        map.insert(BasePath + "Name", this->Name.toVariant());
        map.insert(BasePath + "DBServers", this->DBServers.toVariant().toString().split(','));

        qDebug() << map;

        return map;
    }
};

struct stuProject
{
    QString BasePath;
    tmplConfigurable<QString>       Name;
    tmplConfigurable<bool>          AllowDB;
    tmplConfigurable<bool>          AllowLocal;
    tmplConfigurable<bool>          ApplyToAllProjects;
    tmplConfigurable<QStringList>   DBDestinations; //devdb1,db2

    stuProject(const QString &_basePath) :
        BasePath(_basePath),
        Name(_basePath + "Name",
             "Name of the project. will be used in migrations path : ./migrations/{Name}/...",
             ""
             ),
        AllowDB(_basePath + "AllowDB",
                "",
                false
                ),
        AllowLocal(_basePath + "AllowLocal",
                   "",
                   false
                   ),
        ApplyToAllProjects(_basePath + "ApplyToAllProjects",
                           "Apply to all of non-apply-to-all projects",
                           false
                           ),
        DBDestinations(_basePath + "DBDestinations",
                       "Database destinations. Not used if ApplyToAllProjects=true"
                       )
    { ; }

    void setFromVariant(const QVariant &_value)
    {
        qDebug() << __FUNCTION__ << __LINE__ << _value;

        QVariantMap map = _value.toMap();

        this->Name.setFromVariant(map.value(BasePath + "Name"));
        this->AllowDB.setFromVariant(map.value(BasePath + "AllowDB"));
        this->AllowLocal.setFromVariant(map.value(BasePath + "AllowLocal"));
        this->ApplyToAllProjects.setFromVariant(map.value(BasePath + "ApplyToAllProjects"));
        this->DBDestinations.setFromVariant(map.value(BasePath + "DBDestinations"));
    }

    QVariant toVariant() const
    {
        qDebug() << __FUNCTION__ << __LINE__;

        QVariantMap map;

        map.insert(BasePath + "Name", this->Name.toVariant());
        map.insert(BasePath + "AllowDB", this->AllowDB.toVariant());
        map.insert(BasePath + "AllowLocal", this->AllowLocal.toVariant());
        map.insert(BasePath + "ApplyToAllProjects", this->ApplyToAllProjects.toVariant());
        map.insert(BasePath + "DBDestinations", this->DBDestinations.toVariant().toString().split(','));

        qDebug() << map;

        return map;
    }
};

//////////////////////////////////////////////////////////////////////

inline QString bold(const QString &_s) {
    return QString("\x1b[1m%1\x1b[0m").arg(_s);
}

inline QString reverse(const QString &_s) {
    return QString("\x1b[7m%1\x1b[0m").arg(_s);
}

inline QTextStream& qStdout() {
    static QTextStream rOUT{stdout};
    return rOUT;
}
inline QTextStream& qStdIn() {
    static QTextStream rIN{stdin};
    return rIN;
}

} // namespace Targoman::Migrate

template <>
inline void Targoman::Common::Configuration::tmplConfigurableArray<Targoman::Migrate::stuDBServer>::setFromVariant(const QVariant& _value) {
    Q_UNUSED(_value);
    qDebug() << __FUNCTION__ << __LINE__ << "setFromVariant" << _value;
    ///TODO: implement this
}
template <>
inline QVariant Targoman::Common::Configuration::tmplConfigurableArray<Targoman::Migrate::stuDBServer>::toVariant() const
{
    qDebug() << __FUNCTION__ << "[]" << __LINE__;

    QVariantList ret;

    foreach (auto Item, this->Items) {
        ret.append(Item.toVariant());
    }

    qDebug() << "[]" << ret;

    return ret;
}
template <>
inline void Targoman::Common::Configuration::tmplConfigurableArray<Targoman::Migrate::stuRunningMode>::setFromVariant(const QVariant& _value) {
    Q_UNUSED(_value);
    qDebug() << __FUNCTION__ << __LINE__ << "[] setFromVariant" << _value;
    ///TODO: implement this
}
template <>
inline QVariant Targoman::Common::Configuration::tmplConfigurableArray<Targoman::Migrate::stuRunningMode>::toVariant() const
{
    qDebug() << __FUNCTION__ << "[]" << __LINE__;

    QVariantList ret;

    foreach (auto Item, this->Items) {
        ret.append(Item.toVariant());
    }

    qDebug() << "[]" << ret;

    return ret;
}
template <>
inline void Targoman::Common::Configuration::tmplConfigurableArray<Targoman::Migrate::stuProject>::setFromVariant(const QVariant& _value) {
    Q_UNUSED(_value);
    qDebug() << __FUNCTION__ << __LINE__ << "setFromVariant" << _value;
    ///TODO: implement this
}
template <>
inline QVariant Targoman::Common::Configuration::tmplConfigurableArray<Targoman::Migrate::stuProject>::toVariant() const
{
    qDebug() << __FUNCTION__ << "[]" << __LINE__;

    QVariantList ret;

    foreach (auto Item, this->Items) {
        ret.append(Item.toVariant());
    }

    qDebug() << "[]" << ret;

    return ret;
}

#endif // MIGRATION_TOOL_DEFS_H
