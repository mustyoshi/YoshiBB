CREATE DATABASE IF NOT EXISTS `yoshibb`;
CREATE TABLE `user` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `username` varchar(45) DEFAULT NULL,
  `email` varchar(45) DEFAULT NULL,
  `session` varchar(64) DEFAULT NULL,
  `password` tinyblob,
  `lastactive` bigint(20) DEFAULT NULL,
  `birth` bigint(20) DEFAULT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `id_UNIQUE` (`id`),
  UNIQUE KEY `username_UNIQUE` (`username`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

CREATE TABLE `forum_user_moderation` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `moderator_id` int(11) NOT NULL,
  `post_id` int(11) NOT NULL,
  `reason` varchar(256) DEFAULT NULL,
  `expiration` timestamp NULL DEFAULT NULL,
  `accepted` tinyint(1) DEFAULT '0',
  PRIMARY KEY (`id`,`moderator_id`,`post_id`,`user_id`),
  UNIQUE KEY `id_UNIQUE` (`id`),
  KEY `fk_um_user_id_idx` (`user_id`),
  KEY `fk_um_mod_id_idx` (`moderator_id`),
  KEY `fk_um_post_id_idx` (`post_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE `forum_user_in_group` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `user_id` int(11) NOT NULL,
  `group_id` int(11) NOT NULL,
  PRIMARY KEY (`id`,`user_id`,`group_id`),
  UNIQUE KEY `id_UNIQUE` (`id`),
  KEY `fk_uig_group_id_idx` (`group_id`),
  KEY `fk_uig_user_id_idx` (`user_id`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

CREATE TABLE `forum_user_group_rule` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'id of the group rule.',
  `group_id` int(11) NOT NULL COMMENT 'The id it references',
  `board_id` int(11) DEFAULT NULL,
  `permission_level` tinyint(4) DEFAULT '2' COMMENT 'Permission level is as follows. 0 -> can''t view. 1 -> can view. 2 -> can post. 3 -> can lock non authored posts. 4 -> can moderate posts. 5 -> can moderate user permissions for this board.',
  PRIMARY KEY (`id`),
  KEY `fk_ugr_board_id_idx` (`board_id`),
  KEY `fk_ugr_group_id_idx` (`group_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE `forum_user_group` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'ID of the user group, this will be referenced by a bunch of tables.',
  `name` varchar(16) DEFAULT NULL COMMENT 'Name of the user group for purposes of displaying on the forums.',
  `description` varchar(128) DEFAULT NULL COMMENT 'A short description.',
  `default_permission` int(11) DEFAULT NULL COMMENT 'The default permission level',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

CREATE TABLE `forum_user` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'This is the ID of each user, and it must be unique and increment with each one.',
  `posts` int(11) DEFAULT NULL COMMENT 'The number of posts this user has. This will only ever be incremented.',
  `lastpost_id` int(11) DEFAULT NULL COMMENT 'This is the id of the last post this user has authored. It will not update if the post is deleted.',
  `signature` varchar(512) DEFAULT NULL COMMENT 'Each user gets 512 bytes to use as their signature. Will support a BBCode-esque markup.',
  `last_activity` bigint(20) DEFAULT NULL COMMENT 'This is going to be accurate within five minutes. Anytime they do a thing, it will be updated.',
  `last_message_id` int(11) DEFAULT NULL COMMENT 'Points to the last message they read. The ID will be used to determine if new messages have appeared for them.',
  `active_moderation` int(11) DEFAULT '0',
  PRIMARY KEY (`id`),
  UNIQUE KEY `id_UNIQUE` (`id`),
  KEY `fk_user_post_id_idx` (`lastpost_id`),
  KEY `fk_user_pm_id_idx` (`last_message_id`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

CREATE TABLE `forum_topic` (
  `id` int(11) NOT NULL AUTO_INCREMENT COMMENT 'The id of the topic.',
  `board_id` int(11) NOT NULL DEFAULT '1' COMMENT 'The id of the board this topic is posted in.',
  `post_id` int(11) NOT NULL COMMENT 'The id of the post that this topic was started with.',
  `type` tinyint(4) DEFAULT NULL COMMENT 'This tells if the post is locked or sticky. By using modulos we can restrict it to just 4 values',
  `posts` int(11) DEFAULT NULL,
  `views` int(11) DEFAULT NULL,
  `lastpost_id` int(11) DEFAULT NULL COMMENT 'Holds the ID of the last post in the post.',
  PRIMARY KEY (`id`,`post_id`),
  UNIQUE KEY `id_UNIQUE` (`id`),
  KEY `fk_topic_board_id_idx` (`board_id`),
  KEY `fk_topic_last_post_id_idx` (`lastpost_id`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

CREATE TABLE `forum_private_message` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `sender_id` int(11) NOT NULL,
  `recipient_id` int(11) NOT NULL,
  `subject` varchar(60) DEFAULT NULL,
  `body` varchar(2048) DEFAULT NULL,
  `time` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `parent_id` int(11) DEFAULT NULL COMMENT 'Points to the message that this was in reply to. This will create message chains.',
  PRIMARY KEY (`id`,`sender_id`,`recipient_id`),
  UNIQUE KEY `message_id_UNIQUE` (`id`),
  KEY `fk_pm_sender_id_idx` (`sender_id`),
  KEY `fk_pm_recipient_id_idx` (`recipient_id`),
  KEY `fk_pm_parent_id_idx` (`parent_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE `forum_post_moderation` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `moderator_id` int(11) NOT NULL,
  `post_id` int(11) DEFAULT NULL,
  `old_subject` varchar(60) DEFAULT NULL,
  `old_body` varchar(10240) DEFAULT NULL,
  `time` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`id`,`moderator_id`),
  KEY `fk_pmod_mod_id_idx` (`moderator_id`),
  KEY `fk_pmod_post_id_idx` (`post_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE `forum_post` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `board_id` int(11) DEFAULT NULL,
  `topic_id` int(11) DEFAULT NULL,
  `poster_id` int(11) DEFAULT NULL,
  `subject` varchar(60) DEFAULT NULL,
  `body` varchar(4096) DEFAULT NULL,
  `date` bigint(20) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_post_board_id_idx` (`board_id`),
  KEY `fk_post_topic_id_idx` (`topic_id`),
  KEY `fk_post_user_id_idx` (`poster_id`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;

CREATE TABLE `forum_deleted_post` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `board_id` int(11) DEFAULT NULL,
  `topic_id` int(11) DEFAULT NULL,
  `poster_id` int(11) DEFAULT NULL,
  `subject` varchar(60) DEFAULT NULL,
  `body` varchar(4096) DEFAULT NULL,
  `date` timestamp NULL DEFAULT CURRENT_TIMESTAMP,
  `deleter_id` int(11) DEFAULT NULL,
  PRIMARY KEY (`id`),
  KEY `fk_post_board_id_idx` (`board_id`),
  KEY `fk_post_topic_id_idx` (`topic_id`),
  KEY `fk_post_user_id_idx` (`poster_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

CREATE TABLE `forum_board` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `parent` int(11) DEFAULT '0' COMMENT 'Parent board''s ID, if no id is given, this board is a top level board and will be used as a grouping mechanism.',
  `name` varchar(32) DEFAULT NULL,
  `description` varchar(128) DEFAULT NULL COMMENT 'If the parent is 0, then the description isn''t needed.',
  `lastpost_id` int(11) DEFAULT NULL COMMENT 'The id of the last post in this forum.',
  `topics` int(11) DEFAULT NULL COMMENT 'Number of topics in this board (including child boards).',
  `posts` int(11) DEFAULT NULL COMMENT 'Number of posts in this board (including child boards).',
  PRIMARY KEY (`id`),
  KEY `fk_board_post_id_idx` (`lastpost_id`)
) ENGINE=MyISAM AUTO_INCREMENT=1 DEFAULT CHARSET=latin1;
'You may need to change the schema name for this'
DELIMITER $$
CREATE DEFINER=`root`@`localhost` PROCEDURE `FORUM_TOPIC`(b_id int(11) ,  p_id int(11),  sub varchar(60), bod varchar(4096), dat bigint(20))
BEGIN
DECLARE l_id1 int(11); 
DECLARE l_id int(11); 
INSERT INTO `yoshibb`.`forum_post` (board_id,poster_id,`subject`,body,`date`) VALUES (b_id,p_id,sub,bod,dat); 
set l_id1 = (SELECT LAST_INSERT_ID());
INSERT INTO `yoshibb`.`forum_topic` (board_id,post_id,lastpost_id) VALUES (b_id,p_id,l_id1);
set l_id = (SELECT LAST_INSERT_ID());
UPDATE `yoshibb`.`forum_post` SET topic_id = l_id WHERE id = l_id1; 

END$$
DELIMITER ;
