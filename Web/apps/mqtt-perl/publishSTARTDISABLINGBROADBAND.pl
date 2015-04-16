#!/usr/bin/perl
use lib '/home/mwhitehead/mrw-code/Web/apps/mqtt-perl/';
use MQTT::Client;

my $mqtt = MQTT::Client->new({
   brokerIP => "localhost",
});

# To perform a 'one-shot' publish to a topic:
$mqtt->connect();
$mqtt->publish("local/power/routerstandby",0,1,"enabled");
$mqtt->disconnect();
exit 0;
