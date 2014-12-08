#!/usr/bin/perl
use lib '/home/mwhitehead/Web/apps/mqtt-perl/';
use MQTT::Client;

my $mqtt = MQTT::Client->new({
   brokerIP => "localhost",
   clientID => "twitaprisbump"
});

# To perform a 'one-shot' publish to a topic:
$mqtt->connect();
$mqtt->publish("tapbridge/bump",0,0,"bump");
$mqtt->disconnect();
exit 0;
