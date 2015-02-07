#!/usr/bin/perl -w
use CurrentCost;
use Data::Dumper;

$cc = new CurrentCost(broker=>"204.146.213.96",name=>"map",debug=>1);
$cc->connect();

$cc->publish_power(1200);
$cc->publish_temp(21);

$cc->disconnect;
print "Done\n";

