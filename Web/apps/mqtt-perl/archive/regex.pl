#!/usr/bin/perl
$a = "[matthew1001] Hi Mark, how are you? \@tap_MA";
$b = "[matthew1001] Hi Mark, how are you? \@tap_MA";
print "Index of string b in string a: " . index($a, $b) . "\n";

if (index($a, $b) != -1) {
  print "They match" . "\n";
}

print $a . "\n";

substr($a, index($a, ']'), 1) = ' @]';

print $a . "\n";

substr($a, index(lc($a), "\@tap_ma"), 7) = '';
$a =~ s/\s+$//;

print "<" . $a . ">\n";
