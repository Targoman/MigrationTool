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

#include <iostream>

#include "Defs.h"
#include "Configs.h"
#include "CommandManager.h"
#include "Commands/intfCommand.h"
#include "Commands/cmdShowConf.h"
#include "Commands/cmdNewDB.h"
#include "Commands/cmdNewDBDiff.h"
#include "Commands/cmdNewLocal.h"
#include "Commands/cmdHistory.h"
#include "Commands/cmdList.h"
#include "Commands/cmdMark.h"
#include "Commands/cmdCommit.h"
#include "Commands/cmdCheckBC.h"

using namespace Targoman::Common;

namespace Targoman::Migrate {

using namespace Commands;

CommandManager::CommandManager(QObject *parent) : QObject(parent)
{}

void CommandManager::slotExecute()
{
    try
    {
        Configs::FillRunningParameters();

        intfCommand *Command = nullptr;

        switch (Configs::Command.value())
        {
            case enuAppCommand::showconf:
                Command = cmdShowConf::instancePtr();
                break;

            case enuAppCommand::newdb:
                Command = cmdNewDB::instancePtr();
                break;

            case enuAppCommand::newdbdiff:
                Command = cmdNewDBDiff::instancePtr();
                break;

            case enuAppCommand::newlocal:
                Command = cmdNewLocal::instancePtr();
                break;

            case enuAppCommand::history:
                Command = cmdHistory::instancePtr();
                break;

            case enuAppCommand::list:
                Command = cmdList::instancePtr();
                break;

            case enuAppCommand::mark:
                Command = cmdMark::instancePtr();
                break;

            case enuAppCommand::commit:
                Command = cmdCommit::instancePtr();
                break;

            case enuAppCommand::checkbc:
                Command = cmdCheckBC::instancePtr();
                break;

//            case enuAppCommand::rollback:
//                Command = cmdRollback::instancePtr();
//                break;

//            case enuAppCommand::redo:
//                Command = cmdRedo::instancePtr();
//                break;

//            case enuAppCommand::fresh:
//                Command = cmdFresh::instancePtr();
//                break;

            default:
                break;
        }

        if (Command == nullptr)
            throw exMigrationTool("Invalid command");

        if (false) //SHOW_HELP
        {
            Command->help();
            QCoreApplication::exit(0);
            return;
        }

        //---------------------------------
        if (Command->needDB())
        {
            //register default conn string
            for (QMap<QString, QString>::const_iterator it = Configs::RunningParameters.DBServersDefaultConnectionString.constBegin();
                 it != Configs::RunningParameters.DBServersDefaultConnectionString.constEnd();
                 it++)
            {
                QString DBServerName = it.key();
                QString ConnStringWith = it.value();

                qDebug() << "*addDBEngine" << DBServerName;
                clsDAC::addDBEngine(enuDBEngines::MySQL, DBServerName);

                qDebug() << "*setConnectionString" << DBServerName << "=" << ConnStringWith;
                clsDAC::setConnectionString(ConnStringWith, DBServerName);
            }

            //register project's conn string
            for (QMap<QString, QString>::const_iterator it = Configs::RunningParameters.ProjectDBConnectionStrings.constBegin();
                 it != Configs::RunningParameters.ProjectDBConnectionStrings.constEnd();
                 it++)
            {
                QString ProjectDestinationKey = it.key();
                QString ConnStringWithSchema = it.value();

                qDebug() << "addDBEngine" << ProjectDestinationKey;
                clsDAC::addDBEngine(enuDBEngines::MySQL, ProjectDestinationKey);

                //check if db exists
                auto Parts = ProjectDestinationKey.split('@');
                QString SchemaName = Parts[0];
                QString DBServerName = Parts[1];

                //1: check if db exists
                clsDAC DAC1(DBServerName);
                QString Qry = R"(
                    SELECT *
                      FROM information_schema.SCHEMATA
                     WHERE SCHEMA_NAME=?
                )";
                clsDACResult ResultTable = DAC1.execQuery("", Qry, {
                                                              Configs::DBPrefix.value() + SchemaName
                                                          });

                if (ResultTable.toJson(true).object().isEmpty())
                {
                    qDebug() << "database" << Configs::DBPrefix.value() + SchemaName << "not exists in" << DBServerName;
                    Configs::RunningParameters.NonExistsProjectDBConnectionStrings[ProjectDestinationKey] = ConnStringWithSchema;
                }
                else
                {
                    qDebug() << "setConnectionString" << ProjectDestinationKey << "=" << ConnStringWithSchema;
                    clsDAC::setConnectionString(ConnStringWithSchema, ProjectDestinationKey);
                }
            }
        }

        //---------------------------------
        if (Command->run())
            QCoreApplication::exit(0);
        //else: async
    }
    catch(exTargomanBase &exp)
    {
        TargomanLogError(exp.what());
        QCoreApplication::exit(-1);
        return;
    }
}

} //namespace Targoman::Migrate

/*
rm `find . -name .migrations`

DROP TABLE if EXISTS CommonFuncs.tblMigrations;
DROP TABLE if EXISTS I18N.tblMigrations;
DROP TABLE if EXISTS AAA.tblMigrations;
DROP TABLE if EXISTS Common.tblMigrations;
DROP TABLE if EXISTS Advert.tblMigrations;
DROP TABLE if EXISTS Ticketing.tblMigrations;
*/
