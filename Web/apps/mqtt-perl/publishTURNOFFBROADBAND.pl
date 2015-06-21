#!/usr/bin/perl
use lib '/home/mwhitehead/mrw-code/Web/apps/mqtt-perl/';
use MQTT::Client;
use Sys::Syslog;
use Sys::Syslog qw(:standard :macros);
use Net::Ping;
use Thread;
use threads::shared;
my $currenttime = localtime time;

my @timeelements = split(/[ :]+/, $currenttime);
my $hour = $timeelements[3];

openlog($program, 'cons,pid', 'user');

my $mqtt = MQTT::Client->new({
   brokerIP => "localhost",
   callback_publish => \&publish_callback
});

my $routerStandbyEnabled :shared;
$routerStandbyEnabled = 0;

syslog('info', '%s', 'Checking local/power/routerstandby for router standby setting');

$mqtt->connect();
$mqtt->subscribe("local/power/routerstandby",0);
$listener = Thread->new(\&thread1);

sleep(3);

$socketNumber = $ARGV[0];
$socketHouseGroup = $ARGV[1];
$onOff = $ARGV[2];
$command = "BBSB " . $socketNumber . " " . $socketHouseGroup . " " . $onOff . ":";
print "Command is " . $command . "\n";

if ($routerStandbyEnabled || ($hour >= 23) || ($hour < 7) || $onOff) {
  syslog('info', '%s', 'Broadband command allowed. Either nobody is working from home, or it is nighttime so we can turn it off anyway, or it is an ON command which is allowed whatever time of day.');
  syslog('info', '%s', 'Sending broadband-onoff command: ' . $command . ' (hour is ' . $hour . ')');

  # To perform a 'one-shot' publish to a topic:
  $mqtt->publish("local/rfm12/command",0,0,$command);
  sleep(1);
  $mqtt->publish("local/rfm12/command",0,0,$command);
  sleep(1);
  $mqtt->publish("local/rfm12/command",0,0,$command);
  sleep(1);

  syslog('info', '%s', 'Testing that broadband is off');

  $p = Net::Ping->new("udp");
  $host = 'www.google.com';
  if ($p->ping('www.google.com')) {
    syslog('info', '%s', 'Host ' . $host . ' cannot be reached - broadband switched off');
  } else {
    syslog('info', '%s', 'Host ' . $host . ' can still be contacted - broadband not switched off correctly');
  }
  $p->close();
} else {
  syslog('info', '%s', 'Router standby disabled. Leaving router on.');
}

sub thread1 {
   $mqtt->listen_for_publishes();
}

sub publish_callback {
   my $topic = shift;
   my $msg = shift;
   print $msg . "\n";
   syslog('info', '%s', 'local/power/routerstandby value = ' . $msg);
   if ($msg eq 'enabled') {
      syslog('info', '%s', 'Router standby setting enabled');
      $routerStandbyEnabled = 1;
   }
}

if ($listener->is_running()) {
  $listener->kill('SIGUSR1'); 
}
$mqtt->disconnect();
closelog();
exit 0;
