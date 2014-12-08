#!/usr/bin/perl

use strict;
use warnings;
my $numTempReadings = 7*24;# last week
my $db_name="power";
my $db_user="power";

use DBI;
use Data::Dumper;
my $dbh = DBI->connect("DBI:mysql:$db_name",$db_user,undef,{'RaiseError' => 1}) or die "Couldn't connect to database: " . DBI->errstr;

open TEMP, ">/tmp/temp.gnuplot";
#Temperature graph: 
printf "%-17s %4s\n", "Timestamp", "Temp";
printf TEMP "%-17s %4s\n", "Timestamp", "Temp";
my $sth = $dbh->prepare("SELECT CONCAT(DATE(r.timestamp),' ',HOUR(r.timestamp),':00') as datetime, ROUND(AVG(temp),1) as avg_temp FROM reading r GROUP BY datetime ORDER BY r.timestamp DESC LIMIT $numTempReadings");
$sth->execute;
  while (my $row = $sth->fetchrow_hashref()) {
		$row->{datetime} =~ s!\s+!.!; #replace whitespace in timestamp with a .
		printf "%-17s %4s\n", $row->{datetime}, $row->{avg_temp};
		printf TEMP "%-17s %4s\n", $row->{datetime}, $row->{avg_temp};
  }

$sth->finish;

my $gnuplot_cmd = <<DONE;
set xdata time
set timefmt "%Y-%m-%d.%H:%M"
set key off
set title "Hourly Temperatures"
set mxtics 5
set mytics 5
set grid xtics ytics mxtics mytics
set grid
plot "/tmp/temp.gnuplot" using 1:2
DONE

print $gnuplot_cmd;
#print `echo '$gnuplot_cmd'|gnuplot`;

exit;
