#!/usr/bin/perl
use lib '/home/mwhitehead/Web/apps/mqtt-perl/';
use MQTT::Client;
use utf8;

binmode(STDOUT, ":utf8");

print "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";
print "<rss version=\"2.0\">";
print "<channel>\n";
print "<title>Stats</title>\n";
print "<link></link>\n";
print "<description>Stats about the house</description>\n";

print "<item>\n";
print "<title>Item 0</title>\n";
print "<link>localhost</link>\n";
print "<description>Swap: ";
$swapCommand = q{cat /proc/swaps | grep partition | awk '{print $4}'};
$result = `$swapCommand`;
print $result . "k</description>\n";
print "</item>\n";

$voltageString = "";

my $mqtt = MQTT::Client->new({
   brokerIP => "localhost",
   callback_publish => \&publish_callback
});
my $i = 0;

# To perform a 'one-shot' publish to a topic:
$mqtt->connect();
$mqtt->subscribe("local/rfm12/nodes/02/temp",0);
$mqtt->subscribe("local/rfm12/nodes/03/temp",0);
$mqtt->subscribe("local/rfm12/nodes/02/voltage",0);
$mqtt->subscribe("local/rfm12/nodes/03/voltage",0);
$mqtt->subscribe("local/rfm12/nodes/04/voltage",0);
$mqtt->listen_for_publishes();

sub publish_callback {
   my $topic = shift;
   my $msg = shift;
   $i++;
  
   if (index($topic, '02/voltage') == -1 && index($topic, '03/voltage') == -1) {
     print "<item>\n";
     print "<title>Item " . $i . "</title>\n";
     print "<link>";
     print $topic;
     print "</link>\n";
     print "<description>";
   }
   if ($topic eq 'local/rfm12/nodes/03/temp') {
      print "Lounge: " . $msg . "\N{U+00B0}C";
   } elsif ($topic eq 'local/rfm12/nodes/02/temp') {
      print "Loft:     " . $msg . "\N{U+00B0}C";
   } elsif ($topic eq 'local/rfm12/nodes/03/voltage') {
      $voltageString = $voltageString . "" . sprintf("%.1f", ($msg / 10)) . "v   ";
   } elsif ($topic eq 'local/rfm12/nodes/04/voltage') {
      $voltageString = $voltageString . "" . sprintf("%.1f", ($msg / 10)) . "v";
      print $voltageString;
   } elsif ($topic eq 'local/rfm12/nodes/02/voltage') {
      $voltageString = $voltageString . "" . sprintf("%.1f", ($msg / 10)) . "v   ";
   }
   if (index($topic, '02/voltage') == -1 && index($topic, '03/voltage') == -1) {
     print "</description>\n";
     print "</item>\n";
   }
   if ($msg eq 'quit') {
      $mqtt->disconnect();
      exit 0;
   }

   if ($i == 5) {
     print "</channel>\n";
     print "</rss>\n";
     # After getting the last message, quit
     $mqtt->disconnect();
     exit 0;
   }
}
