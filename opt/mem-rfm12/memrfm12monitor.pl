# Read the gas photoreflectivity sensor and publish whenever a tick has
# occurred to an MQTT broker and record it in our database.

# Matt Whitehead 20-Feb-13

########################################################################

$your_name = "mattw12";

$broker = "127.0.0.1";

$retry_attempts = 50;

$retries = 0;

$subscriberClientID = "rfmSubID3";
$publisherClientID = "rfmPubID9";

########################################################################

use RFM12Monitor;
use DBI;
use POSIX strftime;
use Fcntl;

my $cc = new RFM12Monitor(broker=>$broker,name=>$your_name,debug=>1,pub=>1);

while ($retries < $retry_attempts)
{

  if ($retries == 0)
  {

     # Fork a separate process to receive MQTT commands to pass on to the RFM12 chip
     my $pid = fork();

     if ($pid) {
        # parent
        $0 = "rfm12listenertest";

        $cc->connect($publisherClientID);
        while (1) {
           $rc = $cc->publish_rfm12("DATA: GROUP(53) HEADER(97) BYTES(15) Nd03 7 64 12 39");
           
           if ($rc) {
              print STDERR "RC problem - closing serial port\n";
              close (SERIAL);
              print STDERR "Re-opening serial port\n";
              open (SERIAL,"<$serial_port") || die;
              print STDERR "Port re-opened. Re-connecting to MQTT...\n";
              $cc->disconnect();
              $cc->connect($publisherClientID);
              print STDERR "Reconnected to broker\n";
           }
           sleep(2);
           $rc = $cc->publish_rfm12("RANDOMBYTES: GROUP(53) HEADER(97) BYTES(15) Nd03 7 64 12 39");
           sleep(2);
        } 
     } elsif ($pid == 0) {
        # child
        $0 = "rfm12cmdserver4";

        # This disables buffering in the serial USB port, causing
        # any writes to be pushed down to the RFM12 arduino immediately

        # For the subscriber, set a keep alive > 0 (otherwise it 
        # thrashes the broker for some reason)
        $cc->connect($subscriberClientID, \&publicationReceived);
        $cc->subscribe();
        $cc->waitForMessages();
        exit 0;
     } else {
        die "couldnt fork: $!\n";
     }
  }
  else
  {
        print STDERR strftime ("%d%b%Y-%H:%M:%S - Can't open serial port, retrying in 30 second...\n",localtime(time()));
        sleep(30);
        $serial_port = "/dev/ttyUSB1";
  }
}
                                
print STDERR strftime ("%d%b%Y-%H:%M:%S",localtime(time()))." - Tried ".$retries." times so quitting\n";

sub publicationReceived {
   my $topic = shift;
   my $data = shift;

   print "Passing command to RFM12 chip: '" . $data . "'\n";
   # Write the command to the serial port
}
                                
