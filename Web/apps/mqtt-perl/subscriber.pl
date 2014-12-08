#!/usr/bin/perl
use lib '/home/mwhitehead/Web/apps/mqtt-perl/';
use MQTT::Client;

my $mqtt = MQTT::Client->new({
   brokerIP => "localhost",
   callback_publish => \&publish_callback
});

# Subscribe to a topic and wait for messages:
$mqtt->connect();
print "Hello\n";
$mqtt->subscribe("test",0);
print "Hello2\n";
$mqtt->listen_for_publishes();
print "Hello3\n";

while (1) {
  print "hello4\n";
  sleep(10);
}

sub publish_callback {
   my $topic = shift;
   my $msg = shift;
   print $msg . "\n";
   if ($msg eq 'quit') {
      $mqtt->disconnect();
      exit 0;
   } else {
     $mqtt->listen_for_publishes();
   }
}
