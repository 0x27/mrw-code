#!/usr/bin/perl
use lib '/home/mwhitehead/mrw-code/Web/apps/mqtt-perl/';
use MQTT::Client;

my $mqtt = MQTT::Client->new({
   brokerIP => "192.168.1.2",
   clientID => "mattpubclient"
});

# To perform a 'one-shot' publish to a topic:
$mqtt->connect();
$mqtt->publish("tapbridge/pending",0,0,"");
$mqtt->disconnect();
exit 0;
