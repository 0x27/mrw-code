#!/usr/bin/perl
use lib '/home/mwhitehead/mrw-code/Web/apps/mqtt-perl/';
use MQTT::Client;

my $mqtt = MQTT::Client->new({
   brokerIP => "localhost",
   clientID => "bbsbClient"
});

$socketNumber = $ARGV[0];
$socketHouseGroup = $ARGV[1];
$onOff = $ARGV[2];
$command = "BBSB " . $socketNumber . " " . $socketHouseGroup . " " . $onOff . ":";
print "Command is " . $command . "\n";

# To perform a 'one-shot' publish to a topic:
$mqtt->connect();
$mqtt->publish("local/rfm12/command",0,0,$command);
sleep(1);
$mqtt->publish("local/rfm12/command",0,0,$command);
sleep(1);
$mqtt->publish("local/rfm12/command",0,0,$command);
sleep(1);
$mqtt->disconnect();
exit 0;
