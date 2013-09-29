CREATE TABLE `postings` (
  `user_id` varchar(255) NOT NULL,
  `category` char(32) DEFAULT NULL,
  `title` varchar(200) DEFAULT NULL,
  `price` varchar(200) NOT NULL,
  `building_name` varchar(200) DEFAULT NULL,
  `photo_url` text,
  `photo_url_small` text,
  `audio_url` text,
  `description` varchar(255) DEFAULT NULL,
  `posting_type` int(2) NOT NULL,
  `posting_status` tinyint(4) NOT NULL DEFAULT '0',
  `mtime` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP,
  `id` bigint(20) NOT NULL AUTO_INCREMENT,
  `comments_count` int(11) NOT NULL DEFAULT '0',
  `num_views` int(11) DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `user_id` (`user_id`)
) ENGINE=MyISAM AUTO_INCREMENT=236 DEFAULT CHARSET=latin1
