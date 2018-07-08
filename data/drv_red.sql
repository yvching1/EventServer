CREATE TABLE `drv_red` (
  `ino` int(11) NOT NULL AUTO_INCREMENT,
  `mode` varchar(4) NOT NULL,
  `logtime` varchar(14) NOT NULL,
  `workno` varchar(9) NOT NULL,
  `mkey` varchar(7) NOT NULL,
  `num` varchar(5) NOT NULL,
  `logupdate` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `data` varchar(100) NOT NULL,
  `area` varchar(2) NOT NULL,
  `remask` varchar(100) NOT NULL,
  UNIQUE KEY `ino` (`ino`),
  KEY `logtime` (`logtime`),
  KEY `workno` (`workno`)
) ENGINE=MyISAM AUTO_INCREMENT=64529 DEFAULT CHARSET=utf8 COMMENT='酒測logs';