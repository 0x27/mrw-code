--
-- Database: `power`
--

-- --------------------------------------------------------

--
-- Table structure for table `reading`
--

CREATE TABLE `reading` (
  `id` int(11) NOT NULL auto_increment,
  `timestamp` timestamp NOT NULL default '0000-00-00 00:00:00' on update CURRENT_TIMESTAMP,
  `watts` varchar(5) NOT NULL,
  `temp` float NOT NULL,
  PRIMARY KEY  (`id`)
) ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=168296 ;

-- 
-- Table structure for table `archive`
-- 

CREATE TABLE `archive` (
  `date` varchar(10) NOT NULL,
  `kwatts` int(4) NOT NULL,
  PRIMARY KEY  (`date`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;
