CREATE TABLE IF NOT EXISTS hs_test.account (
	`id` int UNSIGNED AUTO_INCREMENT,
	`name` varchar(64) NOT NULL DEFAULT '',
	`password` varchar(16) NOT NULL DEFAULT '',
	`register_time` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
	PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=gb2312;

 ALTER TABLE hs_test.account ADD UNIQUE (`name`);
