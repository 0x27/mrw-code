#!/usr/bin/perl -w
use XML::LibXML;
use DBI;
use Carp;
use Data::Dumper;

my $db_name = 'power';
my $db_user = 'power';
my $db_type = 'mysql';
my $power_dev = "/dev/ttyUSB0";
my $stats_file = "/var/log/power_stats";
my $last_results_file = "/tmp/last_power_reading";
use CurrentCost;

while(1){
	my $cc = new CurrentCost(broker=>"204.146.213.96",name=>"map",debug=>1);
	$cc->connect();


	my $db = DBI->connect("DBI:$db_type:$db_name",$db_user) or die "Couldn't connect to database: " . DBI->errstr;
	my $xml = XML::LibXML->new();

	binmode STDOUT, ":utf8";
	open(FH,$power_dev);
	my $stats;
	open($stats,'>',$stats_file);

	while(<FH>){
		my $tree;
		chomp;
		eval{ $tree = $xml->parse_string($_) };
		if(! $@){
			$cc->publish_all($_);
			write_last_results_file($last_results_file, $_);
			my $root = $tree->getDocumentElement;
			my $watts = $root->findvalue( "/msg/ch1/watts");
			my $temp = $root->findvalue("/msg/tmpr");
			insert_into_db($db, $watts, $temp);
			insert_into_db_archive($db,$root);
			write_stats_file($stats, $watts, $temp);
		}

	}
	$cc->disconnect;
	sleep 5;
}

sub write_last_results_file
{
	my ($file, $data) = @_;
	open F, ">", $file;
	print F $data;
	close F;
}


sub insert_into_db
{
	my ($db, $watts, $temp, $daily) = @_;

	#insert 6second readings
	my $insert_handle = $db->prepare_cached('INSERT INTO reading  (`timestamp`, `watts` , `temp`)  VALUES (NOW(), ?,?)'); 
	carp "Couldn't prepare queries" unless defined $insert_handle;
  carp "couldnt insert into db" unless $insert_handle->execute($watts, $temp);
}



#insert historical values
sub insert_into_db_archive
{
	my ($db,$root) = @_;
	carp "db not defined" unless $db;

	#Daily
	my $daily = $root->findvalue("/msg/hist/days/d01");
	if( $daily ne '' ){
		my ($sec, $min, $hour, $day, $mon, $year) = localtime(time - (3600*24) );#yesterday
		$year+=1900;
		$mon++;#months start at 0
		$date = sprintf("%04s-%02s-%02s",$year,$mon,$day);
		#first check if we have a reading for yesterday
		my $day_h = $db->prepare('SELECT count(*) FROM `archive` WHERE `date`=?');
		$day_h->execute($date) || carp("error running query: $day_h->errstr");

		if( !$day_h->fetchrow_array() ){ 
			print "Saving new daily value to db: $date = $daily\n";
			my $addDay_h = $db->prepare('INSERT INTO archive (`date`,`kwatts`) VALUES (?,?)');
			$addDay_h->execute($date,$daily) || carp("error running query: $check_h->errstr");
		}
	}

	#Monthly
	my $monthly = $root->findvalue("/msg/hist/mths/m01");
	if( $monthly ne '' ){
		my ($sec, $min, $hour, $day, $mon, $year) = localtime(time);
		$year+=1900;
		# Leave months as they are, as we want last month. 
		# perl counts months from 0
		if($mon==0){
			# if were in jan, then the data is for dec last year
			$mon=12;
			$year--;
		}

		$date = sprintf("%04s-%02s",$year,$mon);
		#first check if we have a reading for yesterday
		my $mon_h = $db->prepare('SELECT count(*) FROM `archive` WHERE `date`=?');
		$mon_h->execute($date) || carp("error running query: $mon_h->errstr");

		if( !$mon_h->fetchrow_array() ){ 
			print "Saving new monthly value to db: $date = $monthly\n";
			my $addMon_h = $db->prepare('INSERT INTO archive (`date`,`kwatts`) VALUES (?,?)');
			$addMon_h->execute($date,$monthly) || carp("error running query: $addMon_h->errstr");
		}
	}

	#Yearly
	my $yearly = $root->findvalue("/msg/hist/yrs/y1");
	if( $yearly ne '' ){
		my ($sec, $min, $hour, $day, $mon, $year) = localtime(time);
		$year+=1900;#perl counts years from 1900
		$year--;#we want to set last years reading

		$date = sprintf("%04s",$year);
		#first check if we have a reading for yesterday
		my $year_h = $db->prepare('SELECT count(*) FROM `archive` WHERE `date`=?');
		$year_h->execute($date) || carp("error running query: $year_h->errstr");

		if( !$year_h->fetchrow_array() ){ 
			print "Saving new yearly value to db: $date = $yearly\n";
			my $addYear_h = $db->prepare('INSERT INTO archive (`date`,`kwatts`) VALUES (?,?)');
			$addYear_h->execute($date,$yearly) || carp("error running query: $addYear_h->errstr");
		}
	}

}





sub write_stats_file
{
	my ($fh, $watts, $temp) = @_;
	print "$watts - $temp \x{2103}\n";
	print $fh "$watts - $temp";

}



