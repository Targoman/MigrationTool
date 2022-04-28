/* Migration File: m00000000_000000_MigrationTool_migration_init.sql */
/* CAUTION: don't forget to use {{dbprefix}} for schemas */

USE `{{dbprefix}}{{Schema}}`;

CREATE TABLE `{{GlobalHistoryTableName}}` (
    `migName` VARCHAR(128) NOT NULL COLLATE 'utf8mb4_general_ci',
    `migAppliedAt` DATETIME NOT NULL,
    PRIMARY KEY (`migName`) USING BTREE
)
COLLATE='utf8mb4_general_ci'
ENGINE=InnoDB
;
