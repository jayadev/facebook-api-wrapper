CREATE TABLE `comments` (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `fkey_post_id` bigint(20) NOT NULL,
  `fkey_user_id` varchar(255) NOT NULL,
  `ip` tinytext NOT NULL,
  `mtime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `comment` text,
  `notify` tinyint(1) NOT NULL DEFAULT '0',
  `spam` tinyint(1) NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `fkey_post_id_index` (`fkey_post_id`),
  KEY `fkey_user_id_index` (`fkey_user_id`)
) ENGINE=MyISAM AUTO_INCREMENT=124 DEFAULT CHARSET=latin1
