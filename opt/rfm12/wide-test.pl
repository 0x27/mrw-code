use lib '/home/mwhitehead/Web/apps/mqtt-perl/';
use POSIX strftime;
use Fcntl;
use MQTT::Client;
use Encode;
use strict;
use Devel::Cycle;
use Devel::Leak;

my $mqtt;

sub connect
{
    my $callbackFunction = shift;

    print "Connecting...\n";

    if (!defined $mqtt) {
       if (defined $callbackFunction) {
          $mqtt = MQTT::Client->new({
             brokerIP => "localhost",
             callback_publish => $callbackFunction
          });
       } else {
          $mqtt = MQTT::Client->new({
             brokerIP => "localhost",
          });
       }
    }

    $mqtt->connect();

    return 0;
}
                   
# Fork a separate process to receive MQTT commands to pass on to the RFM12 chip
my $pid = fork();

if ($pid) {
    # parent
    $0 = "widechar1";

    &connect();
    while (1) {
        my $rc = publish_rfm12("Hello Wide Char Test");
        # TEMP
        $rc = 1;
           
        if ($rc) {
            &disconnect();
            &connect();
            print STDERR "Reconnected to broker\n";
        }
        sleep(1);
    } 
} elsif ($pid == 0) {
    # child
    $0 = "widechar2";

    # For the subscriber, set a keep alive > 0 (otherwise it 
    # thrashes the broker for some reason)
    &connect(\&publicationReceived);
    &subscribe();
    &waitForMessages();
    exit 0;
} else {
    die "couldnt fork: $!\n";
}
                                

sub publicationReceived {
   my $topic = shift;
   my $data = shift;

   print "Received command '" . $data . "'\n";
   # Write the command to the serial port
}

sub subscribe {
   $mqtt->subscribe('local/rfm12/test/commands', 0);
}

sub waitForMessages {
   $mqtt->listen_for_publishes();
}

sub receive_pub {
   my @publication = $mqtt->receivePub();
   print "Received publication at " . scalar localtime() . "\n";
   return @publication;
}

sub publish_rfm12
{
   my $value = shift || die "no value to publish";
   my $rc = 0;

   eval {
        $mqtt->publish("local/rfm12/test/", 0, 0, $value);

  };
  if ($@) {
    print STDERR "EXCEPTION OCCURRED! [$@]\n";
    print STDERR "(Hopefully things will carry on OK though)\n";
    $rc = 1;
  }

  undef($value);
  return $rc;
}

sub disconnect
{
    print "disconnecting\n";
    $mqtt->disconnect();
}
             
