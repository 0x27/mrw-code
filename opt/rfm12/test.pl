use WebSphere::MQTT::Client;
use Data::Dumper;
use strict;

#
my $mqtt = new WebSphere::MQTT::Client(
Hostname => 'localhost',
Port => 1883,
Debug => 1,
Clientid => "PlTestCl",
keep_alive => 0,
);

# Connect to Broker
my $res = $mqtt->connect();
die "Failed to connect: $res\n" if ($res);

print Dumper( $mqtt );


sleep 1;
print "status=".$mqtt->status()."\n";
sleep 1;
print "status=".$mqtt->status()."\n";

# Subscribe to topic
my $res = $mqtt->subscribe( 'test' );
print "Subscribe result=$res\n";

sleep 2;
print "status=".$mqtt->status()."\n";
sleep 2;

# Get Messages
while( 1 ) {
  my @res = $mqtt->receivePub();
  print localtime . "\n";
  print Dumper(@res);
  #errors can be caught by eval { }
}



# Unsubscribe from topic
my $res = $mqtt->unsubscribe( 'test' );
print "Unubscribe result=$res\n";


sleep 2;

print "status=".$mqtt->status()."\n";

# Clean up
$mqtt->terminate();

print Dumper( $mqtt );

