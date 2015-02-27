#!/usr/bin/perl
use lib '/home/mwhitehead/Web/apps/mqtt-perl/';
use MQTT::Client;
use Thread;
use threads::shared;

my $mqtt = MQTT::Client->new({
   brokerIP => "localhost",
#   clientID => "websiteclient9",
   callback_publish => \&publish_callback
});

# To perform a 'one-shot' publish to a topic:
$mqtt->connect();
$mqtt->subscribe("local/rfm12/nodes/04/lastreceived",0);
$listener = Thread->new(\&thread1);

sub thread1 {
  $mqtt->listen_for_publishes();
}

sleep(6);

# If we haven't exited already, return 0.0 for the voltage and exit
$mqtt->disconnect();
sleep(1);
print "------";
sleep(1);
exit 0;

sub publish_callback {
   my $topic = shift;
   my $msg = shift;
   print $msg;
   if ($msg eq 'quit') {
      $mqtt->disconnect();
      exit 0;
   }
   # After getting 1 message, quit
   $mqtt->disconnect();
   exit 0;
}
