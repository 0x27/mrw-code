#!/usr/bin/perl
use lib '/home/mwhitehead/Web/apps/mqtt-perl/';
use MQTT::Client;

my $mqtt = MQTT::Client->new({
   brokerIP => "localhost"
});

# To perform a 'one-shot' publish to a topic:
$mqtt->connect();
# Publish on topic, with qos, retained or not, with data
$mqtt->publish("local/rfm12/command", 1, 0, "text");
$mqtt->disconnect();
exit 0;
