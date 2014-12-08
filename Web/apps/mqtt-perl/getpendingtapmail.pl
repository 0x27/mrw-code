#!/usr/bin/perl
use lib '/home/mwhitehead/Web/apps/mqtt-perl/';
use MQTT::Client;

my $mqtt = MQTT::Client->new({
   brokerIP => "localhost",
#   clientID => "websiteclient3",
   callback_publish => \&publish_callback
});

# To perform a 'one-shot' subscribe on a topic:
$mqtt->connect();
$mqtt->subscribe("tapbridge/pending",0);
$mqtt->listen_for_publishes();

sub publish_callback {
   my $topic = shift;
   my $msg = shift;
   print $msg . "\n";
   if ($msg eq 'quit') {
      $mqtt->disconnect();
      exit 0;
   }
   # After getting 1 message, quit
   $mqtt->disconnect();
   exit 0;
}
