#!/usr/bin/perl
use strict;

my $message = "<[matthew101] this is a message>";

print $message . "\n";

substr($message, index($message, ']'), 1) = '@]';

print $message . "\n";
substr($message, index(lc($message), "\@tap_ma") < 0 ? length $message : index(lc($message), "\@tap_ma"), 7) = '';

print $message . "\n";
      
$message =~ s/\s+$//;

print $message . "\n";

