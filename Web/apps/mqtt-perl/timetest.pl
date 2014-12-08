#!/usr/bin/perl
use strict;

my $currenttime = localtime time;

print $currenttime;
my @timeelements = split(/[ :]+/, $currenttime);
print $timeelements[0] . "\n";
print $timeelements[1] . "\n";
print $timeelements[2] . "\n";
print $timeelements[3] . "\n";
print $timeelements[4] . "\n";
print $timeelements[5] . "\n";
