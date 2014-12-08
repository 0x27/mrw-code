#!/usr/bin/perl -w
use DBI;
use strict;

my $db_name = 'power';
my $db_user = 'power';
my $db_type = 'mysql';

my $db = DBI->connect("DBI:$db_type:$db_name",$db_user) or die "Couldn't connect to database: " . DBI->errstr;

my ($sec, $min, $hour, $day, $mon, $year) = localtime(time - (3600*24) );#yesterday
$year+=1900;
$mon++;#months start at 0
my $yesterday = sprintf("%04s-%02s-%02s %02s:%02s:%02s",$year,$mon,$day,$hour,$min,$sec);


#2008-04-16 22:02:16
my $q = $db->prepare('SELECT `timestamp`,`watts`,`temp` FROM `reading` WHERE `timestamp`< ?');
$q->execute($yesterday) || carp("error running query: $q->errstr");

open FH, ">/tmp/gnuplot_data_tmp";
while( my @data = $q->fetchrow_array() ){
	$data[0] =~ s/ /./;
	print FH "$data[0]\t$data[1]\t$data[2]\n";
}
close FH;

open FH, ">/tmp/gnuplot_cmd_tmp";
my @gnuplot = ('set xdata time',  # The x axis data is time",
						'set timefmt "%Y-%m-%d.%H:%M:%S"',# The dates in the file look like 10-Jun-04
						'set format x "%b %d"',# On the x-axis, we want tics like Jun 10
						'plot "/tmp/gnuplot_data_tmp" using 1:2',
);
print FH join("\n",@gnuplot);
print FH "\n";
close FH;
